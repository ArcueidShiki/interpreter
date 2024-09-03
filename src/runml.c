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

FILE *ml_fp;
FILE *outc_fp;
uint16_t cur_ml_file_row = 1;
uint16_t cur_ml_file_col = 0;
int g_argc;
char **g_argv;
char outc_filename[64];
char outc_executable[64];
bool HAVE_ERRORS = false;
#ifdef DEBUG_MODE
#define DEBUG_START "@[DEBUG:line:%d]: {"
#define DEBUG_END "}\n"
#define LOGD(fmt, ...)                                                       \
    if (DEBUG_MODE)                                                          \
    {                                                                        \
        fprintf(stdout, DEBUG_START fmt DEBUG_END, __LINE__, ##__VA_ARGS__); \
    }
#else
#define LOGD(fmt, ...)
#endif
#define ERROR_START "![ERROR:line:%d]: {"
#define ERROR_END "}\n"
#define LOGE(fmt, ...) fprintf(stderr, ERROR_START fmt ERROR_END, cur_ml_file_row, ##__VA_ARGS__)
#define MAX_UNIQUE_IDENTIFIER 50
#define MAX_IDENTIFIER_LENGTH 12

#define UTILS_START 42
// ======================== Write to C code file Start ========================
bool is_intd(double num)
{
    return (double)((int)num) == num;
}

bool is_intf(float num)
{
    return (float)((int)num) == num;
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
#define UTILS_END 82

typedef enum
{
    STATEMENT,           // one line, only statement could output to stdout
    FUNCTION_DEFINITION, // function definition including its substatement.
} PROGRAM_ITEM_TYPE;

typedef enum
{
    ASSIGNMENT,   // identifier <- expression => {double identifier = (double)expression;}
    PRINT,        // print expression       => {PRINT(expression);}=
    RETURN,       // return expression      => {return expression;}
    FUNCTION_CALL // identifier "(" [ expression ("," expression) *]")" => just add `;` at the end.
} STATEMENT_TYPE;

typedef enum{
    CONSTANT,
    IDENTIFIER, // user defined
    FUNCTION,   // user defined
    EXPRESSION,
    KEYWORD,
    OPERATOR,
} TOKEN_TYPE;

typedef enum
{
    VARIABLE,
    FUNCTION_NAME,
    PARAMETER,
} IDENTIFIER_TYPE; // or SYMBOL_TYPE
typedef struct
{
    IDENTIFIER_TYPE type;
    int id;             // equals to its index in g_indentifiers
    bool row_of_define; // record the definition row.
    bool in_function;
    bool is_defined; // when it goes out of function, should be set undefined.
    bool is_valid;
    char *name;
} Identifier;
typedef struct Statement
{
    STATEMENT_TYPE type;
    bool is_valid;
    bool in_function;       // is in function, start with tab or not.
    char *orgin;            // original line
    char *translated;       // translated line
    int row_of_define;      // current row in ml file.
    struct Statement *prev; // previous statement pointer,
    struct Statement *next; // next statement pointer.
} Statement;
typedef struct
{
    int row_of_define; // record the definition row.
    int para_count;
    int statement_count;
    Identifier *funcname;
    Identifier *parameters;
    Statement *statements;
} Function;

Identifier g_indentifiers[MAX_UNIQUE_IDENTIFIER];
int g_indentifiers_count = 0;

int check_args(int argc, char **argv)
{
    if (argc < 2)
    {
        LOGE("Usage: %s <filepath> <number1, [number2...]>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    g_argc = argc;
    g_argv = argv;
    return 0;
}

void clean()
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm %s*", outc_executable);
    int ret = system(cmd);
    if (ret != 0)
    {
        LOGE("Clean failed with return code %d", ret);
        exit(EXIT_FAILURE);
    }
}

void close_files()
{
    if (ml_fp != NULL)
    {
        fclose(ml_fp);
    }
    if (outc_fp != NULL)
    {
        fclose(outc_fp);
    }
}

void safe_exit()
{
    close_files();
    exit(EXIT_FAILURE);
}

bool is_valid_identifier(Identifier *identifier)
{
    char *name = identifier->name;
    int n = strlen(name);
    if (n > MAX_IDENTIFIER_LENGTH)
    {
        LOGE("Identifier %s is too long should be 1 to 12", identifier);
        identifier->is_valid = false;
        return false;
    }
    for (int i = 0; i < n; i++)
    {
        if (!isalpha(name[i] || !islower(name[i])))
        {
            // Give current line and col
            LOGE("Identifier %s is not valid, it should be lowercase alphabetic characters", identifier);
            identifier->is_valid = false;
            return false;
        }
    }
    identifier->is_valid = true;
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

bool operator_has_operands()
{

}

bool check_bracket(char *line)
{

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
        LOGD("# found at row:%d, col:%ld", cur_ml_file_row, pos - line);
        line[pos - line] = '\0';
    }
}

/**
 * check if token is a command line argument.
 * ./runml program.ml arg0, arg1, arg2, ... argN
 * arg0 should be a valid number.
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
        LOGD("%s is not a cmd line argument, %c", token, *badchar);
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
char *translate_cmdline_arg(char *token)
{
    long N;
    if (!is_cmdline_arg(token, &N))
    {
        // do nothing.
        return token;
    }
    // out of boundary e.g. argc = 3, but arg2 is appear in code. this is an error.
    if (N + 1 > g_argc - 1)
    {
        LOGE("arg%ld is out of boundary", N);
        close_files();
        exit(EXIT_FAILURE);
    }

    char *badchar;
    strtod(g_argv[N + 1], &badchar);
    if (*badchar != '\0')
    {
        LOGE("argv[%ld + 1] is not a valid number, %s", N, g_argv[N + 1]);
        exit(EXIT_FAILURE);
    }
    return g_argv[N + 1];
}

bool is_function_defined()
{
    // TODO Lookup in global function symbol table from top-down.
    return false;
}

bool is_valid_function(Function *f)
{
    if (!is_valid_identifier(f->funcname->name))
    {
        return false;
    }
    // TODO
    return true;
}

void write_header()
{
    char *header =
        "#include <stdio.h>\n"
        "#include <stdbool.h>\n"
        "#include <math.h>\n";
    fputs(header, outc_fp);
}

void write_utils()
{
    FILE *fp = fopen("runml.c", "r");
    if (fp == NULL)
    {
        LOGE("Cannot open runml.c, please don't move runml.c, place runml and runml.c in the same directory");
        return;
    }
    char line[1024];
    int cur_line = 1;
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (UTILS_START < cur_line && cur_line < UTILS_END)
        {
            fputs(line, outc_fp);
        }
        if (cur_line > UTILS_END)
        {
            break;
        }
        cur_line++;
    }
}

void write_main()
{
    char *main = "int main(int argc, char **argv)\n{\n\tml();\n\treturn 0;\n}";
    fputs(main, outc_fp);
}

/**
 * Parse each line: syntax analysis and error check:
 * 1. Identify program-item type of each line.
 * 2. [function body], start with "function", identifier should not same with keyword
 * 3. [statement]
 * cur_pos = ftell(outc_fp);
 * fseek(outc_fp, cur_pos, cur_pos);
 */
void parse_function_body(char *line_read, char *line_write)
{
    rm_comment(line_read);
    /**
     * line start with a identifier, except "\t or whitespace" -> and "double" at beginning.
     * line start with "function" keyword -> and "double" at beginning, replace " " with "(" "," and ")".
     * line start with "return" keyword
     * line start with "print" keyword
     * use Link Node? g_symbol_table
     */
    (void)line_write;
}

void parse_statements(char *line_read, char *line_write)
{
    rm_comment(line_read);
    (void)line_write;
}

/**
 * 1. Tokenize per line
 * 2. First token must be "function" keyword.
 * 3. Record line number of the start of the function body
 * 4. The following tokens identifiers which should be valid identifier(is_valid_identifier). If not, LOGE, exit(EXIT_FAILURE)
 * 5. Create a tmp Identifier, Check its validity, if not, LOGE, but not exit, only report error
 * 5. Set flag function_body_begin
 * 6. The following lines should check if a line start with a tab.
 * 7. branch1: start with tab, then it is in function, and set its function identifier pointer(it contains of row of define).
 *      7.1. Check if it is a valid statement(identifier.type ==  function && is defined, otherwise, set to 0),
 *      7.2. if not, LOGE, but not exit, only report error, you should report all the error.
 * 8. branch2: not start with tab, set flag {function_body_end}, skip this statement. until encounter the funtion defitino body.
 *      General global statement will be processed in the second round of reading ml file.
 * 9. only without error, then going to compilation steps, otherwise, exit(EXIT_FAILURE).
 */
void write_ml_functions()
{
    char line_read[1024];
    char line_write[1024];
    cur_ml_file_row = 0;
    fseek(ml_fp, 0, SEEK_SET);
    while (fgets(line_read, sizeof(line_read), ml_fp) != NULL)
    {
        parse_function_body(line_read, line_write);
        fputs(line_write, outc_fp);
        cur_ml_file_row++;
    }
}

/**
 * Read each line
 * 1. If line started with function. set flag, funtion start row and end row.
 * 2. Then go to next statement line not start with tab.
 * 3. Tokenize the statement, check identifier(variable, function, undefined), keyword, operator(* / + - ), constatnt(numbers) brackets(),
 * 4. If token is illegal symbol other than above, like character is not alpha numberic/digit using strtod. report error,  but not exit.
 * 5. If token is identifier, if it is a function call followed by brackets, check function is defined or not
 * 6. If token is identifier, not a function call, check if it is defined, otherwise assigned it to 0.
 * 7. If token is not identifier, check if it is a keyword (should followed by an expression), a valid number, a operator
 * 8. Check brackets match pairs
 * 9. Check operator has operands / expression on both sides. For example, operator can't appear on the first, after an operator, should be followed by something other than keyowrd and operator
 * 10. Add semicolon at the end of the statement.
 */
void write_ml_statements()
{
    rewind(ml_fp);
    cur_ml_file_row = 0;
    char line_read[1024];
    char line_write[1024];
    fputs("void ml()\n{\n", outc_fp);
    while (fgets(line_read, sizeof(line_read), ml_fp) != NULL)
    {
        parse_statements(line_read, line_write);
        fputs(line_write, outc_fp);
        cur_ml_file_row++;
    }
    fputs("}\n", outc_fp);
}

void check_files()
{
    char *ml_filename = g_argv[1];
    int n = strlen(ml_filename);
    if (n < 4 || strcmp(ml_filename + n - 3, ".ml") != 0)
    {
        LOGE("File %s is not a valid .ml file", ml_filename);
        exit(EXIT_FAILURE);
    }
    ml_fp = fopen(ml_filename, "r");
    if (ml_fp == NULL)
    {
        LOGE("Cannot open %s", ml_filename);
        close_files();
        exit(EXIT_FAILURE);
    }
    snprintf(outc_filename, sizeof(outc_filename), "ml-%d.c", getpid());
    outc_fp = fopen(outc_filename, "w");
    if (outc_fp == NULL)
    {
        close_files();
        exit(EXIT_FAILURE);
    }
}

void translate()
{
    check_files();
    write_header();
    write_utils();
    write_ml_functions();
    write_ml_statements();
    write_main();
    close_files();
}

void compile()
{
    snprintf(outc_executable, sizeof(outc_executable), "./ml-%d", getpid());
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cc -std=c11 -o %s %s", outc_executable, outc_filename);
    int ret = system(cmd);
    if (ret != 0)
    {
        LOGE("Compile failed with return code %d", ret);
#ifndef DEBUG_MODE
        clean();
#endif
        exit(EXIT_FAILURE);
    }
}

void execute()
{
    int ret = system(outc_executable);
    if (ret != 0)
    {
        LOGE("Execute failed with return code %d", ret);
#ifndef DEBUG_MODE
        clean();
#endif
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    check_args(argc, argv);
    translate();
    if (HAVE_ERRORS)
    {
        safe_exit();
    }
    compile();
    execute();
#ifndef DEBUG_MODE
    clean();
#endif

#ifdef DEBUG_MODE
    LOGD("Start with tab; %d", startwith_tab(""));
    LOGD("Start with tab; %d", startwith_tab("\n"));
    LOGD("Start with tab; %d", startwith_tab("\t\t"));
    LOGD("5.0 == 5 ? %d", 5.0 == 5);
    PRINT(5.0);
    PRINT(5.5);
    PRINT(5);
    PRINT(add(5, 5) * add(5, 3));
    PRINT(5.0 + 5.0);
    PRINT(5.0 + 5);
    PRINT(5.0 + 5.0);
#endif
    return 0;
}