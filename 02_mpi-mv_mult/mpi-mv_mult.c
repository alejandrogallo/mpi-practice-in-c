
/* Program translated and modified from "Using MPI, 3rd ed." which was
 * initially writed in Fortran                                      */

/* > new routine
 * int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
 * int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
 * > routine seen before
 * int MPI_Init(int * argc, char *** argv)
 * int MPI_Comm_size(MPI_Comm comm, int * size)
 * int MPI_Comm_rank(MPI_Comm comm, int * rank)
 * int MPI_Bcast(void * buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
 * int MPI_Reduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI Op op, int root, MPI_Comm comm)
 * int MPI_Finalize()
 */

#include <mpi.h>
#include <stdio.h>

#define MAX_ROWS 200
#define MAX_COLS 200

int min(int a, int b) {
    return a < b ? a : b;
}

void print_vector(int n, double v[]) {
    for(int i = 0; i < n; i++)
        printf("[%d:%.1lf] ", i + 1, v[i]);
    puts("");
}

int main(int argc, char* argv[]) {
    int rows, cols;
    double a[MAX_ROWS][MAX_COLS], b[MAX_COLS];
    double c[MAX_ROWS], buffer[MAX_COLS], ans;
    int myid, master, numprocs;
    int i, j, numsent, sender;
    int anstype, row;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    /* master process rank is 0 */
    master = 0;
    rows = 10;
    cols = 10;
    if (myid == master) {
        /* Create arbitrary matrix and vector */
        for(j = 0; j < cols; j++) {
            b[j] = 1;
            for (i = 0; i < rows; i++) {
                a[i][j] = i;
            }
        }
        numsent = 0;
        /* Manager process broadcast vector to all other processes */
        MPI_Bcast(b, cols, MPI_DOUBLE, master, MPI_COMM_WORLD);
        for (i = 1; i < min(numprocs, rows); i++) {
            /* we pack the data into a contiguous buffer,
             * we will show later that MPI can do this for us.
             * */
            for (j = 0; j < cols; j++) {
                buffer[j] = a[i - 1][j];
            }
            /* Performs a standard-mode blocking send.
             * Send a matrix row to each process.
             * >     @buf     nb      type    dest tag   communicator */
            MPI_Send(buffer, cols, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
            numsent++;
        }
        for (i = 0; i < rows; i++) {
            /* Performs a standard-mode blocking receive.
             * Block-receives the result of dot product from other processes.
             *       @buf  nb    type        source          tag          communicator  status  */
            MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            /* Get the sender rank */
            sender = status.MPI_SOURCE;
            /* Get the matrix row number */
            anstype = status.MPI_TAG;
            /* Place dot product result in new vector */
            c[anstype - 1] = ans;
            if (numsent < rows) {
                /* pack data into buffer. See above. */
                for (j = 0; j < cols; j++) {
                    buffer[j] = a[numsent][j];
                }
                /* Send a new matrix row to the sender process. */
                MPI_Send(buffer, cols, MPI_DOUBLE, sender, numsent + 1, MPI_COMM_WORLD);
                numsent++;
            } else {
                /* Send a termination message to all other processes.
                 * MPI_BOTTOM is used to indicate the bottom of the address space */
                MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
            }
        }
        print_vector(cols, c);
    } else {
        /* Broadcast call by worker processes.
         * Note that MPI_Bcast is called by all members of group
         * using the same arguments for comm, root.           */
        MPI_Bcast(b, cols, MPI_DOUBLE, master, MPI_COMM_WORLD);
        if (myid <= rows) {
            while(1) {
                /* Block-receives a matrix row from master process.
                *          @buf    nb     type      source       tag       communicator   status  */
                MPI_Recv(buffer, cols, MPI_DOUBLE, master, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                /* Receive termination message */
                if (status.MPI_TAG == 0) break;
                row = status.MPI_TAG;
                ans = 0.0;
                /* dot product : <A_i, b> */
                for (i = 0; i < cols; i++) {
                    ans += buffer[i] * b[i];
                }
                /* Send dot product result to master process */
                MPI_Send(&ans, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
            }
        }
    }
    MPI_Finalize();
    return 0;
}
