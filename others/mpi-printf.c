
#include <mpi.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZ 2048

int main(int argc, char *argv[]) {
    int numprocs, myid;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Status status;

    if (myid == 0) {
            int buf_len[numprocs];
            char msg[numprocs][BUF_SIZ];

        for (int i = 1; i < numprocs; i++) {
            MPI_Recv(&buf_len[i], 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(msg[i], BUF_SIZ, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("%s\n", msg[i]);
        }

    } else {
        char buffer[BUF_SIZ];
        sprintf(buffer, "rank %d :: %s", myid, "format");
        int buf_len = strlen(buffer);
        MPI_Send(&buf_len, 1, MPI_INT, 0, myid, MPI_COMM_WORLD);
        MPI_Send(buffer, BUF_SIZ, MPI_CHAR, 0, myid, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
