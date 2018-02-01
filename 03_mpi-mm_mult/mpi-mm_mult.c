
/* Program translated in C and modified from "Using MPI, 3rd ed." which was
 * initially writed in Fortran                                      */

/* > new routine
 * int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
 * int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
 * > routine seen before
 * int MPI_Init(int * argc, char *** argv)
 * int MPI_Comm_size(MPI Comm comm, int * size)
 * int MPI_Comm_rank(MPI Comm comm, int * rank)
 * int MPI_Bcast(void * buf, int count, MPI Datatype datatype, int root, MPI Comm comm)
 * int MPI_Reduce(const void * sendbuf, void * recvbuf, int count, MPI Datatype datatype, MPI Op op, int root, MPI Comm comm)
 * int MPI_Finalize()
 */

#include <mpi.h>
#include <mpe.h>
#include <stdio.h>
#include <assert.h>

#define MAX_AROWS 20
#define MAX_ACOLS 1000
#define MAX_BCOLS 20

#define MPI_ASSERT(fx) do { \
  assert(fx == MPI_SUCCESS); \
} while(0)

void print_matrix(int n, int m, double mat[MAX_AROWS][MAX_BCOLS]) {
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++) {
            printf("[%02d,%02d:%.0lf] ", i + 1, j + 1, mat[i][j]);
        }
        puts("");
    }
}

int main(int argc, char* argv[]) {
    int arows, acols, brows, bcols, crows, ccols;
    double a[MAX_AROWS][MAX_ACOLS], b[MAX_ACOLS][MAX_BCOLS];
    double c[MAX_AROWS][MAX_BCOLS], buffer[MAX_ACOLS], ans[MAX_BCOLS];
    double starttime, endtime;
    int myid, master, numprocs;
    int i, j, numsent, sender;
    int anstype, row;
    MPI_Status status;
    MPI_ASSERT(MPI_Init(&argc, &argv));
    MPI_ASSERT(MPI_Comm_rank(MPI_COMM_WORLD, &myid));
    MPI_ASSERT(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
    /* master process rank is 0 */
    arows = 10;
    acols = 20;
    brows = 20;
    bcols = 10;
    crows = arows;
    ccols = bcols;
    master = 0;
    MPE_Init_log();
    if (myid == master) {
        MPE_Describe_state(1, 2, "Bcast", "red:vlines3");
        MPE_Describe_state(3, 4, "Compute", "blue:gray3");
        MPE_Describe_state(5, 6, "Send", "green:light_gray");
        MPE_Describe_state(7, 8, "Recv", "yellow:gray");
        /* Create arbitrary matrices */
        for(j = 0; j < acols; j++) {
            for (i = 0; i < arows; i++) {
                a[i][j] = 1;
            }
        }
        for(j = 0; j < bcols; j++) {
            for (i = 0; i < acols; i++) {
                b[i][j] = 1;
            }
        }
        numsent = 0;
        /* Manager process broadcast vector to all other processes */
        for (i = 0; i < brows; i++) {
            MPI_ASSERT(MPI_Bcast(b[i], bcols, MPI_DOUBLE, master, MPI_COMM_WORLD));
        }
        for(i = 1; i < numprocs; i++) {
            for (j = 0; j < acols; j++) {
                buffer[j] = a[i][j];
            }
            MPE_Log_event(5, i, "send");
            MPI_ASSERT(MPI_Send(&buffer, acols, MPI_DOUBLE, i, i, MPI_COMM_WORLD));
            MPE_Log_event(6, i, "sent");
            numsent++;
        }
        for (i = 0; i < crows; i++) {
            MPE_Log_event(7, i, "recv");
            /* Performs a standard-mode blocking receive.
             * Block-receives the result of dot product from other processes.
             *       @buf  nb    type        source          tag          communicator  status  */
            MPI_ASSERT(MPI_Recv(&ans, ccols, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
            /* Get the sender rank */
            sender = status.MPI_SOURCE;
            /* Get the matrix row number */
            anstype = status.MPI_TAG;
            MPE_Log_event(8, anstype, "recvd");
            /* Place dot product result in new vector */
            for (j = 0; j < ccols; j++) {
                c[anstype - 1][j] = ans[j];
            }
            if (numsent < arows) {
                /* pack data into buffer. See above. */
                for (j = 0; j < acols; j++) {
                    buffer[j] = a[numsent][j];
                }
                MPE_Log_event(5, i, "send");
                /* Send a new matrix row to the sender process. */
                MPI_ASSERT(MPI_Send(buffer, acols, MPI_DOUBLE, sender, numsent + 1, MPI_COMM_WORLD));
                MPE_Log_event(6, i, "sent");
                numsent++;
            } else {
                MPE_Log_event(5, 0, "send");
                /* Send a termination message to all other processes.
                 * MPI_BOTTOM is used to indicate the bottom of the address space */
                MPI_ASSERT(MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD));
                MPE_Log_event(6, 0, "sent");
            }
        }
        print_matrix(crows, ccols, c);
    } else {
        /* Broadcast call by worker processes.
         * Note that MPI_Bcast is called by all members of group
         * using the same arguments for comm, root.           */
        MPE_Log_event(1, 0, "bstart");
        for (i = 0; i < brows; i++) {
            MPI_ASSERT(MPI_Bcast(b[i], bcols, MPI_DOUBLE, master, MPI_COMM_WORLD));
        }
        MPE_Log_event(2, 0, "bend");
        MPE_Log_event(7, i, "recv");
        while(1) {
            /* Block-receives a matrix row from master process.
            *          @buf    nb     type      source       tag       communicator   status  */
            MPI_ASSERT(MPI_Recv(buffer, acols, MPI_DOUBLE, master, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
            /* Receive termination message */
            if (status.MPI_TAG == 0) break;
            row = status.MPI_TAG;
            MPE_Log_event(8, row, "recvd");
            MPE_Log_event(3, row, "compute");
            /* dot product : <A_i, b> */
            for (i = 0; i < bcols; i++) {
                ans[i] = 0.0;
                for(j = 0; j < acols; j++) {
                    ans[i] += buffer[j] * b[j][i];
                }
            }
            MPE_Log_event(4, row, "computed");
            MPE_Log_event(5, row, "send");
            /* Send dot product result to master process */
            MPI_ASSERT(MPI_Send(&ans, bcols, MPI_DOUBLE, master, row, MPI_COMM_WORLD));
            MPE_Log_event(6, row, "sent");
        }
    }
    MPE_Finish_log("pmatmat.log");
    MPI_ASSERT(MPI_Finalize());
    return 0;
}
