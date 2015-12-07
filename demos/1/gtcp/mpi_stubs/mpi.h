/* -----------------------------------------------------------------------
   LAMMPS 2003 (July 31) - Molecular Dynamics Simulator
   Sandia National Laboratories, www.cs.sandia.gov/~sjplimp/lammps.html
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------ */

#ifndef MPI_STUBS
#define MPI_STUBS

// include this here so that you get a superset of both the ADIOS mpidummy
// and the LAMMPS mpi stub. That way it can work for both.
#include <mpidummy.h>
#include "stdlib.h"

/* use C bindings for MPI interface */

#ifdef __cplusplus
extern "C" {
#endif

/* Dummy defs for MPI stubs */
// need to add these for ADIOS to build correctly
#define MPI_MODE_WRONLY 0
#define MPI_MODE_CREATE 0
#define MPI_MODE_RDWR 0
#define MPI_UNSIGNED_LONG_LONG 8
// need to add this for the gtc analytics
#define MPI_LONG 9

/* #define MPI_COMM_WORLD 0 */

/* #define MPI_SUCCESS 0 */

/* #define MPI_INT 1 */
#define MPI_FLOAT 2
#define MPI_DOUBLE 3
/* #define MPI_CHAR 4 */
/* #define MPI_BYTE 5 */
#define MPI_LONG_LONG 6
#define MPI_DOUBLE_INT 7

/* #define MPI_SUM 1 */
#define MPI_MAX 2
#define MPI_MIN 3
#define MPI_MAXLOC 4
#define MPI_MINLOC 5
/* #define MPI_LOR 6 */

/* #define MPI_ANY_SOURCE -1 */

/* #define MPI_Comm int */
#define MPI_Request int
/* #define MPI_Datatype int */
#define MPI_Op int

#define MPI_IN_PLACE NULL

/* #define MPI_MAX_PROCESSOR_NAME 128 */

/* /\* MPI data structs *\/ */

// ==========    
// COPY THIS INTO ADIOS mpidummy.h, replacing ADIOS' existing MPI_Status definition.   
/* struct _MPI_Status { */
/*   int MPI_SOURCE; */
/* }; */
/* typedef struct _MPI_Status MPI_Status; */
//==========
    
/* /\* Function prototypes for MPI stubs *\/ */

/* int MPI_Init(int *argc, char ***argv); */
/* int MPI_Initialized(int *flag); */
/*   //void MPI_Get_processor_name(char *name, int *resultlen); */

/* int MPI_Comm_rank(MPI_Comm comm, int *me); */
/* int MPI_Comm_size(MPI_Comm comm, int *nprocs); */
int MPI_Abort(MPI_Comm comm, int errorcode);
/* int MPI_Finalize(); */
/* double MPI_Wtime(); */

int MPI_Type_size(int, int *);

int MPI_Send(void *buf, int count, MPI_Datatype datatype,
             int dest, int tag, MPI_Comm comm);
//need to add this for gtcp
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, 
              int source, int tag, MPI_Comm comm, MPI_Request *request); 
int MPI_Rsend(void *buf, int count, MPI_Datatype datatype,
              int dest, int tag, MPI_Comm comm);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
             int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
              int source, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Wait(MPI_Request *request, MPI_Status *status);
int MPI_Waitall(int n, MPI_Request *request, MPI_Status *status);
int MPI_Waitany(int count, MPI_Request *request, int *index,
                MPI_Status *status);
int MPI_Sendrecv(void *sbuf, int scount, MPI_Datatype sdatatype,
                  int dest, int stag, void *rbuf, int rcount,
                  MPI_Datatype rdatatype, int source, int rtag,
                  MPI_Comm comm, MPI_Status *status);
/* int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count); */

/* int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *comm_out); */
/* int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *comm_out); */
/* int MPI_Comm_free(MPI_Comm *comm); */

int MPI_Cart_create(MPI_Comm comm_old, int ndims, int *dims, int *periods,
                    int reorder, MPI_Comm *comm_cart);
int MPI_Cart_get(MPI_Comm comm, int maxdims, int *dims, int *periods,
                 int *coords);
int MPI_Cart_shift(MPI_Comm comm, int direction, int displ,
                   int *source, int *dest);
int MPI_Cart_rank(MPI_Comm comm, int *coords, int *rank);

int MPI_Barrier(MPI_Comm comm);
/* int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, */
/*               int root, MPI_Comm comm); */
int MPI_Allreduce(void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Reduce(void *sendbuf, void *recvbuf, int count,
                   MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Scan(void *sendbuf, void *recvbuf, int count,
             MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
/* int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, */
/*                   void *recvbuf, int recvcount, MPI_Datatype recvtype, */
/*                   MPI_Comm comm); */
/* int MPI_Allgatherv(void *sendbuf, int sendcount, MPI_Datatype sendtype, */
/*                    void *recvbuf, int *recvcounts, int *displs, */
/*                    MPI_Datatype recvtype, MPI_Comm comm); */
int MPI_Reduce_scatter(void *sendbuf, void *recvbuf, int *recvcounts,
                       MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
/* int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype, */
/*                void *recvbuf, int recvcount, MPI_Datatype recvtype, */
/*                int root, MPI_Comm comm); */
/* int MPI_Gatherv(void *sendbuf, int sendcount, MPI_Datatype sendtype, */
/*                 void *recvbuf, int *recvcounts, int *displs, */
/*                 MPI_Datatype recvtype, int root, MPI_Comm comm); */
/* int MPI_Scatterv(void *sendbuf, int *sendcounts, int *displs, */
/*                  MPI_Datatype sendtype, void *recvbuf, int recvcount, */
/*                  MPI_Datatype recvtype, int root, MPI_Comm comm); */

int MPI_Error_class (int errorcode, int *errorclass);

#ifdef __cplusplus
}
#endif

#endif
