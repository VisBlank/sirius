#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "adios_read.h"

int main (int argc, char ** argv)
{
    MPI_Comm comm = MPI_COMM_WORLD;
    ADIOS_FILE * fp;
    ADIOS_VARINFO * vi;
    ADIOS_SELECTION * sel;
    uint64_t start[3], count[3]; 
    int rank, size;
    double * data;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &size);

    adios_read_init_method (ADIOS_READ_METHOD_BP, comm, "verbose=1");

    fp = adios_read_open_file ("MOMENTS_00000.bp",
                               ADIOS_READ_METHOD_BP,
                               comm);

    if (fp == NULL)
    {
        fprintf (stderr, "adios_read_open_file failed.\n");
        return -1;
    }

    vi = adios_inq_var (fp, "moments");

    //printf ("ndim = %d, dims[0] = %lu, dims[1] = %lu, dims[2] = %lu", vi->ndim, vi->dims[0], vi->dims[1], vi->dims[2]);
    if (vi->dims[0] != size)
    {
        fprintf (stderr, "Needs %d\n MPI processors\n", vi->ndim);
        return -1;
    }

    start[0] = rank;
    start[1] = 0;
    start[2] = 0;
    count[0] = 1;
    count[1] = vi->dims[1];
    count[2] = vi->dims[2];

    data = (double *) malloc (count[0] * count[1] * count[2] * 8);
    sel = adios_selection_boundingbox (vi->ndim, start, count);
    adios_schedule_read (fp,
                         sel,
                         "moments",
                         0,
                         1,
                         data);

    adios_perform_reads (fp, 1);

    int i;
    if (rank == 1)
    {
        for (i = 0; i < 10; i++)
            printf ("rank = %d, data = %e\n", rank, *(data + i));
    }
    adios_free_varinfo (vi);
    adios_read_close (fp);

    adios_read_finalize_method (ADIOS_READ_METHOD_BP);

    free (data);
    MPI_Finalize ();

    return 0;
}
