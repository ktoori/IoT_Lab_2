#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>

#define G 6.67430e-11 

typedef struct {
    double x, y;      
    double vx, vy;      
    double mass;      
} Body;

void read_input(const char* filename, int* n, Body** bodies) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: cannot open file %s\n", filename);
        exit(1);
    }

    fscanf(f, "%d", n);
    *bodies = (Body*)malloc((*n) * sizeof(Body));

    int i;
    for (i = 0; i < *n; i++) {
        fscanf(f, "%lf %lf %lf %lf %lf",
            &(*bodies)[i].mass,
            &(*bodies)[i].x,
            &(*bodies)[i].y,
            &(*bodies)[i].vx,
            &(*bodies)[i].vy);
    }

    fclose(f);
    printf("Number of points: %d\n", *n);
}

void write_output(FILE* f, double t, int n, Body* bodies) {
    int i;
    fprintf(f, "%.6f", t);
    for (i = 0; i < n; i++) {
        fprintf(f, "\t%.6f\t%.6f", bodies[i].x, bodies[i].y);
    }
    fprintf(f, "\n");
}


void compute_forces(int n, Body* bodies, double** fx_local, double** fy_local, int num_threads) {
    int q, k, i, tid;
    double dx, dy, r, r3, force_x, force_y;

    for (i = 0; i < num_threads; i++) {
        memset(fx_local[i], 0, n * sizeof(double));
        memset(fy_local[i], 0, n * sizeof(double));
    }

#pragma omp parallel for schedule(dynamic) private(k, dx, dy, r, r3, force_x, force_y, tid)
    for (q = 0; q < n; q++) {
        tid = omp_get_thread_num();

        for (k = q + 1; k < n; k++) {  
            dx = bodies[k].x - bodies[q].x;
            dy = bodies[k].y - bodies[q].y;
            r = sqrt(dx * dx + dy * dy);

            if (r < 1e-10) continue;  

            r3 = r * r * r;
            force_x = G * bodies[q].mass * bodies[k].mass * dx / r3;
            force_y = G * bodies[q].mass * bodies[k].mass * dy / r3;

            fx_local[tid][q] += force_x;
            fy_local[tid][q] += force_y;
            fx_local[tid][k] -= force_x;  
            fy_local[tid][k] -= force_y;
        }
    }
}

void update_velocities_and_positions(int n, Body* bodies, double** fx_local, double** fy_local, int num_threads, double dt) {
    int q, i;
    double fx_total, fy_total;

#pragma omp parallel for private(fx_total, fy_total, i)
    for (q = 0; q < n; q++) {
        fx_total = 0.0;
        fy_total = 0.0;
        for (i = 0; i < num_threads; i++) {
            fx_total += fx_local[i][q];
            fy_total += fy_local[i][q];
        }

        bodies[q].vx += (fx_total / bodies[q].mass) * dt;
        bodies[q].vy += (fy_total / bodies[q].mass) * dt;
        bodies[q].x += bodies[q].vx * dt;
        bodies[q].y += bodies[q].vy * dt;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <tend> <input_file> [num_threads]\n", argv[0]);
        printf("Example: %s 10.0 input.txt 4\n", argv[0]);
        return 1;
    }

    double tend = atof(argv[1]);
    char* input_file = argv[2];
    int num_threads = (argc >= 4) ? atoi(argv[3]) : omp_get_max_threads();

    omp_set_num_threads(num_threads);

    printf("Time interval: 0 to %.2f\n", tend);
    printf("Threads: %d\n", num_threads);

    int n;
    Body* bodies;
    read_input(input_file, &n, &bodies);

    double** fx_local = (double**)malloc(num_threads * sizeof(double*));
    double** fy_local = (double**)malloc(num_threads * sizeof(double*));
    int i;
    for (i = 0; i < num_threads; i++) {
        fx_local[i] = (double*)malloc(n * sizeof(double));
        fy_local[i] = (double*)malloc(n * sizeof(double));
    }

    FILE* output = fopen("output.csv", "w");
    if (!output) {
        printf("Error: cannot create output.csv\n");
        return 1;
    }

    double dt = 0.01;
    int num_steps = (int)(tend / dt);
    double t = 0.0;
    int step;

    printf("Time step: %.4f\n", dt);
    printf("Total steps: %d\n", num_steps);

    double start_time = omp_get_wtime();

    write_output(output, t, n, bodies);

    for (step = 1; step <= num_steps; step++) {
        compute_forces(n, bodies, fx_local, fy_local, num_threads);
        update_velocities_and_positions(n, bodies, fx_local, fy_local, num_threads, dt);

        t = step * dt;
        write_output(output, t, n, bodies);

    }

    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;

    printf("\nSimulation complete\n");
    printf("Execution time: %.4f seconds\n", elapsed);
    printf("Performance: %.2f steps/sec\n", num_steps / elapsed);
    printf("Results in output.csv\n");

    for (i = 0; i < num_threads; i++) {
        free(fx_local[i]);
        free(fy_local[i]);
    }
    free(fx_local);
    free(fy_local);

    fclose(output);
    free(bodies);

    return 0;
}
