/*****************************************************************************
* Filename: mandelmovie.c
* Description: Generates Mandelbrot images using multiprocessing
* Author: Christian Gulak
* Date: 11-22-24
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

// Constants
#define TOTAL_IMAGES 50
#define SEM_NAME "/mandel_sem"

// Function prototypes
void generate_frame(int frame, double x, double y, double scale, const char *outfile);

int main(int argc, char *argv[]) {
    int num_processes = 1; // Default number of child processes
    char c;

    // Parse command-line arguments
    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                num_processes = atoi(optarg);
                break;
            default:
                printf("Usage: %s -p <number of processes>\n", argv[0]);
                return 1;
        }
    }

    // Validate number of processes
    if (num_processes < 1) {
        fprintf(stderr, "Error: Number of processes must be at least 1.\n");
        return 1;
    }

    printf("Using %d processes to generate %d images\n", num_processes, TOTAL_IMAGES);

    // Initialize semaphore
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, num_processes);
    if (sem == SEM_FAILED) {
        perror("Failed to create semaphore");
        return 1;
    }

    // Generate images
    double xcenter = 0.0, ycenter = 0.0, scale = 4.0;

    for (int frame = 0; frame < TOTAL_IMAGES; frame++) {
        sem_wait(sem); // Wait for a semaphore slot

        if (fork() == 0) { // Child process
            char outfile[50];
            snprintf(outfile, sizeof(outfile), "mandel%02d.jpg", frame);
            generate_frame(frame, xcenter, ycenter, scale, outfile);

            printf("Frame %d generated: %s\n", frame, outfile);
            sem_post(sem); // Release semaphore slot
            return 0;      // Exit child process
        }

        // Update parameters for the next frame
        scale *= 0.9; // Zoom in
        xcenter += scale * 0.05; // Shift x-center
        ycenter += scale * 0.05; // Shift y-center
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Cleanup semaphore
    if (sem_close(sem) == -1) {
        perror("Failed to close semaphore");
    }
    if (sem_unlink(SEM_NAME) == -1) {
        perror("Failed to unlink semaphore");
    }

    printf("All frames generated. Compile into a movie using ffmpeg.\n");
    return 0;
}

/**
 * Generates a single Mandelbrot frame and saves it to a file.
 *
 * @param frame The frame index
 * @param x The x-center of the Mandelbrot set
 * @param y The y-center of the Mandelbrot set
 * @param scale The scale for zooming
 * @param outfile The output filename
 */
void generate_frame(int frame, double x, double y, double scale, const char *outfile) {
    char command[256];
    snprintf(command, sizeof(command), "./mandel -x %f -y %f -s %f -o %s", x, y, scale, outfile);
    system(command);
}

