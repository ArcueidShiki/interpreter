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

/**
 * Ref: https://www.toptal.com/scala/writing-an-interpreter
 * TODO: Program runml should do the following:
 * 1. Read each line of the file
 * 2. Extract tokens from each line:
 *      keywords[function, return, arg0, arg1, '\t'],
 *      operands[x, y],
 *      operators[<-, +, *, (, ), ',', #, whitespace],
 *      variables[value, one],
 *      functions[print, increment],
 *      parameters[local para]
 *      types[real, integer]
 * 3. Check new line syntax based on previous line tokens
 * 4. Write translated tokens to c code and format.
 * 5. Validates and Report any errors commencing with the ! character. (Must detect all invalid ml programs)
 * 6. Generate C11 code in a file name, for exmaple ml-$pid.c
 * 7. Compile ml-$pid.c
 * 8. Execute ./ml-$pid, passing any optional command-line arguments(real numbers)
 * 9. Removes any files that it created.
 *
 * Requirements:
 * All syntax errors detected in invalid ml programs must be reported via stderr on a line commencing with the '!' character. Your runml program must be able to detect all invalid ml programs - EXCEPT that your program will not be tested with any invalid expressions, so you do not need to validate the syntax of expressions.
 * The only 'true' output produced by your translated and compiled program (when running) is the result of executing ml's print statement. Any 'debug' printing should appear on a line commencing with the '@' character.
 * When printed, numbers that are exact integers must be printed without any decimal places; other numbers must be printed with exactly 6 decimal places.
 */

int main(int argc, char **argv)
{
    check_args(argc, argv);
    int fd = check_file(argv[1]);
    close(fd);
    printf("Hello, World!\n");
    printf("this can work!\n");
}

