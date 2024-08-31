/**
 * CITS2002 Project 1 2024
 * Student1:    24323312    Jingtong Peng
 * Student2:    24364937    Lingyu Chen
 * Platform:    Linux (or MacOs)
 * Dev:         https://github.com/ArcueidShiki/interpreter
 */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

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
#define MAX_IDENTIFIER_LENGTH 12

uint16_t cur_ml_file_row = 0;
uint16_t cur_ml_file_col = 0;
char *g_indentifiers[MAX_UNIQUE_IDENTIFIER];
int g_identifiers_count = 0;

// ======================== Write to C code file Start ========================
bool is_intd(double num)
{
    return floor(num) == num;
}

bool is_intf(float num)
{
    return floor(num) == num;
}

// tested
#define ISINT(expr) _Generic((expr), \
    short: true,                     \
    int: true,                       \
    long: true,                      \
    long long: true,                 \
    float: is_intf(expr),            \
    double: is_intd(expr))

#define FMT(expr) _Generic((expr), \
    short: "%hd\n",                \
    int: "%d\n",                   \
    long: "%ld\n",                 \
    long long: "%lld\n",           \
    float: "%f\n",                 \
    double: "%f\n")

#define PRINT(expr)                        \
    if (ISINT(expr))                       \
    {                                      \
        long long res = (long long)(expr); \
        printf(FMT(res), res);             \
    }                                      \
    else                                   \
    {                                      \
        printf(FMT(expr), (expr));         \
    }
// ======================== Write to C code file End ========================

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

typedef struct Variable
{
    /* Don't need to be defined, but need to be declared.*/
} Variable;
typedef struct Statement
{
    char *expression;
} Statement;

typedef struct
{
    char *funcname;
    int num_of_statement;
    char **statements;
    Variable *variables;
    Variable *parameters;
} Function;

typedef struct
{
} FileHandler;

int check_args(int argc, char **arv)
{
    if (argc != 2)
    {
        LOGE("Usage: %s <filepath>\n", arv[0]);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int check_file(char *filepath)
{
    int n = strlen(filepath);
    if (n < 4 || strcmp(filepath + n - 3, ".ml") != 0)
    {
        LOGE("File %s is not a valid .ml file", filepath);
    }
    int fd = open(filepath, O_RDONLY);
    if (fd == -1)
    {
        LOGE("Cannot open %s, fd: %d\n", filepath, fd);
        exit(EXIT_FAILURE);
    }
    LOGD("File %s opened successful, fd: %d", filepath, fd);
    return fd;
}

bool is_valid_indentifier(char *identifier)
{
    int n = strlen(identifier);
    if (n > MAX_IDENTIFIER_LENGTH)
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

bool startwith_tab(char *line)
{
    int n = strlen(line);
    if (n >= 1)
    {
        return line[0] == '\t';
    }
    return false;
}

bool endwith_semicolon(char *line)
{
    int n = strlen(line);
    if (n == 0)
    {
        return false;
    }
    if (line[n - 1] == ';')
    {
        // TODO get ml file line and col
        LOGE("Line: %s is not valid with a terminating semicolon", line);
        return true;
    }
    return false;
}

bool is_valid_line(char *line)
{
    if (endwith_semicolon(line))
    {
        return false;
    }
    return true;
}

/**
 * rm comment first, then process each line.
 */
void rm_comment(char *line)
{
    char comment = '#';
    char *pos = strchr(line, comment);
    if (pos != NULL)
    {
        LOGD("# found at %ld", pos - line);
        line[pos - line] = '\0';
    }
}

/**
 * check if token is a command line argument.
 * if it is, set the value to N.
 */
bool is_cmdline_arg(char *token, long *N)
{
    int n = strlen(token);
    if (n < 4)
    {
        return false;
    }
    if (strncmp(token, "arg", 3) != 0)
    {
        return false;
    }
    char *badchar;
    *N = strtol(token + 3, &badchar, 10);
    if (*badchar != '\0')
    {
        LOGD("%s is not a valid number, %c", token, *badchar);
        return false;
    }
    return true;
}

/**
 * handle arg0, arg1 ... argN
 *      -> argv[0]      "runml"
 * arg0 -> argv[1]      "program.ml"
 * arg1 -> argv[2]      "number1"
 * arg2 -> argv[3]      "number2"
 * argN -> argv[N + 1]  "numberN"
 * from arg1 should check if it is a valid number.
 * @return translated command line argument.
 * @param token: is a vali string end with '\0'
 */
char *translate_cmdline_arg(int argc, char **argv, char *token)
{
    long N;
    if (!is_cmdline_arg(token, &N))
    {
        // do nothing.
        return token;
    }
    // out of boundary e.g. argc = 3, but arg2 is appear in code. this is an error.
    if (N + 1 > argc - 1)
    {
        LOGE("arg%ld is out of boundary", N);
        return token;
    }
    return argv[N + 1];
}

bool is_function_defined(char *function_name)
{
    // TODO Lookup in global function symbol table from top-down.
    (void)function_name; // disable warning
    return false;
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
    LOGD("Start with tab; %d", startwith_tab(""));
    LOGD("Start with tab; %d", startwith_tab("\n"));
    LOGD("Start with tab; %d", startwith_tab("\t\t"));
    (void)f;
    LOGD("5.0 == 5 ? %d", 5.0 == 5);
    PRINT(5.0);
    PRINT(5.5);
    PRINT(5);
    PRINT(5.0 + 5.5);
    PRINT(5.0 + 5.0);
    PRINT(5.0 + 5);
    PRINT(5.0 + 5.0);
}