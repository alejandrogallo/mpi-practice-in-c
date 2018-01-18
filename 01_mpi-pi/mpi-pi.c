
/* Example from "Using MPI, 3rd ed." */

/*
 * int MPI_Init(int * argc, char *** argv)
 * int MPI_Comm_size(MPI Comm comm, int * size)
 * int MPI_Comm_rank(MPI Comm comm, int * rank)
 * int MPI_Bcast(void * buf, int count, MPI Datatype datatype, int root, MPI Comm comm)
 * int MPI_Reduce(const void * sendbuf, void * recvbuf, int count, MPI Datatype datatype, MPI Op op, int root, MPI Comm comm)
 * int MPI_Finalize()
 */


#include <mpi.h>
#include <math.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int n, myid, numprocs, i;
    double PI25DT = 3.141592653589793238462643;
    double mypi, pi, h, sum, x;
    /* Initialization
     * Here we didn't check the return value (error).
     * */
    MPI_Init(&argc, &argv);
    /* Pass the total amount of processes to numprocs
     *            communicator     @var            */
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    /* Pass the current process rank to myid
     * root process rank is 0 here.
     * >          communicator    @var    */
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    while (1) {
        if (myid == 0) {
            printf("Enter the number of intervals: (0 quits) ");
            /* flush the standard output
             * "The stdout stream is buffered, so will only
             * display what's in the buffer after it reaches a newline"
             * --- https://stackoverflow.com/a/1716621
             * */
            fflush(stdout);
            scanf("%d", &n);
        }
        /* Returns an elapsed time on the calling processor. */
        double starttime = MPI_Wtime();
        /*
         * Broadcast the variable n of type MPI_INT to
         * all processes (in MPI_COMM_WORLD) other than
         * the root process.
         * All processes except the root wait for the next
         * value of n.
         * This is a collective communication routine.
         * >     @var nb   type root communicator    */
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (n == 0)
            break;
        /* Computing part */
        else {
            h = 1.0 / (double) n;
            sum = 0.0;
            /* Process i sum up the ith, i+numprocs'th, etc. rectangle.
             * */
            for (i = myid + 1; i <= n; i += numprocs) {
                x = h * ((double)i - 0.5);
                sum += (4.0 / (1.0 + x * x));
            }
            mypi = h * sum;
            /* Reduces values on all processes within a group.
             * Here we add up all mypi value into the var pi of the root process.
             * >       @src  @dest nb    type        op  proc  communicator    */
            MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            double endtime = MPI_Wtime();
            if (myid == 0){
                /* The root process print the result. */
                printf("pi is approximately %.16f, Error is %.16f.\n", pi, fabs(pi - PI25DT));
                /* MPI_Wtick returns the resolution of MPI_Wtime. */
                printf("time is %.9lf seconds, time resolution is %.3e seconds.\n", endtime - starttime, MPI_Wtick());
            }
        }
    }
    /* Terminate the MPI env */
    MPI_Finalize();
    return 0;
}
