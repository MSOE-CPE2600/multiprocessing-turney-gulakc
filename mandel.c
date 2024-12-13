/*****************************************************************************
* Filename: mandel.c
* Description: Generates Mandelbrot images using multiprocessing
* Author: Christian Gulak
* Date: 11-22-24
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "jpegrw.h"

// Struct to hold arguments for each thread
typedef struct {
    imgRawImage *img;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int max;
    int start_row;
    int end_row;
} ThreadArgs;

// Local routines
static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void *compute_region(void *args);
static void compute_image(imgRawImage *img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads);
static void show_help();

int main(int argc, char *argv[]) {
    char c;

    // These are the default configuration values used if no command line arguments are given
    const char *outfile = "mandel.jpg";
    double xcenter = 0;
    double ycenter = 0;
    double xscale = 4;
    double yscale = 0; // Calc later
    int image_width = 1000;
    int image_height = 1000;
    int max = 1000;
    int num_threads = 1; // Default to 1 thread

    // For each command line argument given, override the appropriate configuration value
    while ((c = getopt(argc, argv, "x:y:s:W:H:m:o:t:h")) != -1) {
        switch (c) {
            case 'x':
                xcenter = atof(optarg);
                break;
            case 'y':
                ycenter = atof(optarg);
                break;
            case 's':
                xscale = atof(optarg);
                break;
            case 'W':
                image_width = atoi(optarg);
                break;
            case 'H':
                image_height = atoi(optarg);
                break;
            case 'm':
                max = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            case 't':
                num_threads = atoi(optarg);
                if (num_threads < 1 || num_threads > 20) {
                    fprintf(stderr, "Error: Number of threads must be between 1 and 20.\n");
                    exit(1);
                }
                break;
            case 'h':
                show_help();
                exit(1);
                break;
        }
    }

    // Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
    yscale = xscale / image_width * image_height;

    // Display the configuration of the image
    printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s threads=%d\n",
           xcenter, ycenter, xscale, yscale, max, outfile, num_threads);

    // Create a raw image of the appropriate size
    imgRawImage *img = initRawImage(image_width, image_height);

    // Fill it with black
    setImageCOLOR(img, 0);

    // Compute the Mandelbrot image
    compute_image(img, xcenter - xscale / 2, xcenter + xscale / 2, ycenter - yscale / 2, ycenter + yscale / 2, max, num_threads);

    // Save the image in the stated file
    storeJpegImageFile(img, outfile);

    // Free the mallocs
    freeRawImage(img);

    return 0;
}

// Function executed by each thread
void *compute_region(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    imgRawImage *img = threadArgs->img;
    double xmin = threadArgs->xmin;
    double xmax = threadArgs->xmax;
    double ymin = threadArgs->ymin;
    double ymax = threadArgs->ymax;
    int max = threadArgs->max;
    int start_row = threadArgs->start_row;
    int end_row = threadArgs->end_row;

    int width = img->width;

    // Compute the assigned rows
    for (int j = start_row; j < end_row; j++) {
        for (int i = 0; i < width; i++) {
            // Determine the point in x,y space for that pixel
            double x = xmin + i * (xmax - xmin) / width;
            double y = ymin + j * (ymax - ymin) / img->height;

            // Compute the iterations at that point
            int iters = iterations_at_point(x, y, max);

            // Set the pixel in the bitmap
            setPixelCOLOR(img, i, j, iteration_to_color(iters, max));
        }
    }

    return NULL;
}

// Updated compute_image function
void compute_image(imgRawImage *img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads) {
    pthread_t threads[num_threads];
    ThreadArgs threadArgs[num_threads];

    int height = img->height;
    int rows_per_thread = height / num_threads;

    // Create threads
    for (int t = 0; t < num_threads; t++) {
        threadArgs[t].img = img;
        threadArgs[t].xmin = xmin;
        threadArgs[t].xmax = xmax;
        threadArgs[t].ymin = ymin;
        threadArgs[t].ymax = ymax;
        threadArgs[t].max = max;
        threadArgs[t].start_row = t * rows_per_thread;
        threadArgs[t].end_row = (t == num_threads - 1) ? height : (t + 1) * rows_per_thread;

        pthread_create(&threads[t], NULL, compute_region, &threadArgs[t]);
    }

    // Join threads
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color(int iters, int max) {
    int color = 0xFFFFFF * iters / (double)max;
    return color;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/
int iterations_at_point(double x, double y, int max) {
    double x0 = x;
    double y0 = y;

    int iter = 0;

    while ((x * x + y * y <= 4) && iter < max) {
        double xt = x * x - y * y + x0;
        double yt = 2 * x * y + y0;

        x = xt;
        y = yt;

        iter++;
    }

    return iter;
}

// Show help message
void show_help() {
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>  X coordinate of image center point. (default=0)\n");
    printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
    printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
    printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
    printf("-o <file>   Set output file. (default=mandel.jpg)\n");
    printf("-t <threads> Number of threads to use (default=1, max=20)\n");
    printf("-h          Show this help text.\n");
}
