/**
 * This file is for test, will not be submitted
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    printf("This is Test:\n");

    // current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current directory: %s\n", cwd);
    }
    else
    {
        perror("getcwd() error");
        return 1;
    }
    strcpy(cwd + strlen(cwd), "/runml_release");
    assert(system(cwd) == 0);
    return 0;
}