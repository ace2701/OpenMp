// Compilation: g++ -pedantic -pipe -O3 -march=native gol_serial.cpp -o gol_serial
// Run: ./gol_serial.exe [1] [2] [3], where [1] is the grid size, [2] is the number of generations, [3] is the delay per generation in ms.
// E.g.: ./gol_serial.exe 20 50 300 launches a 20 x 20 grid for 50 generations with 300 ms delay between generations

#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>

using std::chrono::duration;
using std::chrono::high_resolution_clock;

enum Reason { EMPTY, SURVIVE, LONELY, CROWDED, BORN };

int countAlive(char *m, int N, int i, int j)
{
    int alive = 0;

    for (int di = -1; di <= 1; di++)
        for (int dj = -1; dj <= 1; dj++)
            if ((di || dj) && m[((i + di + N) % N) * N + (j + dj + N) % N] == 'X')
                alive++;

    return alive;
}

// Game of life algorithm
char analyzeCell(char *m, int N, int i, int j, char *reason, char *neighbours)
{
    int alive = countAlive(m, N, i, j);
    bool isAlive = m[i * N + j] == 'X';

    *neighbours = alive;

    if (isAlive && alive < 2)
    {
        *reason = LONELY;
        return '.';
    }
    if (isAlive && alive > 3)
    {
        *reason = CROWDED;
        return '.';
    }
    if (isAlive)
    {
        *reason = SURVIVE;
        return 'X';
    }
    if (alive == 3)
    {
        *reason = BORN;
        return 'X';
    }

    *reason = EMPTY;
    return '.';
}

void initMatrix(char *m, int N)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            m[i * N + j] = rand() % 4 == 0 ? 'X' : '.';
}

void printGeneration(char *reasons, char *neighbours, int N, int gen, int total,
                     int survive, int born, int lonely, int crowded, double ms)
{
    printf("\033[H\033[2J");
    printf("Generasi %d / %d   (grid %d x %d, serial, %.3f ms)\n\n", gen, total, N, N, ms);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            switch (reasons[i * N + j])
            {
            case SURVIVE: printf("\033[32m██\033[0m"); break;
            case BORN:    printf("\033[36m██\033[0m"); break;
            case LONELY:  printf("\033[31m▒▒\033[0m"); break;
            case CROWDED: printf("\033[33m▒▒\033[0m"); break;
            default:      printf("\033[90m· \033[0m"); break;
            }
        }
        printf("\n");
    }

    printf("\n\033[32m██\033[0m bertahan: %d   ", survive);
    printf("\033[36m██\033[0m berkembang biak: %d   ", born);
    printf("\033[31m▒▒\033[0m mati kesepian: %d   ", lonely);
    printf("\033[33m▒▒\033[0m mati kepadatan: %d\n\n", crowded);

    const char *text[] = {"", "BERTAHAN, tetangga hidup %d (2 atau 3)",
                          "MATI karena kesepian, tetangga hidup %d (< 2)",
                          "MATI karena kepadatan, tetangga hidup %d (> 3)",
                          "HIDUP karena berkembang biak, tetangga hidup %d (tepat 3)"};
    bool shown[5] = {true, false, false, false, false};

    for (int k = 0; k < N * N; k++)
    {
        int r = reasons[k];
        if (shown[r])
            continue;
        shown[r] = true;
        printf("Sel (%d,%d): ", k / N, k % N);
        printf(text[r], neighbours[k]);
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    int N = argc > 1 ? atoi(argv[1]) : 20;
    int generations = argc > 2 ? atoi(argv[2]) : 50;
    int delay = argc > 3 ? atoi(argv[3]) : 300;

    char *c_m = (char *)malloc(N * N);
    char *n_m = (char *)malloc(N * N);
    char *reasons = (char *)malloc(N * N);
    char *neighbours = (char *)malloc(N * N);

    srand(1);
    initMatrix(c_m, N);

    for (int gen = 1; gen <= generations; gen++)
    {
        int survive = 0, born = 0, lonely = 0, crowded = 0;

        auto start = high_resolution_clock::now();

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                n_m[i * N + j] = analyzeCell(c_m, N, i, j, &reasons[i * N + j], &neighbours[i * N + j]);

                switch (reasons[i * N + j])
                {
                case SURVIVE: survive++; break;
                case BORN:    born++; break;
                case LONELY:  lonely++; break;
                case CROWDED: crowded++; break;
                }
            }

        auto stop = high_resolution_clock::now();
        duration<double, std::milli> ms = stop - start;

        if (N <= 60)
            printGeneration(reasons, neighbours, N, gen, generations, survive, born, lonely, crowded, ms.count());
        else
            printf("Generasi %d / %d (grid %d x %d, serial): %.3f ms\n", gen, generations, N, N, ms.count());

        char *tmp = c_m;
        c_m = n_m;
        n_m = tmp;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    free(c_m);
    free(n_m);
    free(reasons);
    free(neighbours);

    return 0;
}