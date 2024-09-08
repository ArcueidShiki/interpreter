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
uint16_t CUR_ML_ROW = 1;
uint16_t CUR_ML_COL = 0;
int g_argc;
char **g_argv;
char outc_filename[64];
char outc_executable[64];
bool TRANSLATE_PASS = true;
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
#define LOGE(fmt, ...) fprintf(stderr, ERROR_START fmt ERROR_END, CUR_ML_ROW, ##__VA_ARGS__)
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

typedef enum
{
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

typedef struct Function
{
    int start; // start line of function definition
    int end;   // end line of function definition
    char *name;
    int num_of_paras;
    struct Function *parent; // null means global function.
} Function;

typedef struct Identifier
{
    int id;            // for indexing.
    int row_of_define; // record the definition row.
    bool initialized;  // only for VARIABLE, false set value to 0.0
    char *name;
    struct Identifier *scope; // function(points to a FUNCTION_NAME identifer) || global(null)
    IDENTIFIER_TYPE type;
} Identifier;

#if 0
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
    Identifier *scope;      // function(points to a FUNCTION_NAME identifer) or global(null)
} Statement;
#endif

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
        ml_fp = NULL;
    }
    if (outc_fp != NULL)
    {
        fclose(outc_fp);
        outc_fp = NULL;
    }
}

void safe_exit()
{
    LOGE("An error occurred, exiting...");
    close_files();
    exit(EXIT_FAILURE);
}

void trim(char *str)
{
    char *end;
    // trim leading sapce
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return;
    end = str + strlen(str) - 1;

    // trim trailing space
    while (end > str && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
}

bool is_valid_identifier(char *token)
{
    trim(token);
    int n = strlen(token);
    if (n > MAX_IDENTIFIER_LENGTH)
    {
        LOGE("Identifier %s is too long should be 1 to 12", token);
        return false;
    }
    for (int i = 0; i < n; i++)
    {
        if (!isalpha(token[i]) || !islower(token[i]))
        {
            LOGE("Identifier [%s] is not valid, it should be lowercase alphabetic characters", token);
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

#if 0
bool operator_has_operands()
{
}

bool check_bracket(char *line)
{
}
#endif

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
        LOGD("# found at row:%d, col:%ld", CUR_ML_ROW, pos - line);
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
 * @param token: is a valid string end with '\0'
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

#if 0
bool is_function_defined(char *token)
{
    // TODO Lookup in global function symbol table from top-down.
    // Identifer in g_indentifiers, type is FUNCTION_NAME
    return false;
}
#endif

bool is_valid_function(Function *f)
{
    if (!is_valid_identifier(f->name))
    {
        return false;
    }
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

bool is_func_define_line(char line[])
{
    // function definition line, must can be tokenized with separator " ", otherwise it is not a valid function define.
    char *first_token = strtok(line, " ");
    if (first_token == NULL)
    {
        return false;
    }
    if (strcmp(first_token, "function") != 0)
    {
        return false;
    }
    return true;
}

bool is_blank_line(char *line)
{
    int n = strlen(line);
    if (n == 0)
    {
        return true;
    }
    if (line[0] == '\n')
    {
        return true;
    }
    for (int i = 0; i < n - 1; i++)
    {
        if (!isspace(line[i]) || line[i] != '\t')
        {
            return false;
        }
    }
    return true;
}

bool is_comment_line(char *line)
{
    int n = strlen(line);
    if (n == 0)
    {
        return false;
    }
    if (line[0] == '#')
    {
        return true;
    }
    return false;
}

#if 0
bool is_valid_constant(char *token)
{
}
#endif

bool is_operator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/';
}

bool is_valid_expression(char *expr)
{
    int brackets = 0;
    bool prev_is_operator = true;
    for (const char *p = expr; *p != '\0'; p++)
    {
        if (isdigit(*p))
        {
            prev_is_operator = false;
        }
        else if (is_operator(*p))
        {
            if (prev_is_operator)
            {
                LOGE("Two operators near each other");
                return false;
            }
            prev_is_operator = true;
        }
        else if (*p == '(')
        {
            brackets++;
            prev_is_operator = true;
        }
        else if (*p == ')')
        {
            if (brackets == 0 || prev_is_operator)
            {
                LOGE("Invalid brackets");
                return false;
            }
            brackets--;
            prev_is_operator = false;
        }
    }
    return true;
}

bool is_assignment_line(char line_read[])
{
    /**
     * 1. only have one valid "<-",
     * 2. left is alpha characters with zero or more whitespaces,
     * 3. right is number or expression.
     */
    char line[1024];
    strcpy(line, line_read);
    LOGD("is_assignment_line Line: %s", line);
    char *pos = strstr(line, "<-");
    if (pos == NULL)
    {
        return false;
    }
    char *left = strtok(line, "<-");
    if (left == NULL)
    {
        LOGE("Assignment line is missing left side");
        TRANSLATE_PASS = false;
        return false;
    }
    if (!is_valid_identifier(left))
    {
        LOGE("Assignment line left side is not a valid identifier");
        TRANSLATE_PASS = false;
        return false;
    }
    char *right = strtok(NULL, "<-");
    if (right == NULL)
    {
        LOGE("Assignment line is missing right side");
        TRANSLATE_PASS = false;
        return false;
    }
    if (!is_valid_expression(right))
    {
        LOGE("Assignment line right side is not a valid expression");
        TRANSLATE_PASS = false;
        return false;
    }
    return true;
}

void transalte_assignment_line(char line_read[], char line_write[])
{
    char *left = strtok(line_read, "<-");
    char *right = strtok(NULL, "<-");
    snprintf(line_write, 1024, "double %s = %s;\n", left, right);
}

bool is_print_line(char *line)
{
    /**
     * 1. only have one valid "print",
     * 2. right with an expression.
     */
    char *pos = strstr(line, "print");
    if (pos == NULL)
    {
        return false;
    }
    return true;
}

void translate_print_line(char *line_read, char *line_write)
{
    char *pos = strstr(line_read, "print");
    snprintf(line_write, 1024, "PRINT(%s);\n", pos + 5);
}

bool is_return_line(char *line)
{
    /**
     * 1. only have one valid "return",
     * 2. right with an expression.
     */
    char *pos = strstr(line, "return");
    if (pos == NULL)
    {
        return false;
    }
    return true;
}

void translate_return_line(char *line_read, char *line_write)
{
    strcpy(line_write, line_read);
    strcat(line_write, ";\n");
}

void translate_function_call(char *line_read, char *line_write)
{
    strcpy(line_write, line_read);
    strcat(line_write, ";\n");
}

bool process_func_define(char line_read[], char line_write[])
{
    bool success = true;
    strcpy(line_write, "double ");
    char *token;
    token = strtok(line_read, " "); // function
    token = strtok(NULL, " ");      // funcname
    success &= is_valid_identifier(token);
    Identifier funcname = {
        .id = g_indentifiers_count++,
        .name = token,
        .row_of_define = CUR_ML_ROW,
        .scope = NULL,
        .type = FUNCTION_NAME,
        .initialized = false,
    };

    g_indentifiers[funcname.id] = funcname;

    if (token == NULL)
    {
        LOGE("Function name is missing");
        return false;
    }
    strcat(line_write, token);
    strcat(line_write, "(");
    while (token != NULL) // para1 para2 ... paran
    {
        LOGD("Token: %s", token);
        token = strtok(NULL, " ");
        Identifier para = {
            .id = g_indentifiers_count++,
            .name = token,
            .row_of_define = CUR_ML_ROW,
            .scope = &g_indentifiers[funcname.id],
            .type = PARAMETER,
        };
        g_indentifiers[para.id] = para;
        strcat(line_write, token);
        strcat(line_write, ",");
    }
    line_write[strlen(line_write) - 1] = ')';
    strcat(line_write, "\n{");
    return true;
}

void parse_statement(char *line_read, char *line_write)
{
    rm_comment(line_read);
    if (is_assignment_line(line_read))
    {
        transalte_assignment_line(line_read, line_write);
        LOGD("TRANSLATE_PASS: %d", TRANSLATE_PASS);
    }
    else if (is_print_line(line_read))
    {
        translate_print_line(line_read, line_write);
        LOGD("TRANSLATE_PASS: %d", TRANSLATE_PASS);
    }
    else if (is_return_line(line_read))
    {
        translate_return_line(line_read, line_write);
        LOGD("TRANSLATE_PASS: %d", TRANSLATE_PASS);
    }
    else
    {
        translate_function_call(line_read, line_write);
        LOGD("TRANSLATE_PASS: %d", TRANSLATE_PASS);
    }
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
 * 10. if tokens in expresion don't contain spaces.
 * 11. default return double
 * 12. notice is global variable already define, should use it in function body.
 * 13. if variable is not defined, replace str with 0.0, you can only process from top down, global variable and function body. do at the same time.
 * 14. if variable is not defined, doesn't appect the funciton call
 */
void write_ml_definitions()
{
    char line_read[1024] = {0};
    char line_write[1024] = {0};
    CUR_ML_ROW = 0;
    fseek(ml_fp, 0, SEEK_SET);
    bool in_function = false;
    // Function *cur;
    while (fgets(line_read, sizeof(line_read), ml_fp) != NULL)
    {
        CUR_ML_ROW++;
        if (is_blank_line(line_read) || is_comment_line(line_read))
            continue;
        rm_comment(line_read);
        if (!startwith_tab(line_read))
        {
            if (is_assignment_line(line_read))
            {
                // process assignment line because function may use global variable.
                transalte_assignment_line(line_read, line_write);
                fputs(line_write, outc_fp);
            }
        }
        // function funcname para1 para2 ... paran
        else if (is_func_define_line(line_read))
        {
            in_function = true;
            process_func_define(line_read, line_write);
            fputs(line_write, outc_fp);
        }
        else if (in_function)
        {
            if (startwith_tab(line_read))
            {
                parse_statement(line_read, line_write);
            }
            else // end of function body.
            {
                strcpy(line_write, "};\n");
                in_function = false;
            }
            fputs(line_write, outc_fp);
        }
        else // global statements, do nothing.
        {
            continue;
        }
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
 * 11. all the ml logic execute in ml() function, and called in main funciton of c file, therefore, we need read ml file twice.
 */
void write_ml_executions()
{
    rewind(ml_fp);
    CUR_ML_ROW = 0;
    char line_read[1024] = {0};
    char line_write[1024] = {0};
    fputs("void ml()\n{\n", outc_fp);
    while (fgets(line_read, sizeof(line_read), ml_fp) != NULL)
    {
        CUR_ML_ROW++;
        if (is_blank_line(line_read) || is_comment_line(line_read))
            continue;
        if (!startwith_tab(line_read) && !is_assignment_line(line_read))
        {
            parse_statement(line_read, line_write);
            fputs(line_write, outc_fp);
        }
    }
    LOGD("TRANSLATE_PASS: %d", TRANSLATE_PASS);
    fputs("}\n", outc_fp);
}

bool is_file_empty(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size == 0;
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
        safe_exit();
    }
    if (is_file_empty(ml_fp))
    {
        LOGE("File %s is empty", g_argv[1]);
        safe_exit();
    }
    snprintf(outc_filename, sizeof(outc_filename), "ml-%d.c", getpid());
    outc_fp = fopen(outc_filename, "w");
    if (outc_fp == NULL)
    {
        safe_exit();
    }
}

void translate()
{
    check_files();
    write_header();
    write_utils();
    write_ml_definitions();
    write_ml_executions();
    write_main();
    close_files();
    if (!TRANSLATE_PASS)
        safe_exit();
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

double add(double a, double b)
{
    return a + b;
}

int main(int argc, char **argv)
{
    check_args(argc, argv);
    translate();
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