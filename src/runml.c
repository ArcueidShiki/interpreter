/**
 * CITS2002 Project 1 2024
 * Student1:    24323312    Jingtong Peng
 * Student2:    24364937    Lingyu Chen
 * Platform:    Linux (or MacOs)
 */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int check_args(int argc, char **arv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filepath>\n", arv[0]);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int check_file(char *filepath)
{
    int fd = open(filepath, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open %s, fd: %d\n", filepath, fd);
        exit(EXIT_FAILURE);
    }
    printf("File %s opened successful, fd: %d\n", filepath, fd);
    return fd;
}

int main(int argc, char **argv)
{
    check_args(argc, argv);
    int fd = check_file(argv[1]);
    close(fd);
    printf("Hello, World!\n");
    printf("this can work!\n");
}

