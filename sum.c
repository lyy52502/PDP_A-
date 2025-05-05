#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>



// Here I divide the data into different k blocks(the size of block is n/p),and consider two situations: with and without remainder
void allocate_arr(int n, int k, int size, int* first_k, int* last_k) {
    int q = n / size;
    int remainder = n % size;

    if (remainder == 0) {
        *first_k = k * q;
        *last_k = (k + 1) * q - 1;
    } else {
        // if k< reaminder, we know that the extra r elements should be dispersed to the front r blocks, 
        // each block has 1 more element, so the The first r blocks has q+1 elements 
        if (k < remainder) {
            *first_k = k * (q + 1);
            *last_k = (k + 1) * (q + 1) - 1;
        } else {
            // if k>remainder, 
            *first_k = remainder * (q + 1) + (k - remainder) * q;
            *last_k = *first_k + q - 1;
        }
    }
}

int Tree_sum(int local_sum, int rank, int size){
    if (size == 1) {
        return local_sum;  
    }
    int step=1;
    // Even case
    if (size % 2 ==0){
        while (step<size){
            if(rank % (2*step)==0){
                if(rank+step < size){
                    int received;
                    MPI_Recv(&received, 1, MPI_INT, rank+step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    local_sum+=received;
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
    if (argc != 3) {
        printf("Usage: %s <N> <scale>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]); 
    int scale=atoi(argv[2]);
    double start_time, end_time;
    
    
    MPI_Init(&argc, &argv);
    start_time = MPI_Wtime();
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    int first_k, last_k;
    // Here 1 for strong scalability, 0 for weak scalability
    if (scale==1){
        allocate_arr(n, rank, size, &first_k, &last_k);
    }else{
        allocate_arr(n*size, rank, size, &first_k, &last_k);
    }
    // Each process starts with a random value based on rank, reference: https://cplusplus.com/reference/cstdlib/srand/ 
    srand(rank + 1);
    int local_sum=0;
    for (int i=first_k; i<=last_k; i++){
        local_sum+=rand()%100;
    }
 
    
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
