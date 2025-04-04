#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define n (1 << 22)  // 2^22

int Tree_sum(int local_sum, int rank, int size){
    int step=1;
    // Even case
    if (size % 2 ==0){
        while (step<size){
            if(rank % (2*step)==0){
                if(rank+step < size){
                    int received;
                    MPI_Recv(&received, 1, MPI_INT, rank+step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    local_sum+=received;+
                }
            }else{
                MPI_Send(&local_sum, 1, MPI_INT, rank-step, 0, MPI_COMM_WORLD );
                break;
            }
            step*=2;
        }
    // Odd case, we need to add the local sum of left processor to global sum
    }else{
        // last process
        if(rank==size-1){
            // Sending data to the previous one 
            MPI_Send(&local_sum, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
        // now the pervious processor is receiver, so we update it's local sum
        }else if (rank==size-2){
            int received;
            MPI_Recv(&received, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_sum+=received;
        }

        MPI_Barrier(MPI_COMM_WORLD);

        while (step<size-1){
            if(rank % (2*step)==0){
                if(rank+step<size-1){
                    int received;
                    MPI_Recv(&received, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    local_sum+=received;
                }
            }else{
                MPI_Send(&local_sum, 1, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
                break;

            }
            step*=2;
        }
    }
    return local_sum;

}     

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 

    int local_n=n/size;//divde work

    // Each process starts with a random value based on rank, reference: https://cplusplus.com/reference/cstdlib/srand/ 
    srand(rank + 1);
    int local_sum=0;
    for (int i=0; i<local_n; i++){
        local_sum+=rand()%100;
    }
    
    double start_time, end_time;
    start_time = MPI_Wtime();
    int global_sum =Tree_sum(local_sum, rank, size);
    end_time = MPI_Wtime();

    // Rank 0 holds the final result
    if (rank == 0) {
        printf("Final Sum: %d\n", global_sum);
        printf("Execution Time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}