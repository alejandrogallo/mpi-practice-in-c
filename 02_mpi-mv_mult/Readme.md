# MPI Matrix-Vector Multiplication

Manager-worker algorithm : Matrix-Vector Multiplication

# MPI Routine

* new routine
    * `int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)`
    * `int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)`
* routine seen before
    * `int MPI_Init(int * argc, char *** argv)`
    * `int MPI_Comm_size(MPI Comm comm, int * size)`
    * `int MPI_Comm_rank(MPI Comm comm, int * rank)`
    * `int MPI_Bcast(void * buf, int count, MPI Datatype datatype, int root, MPI Comm comm)`
    * `int MPI_Finalize()`


# Compilation & Execution

Compilation :
```
mpicc mpi-mv_prod.c -o mpi-mv_prod
```
Execution :
```
mpirun -np 2 mpi-mv_prod
```

# Reference 

* Using MPI, 3rd ed. `p.32-38`
