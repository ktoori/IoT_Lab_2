#define _CRT_SECURE_NO_WARNINGS
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

typedef struct {
    double x;
    double y;
} Complex;

typedef struct {
    double x;
    double y;
    int in_set;
} Point;

int mandelbrot(Complex c, int max_iter) {
    Complex z;
    z.x = 0.0;
    z.y = 0.0;

    int iter = 0;

    while (iter < max_iter) {
        double module = sqrt(z.x * z.x + z.y * z.y);
        if (module > 2.0) break;

        double new_x = z.x * z.x - z.y * z.y + c.x;
        double new_y = 2.0 * z.x * z.y + c.y;

        z.x = new_x;
        z.y = new_y;
        iter++;
    }

    return iter;
}

int main(int argc, char* argv[]) {
    int nthreads = atoi(argv[1]);
    int npoints = atoi(argv[2]);
    int max_iter = 1000;
    int time_iter = 20;

    omp_set_num_threads(nthreads);

    double x_min = -2.0;
    double x_max = 1.0;
    double y_min = -1.5;
    double y_max = 1.5;

    Point* points = (Point*)malloc(npoints * sizeof(Point));

    unsigned int seed = (unsigned int)time(NULL);
    
    double start_time = omp_get_wtime();
    
    for (int k = 0; k < time_iter; k++) {
        #pragma omp parallel
        {
            unsigned int thread_seed = seed + omp_get_thread_num();
            
            #pragma omp for schedule(static)
            for (int i = 0; i < npoints; i++) {
                double x = x_min + (x_max - x_min) * ((double)rand_r(&thread_seed) / RAND_MAX);
                double y = y_min + (y_max - y_min) * ((double)rand_r(&thread_seed) / RAND_MAX);

                Complex c;
                c.x = x;
                c.y = y;

                int iter = mandelbrot(c, max_iter);

                points[i].x = x;
                points[i].y = y;
                points[i].in_set = (iter == max_iter) ? 1 : 0;
            }
        }
    }

    double end_time = omp_get_wtime();

    FILE* fp = fopen("mandelbrot.csv", "w");
    
    fprintf(fp, "x,y\n");
    int points_in_set = 0;
    for (int i = 0; i < npoints; i++) {
        if (points[i].in_set == 1) {
            fprintf(fp, "%.4f,%.4f\n", points[i].x, points[i].y);
            points_in_set++;
        }
    }
    
    fclose(fp);
    free(points);

    printf("Execution time: %f sec\n", (end_time - start_time) / time_iter);
    printf("Mandelbrot set points: %d из %d\n", points_in_set, npoints);
    
    return 0;
}