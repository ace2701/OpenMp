#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000

void initialize_matrix(double A[N][N], double B[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = 1.0;
            B[i][j] = 2.0;
        }
    }
}

void matrix_multiply_serial(
    double A[N][N],
    double B[N][N],
    double C[N][N]
) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0.0;

            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void matrix_multiply_openmp(
    double A[N][N],
    double B[N][N],
    double C[N][N],
    int thread_count
) {
    #pragma omp parallel for num_threads(thread_count)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0.0;

            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

double calculate_checksum(double C[N][N]) {
    double sum = 0.0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum += C[i][j];
        }
    }

    return sum;
}

int main(void) {

    static double A[N][N];
    static double B[N][N];
    static double C[N][N];

    int thread_counts[] = {1, 2, 4, 8};
    int num_tests = 4;

    initialize_matrix(A, B);

    printf("============================================\n");
    printf(" OPENMP MATRIX MULTIPLICATION STUDY CASE\n");
    printf("============================================\n");
    printf("Matrix Size : %d x %d\n", N, N);
    printf("Tests       : 1, 2, 4, 8 threads\n\n");

    /* SERIAL BASELINE */
    double start_serial = omp_get_wtime();

    matrix_multiply_serial(A, B, C);

    double end_serial = omp_get_wtime();
    double serial_time = end_serial - start_serial;
    double serial_checksum = calculate_checksum(C);

    printf("Serial Time : %.6f seconds\n", serial_time);
    printf("Checksum    : %.2f\n\n", serial_checksum);

    printf("--------------------------------------------\n");
    printf("Threads | Parallel Time | Speedup | Efficiency\n");
    printf("--------------------------------------------\n");

    for (int t = 0; t < num_tests; t++) {

        int threads = thread_counts[t];

        double start_parallel = omp_get_wtime();

        matrix_multiply_openmp(A, B, C, threads);

        double end_parallel = omp_get_wtime();

        double parallel_time = end_parallel - start_parallel;
        double speedup = serial_time / parallel_time;
        double efficiency = (speedup / threads) * 100.0;

        double checksum = calculate_checksum(C);

        printf(
            "%7d | %14.6f | %7.2fx | %9.2f%%\n",
            threads,
            parallel_time,
            speedup,
            efficiency
        );

        printf("Checksum (%d threads): %.2f\n", threads, checksum);
    }

    printf("--------------------------------------------\n");
    printf("OpenMP study completed successfully.\n");

    return 0;
}