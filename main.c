#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include<omp.h>

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
    int process_Rank, size_cluster;
    int num_threads = 1, min = 10000, max = 99999, sum_thresh = 30;

    int padding = (ARRAY_SIZE % size_cluster) ? size_cluster - (ARRAY_SIZE % size_cluster) : 0;
    int total_size = ARRAY_SIZE + padding;
    int* data = NULL;

    data = (int*)malloc(total_size * sizeof(int));
    generate_data(data, ARRAY_SIZE, min, max);

    return 0;
}