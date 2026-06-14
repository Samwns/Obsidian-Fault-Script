#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static double elapsed_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0
        + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: bench-runner command [args...]\n");
        return 2;
    }

    struct timespec start;
    struct timespec end;
    struct rusage usage;
    int status = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t child = fork();
    if (child == 0) {
        execvp(argv[1], &argv[1]);
        fprintf(stderr, "exec failed: %s\n", strerror(errno));
        _exit(127);
    }
    if (child < 0) {
        perror("fork");
        return 2;
    }

    if (wait4(child, &status, 0, &usage) < 0) {
        perror("wait4");
        return 2;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 128;
    printf("%.4f,%ld,%d\n", elapsed_ms(start, end), usage.ru_maxrss, exit_code);
    return exit_code == 0 ? 0 : exit_code;
}
