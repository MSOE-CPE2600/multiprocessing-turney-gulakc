# System Programming Lab 11 Multiprocessing
time ./mandelmovie -p 1
time ./mandelmovie -p 2
time ./mandelmovie -p 5
time ./mandelmovie -p 10
time ./mandelmovie -p 20
https://msoe365-my.sharepoint.com/:x:/g/personal/gulakc_msoe_edu/EQBiruIW4cBGshLre9lJo9oBlL0gSb3cjCpYoiEKCelWqg?e=WdXjkA&nav=MTVfezAwMDAwMDAwLTAwMDEtMDAwMC0wMDAwLTAwMDAwMDAwMDAwMH0

The mandelmovie program generates a series of Mandelbrot set images using multiprocessing. It accepts the number of child processes (-p) as input, and each child generates a frame with adjusted zoom and center parameters to simulate zooming in. The program ensures no more than the specified number of processes run simultaneously using fork() and wait().

Each frame is saved as a .jpg, and the ffmpeg tool stitches these frames into a video. Runtime was collected for 1, 2, 5, 10, and 20 processes to analyze the performance impact of parallelization. Results were plotted, showing how increasing processes reduces runtime.