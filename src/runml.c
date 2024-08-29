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
#include <ctype.h>

#ifdef DEBUG_MODE
#define DEBUG_START "@[DEBUG]: {"
#define DEBUG_END "}\n"
#define LOGD(fmt, ...)                                             \
    if (DEBUG_MODE)                                                \
    {                                                              \
        fprintf(stdout, DEBUG_START fmt DEBUG_END, ##__VA_ARGS__); \
    }
#else
#define LOGD(fmt, ...)
#endif
#define ERROR_START "![ERROR]: {"
#define ERROR_END "}\n"
#define LOGE(fmt, ...) fprintf(stderr, ERROR_START fmt ERROR_END, ##__VA_ARGS__)
#define MAX_UNIQUE_IDENTIFIER 50
#define IDENTIFIER_LENGTH 12
uint16_t cur_ml_file_line = 0;
uint16_t cur_ml_file_col = 0;
char *g_indentifiers[MAX_UNIQUE_IDENTIFIER];
typedef struct
{
    void (*report_error)();
} ErrorHandler;

typedef struct
{
    /* Turns a sequence of characters(plain text) into a sequence of tokens */
} Lexer;
typedef struct
{
    /* Take a sequence of tokens and produces an abstract syntax tree(AST) */
} Parser;

typedef struct
{
    /* Translate code into c11 code. and write it to .c file */
} Translator;

typedef struct
{
    /* Compile the c file */
} Compiler;
typedef struct
{
    /* Execute compiled output */
} Executor;

typedef struct
{
    /* Remove tmp files */
} Cleaner;

typedef struct Identifier
{
    char *name;
} Identifier;
typedef struct Statement
{
    char *expression;
} Statement;

typedef struct
{
    char *funcname;
    int num_of_statement;
    char **statements;
    Identifier *identifiers;
    Identifier *parameters;
} Function;

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
    LOGD("File %s opened successful, fd: %d", filepath, fd);
    return fd;
}

bool is_integer(char *expression)
{
    // For requirement 4.
    (void)expression; // disable warning
    return false;
}

bool is_valid_indentifier(char *identifier)
{
    int n = strlen(identifier);
    if (n > IDENTIFIER_LENGTH)
    {
        LOGE("Identifier %s is too long should be 1 to 12", identifier);
        return false;
    }
    for (int i = 0; i < n; i++)
    {
        if (!isalpha(identifier[i] || !islower(identifier[i])))
        {
            // Give current line and col
            LOGE("Identifier %s is not valid, it should be lowercase alphabetic characters", identifier);
            return false;
        }
    }
    return true;
}

bool is_function_defined(char *function_name)
{
    // TODO Lookup in global function symbol table from top-down.
    (void)function_name; // disable warning
    return false;
}

bool startwith_tab(char *line)
{
    return line[0] == '\t';
}

bool is_valid_function(Function *f)
{
    if (!is_valid_indentifier(f->funcname))
    {
        return false;
    }
    if (!is_function_defined(f->funcname))
    {
        return false;
    }
    for (int i = 0; i < f->num_of_statement; i++)
    {
        if (!startwith_tab(f->statements[i]))
        {
            LOGE("Function %s statements not start with '\\t'", f->funcname);
            return false;
        }
    }
    // TODO
    return true;
}

bool have_end_semicolon(char *line)
{
    bool have = line[strlen(line) - 1] == ';';
    if (have)
    {
        // TODO get ml file line and col
        LOGE("Line: %s is not valid with a terminating semicolon", line);
    }
    return have;
}

bool is_valid_line(char *line)
{
    if (have_end_semicolon(line))
    {
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    check_args(argc, argv);
    int fd = check_file(argv[1]);

    close(fd);
    LOGD("This is a debug message");
    LOGD("This is a debug message %d", 1);
    LOGD("This is a debug message %s", "test");
    char *statements[] = {"\t12312", "  abcasdad", "    12312asdas"};
    Function f = {.statements = statements};
    LOGD("Start with tab: %d", startwith_tab(f.statements[0]));
    LOGD("Start with tab; %d", startwith_tab(f.statements[1]));
    LOGD("Start with tab; %d", startwith_tab(f.statements[2]));
}
