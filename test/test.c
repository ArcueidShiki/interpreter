/**
 * This file is for test, will not be submitted
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_TESTCASES 8
const char *testcases[] = {
    "../test/valid/sample01.ml",
    "../test/valid/sample02.ml",
    "../test/valid/sample03.ml",
    "../test/valid/sample04.ml",
    "../test/valid/sample05.ml",
    "../test/valid/sample06.ml",
    "../test/valid/sample07.ml",
    "../test/valid/sample08.ml 1312",
    "../test/valid/sample09.ml",
    "../test/valid/sample10.ml",
};

void test_debug()
{
    char *arg0 = "./runml_debug ";
    for (int i = 0; i < MAX_TESTCASES; i++)
    {
        const char *arg1 = testcases[i];
        printf("Test case: %s\n", arg1);
        char command[1024];
        memset(command, 0, sizeof(command));
        strncat(command, arg0, strlen(arg0));
        strncat(command, arg1, strlen(arg1));
        assert(system(command) == 0);
    }
}

void test_release()
{
    char *arg0 = "./runml_release ";
    for (int i = 0; i < MAX_TESTCASES; i++)
    {
        const char *arg1 = testcases[i];
        printf("Test case: %s\n", arg1);
        char command[1024];
        memset(command, 0, sizeof(command));
        strncat(command, arg0, strlen(arg0));
        strncat(command, arg1, strlen(arg1));
        assert(system(command) == 0);
    }
}

int test()
{
    // current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current directory: %s\n", cwd);
    }
    else
    {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
    test_debug();
    test_release();
    printf("All test cases passed\n");
    return 0;
}

int main(void)
{
    test();
    return EXIT_SUCCESS;
}