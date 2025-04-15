#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include<omp.h>
#include <mpi.h>

#define ARRAY_SIZE 1000000000

int sum_of_digits(int n) {
    int sum = 0;
    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }
    return sum;
}

void generate_data(int* array, int size, int min_value, int max_value) {
    for (int i = 0; i < size; ++i) {
        array[i] = (rand() % (max_value - min_value + 1)) + min_value;
    }
}

void filter_and_sum(int* data, int size, int min_val, int max_val, int sum_thresh, long long* partial_sum, int* count) {
    #pragma omp parallel for reduction(+:*partial_sum, *count) schedule(static)
    for (int i = 0; i < size; ++i) {
        if (data[i] >= min_val && data[i] <= max_val && sum_of_digits(data[i]) > sum_thresh) {
            *partial_sum += data[i];
            (*count)++;
        }
    }
}

int main(int argc, char** argv) {
    int process_rank, size_cluster;
    int total_count = 0, local_count = 0;
    int num_threads = 1, min = 10000, max = 99999, sum_thresh = 30;
    long long int total_sum = 0, partial_sum = 0;

    int padding = (ARRAY_SIZE % size_cluster) ? size_cluster - (ARRAY_SIZE % size_cluster) : 0;
    int total_size = ARRAY_SIZE + padding;
    int* data = NULL;
    int scattered_size = total_size / size_cluster;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size_cluster);

    parse_args(argc, argv, &num_threads, &min, &max, &sum_thresh);
    omp_set_num_threads(num_threads);

    int* data = NULL;
    if (process_rank == 0) {
        data = (int*)malloc(total_size * sizeof(int));
        generate_data(data, ARRAY_SIZE, min, max);
        for (int i = ARRAY_SIZE; i < total_size; ++i) data[i] = 0;
    }

    int* scattered = (int*)malloc(scattered_size * sizeof(int));
    MPI_Scatter(data, scattered_size, MPI_INT, scattered, scattered_size, MPI_INT, 0, MPI_COMM_WORLD);

    filter_and_sum(scattered, scattered_size, min, max, sum_thresh, &partial_sum, &local_count);

    MPI_Reduce(&partial_sum, &total_sum, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    free(scattered);
    MPI_Finalize();

    return 0;
}