#include "config.h"

#if NO_DATATAP == 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ffs.h>
#include <atl.h>
#include <evpath.h>
//#include <mpi.h>

#ifdef _NOMPI
    /* Sequential processes can use the library compiled with -D_NOMPI */
#   include "mpidummy.h"
#define MPI_SUM 0
#else
    /* Parallel applications should use MPI to communicate file info and slices of data */
#   include "mpi.h"
#endif

#include <pthread.h>
#include "adios.h"
#include "adios_read.h"
#include "adios_read_hooks.h"
#include "adios_error.h"
#include "globals.h"

#include <sys/queue.h>
#if HAVE_PORTALS == 1
#include <thin_portal.h>
#elif HAVE_INFINIBAND == 1
#include <thin_ib.h>
#endif

#include <sys/socket.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <gen_thread.h>
//#include "get_clock.h"
//#include "queue.h"
//#include "attributes.h"
//#include "memwatch.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

#if HAVE_PORTALS == 1
#include "lgs.h"
#include "cercs_env.h"

// for cpu affinity
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
static int dt_thread_pinned = 0;
static int dt_thread_cpuid = -1;
static int app_thread_cpuid = -1;

#include "read_datatap.h"

// for Timing
FILE *server_timing_file = NULL;
FILE *file_timing_file = NULL;


// this sructure holds all global data for Datatap Read method
datatap_read_method_data *dt_read_data = NULL;

// compare used-provided varname with the full path name of variable v
// return zero if matches and non-zero otherwise
static int compare_var_name (char *varname, datatap_var_info *v) 
{
    if (varname[0] == '/') { // varname is full path
        char fullpath[256];
        if(!strcmp(v->varpath, "/")) {
            sprintf(fullpath, "/%s\0", v->varname);  
        } 
        else {   
            sprintf(fullpath, "%s/%s\0", v->varpath, v->varname);  
        }
        return strcmp(fullpath, varname);
    }
    else { // varname doesn't include path
        return strcmp(v->varname, varname);
    }
}

static FMField *find_field (char *name, char *path, FMFieldList flist)
{
    char *temp_name;
    char *full_path_name = get_full_path_name(name, path);
    temp_name = getFixedName(full_path_name);
    free(full_path_name);
    FMField *f = flist;
    while (f->field_name != NULL) {
        if(!strcmp(temp_name, f->field_name)) {
            free(temp_name);
            return f;
        }
        else {
            f++;
        }
#if 0
        char *name_pos = f->field_name + (strlen(f->field_name) - strlen(name));
        if(!strncmp(path, f->field_name, strlen(path)) &&
           !strcmp(name, name_pos)) {
            return f;
        }
        else {
            f++;
        }
#endif
    }
    free(temp_name);
    return f;
}

int64_t read_array(datatap_read_file_data *ds, datatap_var_info *var, 
                    uint64_t *start, uint64_t *count, void *data)
{
    int type_size = common_read_type_size(var->type, NULL);
    int64_t total_size = 0;

    // go over the whole list of data chunks and reorganize arrays
    Queue *data_q = ds->f_info->dt_queue;

    // here we wait until all chunked are ready from data handler
    pthread_mutex_lock(&(ds->f_info->mutex));
    while(queue_size(data_q) < ds->f_info->num_chunks) {
        pthread_cond_wait(&(ds->f_info->cond2), &(ds->f_info->mutex));
    }
    pthread_mutex_unlock(&(ds->f_info->mutex));

    ListElmt *current_chunk = list_head(data_q);
    while(current_chunk) {
        QueueItem *qi = (QueueItem *)current_chunk->data;  
        FMFieldList filed_list = qi->var_list->field_list; // TODO
        
        // first, find the var   
        FMField *f = find_field(var->varname, var->varpath, filed_list);

        if(!f) { 
            // actually this will not happen because the filed_list will
            // contain all vars even though some may not be written
            current_chunk = current_chunk->next;
            continue;
        }

        char *source_addr = (char *)qi->data + f->field_offset; // this is the address
        void *source_addr2;
  
        // here we need to distinguish static and dynamic arrays
        int dim_are_nums = 1;

        {
            char *dim_start = strchr(f->field_type, '[');
            char *dim_end = strrchr(f->field_type, ']');
            while(dim_start < dim_end) {
                if(*dim_start == '[' || *dim_start == ']') {
                    dim_start ++;
                    continue;
                } else if(!isdigit(*dim_start)) {
                    dim_are_nums = 0;
                    break;
                }
                else {
                    dim_start ++;
                    continue;
                }
            }
        }

        if(dim_are_nums) {
            source_addr2 = source_addr;
        } else {
            switch(var->type) {
                case adios_byte:
                case adios_unsigned_byte:
                    source_addr2 = *((char **)source_addr);
                    break;

                case adios_string: // TODO
                    return strlen((char *)source_addr) + 1;
                    //source_addr2 = *((char **)source_addr);
                    //break;

                case adios_short:
                case adios_unsigned_short:
                    source_addr2 = *((short int **)source_addr);
                    break;

                case adios_integer:
                case adios_unsigned_integer:
                    source_addr2 = *((int **)source_addr);
                    break;

                case adios_long:
                case adios_unsigned_long:
                    source_addr2 = *((long int **)source_addr);
                    break;

                case adios_real:
                    source_addr2 = *((float **)source_addr);
                    break;

                case adios_double:
                    source_addr2 = *((double **)source_addr);
                    break;

                case adios_long_double:
                    source_addr2 = *((long double **)source_addr);
                    break;

                case adios_complex:
                case adios_double_complex:
                default:
                    adios_error(err_invalid_read_method, "complex data type is not supported.");
                    return -1;
            }
        }

        // find the array dimension info 
        int j;
        datatap_var_chunk *chunk = var->chunks; 
        while(chunk) {
            if(chunk->rank == qi->rank) {
                break;
            } 
            else {
                chunk = chunk->next;
            }
        }

        if(!chunk) { // this chunk doesn't contain the var so skip it
            current_chunk = current_chunk->next;
            continue;
        }

        // now we copy data into user's read buffer
        uint64_t global_start_r = 0;
        for(j = var->ndims-1; j >= 0; j --) {
            global_start_r *= chunk->global_bounds[j];
            global_start_r += start[j];
        }


        total_size = type_size;
        for(j = var->ndims-1; j >= 0; j --) {
            total_size *= count[j];
        }

        if(ds->host_language_fortran == adios_flag_yes && futils_is_called_from_fortran()) {

            // find the smallest slice
            int min_dim = 0;
            uint64_t stride_size = type_size;
            for(j = 0; j < var->ndims; j ++) {
                int lower = chunk->global_offsets[j] - start[j];
                int higher = (start[j]+count[j]) - (chunk->global_offsets[j]+chunk->local_bounds[j]);
                if(lower == 0 && higher == 0) {
                    // this means this dimnesion fits, let's move on to the next dimension
                    stride_size *= chunk->local_bounds[j];
                    min_dim ++; 
                    continue;
                }
                else if((lower > 0 && higher >= 0) ||
                        (lower >= 0 && higher > 0)) {
                    // this means this dimension is strided. this dimension is the highest possible 
                    // dimension
                    stride_size *= chunk->local_bounds[j];
                    break;
                }
            }

            uint64_t num_strides = 1;
            for(j = min_dim+1; j < var->ndims; j ++) {
                num_strides *= chunk->local_bounds[j];
            }

            uint64_t current_pos[var->ndims]; // track current stride's starting global offset
            for(j = 0; j < var->ndims; j ++) {
                current_pos[j] = chunk->global_offsets[j];
            }
            uint64_t k = 0;       
            char *stride_start_addr = source_addr2; 
            int should_stop = 0;
            for(; k < num_strides; k ++) {
                // calculate the coordinates within the bounding box
                uint64_t my_start = 0;
                for(j = var->ndims-1; j >= 0; j --) {
                    my_start *= count[j];
                    //my_start += (current_pos[j] - chunk->global_offsets[j]);
                    my_start += (current_pos[j] - start[j]);
                } 
                char *position = (char *)data + my_start * type_size;

fprintf(stderr, "im here %s mystart %lu size %lu %s:%d\n", var->varname,my_start, stride_size,__FILE__,__LINE__);
                memcpy(position, stride_start_addr, stride_size);

                // now advance to copy next stride
                stride_start_addr += stride_size; 
                should_stop = 0;
                for(j = min_dim+1; j < var->ndims; j ++) {
                    if(!should_stop) {
                        if(current_pos[j] == chunk->global_offsets[j]+chunk->local_bounds[j]-1) {
                            // don't set should_stop so we move on to next dimension
                            current_pos[j] = chunk->global_offsets[j];
                        }
                        else {
                            current_pos[j] ++;
                            should_stop = 1;
                        }
                    } 
                    else {
                        break;
                    }
                }
            }  
        }
        else if(ds->host_language_fortran == adios_flag_yes && !futils_is_called_from_fortran()) {

            // find the smallest slice
            int min_dim = var->ndims-1;
            uint64_t stride_size = type_size;
            for(j = var->ndims-1; j >= 0; j --) {
                int lower = chunk->global_offsets[j] - start[j];
                int higher = (start[j]+count[j]) - (chunk->global_offsets[j]+chunk->local_bounds[j]);
                if(lower == 0 && higher == 0) {
                    // this means this dimnesion fits, let's move on to the next dimension
                    stride_size *= chunk->local_bounds[j];
                    min_dim --;
                    continue;
                }
                else if((lower > 0 && higher >= 0) ||
                        (lower >= 0 && higher > 0)) {
                    // this means this dimension is strided. this dimension is the highest possible
                    // dimension
                    stride_size *= chunk->local_bounds[j];
                    break;
                }
            }

            uint64_t num_strides = 1;
            for(j = min_dim-1; j >= 0; j --) { // TODO: j=min_dim for pixmon!!
                num_strides *= chunk->local_bounds[j];
            }

            uint64_t current_pos[var->ndims]; // track current stride's starting global offset
            for(j = 0; j < var->ndims; j ++) {
                current_pos[j] = chunk->global_offsets[j];
            }
            uint64_t k = 0;
            char *stride_start_addr = source_addr2;
            int should_stop = 0;
            for(; k < num_strides; k ++) {
                // calculate the coordinates within the bounding box
                uint64_t my_start = 0;
                //for(j = var->ndims-1; j >= 0; j --) {
                for(j = 0; j < var->ndims; j ++) {
                    my_start *= count[j];
                    //my_start += (current_pos[j] - chunk->global_offsets[j]);
                    my_start += (current_pos[j] - start[j]);
                }
                char *position = (char *)data + my_start * type_size;

                // check if this stride fit into the read region
                char *end_position = position + stride_size;
                if((position < data + total_size) && 
                   (end_position <= data + total_size) &&
                   (position >= data) &&
                   (end_position > data) ) {   

//fprintf(stderr, "im here %s mystart %lu size %lu %s:%d\n", var->varname,my_start, stride_size,__FILE__,__LINE__);
                    memcpy(position, stride_start_addr, stride_size);

                }
                else {
fprintf(stderr, "im here out of bond \n", __FILE__,__LINE__);
                }

                // now advance to copy next stride
                stride_start_addr += stride_size;
                should_stop = 0;
                for(j = min_dim-1; j >= 0; j --) {
                    if(!should_stop) {
                        if(current_pos[j] == chunk->global_offsets[j]+chunk->local_bounds[j]-1) {
                            // don't set should_stop so we move on to next dimension
                            current_pos[j] = chunk->global_offsets[j];
                        }
                        else {
                            current_pos[j] ++;
                            should_stop = 1;
                        }
                    }
                    else {
                        break;
                    }
                }
            }
        } 

        current_chunk = current_chunk->next;
    }

    // TODO: the data size is messed up because of ghost zone
    total_size = type_size;
    int j;
    for(j = var->ndims-1; j >= 0; j --) {
        total_size *= count[j]; 
    }
    return total_size;       
}

/*
 * chunked read
 */
pg_chunk *get_next_chunk(ADIOS_FILE *fp) 
{
    datatap_read_file_data *ds = (datatap_read_file_data *) fp->fh;

    QueueItem *qi = NULL;
    Queue *data_q = ds->f_info->dt_queue;

    // take the next chunk from queue
    pthread_mutex_lock(&(ds->f_info->mutex));
    if(ds->f_info->num_chunks == ds->f_info->num_chunks_queued) {
        if(queue_size(data_q) == 0) {
            // no more chunks
            pthread_mutex_unlock(&(ds->f_info->mutex));
            return NULL;
        }  
    }
    while(queue_size(data_q) == 0) {
        pthread_cond_signal(&(ds->f_info->cond1)); 
        pthread_cond_wait(&(ds->f_info->cond2), &(ds->f_info->mutex));
    }
    queue_dequeue(data_q, &qi);
    pthread_cond_signal(&(ds->f_info->cond1)); 
    pthread_mutex_unlock(&(ds->f_info->mutex));

    FMFieldList filed_list = qi->var_list->field_list; 

    int i;
    for(i = 0; i < ds->f_info->num_chunks; i ++) { 
        if(qi->rank == ds->pgs[i].rank) {
            qi->pg_info = &(ds->pgs[i]);  
            break;
        }
    }

    return qi;
}

void return_chunk_buffer(ADIOS_FILE *fp, pg_chunk *chunk)
{
    if(chunk) {
        free(chunk->data);
        free(chunk);
    }
}

void *read_var_in_chunk(pg_chunk *pg, char *varname, datatap_var_info **var_info, datatap_var_chunk **var_chunk)
{
    datatap_var_info *current_var;
    int i;
    for(i = 0; i < pg->pg_info->num_vars; i ++) {
        current_var = pg->pg_info->vars[i];
        if(!compare_var_name(varname, current_var)) {
            if(var_info) *var_info = current_var;
            if(var_chunk) *var_chunk = pg->pg_info->var_chunks[i];        

            FMFieldList filed_list = pg->var_list->field_list; // TODO

            // first, find the var
            FMField *f = find_field(varname, current_var->varpath, filed_list);
            char *source_addr = (char *)pg->data + f->field_offset; // this is the address

            if(!current_var->ndims) { // scalar
                //return (*var_chunk)->data; 
                return source_addr; 
            }
            else { // array
                void *source_addr2;

                // here we need to distinguish static and dynamic arrays
                int dim_are_nums = 1;

                {
                    char *dim_start = strchr(f->field_type, '[');
                    char *dim_end = strrchr(f->field_type, ']');
                    while(dim_start < dim_end) {
                        if(*dim_start == '[' || *dim_start == ']') {
                            dim_start ++;
                            continue;
                        } else if(!isdigit(*dim_start)) {
                            dim_are_nums = 0;
                            break;
                        }
                        else {
                            dim_start ++;
                            continue;
                        }
                    }
                }

                if(dim_are_nums) {
                    source_addr2 = source_addr;
                } else {
                    switch(current_var->type) {
                        case adios_byte:
                        case adios_unsigned_byte:
                            source_addr2 = *((char **)source_addr);
                            break;

                        case adios_string: // TODO
                            return strlen((char *)source_addr) + 1;
                            //source_addr2 = *((char **)source_addr);
                            //break;

                        case adios_short:
                        case adios_unsigned_short:
                            source_addr2 = *((short int **)source_addr);
                            break;

                        case adios_integer:
                        case adios_unsigned_integer:
                            source_addr2 = *((int **)source_addr);
                            break;

                        case adios_long:
                        case adios_unsigned_long:
                            source_addr2 = *((long int **)source_addr);
                            break;

                        case adios_real:
                            source_addr2 = *((float **)source_addr);
                            break;

                        case adios_double:
                            source_addr2 = *((double **)source_addr);
                            break;

                        case adios_long_double:
                            source_addr2 = *((long double **)source_addr);
                            break;

                        case adios_complex:
                        case adios_double_complex:
                        default:
                            adios_error(err_invalid_read_method, "complex data type is not supported.");
                            return -1;
                    }
                }

                return source_addr2;
            }
        }
    }
   
    if(var_info) *var_info = NULL;
    if(var_chunk) *var_chunk = NULL;
    return NULL;
}

ADIOS_VARINFO *inq_var_in_chunk(ADIOS_GROUP *gp, pg_chunk *pg, char *varname)
{

    ADIOS_VARINFO *v = (ADIOS_VARINFO *) malloc(sizeof(ADIOS_VARINFO));
    if(!v) {
        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
        return NULL;
    }
    memset(v, 0, sizeof(ADIOS_VARINFO));

    datatap_var_info *current_var;
    int i;
    for(i = 0; i < pg->pg_info->num_vars; i ++) {
        current_var = pg->pg_info->vars[i];
        if(!compare_var_name(varname, current_var)) {
            v->grpid = gp->grpid;
            int j;
            for(j = 0; j < gp->vars_count; j ++) {
                if(!strcmp(gp->var_namelist[j], varname)) {
                    v->varid = j; // TODO: this may not be cmpatible with BP
                    break;
                }
            }
            v->type = current_var->type;
            v->ndim = current_var->ndims;
            v->timedim = current_var->time_dim;
            if(!v->ndim) { // scalar and string
                if(v->timedim != -1) { // scalar with time dimension
                    v->ndim = 1;
                    v->dims = (uint64_t *) malloc(sizeof(uint64_t));
                    if(!v->dims) {
                        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                        return NULL;
                    }
                    v->dims[0] = 1; // TODO: only one timestep in the file
                }
                int value_size = current_var->data_size;
                v->value = malloc(value_size);
                if(!v->value) {
                    adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                    return NULL;
                }
                memcpy(v->value, current_var->chunks->data, value_size);
            }
            else { // arrays
                v->dims = (uint64_t *) malloc(v->ndim * sizeof(uint64_t));
                if(!v->dims) {
                    adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                    return NULL;
                }
                int k;
                for(k = 0; k < v->ndim; k ++) {
                    //v->dims[k] = ds->pgs[i].vars[j].global_bounds[k];
                    v->dims[k] = current_var->chunks->global_bounds[k];
                }
            }
            return v;
        }
    }

    adios_error(err_invalid_varname, "Cannot find var %s\n", varname);
    return NULL;
}

void data_handler (void *data, int length, void *user_data, attr_list attr, int rank, void *timing, file_info *f)
{
    // TODO: move this to initialization
    if(!dt_thread_pinned) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(dt_thread_cpuid, &cpuset);
        int rc = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
        if(rc) {
            fprintf(stderr, "cannot set application thread affinity\n");
        }
        dt_thread_pinned = 1;
    }

    recvtime *r = (recvtime*)timing;
    IOhandle *h = (IOhandle*)user_data;
    elapsedtime *e = updateTimes(h, r, length);
    
    r->fS = getlocaltime();

    r->startDecode = getlocaltime();

    // decode the data and insert the data into the queue
    int decoded_length = FFS_est_decode_length(h->iocontext, data, length);

    // TODO: make sure we free this later (after writing to hdf5 file)
    char *decoded_data = (char *)malloc(decoded_length);

    if(!decoded_data) {
        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
        exit(-1);
    }

    FFSTypeHandle ffshandle = FFSTypeHandle_from_encode(h->iocontext, data);
    FMFormat form = FMFormat_of_original(ffshandle);

    // TODO
    FMStructDescList var_list = get_localized_formats(form);
    establish_conversion(h->iocontext, ffshandle, var_list);
    FFSdecode_to_buffer(h->iocontext, data, decoded_data);

    // The encoded data can be recycled now
    returnbuffer(h, data, length);

    r->endDecode = getlocaltime();

    QueueItem * qi =(QueueItem *) malloc(sizeof(QueueItem));
    if(!qi) {
        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
        exit(-1);
    }
    qi->data = decoded_data;
    qi->length = decoded_length;
    qi->var_list = var_list;
    qi->rank = rank; // TODO: hack it!   

    // put message in queue
    pthread_mutex_lock(&(f->mutex));
    while(queue_size(f->dt_queue) >= f->dt_queue_max_length) {
        // wait until queue is not full
        pthread_cond_signal(&(f->cond2));
        pthread_cond_wait(&(f->cond1), &(f->mutex));
    }
    queue_enqueue(f->dt_queue, qi);
    f->num_chunks_queued ++;
    unsigned int g_q_size = queue_size(f->dt_queue);
    pthread_cond_signal(&(f->cond2));
    pthread_mutex_unlock(&(f->mutex));

    r->fE = getlocaltime();

    fprintf(server_timing_file, "%llu\t%u\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%u\t%d\n",
        e->recvcount, length,(r->sendDataRead - r->recvRequest),
        (r->completeDataRead - r->startDataRead),
        (r->startDataRead - r->sendDataRead),
        (r->endDecode - r->startDecode),
        r->fE,
        r->fS,
        r->recvRequest,
        g_q_size,
        rank
        );
    fflush(server_timing_file);
   
    // TODO: check if it's time to stop
}

void * dt_server_thread_func (void *arg)
{
    MPI_Comm orig_comm = (MPI_Comm) arg;

    if(!dt_thread_pinned) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(dt_thread_cpuid, &cpuset);
        int rc = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
        if(rc) {
            fprintf(stderr, "cannot set application thread affinity\n");
        }
        dt_thread_pinned = 1;
    }

#if 0 // open-mpi   
    // duplicate a MPI communicator for synchronization between dt servers
    int rc = MPI_Comm_dup(orig_comm, &(dt_read_data->dt_comm));
    if(rc != MPI_SUCCESS) {
        error(err_unspecified, "Cannot duplicate communicator for Datatap.");
        pthread_exit(NULL);
    }
#else 
    dt_read_data->dt_comm = orig_comm;
#endif

#ifdef _NOMPI
    dt_read_data->dt_comm_size = 1;
    dt_read_data->dt_comm_rank = 0;
#else 
    MPI_Comm_size(dt_read_data->dt_comm, &(dt_read_data->dt_comm_size));
    MPI_Comm_rank(dt_read_data->dt_comm, &(dt_read_data->dt_comm_rank));
#endif
  
    char param_file[30];
    int appid, was_set;
    appid = globals_adios_get_application_id(&was_set);
    if(!was_set) {
        adios_error(err_unspecified, "Application ID was not set.");
        sprintf(param_file, "datatap_param\0");
    }
    else {
        sprintf(param_file, "datatap_param%d\0", appid);
    }

    // for dt server timing
    char name_buffer[40];
    sprintf(name_buffer, "dt_server_timing.%d.%d\0", appid, dt_read_data->dt_comm_rank);
    server_timing_file = fopen(name_buffer, "w");
    if(!server_timing_file) {
        fprintf(stderr, "cannot open file %s.\n", name_buffer);
        exit(-1);
    }

    sprintf(name_buffer, "dt_file_timing.%d.%d\0", appid, dt_read_data->dt_comm_rank);
    file_timing_file = fopen(name_buffer, "w");
    if(!file_timing_file) {
        fprintf(stderr, "cannot open file %s.\n", name_buffer);
        exit(-1);
    }

//added by Jianting
    char *contact_string=cercs_getenv("LGS_CONTACT");
    init("appname", -1, contact_string);
    dt_read_data->dt_cm=LGS_getCM();

    // initialize ptlpbio interface
//    dt_read_data->dt_cm = CManager_create();
//    CMlisten_specific(dt_read_data->dt_cm, NULL);


    lrand48();

    dt_read_data->dt_handle = EVthin_portals_listen(dt_read_data->dt_cm, 120,
                              0, data_handler, dt_read_data->dt_comm);

    char app[20];
    sprintf(app, "%d\0", appid);
    //outputConnectionDataToLGS(app, dt_read_data->dt_handle, dt_read_data->dt_comm_rank);
    outputConnectionDataToLGS(app, dt_read_data->dt_handle, dt_read_data->dt_comm_rank, dt_read_data->dt_comm_size, dt_read_data->dt_comm);

    // dt server(rank 0) gather contact info from other servers and write to
    // a file so upstream writers can connect to this application
//    outputConnectionData(param_file, dt_read_data->dt_handle);

    dt_read_data->dt_server_ready = 1;

    // serve the network
//    CMrun_network(dt_read_data->dt_cm);

    // TODO: cleanup and exit
//    CManager_close(dt_read_data->dt_cm);
    return NULL;
}

int adios_read_datatap_init (MPI_Comm comm)
{
    setenv("CMSelfFormats", "1", 1);

    // initialize Datatap read method structure
    dt_read_data = (datatap_read_method_data *) malloc(sizeof(datatap_read_method_data));
    if(!dt_read_data) {
        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
        return -1;
    }    
    memset(dt_read_data, 0, sizeof(datatap_read_method_data));

    // enable threading for EVPath
    gen_pthread_init();

    // set cpu affinity
    int myrank;
    MPI_Comm_rank(comm, &myrank);
    app_thread_cpuid = (myrank % 2);
    dt_thread_cpuid = app_thread_cpuid + 2;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(app_thread_cpuid, &cpuset);
    int err = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    if(err) { 
        fprintf(stderr, "cannot set application thread affinity\n");
    }

    // fork the thread to poll network 
    int rc = pthread_create(&(dt_read_data->dt_server_thread), NULL, 
                            dt_server_thread_func, (void *)comm);
    if(rc) {
        adios_error(err_unspecified, "Failed to create Datatap server thread.");
        return -1; 
    }

    // TODO: wait until the dt server thread is ready
    while(!dt_read_data->dt_server_ready) { }
//    dt_server_thread_func((void *)comm);
    
    return 0;
}

int adios_read_datatap_finalize ()
{
    // notify and wait for dt server thread to exit
    datatap_stop_server(dt_read_data->dt_handle);
    //pthread_join(dt_read_data->dt_server_thread, NULL);

    fclose(server_timing_file);
    fclose(file_timing_file);

#if 0
    // TODO: we need a datatap cleanup function
    pthread_mutex_destroy(&(dt_read_data->mutex));
    pthread_cond_destroy(&(dt_read_data->cond1));
    pthread_cond_destroy(&(dt_read_data->cond2));    
    free(dt_read_data->dt_queue);
#endif

    free(dt_read_data);
    return 0;
}

ADIOS_FILE *adios_read_datatap_fopen(const char *fname, MPI_Comm comm)
{
    ADIOS_FILE *fp;
    adios_errno = 0;

    datatap_read_file_data *ds = (datatap_read_file_data *) malloc(sizeof(datatap_read_file_data));
    if(!ds) {
        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
        return NULL;                
    }
    ds->file_name = strdup(fname);    
     
    // here we need to syncrhonize with other reader processes to see what they find
    // 1: file available locally
    // 0: file not found
    int my_status, global_status;
    int total_num_readers, my_rank;

#ifdef _NOMPI
    total_num_readers = 1;
    my_rank = 0;
#else
    MPI_Comm_size(comm, &total_num_readers);
    MPI_Comm_rank(comm, &my_rank);
#endif

    ds->comm = comm;
    ds->my_rank = my_rank;
    ds->comm_size = total_num_readers;

    int is_EOF = 0;

    is_EOF = 0;

    // first we need to make sure the file has been 'written'
    // TODO: always start from timstep 0a
    // NOTE: now datatap_get_file_info() is blocking call so we don't need to synchronize with peer readers
    ds->f_info = datatap_get_file_info(dt_read_data->dt_handle, fname, dt_read_data->num_io_dumps, &is_EOF);
    if(!ds->f_info) {
        if(is_EOF) {
            adios_errno = err_end_of_file;
            adios_error(err_end_of_file, "Reach the end of file (%s).", fname);
            free(ds->file_name);
            free(ds);
        }
    }       
    
    // we don't know what to read yet
    ds->num_vars = 0;
    ds->vars = NULL;

    // pg list is used for chunked read
    ds->pgs = (datatap_pg_info *) malloc(ds->f_info->num_chunks * sizeof(datatap_pg_info));
    if(!ds->pgs) {
        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
        return NULL;
    }

    // TODO: add a loop over chunks
    int i;
    for(i = 0; i < ds->f_info->num_chunks; i ++) {  
        chunk_info *current_chunk = &(ds->f_info->chunks[i]);  
   
        // parse the var info
        char *current_pos = current_chunk->var_info_buffer;
        int total_var_info_size = *(int *)current_pos; // total size
        current_pos += sizeof(int);

        if(i == 0) {
            ds->host_language_fortran = *(enum ADIOS_FLAG *)current_pos; // host language
        }
        current_pos += sizeof(int);

        int group_name_len = *(int *)current_pos; // size of group name
        current_pos += sizeof(int); 
  
        if(i == 0) { // TODO: let's assume one group name
            ds->group_name = strdup(current_pos); // group name
        }
        current_pos += group_name_len;

        int num_vars_in_pg = *(int *)current_pos; // total num of vars  
        current_pos += sizeof(int);
         
        // for chunked read
        ds->pgs[i].rank = current_chunk->rank;
        ds->pgs[i].num_vars = num_vars_in_pg;
        ds->pgs[i].vars = (datatap_var_info **) malloc(num_vars_in_pg*sizeof(datatap_var_info *));
        if(!ds->pgs[i].vars) {
            adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
            return NULL;
        }
        ds->pgs[i].var_chunks = (datatap_var_chunk **) malloc(num_vars_in_pg*sizeof(datatap_var_chunk *));
        if(!ds->pgs[i].var_chunks) {
            adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
            return NULL;
        }
        int k = 0;

        datatap_var_info *current_var;
        char *end = current_chunk->var_info_buffer + total_var_info_size;
        while(current_pos < end) {
            // size of this var info
            int var_info_size = *(int *) current_pos;             
            current_pos += sizeof(int);        
            int var_id = *(int *) current_pos;
            current_pos += sizeof(int);        
            int varname_len = *(int *) current_pos;        
            current_pos += sizeof(int);        
            char *varname = current_pos;  
            current_pos += varname_len;
            int varpath_len = *(int *) current_pos;
            current_pos += sizeof(int);
            char *varpath = current_pos;
            current_pos += varpath_len;

            // now we go through the current list of vars to see if this is new
            current_var = ds->vars;
            while(current_var != NULL) {
                // TODO: compare var id
                //if(!strcmp(current_var->varname, varname) && 
                //    !strcmp(current_var->varpath, varpath)) {
                if(var_id == current_var->id) {
                    break;
                }
                else {
                    current_var = current_var->next;
                }
            }

            if(!current_var) { // this is a new var
                current_var = (datatap_var_info *) malloc(sizeof(datatap_var_info));
                if(!current_var) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;
                }
                current_var->id = var_id;
                current_var->varname = strdup(varname);
                current_var->varpath = strdup(varpath);
                current_var->type = *(enum ADIOS_DATATYPES *) current_pos;
                current_pos += sizeof(enum ADIOS_DATATYPES);
                current_var->time_dim = *(int *) current_pos;
                current_pos += sizeof(int);
                current_var->ndims = *(int *) current_pos;
                current_pos += sizeof(int);
                current_var->num_chunks = 0;
                current_var->chunks = NULL;
                if(!current_var->ndims) { // scalars and strings
                    current_var->data_size = common_read_type_size(current_var->type, current_pos);
                }
                current_var->next = ds->vars;
                ds->vars = current_var;
                ds->num_vars ++;
            }
            else {
                current_pos += sizeof(enum ADIOS_DATATYPES);
                //current_var->time_dim = *(int *) current_pos;
                current_pos += sizeof(int);
                //current_var->ndims = *(int *) current_pos;
                current_pos += sizeof(int);
            }            

            // for chunked read
            ds->pgs[i].vars[k] = current_var;

            datatap_var_chunk *new_chunk = (datatap_var_chunk *) malloc(sizeof(datatap_var_chunk));             
            if(!new_chunk) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;
            }
            new_chunk->next = current_var->chunks;
            current_var->chunks = new_chunk;
            current_var->num_chunks ++; 

            // for chunked read
            ds->pgs[i].var_chunks[k] = new_chunk;

            new_chunk->rank = current_chunk->rank;
            if(!current_var->ndims) { // scalars and strings
                // copy data value             
                new_chunk->data = malloc(current_var->data_size);
                if(!new_chunk->data) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;                                  
                }

                memcpy(new_chunk->data, current_pos, current_var->data_size);         
                current_pos += current_var->data_size;
            }
            else { // arrays
                new_chunk->local_bounds = (uint64_t *) malloc(current_var->ndims * sizeof(uint64_t));  
                if(!new_chunk->local_bounds) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;                                  
                }
                new_chunk->global_bounds = (uint64_t *) malloc(current_var->ndims * sizeof(uint64_t));  
                if(!new_chunk->global_bounds) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;                                  
                }
                new_chunk->global_offsets = (uint64_t *) malloc(current_var->ndims * sizeof(uint64_t));  
                if(!new_chunk->global_offsets) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;                                  
                }
                int j;
                for(j = 0; j < current_var->ndims; j ++) {
                    new_chunk->local_bounds[j] = *(uint64_t *)current_pos;
                    current_pos += sizeof(uint64_t);  
                    new_chunk->global_bounds[j] = *(uint64_t *)current_pos;
                    current_pos += sizeof(uint64_t);  
                    new_chunk->global_offsets[j] = *(uint64_t *)current_pos;
                    current_pos += sizeof(uint64_t);  
                }                          
            }
        }    

        // TODO: at this point, we no longer need var_info_buffer so free it
        // free(current_chunk->var_info_buffer);
    }
        

    // TODO: replicate meta-data among peer reader
    ds->num_vars_peer = 0;
    ds->vars_peer = NULL;

    if(total_num_readers > 1)
    {
        char *send_buf = NULL;
        char *recv_buf = NULL; 
        int send_count = VAR_INFO_SIZE, recv_count = VAR_INFO_SIZE;
        if(my_rank == 0) {
            for(i = 0; i < ds->f_info->num_chunks; i ++) {
                chunk_info *current_chunk = &(ds->f_info->chunks[i]);
                if(current_chunk->rank == 0) {
                    send_buf = ds->f_info->chunks[i].var_info_buffer;
                    break;
                } 
            }
            
            // TODO: before we do this, we need to make sure this one has sth special
            // the only case we need to deal with is local array with only one chunk
            // we will check this when calling read_var

            int rc = MPI_Bcast(send_buf, send_count, MPI_BYTE, 0, comm);
            if(rc != MPI_SUCCESS) {
                fprintf(stderr, "rank %d: MPI_Scatter returns error (%d). %s:%d\n",
                    my_rank, rc, __FILE__, __LINE__);
                return NULL;
            }
        }
        else {
            recv_buf = (char *) malloc(VAR_INFO_SIZE); 
            if(!recv_buf) {
                adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                return NULL;
            }

            int rc = MPI_Bcast(recv_buf, recv_count, MPI_BYTE, 0, comm);
            if(rc != MPI_SUCCESS) {
                fprintf(stderr, "rank %d: MPI_Scatter returns error (%d). %s:%d\n",
                    my_rank, rc, __FILE__, __LINE__);
                return NULL;
            }
        }

        if(my_rank != 0) {
            // parse the var_info buffer
            char *current_pos = recv_buf;
            int total_var_info_size = *(int *)current_pos; // total size
            current_pos += sizeof(int);

            current_pos += sizeof(enum ADIOS_FLAG); // host language

            int group_name_len = *(int *)current_pos; // size of group name
            current_pos += sizeof(int);
            current_pos += group_name_len;

            int num_vars = *(int *)current_pos; // total num of vars
            current_pos += sizeof(int);

            datatap_var_info *current_var;
            char *end = recv_buf + total_var_info_size;
            while(current_pos < end) {
                // size of this var info
                int var_info_size = *(int *) current_pos;
                char *start_of_next_var = current_pos + var_info_size;
                current_pos += sizeof(int);
                int var_id = *(int *) current_pos;
                current_pos += sizeof(int);
                int varname_len = *(int *) current_pos;
                current_pos += sizeof(int);
                char *varname = current_pos;
                current_pos += varname_len;
                int varpath_len = *(int *) current_pos;
                current_pos += sizeof(int);
                char *varpath = current_pos;
                current_pos += varpath_len;

                // now we go through the current list of vars to see if this is new
                current_var = ds->vars;
                while(current_var != NULL) {
                    //if(!strcmp(current_var->varname, varname) &&
                    //    !strcmp(current_var->varpath, varpath)) {
                    if(var_id == current_var->id) {
                        // this var is locally available, so skip it
                        break;
                    }
                    else {
                        current_var = current_var->next;
                    }
                }

                if(current_var) { // this var is locally available
                    current_pos = start_of_next_var;
                    continue;
                }

                current_var = (datatap_var_info *) malloc(sizeof(datatap_var_info));
                if(!current_var) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;
                }
                current_var->id = var_id;
                current_var->varname = strdup(varname);
                current_var->varpath = strdup(varpath);
                current_var->type = *(enum ADIOS_DATATYPES *) current_pos;
                current_pos += sizeof(enum ADIOS_DATATYPES);
                current_var->time_dim = *(int *) current_pos;
                current_pos += sizeof(int);
                current_var->ndims = *(int *) current_pos;
                current_pos += sizeof(int);
                current_var->num_chunks = 0;
                current_var->chunks = NULL;
                if(!current_var->ndims) { // scalars and strings
                    current_var->data_size = common_read_type_size(current_var->type, current_pos);
                }
                current_var->next = ds->vars_peer;
                ds->vars_peer = current_var;
                ds->num_vars_peer ++;

                datatap_var_chunk *new_chunk = (datatap_var_chunk *) malloc(sizeof(datatap_var_chunk));
                if(!new_chunk) {
                    adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                    return NULL;
                }
                new_chunk->next = current_var->chunks;
                current_var->chunks = new_chunk;
                current_var->num_chunks ++;

                new_chunk->rank = 0;
                if(!current_var->ndims) { // scalars and strings
                    // copy data value
                    new_chunk->data = malloc(current_var->data_size);
                    if(!new_chunk->data) {
                        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                        return NULL;
                    }
                    memcpy(new_chunk->data, current_pos, current_var->data_size);
                    current_pos += current_var->data_size;
                }
                else { // arrays
                    new_chunk->local_bounds = (uint64_t *) malloc(current_var->ndims * sizeof(uint64_t));
                    if(!new_chunk->local_bounds) {
                        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                        return NULL;
                    }
                    new_chunk->global_bounds = (uint64_t *) malloc(current_var->ndims * sizeof(uint64_t));
                    if(!new_chunk->global_bounds) {
                        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                        return NULL;
                    }
                    new_chunk->global_offsets = (uint64_t *) malloc(current_var->ndims * sizeof(uint64_t));
                    if(!new_chunk->global_offsets) {
                        adios_error(err_no_memory, "Cannot allocate memory for Datatap.");
                        return NULL;
                    }
                    int i;
                    for(i = 0; i < current_var->ndims; i ++) {
                        new_chunk->local_bounds[i] = *(uint64_t *)current_pos;
                        current_pos += sizeof(uint64_t);
                        new_chunk->global_bounds[i] = *(uint64_t *)current_pos;
                        current_pos += sizeof(uint64_t);
                        new_chunk->global_offsets[i] = *(uint64_t *)current_pos;
                        current_pos += sizeof(uint64_t);
                    }
                }
            }
            
            free(recv_buf);   
        }
    }

    // TODO: here we need to ajust array dimension if the reader is in C 
    // but writer is in Fortran or vice versa
    if(ds->host_language_fortran == adios_flag_yes && !futils_is_called_from_fortran()) {
        // reader is in C but writer is in Fortran, there are several things to adjust:
        // array dimension index starts from 1 in Fortran --> start from 0 in C
        // change array dimension order (including time dimension)
        datatap_var_info *v = ds->vars;        
        while(v) {
            datatap_var_chunk *chunk = v->chunks; 
            while(chunk) {
                int i;
                uint64_t temp;
                for(i = 0; i < v->ndims/2; i ++) {
                    temp = chunk->local_bounds[v->ndims-i-1];
                    chunk->local_bounds[v->ndims-i-1] = chunk->local_bounds[i];
                    chunk->local_bounds[i] = temp;
                    temp = chunk->global_bounds[v->ndims-i-1];
                    chunk->global_bounds[v->ndims-i-1] = chunk->global_bounds[i];
                    chunk->global_bounds[i] = temp;
                    temp = chunk->global_offsets[v->ndims-i-1];
                    chunk->global_offsets[v->ndims-i-1] = chunk->global_offsets[i];
                    chunk->global_offsets[i] = temp;
                }
                chunk = chunk->next;
            }
            if(v->time_dim > 0) { // -1 means no time dimension
                v->time_dim = v->ndims - v->time_dim;  
            }
            v = v->next;
        }

        v = ds->vars_peer;
        while(v) {
            datatap_var_chunk *chunk = v->chunks;
            while(chunk) {
                int i;
                for(i = 0; i < v->ndims; i ++) {
                    chunk->local_bounds[i] --;
                    //chunk->global_offsets[i] --;
                }
                uint64_t temp;
                for(i = 0; i < v->ndims/2; i ++) {
                    temp = chunk->local_bounds[v->ndims-i-1];
                    chunk->local_bounds[v->ndims-i-1] = chunk->local_bounds[i];
                    chunk->local_bounds[i] = temp;
                    temp = chunk->global_bounds[v->ndims-i-1];
                    chunk->global_bounds[v->ndims-i-1] = chunk->global_bounds[i];
                    chunk->global_bounds[i] = temp;
                    temp = chunk->global_offsets[v->ndims-i-1];
                    chunk->global_offsets[v->ndims-i-1] = chunk->global_offsets[i];
                    chunk->global_offsets[i] = temp;
                }
                chunk = chunk->next;
            }
            v->time_dim = v->ndims - v->time_dim - 1;
            v = v->next;
        }
    }
    else if(ds->host_language_fortran == adios_flag_no && futils_is_called_from_fortran()) {
        // adjuct dimension C --> Fortran  
        // TODO: for the demo, this will not happen so leave it here
    }

    fp = (ADIOS_FILE *) malloc(sizeof(ADIOS_FILE));
    if(!fp) {
        adios_error(err_no_memory, "Cannot allocate memory for file info.");
        free(ds);
        return NULL;
    }
 
    fp->fh = (uint64_t) ds;
    fp->groups_count = 1; // TODO: assume one group per file
    fp->vars_count = 0; // TODO: just do not use this filed
    fp->attrs_count = 0; // TODO: do not support attributes yet

    // TODO: since we require fopen/fclose for each timestep, 
    // so there is always only 1 timestep in file
    // TODO: set ntimesteps to be max to make pixie3d working
    fp->tidx_start = ds->f_info->timestep;  
    //fp->ntimesteps = INT32_MAX;
    fp->ntimesteps = 1;

    fp->file_size = 0; 
    fp->version = 1;
    fp->endianness = 0; 
    alloc_namelist(&fp->group_namelist, fp->groups_count); 
    for (i = 0; i < fp->groups_count; i++) {
        if (!fp->group_namelist[i])  {
            adios_error(err_no_memory, "Cannot allocate buffer in adios_fopen()");
            return NULL;
        }
        else {
            strcpy(fp->group_namelist[i], ds->group_name); 
        }
    }

    // TODO: return code of adios_errno for fopen:
    // 0: file metadata is available and everything is success
    // we need a value telling that the file is not available yet
    // we also need a value telling that writer is finishing so don't
    // read this file any more

    return fp;	
}

int adios_read_datatap_fclose(ADIOS_FILE *fp)
{
    datatap_read_file_data *ds = (datatap_read_file_data *) fp->fh;

    adios_errno = 0;

    dt_read_data->num_io_dumps ++;

    // recycle the queue
    QueueItem *qi;

    while((!queue_dequeue(ds->f_info->dt_queue, &qi)) && qi) {
        free(qi->data);
        free(qi);  
    }

    // record timing of the file
    ds->f_info->release_time = getlocaltime();
    fprintf(file_timing_file, "%d\t%f\t%f\t%f\t%f\n",
            ds->f_info->timestep,
            ds->f_info->create_time,
            ds->f_info->ready_time,
            ds->f_info->open_time,
            ds->f_info->release_time
            );
    fflush(file_timing_file);

    // notify dt server thread that we are done with this file 
    int rc = datatap_release_file(dt_read_data->dt_handle, ds->f_info);
    if(rc != 0) {
        adios_error(err_unspecified, "Could not close file.");
        return -1;
    }

    free_namelist((fp->group_namelist), fp->groups_count);
    if (ds->file_name) { 
        free(ds->file_name); 
        ds->file_name = NULL; 
    }
    
    if (ds->group_name) {
        free(ds->group_name);
        ds->group_name = NULL;
    }

    // TODO release pg_info and var_info 
    datatap_var_info *v = ds->vars;
    datatap_var_info *tmp_v;
    while(v) {
        tmp_v = v;
        free(v->varname);
        free(v->varpath);
        datatap_var_chunk *chunk = v->chunks;
        datatap_var_chunk *tmp_chunk;
        while(chunk) {
            tmp_chunk = chunk;
            if(v->ndims) {
                free(chunk->local_bounds);
                free(chunk->global_bounds);
                free(chunk->global_offsets);
            }
            else {
                free(chunk->data);     
            }
            chunk = chunk->next;
            free(tmp_chunk);
        }
        v = v->next;
        free(tmp_v);
    }

    v = ds->vars_peer;
    while(v) {
        tmp_v = v;
        free(v->varname);
        free(v->varpath);
        datatap_var_chunk *chunk = v->chunks;
        datatap_var_chunk *tmp_chunk;
        while(chunk) {
            tmp_chunk = chunk;
            if(v->ndims) {
                free(chunk->local_bounds);
                free(chunk->global_bounds);
                free(chunk->global_offsets);
            }
            else {
                free(chunk->data);
            }
            chunk = chunk->next;
            free(tmp_chunk);
        }
        v = v->next;
        free(tmp_v);
    }

    
    datatap_pg_info *pgs = ds->pgs; // size is f_info->num_chunks
    int i;
    for(i = 0; i < ds->f_info->num_chunks; i ++) {
        free(pgs->vars);
        free(pgs->var_chunks);
    }
    free(ds->pgs);

    free(ds);
    free(fp);
    return 0;
}

ADIOS_GROUP * adios_read_datatap_gopen (ADIOS_FILE *fp, const char *grpname)
{
    if(!grpname) {
        adios_error(err_invalid_group, "Group name is not valid");
        return NULL;
    }
    int i;
    for(i = 0; i < fp->groups_count; i ++) {   
        if(!strcmp(grpname, fp->group_namelist[i])) {
            return adios_read_datatap_gopen_byid(fp, i);
        }
    }
    adios_error(err_invalid_group, "Group %s is not valid", grpname);
    return NULL;
}

ADIOS_GROUP * adios_read_datatap_gopen_byid (ADIOS_FILE *fp, int grpid)
{
    datatap_read_file_data *ds = (datatap_read_file_data *) fp->fh;
    ADIOS_GROUP * gp;

    adios_errno = 0;

    gp = (ADIOS_GROUP *) malloc(sizeof(ADIOS_GROUP));
    if (!gp) {
        adios_error(err_no_memory, "Could not allocate memory for group info");
        return NULL;
    }

    // TODO: again, assume one group per file
    gp->grpid = grpid;
    gp->gh = (uint64_t) 0; // TODO: we should re-organize the metadata
    gp->fp = fp;
    gp->attrs_count = 0; // attributes are not supported yet
    
    // generate a list of variables with distinct names among all pgs
    gp->vars_count = ds->num_vars + ds->num_vars_peer;
    
    // to return a globally consistently ordered var list, we sort the list by var id
    datatap_var_info **vars = (datatap_var_info *) malloc(sizeof(datatap_var_info *) * gp->vars_count);
    if(!vars) {
        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_gopen_byid()");
        return NULL;
    }
    memset(vars, 0, sizeof(datatap_var_info *) * gp->vars_count);

    datatap_var_info *current_var = ds->vars_peer;
    datatap_var_info *var_to_sort = current_var;
    while(var_to_sort) {
        int j;
        for(j = 0; j < gp->vars_count; j ++) {
            if(vars[j] == NULL) {
                vars[j] = var_to_sort;
                current_var = current_var->next;
                var_to_sort = current_var;
                break; 
            }
            else if(vars[j]->id < var_to_sort->id) {
                // move vars[j] 
                datatap_var_info *tmp; 
                tmp = vars[j];
                vars[j] = var_to_sort;
                var_to_sort = tmp;
                break;
            }
        }
    }

    current_var = ds->vars;
    var_to_sort = current_var;
    while(var_to_sort) {
        int j;
        for(j = 0; j < gp->vars_count; j ++) {
            if(vars[j] == NULL) {
                vars[j] = var_to_sort;
                current_var = current_var->next;
                var_to_sort = current_var;
                break;
            }
            else if(vars[j]->id < var_to_sort->id) {
                // move vars[j]
                datatap_var_info *tmp;
                tmp = vars[j];
                vars[j] = var_to_sort;
                var_to_sort = tmp;
                break;
            }
        }
    }
    // now the vars list is in descendent order
    gp->var_namelist = (char **) malloc(gp->vars_count * sizeof(char *));
    if(!gp->var_namelist) {
        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_gopen_byid()");
        return NULL;
    }

    int i;
    for(i = 0; i < gp->vars_count; i ++) {
        int index = gp->vars_count - i - 1;
        gp->var_namelist[i] = (char *) malloc(strlen(vars[index]->varname) +
            strlen(vars[index]->varpath) + 2);
        if(!gp->var_namelist[i]) {
            adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_gopen_byid()");
            return NULL;
        }
        // return full path name
        // TODO: make sure the size of var_namelist[j] is enough
        if(!strcmp(vars[index]->varpath, "/")) {
            sprintf(gp->var_namelist[i], "/%s\0\0", vars[index]->varname);
        }
        else {
            sprintf(gp->var_namelist[i], "%s/%s\0", vars[index]->varpath,
                vars[index]->varname);
        }
    }

    // here we construct a bitmap of var list and send it to rank 0 so rank 0 knows 
    // which vars are only avaialble in its local memory
    memset(ds->var_bitmap, 0, VAR_BITMAP_SIZE);
    current_var = ds->vars_peer;
    while(current_var) {           
        int byte_pos = current_var->id / 8;
        int bit_pos = current_var->id % 8;
        unsigned char mask = 0x01 << bit_pos;
        ds->var_bitmap[byte_pos] = ds->var_bitmap[byte_pos] | mask;
        current_var = current_var->next;
    }

    // send back to rank 0 so it knows which var is missing on other processes
    unsigned char *combined_bitmap = NULL;
    int combined_size = ds->comm_size * VAR_BITMAP_SIZE;
    if(ds->my_rank == 0) {
        combined_bitmap = (unsigned char *) malloc(combined_size);
        if(!combined_size) {
            adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_gopen_byid()");
            return NULL;
        }
    }
#if _NOMPI
    memset(combined_bitmap, 0, combined_size);
    memcpy(combined_bitmap, ds->var_bitmap, VAR_BITMAP_SIZE);
#else
    MPI_Gather(ds->var_bitmap, VAR_BITMAP_SIZE, MPI_BYTE, combined_bitmap, VAR_BITMAP_SIZE, MPI_BYTE, 0, ds->comm);
#endif

    if(ds->my_rank == 0) {
        // now determine which var is missing on other processes 
        for(i = 0; i < ds->comm_size; i ++) {
            int j;
            for(j = 0; j < VAR_BITMAP_SIZE; j ++) {
                ds->var_bitmap[j] = ds->var_bitmap[j] | combined_bitmap[i*VAR_BITMAP_SIZE+j];
            }
        }
        free(combined_bitmap);
        // if the corresponding bit is set in ds->var_bitmap, then the var is missing on other processes
    }



    gp->attr_namelist = 0;

    gp->timestep = ds->f_info->timestep;
    gp->lasttimestep = ds->f_info->timestep;

    // now we need to wait here until we see all data are ready
    pthread_mutex_lock(&(ds->f_info->mutex));
    while(queue_size(ds->f_info->dt_queue) != ds->f_info->num_chunks) {
        pthread_cond_wait(&(ds->f_info->cond2), &(ds->f_info->mutex));
    }
    pthread_mutex_unlock(&(ds->f_info->mutex));

    return gp;
}

int adios_read_datatap_gclose (ADIOS_GROUP *gp)
{
//    datatap_read_file_data *ds = (datatap_read_file_data *) gp->fp->fh;
    adios_errno = 0;
    free_namelist ((gp->attr_namelist),gp->attrs_count);
    int i;
    for(i = 0; i < gp->vars_count; i ++) {
        free(gp->var_namelist[i]); 
    }
    free(gp->var_namelist);
//    free_namelist ((gp->var_namelist),gp->vars_count);
    free(gp);
    return 0;

}

int adios_read_datatap_get_attr (ADIOS_GROUP *gp, const char *attrname, 
                                 enum ADIOS_DATATYPES *type,
                                 int *size, void **data)
{
    // TODO: borrowed from dimes
    adios_error(err_invalid_read_method, "adios_read_datatap_get_attr is not implemented.");
    *size = 0;
    *type = adios_unknown;
    *data = 0;
    return adios_errno;
}

int adios_read_datatap_get_attr_byid (ADIOS_GROUP *gp, int attrid, 
                                      enum ADIOS_DATATYPES *type, 
                                      int *size, void **data)
{
    // TODO: borrowed from dimes
    adios_error(err_invalid_read_method, "adios_read_datatap_get_attr_byid is not implemented.");
    *size = 0;
    *type = adios_unknown;
    *data = 0;
    return adios_errno;
}

ADIOS_VARINFO * adios_read_datatap_inq_var (ADIOS_GROUP *gp, const char *varname) 
{
    // TODO: usually user will read those variables reperesenting dimensions directly
//    error(err_invalid_read_method, "adios_read_datatap_inq_var is not implemented.");

    // find the var among all pgs
    ADIOS_VARINFO *v = (ADIOS_VARINFO *) malloc(sizeof(ADIOS_VARINFO));
    if(!v) {
        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
        return NULL;
    }
    memset(v, 0, sizeof(ADIOS_VARINFO));

    datatap_read_file_data *ds = (datatap_read_file_data *) gp->fp->fh;
    
    int found = 0;
    datatap_var_info *current_var = ds->vars;
    while(current_var) {
        if(!compare_var_name(varname, current_var)) { 
            found = 1;
            break;
        }
        else {
            current_var = current_var->next;
        } 
    } 

    if(!found) {
        current_var = ds->vars_peer;
        while(current_var) {
            if(!compare_var_name(varname, current_var)) {
                found = 1;
                break;
            }
            else {
                current_var = current_var->next;
            }
        }
    }
  
    if(found) {
        v->grpid = gp->grpid;
        int i;
        for(i = 0; i < gp->vars_count; i ++) {
            if(!strcmp(gp->var_namelist[i], varname)) {
                v->varid = i; // TODO: this may not be cmpatible with BP
                break;
            }
        }
        v->type = current_var->type;
        v->ndim = current_var->ndims;
        v->timedim = current_var->time_dim;
        if(!v->ndim) { // scalar and string
            if(v->timedim != -1) { // scalar with time dimension
                v->ndim = 1;
                v->dims = (uint64_t *) malloc(sizeof(uint64_t));
                if(!v->dims) {
                    adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                    return NULL;
                }
                v->dims[0] = 1; // TODO: only one timestep in the file
            }
            //int value_size = common_read_type_size(v->type, current_var->chunks->data);
            int value_size = current_var->data_size;
            v->value = malloc(value_size);
            if(!v->value) {
                adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                return NULL;
            }
            memcpy(v->value, current_var->chunks->data, value_size);
        }
        else { // arrays
            v->dims = (uint64_t *) malloc(v->ndim * sizeof(uint64_t));
            if(!v->dims) {
                adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                return NULL;
            }
            int k;
            for(k = 0; k < v->ndim; k ++) {
                //v->dims[k] = ds->pgs[i].vars[j].global_bounds[k];
                v->dims[k] = current_var->chunks->global_bounds[k];
            }
        }
        return v;
    }
    else {
        adios_error(err_invalid_varname, "Cannot find var %s\n", varname);
        return NULL;
    }

    
#if 0
    int i, j;
    for(i = 0; i < ds->num_pgs; i ++) {
        for(j = 0; j < ds->pgs[i].num_vars; j ++) {
            // the parameter varname can be full path or just var name, so we
            // need to first find it by matching the name
            if(!compare_var_name(varname, &(ds->pgs[i].vars[j]))) {     
                v->grpid = gp->grpid;
                v->varid = j; // TODO: this may not be cmpatible with BP 
                v->type = ds->pgs[i].vars[j].type;
                v->ndim = ds->pgs[i].vars[j].ndims; 
                v->timedim = ds->pgs[i].vars[j].time_dim;
                if(!v->ndim) { // scalar
                    if(v->timedim != -1) { // scalar with time dimension
                        v->ndim = 1;
                        v->dims = (uint64_t *) malloc(sizeof(uint64_t));
                        if(!v->dims) {
                            adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                            return NULL;
                        }
                        v->dims[0] = 1; // TODO: only one timestep in the file
                    }
                    int value_size = common_read_type_size(v->type, ds->pgs[i].vars[j].data);
                    v->value = malloc(value_size); 
                    if(!v->value) {
                        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                        return NULL;
                    }
                    memcpy(v->value, ds->pgs[i].vars[j].data, value_size);
                }
                else { // arrays  
                    v->dims = (uint64_t *) malloc(v->ndim * sizeof(uint64_t));   
                    if(!v->dims) {
                        adios_error(err_no_memory, "Cannot allocate buffer in adios_read_datatap_inq_var()");
                        return NULL;
                    }
                    int k;
                    for(k = 0; k < v->ndim; k ++) {
                        v->dims[k] = ds->pgs[i].vars[j].global_bounds[k]; 
                    }
                }
                return v;
            }
        }
    }

    return NULL;    
#endif

}

ADIOS_VARINFO * adios_read_datatap_inq_var_byid (ADIOS_GROUP *gp, int varid)
{
    if(varid >= 0 && varid < gp->vars_count) {
        return adios_read_datatap_inq_var(gp, gp->var_namelist[varid]);
    }
    else {
        adios_error(err_invalid_varid, "Cannot find var %d\n", varid);
        return NULL;
    }
}

void adios_read_datatap_free_varinfo (ADIOS_VARINFO *vp)
{
    if(!vp) return;

    if(!vp->ndim) { // scalar
        if(vp->timedim != -1) { // scalar with time dimension
            free(vp->dims);
        }
        free(vp->value);
    }
    else { // arrays
        free(vp->dims);
    }
}

int64_t adios_read_datatap_read_var (ADIOS_GROUP *gp, const char *varname,
                                     const uint64_t *start, const uint64_t *count,
                                     void *data)
{
    int64_t total_size;
    datatap_read_file_data *ds = (datatap_read_file_data *) gp->fp->fh;
    int found = 0;

    datatap_var_info *current_var = ds->vars;
    while(current_var) {
        if(!compare_var_name(varname, current_var)) {
            // found it locally
            found = 1;
            if(!current_var->ndims) { // scalar
                // TODO: check time dimension if there is any
                if(current_var->time_dim != -1 &&
                    (gp->fp->tidx_start != start[0] || count[0] > 1)) {
                // TODO: check time dimension if there is any
                    adios_error(err_no_data_at_timestep, "Specified time step is not available.");
                    return -1;
                }

                total_size = current_var->data_size;
                memcpy(data, current_var->chunks->data, total_size);

                return total_size;
            }
            else { // arrays
                // TODO: check time dimension if there is any
                if(current_var->time_dim != -1) {
                    uint64_t ti = current_var->time_dim;
                    if(futils_is_called_from_fortran()) {
                        ti --;
                    }
                    // TODO: in Fortran index starts from 1 but in C index starts from 0
                    if(count[ti] > 1 || start[ti] != current_var->chunks->global_offsets[ti]) {
                        adios_error(err_no_data_at_timestep, "Specified time step is not available.");
                        return -1;
                    }
                }

                total_size = read_array(ds, current_var, start, count, data);

                // rank 0 should be responsible for send data out to peer readers
                if(ds->comm_size > 1) {
                    if(ds->my_rank == 0) {
                        // check if this array is missing on peer readers  
                        //if(current_var->num_chunks == 1) {
                        int byte_pos = current_var->id / 8;
                        int bit_pos = current_var->id % 8;
                        unsigned char mask = 0x01 << bit_pos;
                        //if(current_var->num_chunks == 1) {
                        if((ds->var_bitmap[byte_pos] & mask) != 0x00) {
                            int rc = MPI_Bcast(data, total_size, MPI_BYTE, 0, ds->comm); 
                            if(rc != MPI_SUCCESS) {
                                fprintf(stderr, "rank %d: MPI_Bcast() returns error (%d). %s:%d\n",
                                    ds->my_rank, rc, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    } 
                }
                return total_size;
            }
        }
        else {
            current_var = current_var->next;
        }
    }

    if(!found) {
        current_var = ds->vars_peer;
        while(current_var) {
            if(!compare_var_name(varname, current_var)) {
                // found it on remote peer reader
                if(!current_var->ndims) { // scalar
                    // TODO: check time dimension if there is any
                    if(current_var->time_dim != -1 &&
                        (gp->fp->tidx_start != start[0] || count[0] > 1)) {
                        adios_error(err_no_data_at_timestep, "Specified time step is not available.");
                        return -1;
                    }

                    total_size = current_var->data_size;
                    memcpy(data, current_var->chunks->data, total_size);
                    return total_size;
                }
                else { // arrays
                    // TODO: check time dimension if there is any
                    if(current_var->time_dim != -1) {
                        uint64_t ti = current_var->time_dim;
                        // TODO: in Fortran index starts from 1 but in C index starts from 0
                        if(futils_is_called_from_fortran()) {
                            ti --;
                        }

                        if(count[ti] > 1 || start[ti] != current_var->chunks->global_offsets[ti]) {
                            adios_error(err_no_data_at_timestep, "Specified time step is not available.");
                            return -1;
                        }
                    }

                    if(ds->my_rank != 0) {
                        int i = 0;
                        total_size = common_read_type_size(current_var->type, NULL); 
                        for(; i < current_var->ndims; i ++) {
                             total_size *= count[i];
                        }
                                     
                        if(ds->comm_size > 1) {
                            // check if this array is missing on peer readers
                            int rc = MPI_Bcast(data, total_size, MPI_BYTE, 0, ds->comm);
                            if(rc != MPI_SUCCESS) {
                                fprintf(stderr, "rank %d: MPI_Bcast() returns error (%d). %s:%d\n",
                                    ds->my_rank, rc, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }

                    return total_size;
                }
            }
            else {
                current_var = current_var->next;
            }
        }
    }

    adios_error(err_invalid_varname, "Cannot find var %s\n", varname);
    return -1;
}

int64_t adios_read_datatap_read_var_byid (ADIOS_GROUP *gp, int varid,
                                          const uint64_t *start,
                                          const uint64_t *count,
                                          void *data)
{
    if(varid >= 0 && varid < gp->vars_count) {
        return adios_read_datatap_read_var (gp, gp->var_namelist[varid], start, count, data);
    }
    else {
        adios_error(err_invalid_varid, "Cannot find var %d\n", varid);
        return -1;
    }

}

void adios_read_datatap_reset_dimension_order (ADIOS_FILE *fp, int is_fortran)
{
    // TODO
    adios_error(err_invalid_read_method, "adios_read_datatap_reset_dimension_order is not implemented.");
}

#else // dummy

int adios_read_datatap_init (MPI_Comm comm)
{
    return 0;
}

int adios_read_datatap_finalize ()
{
    return 0;
}

ADIOS_FILE *adios_read_datatap_fopen(const char *fname, MPI_Comm comm)
{
    return NULL;
}

int adios_read_datatap_fclose(ADIOS_FILE *fp)
{
    return 0;
}

ADIOS_GROUP * adios_read_datatap_gopen (ADIOS_FILE *fp, const char *grpname)
{
    return NULL;
}

ADIOS_GROUP * adios_read_datatap_gopen_byid (ADIOS_FILE *fp, int grpid)
{
    return NULL;
}

int adios_read_datatap_gclose (ADIOS_GROUP *gp)
{
    return 0;

}

int adios_read_datatap_get_attr (ADIOS_GROUP *gp, const char *attrname,
                                 enum ADIOS_DATATYPES *type,
                                 int *size, void **data)
{
    return 0;
}
int adios_read_datatap_get_attr_byid (ADIOS_GROUP *gp, int attrid,
                                      enum ADIOS_DATATYPES *type,
                                      int *size, void **data)
{
    adios_error(err_invalid_read_method, "adios_read_datatap_get_attr_byid is not implemented.");
    *size = 0;
    *type = adios_unknown;
    *data = 0;
    return adios_errno;
}

ADIOS_VARINFO * adios_read_datatap_inq_var (ADIOS_GROUP *gp, const char *varname)
{
    return NULL;
}

ADIOS_VARINFO * adios_read_datatap_inq_var_byid (ADIOS_GROUP *gp, int varid)
{
    return NULL;
}

void adios_read_datatap_free_varinfo (ADIOS_VARINFO *vp)
{
}

int64_t adios_read_datatap_read_var (ADIOS_GROUP *gp, const char *varname,
                                     const uint64_t *start, const uint64_t *count,
                                     void *data)
{
    return 0;
}

int64_t adios_read_datatap_read_var_byid (ADIOS_GROUP *gp, int varid,
                                          const uint64_t *start,
                                          const uint64_t *count,
                                          void *data)
{
    return 0;
}

void adios_read_datatap_reset_dimension_order (ADIOS_FILE *fp, int is_fortran)
{

}

#endif 



#endif