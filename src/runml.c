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
// ======================== Write this utils to code file ========================
bool is_intd(double num)
{
    return (double)((int)num) == num;
}

bool is_intf(float num)
{
    return (float)((int)num) == num;
}

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
// ================================================================
#else
#define LOGD(fmt, ...)
#endif // DEBUG_MODE
#define ERROR_START "![ERROR:line:%d,ml:%d]: {"
#define ERROR_END "}\n"
#define LOGE(fmt, ...) fprintf(stderr, ERROR_START fmt ERROR_END, __LINE__, CUR_ML_ROW, ##__VA_ARGS__)
#define MAX_SYMBOL_COUNT 50
#define MAX_SYMBOL_LENGTH 12

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
    char name[13];
    int num_of_paras;
    struct Function *parent; // null means global function.
} Function;

typedef struct Identifier
{
    bool initialized;  // only for VARIABLE, false set value to 0.0
    int id;            // for indexing.
    int row_of_define; // record the definition row.
    char *name;
    bool infunc;
    int parent_id;
    IDENTIFIER_TYPE type;
} Identifier;

Identifier g_symbols_table[MAX_SYMBOL_COUNT]; // global symbols table.
int g_symbols_count = 0;
#define MAIN -1
int g_func_id = MAIN;

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

void free_mem()
{
    for (int i = 0; i < g_symbols_count; i++)
    {
        if (g_symbols_table[i].name != NULL)
        {
            free(g_symbols_table[i].name);
            g_symbols_table[i].name = NULL;
        }
    }
}

void safe_exit()
{
    LOGE("An error occurred, exiting...");
    close_files();
    free_mem();
    exit(EXIT_FAILURE);
}

char *trim(char *str)
{
    char *end;
    // trim leading sapce
    while (isspace((unsigned char)*str) || *str == '\t')
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;

    // trim trailing space
    while (end > str && (isspace((unsigned char)*end) || *end == '\t'))
        end--;
    *(end + 1) = '\0';
    return str;
}

bool is_operator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/';
}

int is_special_char(char c)
{
    return c == '(' || c == ')' || c == ',';
}

bool is_number(char *str)
{
    char *badchar;
    strtod(str, &badchar);
    return *badchar == '\0';
}

bool is_keyword(char *str)
{
    return strcmp(str, "return") == 0 || strcmp(str, "print") == 0 || strcmp(str, "function") == 0;
}

bool is_cmdline_arg(char *str)
{
    if (strncmp(str, "arg", 3) != 0)
    {
        return false;
    }
    char *end;
    strtol(str + 3, &end, 10);
    return *end == '\0';
}

bool is_valid_identifier_name(char *name)
{
    name = trim(name);
    int n = strlen(name);
    if (n > MAX_SYMBOL_LENGTH || n <= 0)
    {
        LOGE("Identifier %s should be 1 to 12 long", name);
        TRANSLATE_PASS = false;
        return false;
    }
    for (int i = 0; i < n; i++)
    {
        if (!('a' <= name[i] && name[i] <= 'z'))
        {
            return false;
        }
    }
    return true;
}

Identifier *find_identifier(char *name, IDENTIFIER_TYPE type)
{
    if (name[0] == 0)
    {
        LOGD("Identifier name is NULL");
        return NULL;
    }
    for (int i = 0; i < g_symbols_count; i++)
    {
        Identifier symbol = g_symbols_table[i];
        if (symbol.name == NULL)
        {
            continue;
        }
        if (symbol.type == type && strcmp(name, symbol.name) == 0)
        {
            // same type and name
            if (symbol.parent_id == MAIN && symbol.row_of_define <= CUR_ML_ROW)
            {
                // global variable
                return &g_symbols_table[i];
            }
            if (symbol.parent_id == g_func_id && symbol.row_of_define <= CUR_ML_ROW)
            {
                // function local variable
                return &g_symbols_table[i];
            }
        }
    }
    return NULL;
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

void replace(const char *str, const char *old, const char *new, char *result)
{
    char *pos;
    const char *cur = str;
    char buf[1024];
    int lold = strlen(old);
    result[0] = 0;
    while ((pos = strstr(cur, old)) != NULL)
    {
        strncpy(buf, cur, pos - cur);
        buf[pos - cur] = 0;
        strcat(result, buf);
        strcat(result, new);
        cur = pos + lold;
    }
    strcat(result, cur);
}

/**
 *      -> argv[0]      "runml"
 * arg0 -> argv[1]      "program.ml"
 * arg1 -> argv[2]      "number1"
 * arg2 -> argv[3]      "number2"
 * argN -> argv[N + 1]  "numberN"
 */
void replace_cmdline_args(const char *line, char *res)
{
    memset(res, 0, 1024);
    const char *tmp = line;
    for (int i = 2; i < g_argc; i++)
    {
        char argN[64];
        snprintf(argN, sizeof(argN), "arg%d", i - 1);
        if (strstr(line, argN) == NULL)
        {
            continue;
        }
        if (!is_number(g_argv[i]))
        {
            LOGE("%s is not a valid number", argN);
            TRANSLATE_PASS = false;
            return;
        }
        memset(res, 0, 1024);
        replace(tmp, argN, g_argv[i], res);
        tmp = res;
    }
    if (res[0] == 0)
        strcpy(res, line);
}

/**
 * Extract tokens from an expression
 */
void tokenize(const char *expr, char tokens[][64], int *token_count)
{
    int len = strlen(expr);
    int i = 0;
    *token_count = 0;
    while (i < len)
    {
        if (isspace(expr[i]))
        {
            i++;
            continue;
        }
        if (is_operator(expr[i]) || is_special_char(expr[i]))
        {
            tokens[*token_count][0] = expr[i];
            tokens[(*token_count)++][1] = '\0';
            i++;
        }
        else if (isalnum(expr[i]))
        {
            int start = i;
            while (i < len && (isalnum(expr[i]) || expr[i] == '.'))
            {
                i++;
            }
            strncpy(tokens[*token_count], &expr[start], i - start);
            tokens[(*token_count)++][i - start] = '\0';
        }
        else
        {
            LOGE("Unknown character %c", expr[i]);
            TRANSLATE_PASS = false;
            i++;
        }
    }
}

/**
 * @return true if the expression is valid, otherwise false.
 * @param expr: the expression to be parsed.
 * @param result: translated expr.
 */
bool parse_expr(char *expr, char *result)
{
    LOGD("expression: %s", expr);
    memset(result, 0, 512);
    char tokens[MAX_SYMBOL_COUNT][64];
    int token_count = 0;
    int brackets = 0;
    tokenize(expr, tokens, &token_count);
    if (token_count == 0)
    {
        TRANSLATE_PASS = false;
        return false;
    }
    for (int i = 0; i < token_count; i++)
    {
        if (strcmp(tokens[i], "(") == 0)
        {
            // TODO verify
            strcat(result, tokens[i]);
            LOGD("Result: [%s]", result);

            brackets++;
        }
        else if (strcmp(tokens[i], ")") == 0)
        {
            brackets--;
            if (brackets < 0)
            {
                LOGE("Invalid brackets");
                TRANSLATE_PASS = false;
                return false;
            }
            else
            {
                strcat(result, tokens[i]);
                LOGD("Result: [%s]", result);
            }
        }
        else if (strcmp(tokens[i], ",") == 0)
        {
            if (i == 0 || i == token_count - 1)
            {
                LOGE("Syntax error, missing operand near [%s], %s %d %d %s %s", tokens[i], expr, i, token_count - 1, tokens[i - 1], tokens[i + 1]);
                TRANSLATE_PASS = false;
                return false;
            }
            else
            {
                strcat(result, tokens[i]);
                LOGD("Result: [%s]", result);
            }
        }
        else if (is_operator(tokens[i][0]))
        {
            if (i == 0 || i == token_count - 1)
            {
                LOGE("Syntax error, missing operand near %s", tokens[i]);
                TRANSLATE_PASS = false;
                return false;
            }
            else
            {
                strcat(result, tokens[i]);
                LOGD("Result: [%s]", result);
            }
        }
        else if (is_keyword(tokens[i]))
        {
            LOGE("Syntax error, keyword %s in exprssion is not allowed here", tokens[i]);
            TRANSLATE_PASS = false;
            return false;
        }
        else if (is_number(tokens[i]))
        {
            strcat(result, tokens[i]);
            LOGD("Constant Number: %s, res:[%s]", tokens[i], result);
        }
        else if (is_cmdline_arg(tokens[i]))
        {
            int N = atoi(tokens[i] + 3);
            if (N < 1 && N >= g_argc - 1)
            {
                LOGE("Invalid command line argument %s", tokens[i]);
                TRANSLATE_PASS = false;
                return false;
            }
            strcat(result, g_argv[N + 1]);
        }
        else if (is_valid_identifier_name(tokens[i]))
        {
            if (i != token_count - 1 && strcmp(tokens[i + 1], "(") == 0)
            {
                // is a function call
                Identifier *func = find_identifier(tokens[i], FUNCTION_NAME);
                if (func == NULL)
                {
                    LOGE("Syntax error, function [%s] is not defined", tokens[i]);
                    TRANSLATE_PASS = false;
                    return false;
                }
                strcat(result, tokens[i]);
                LOGD("Result: [%s]", result);
            }
            else
            {
                Identifier *para = find_identifier(tokens[i], PARAMETER);
                if (para != NULL)
                {
                    strcat(result, tokens[i]);
                    LOGD("Result: [%s]", result);
                    continue;
                }
                Identifier *var = find_identifier(tokens[i], VARIABLE);
                strcat(result, var == NULL ? "0 " : tokens[i]);
                LOGD("Result: [%s]", result);
            }
        }
        else
        {
            LOGE("Syntax error, unknown token %s", tokens[i]);
            TRANSLATE_PASS = false;
            return false;
        }
    }
    if (brackets != 0)
    {
        LOGE("Syntax error, Unmatched brackets (");
        TRANSLATE_PASS = false;
        return false;
    }
    if (result[0] == 0)
    {
        strcpy(result, expr);
    }
    LOGD("Valid expression: %s", result);
    return true;
}

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

void write_header()
{
    char *header = "#include <stdio.h>\n"
                   "#include <stdbool.h>\n"
                   "#include <math.h>\n";
    fputs(header, outc_fp);
}

void write_utils()
{
    const char *utils = "bool is_intd(double num)\n"
                        "{\n"
                        "    return (double)((int)num) == num;\n"
                        "}\n"
                        "\n"
                        "bool is_intf(float num)\n"
                        "{\n"
                        "    return (float)((int)num) == num;\n"
                        "}\n"
                        "\n"
                        "#define ISINT(expr) _Generic((expr), \\\n"
                        "    short: true,                     \\\n"
                        "    int: true,                       \\\n"
                        "    long: true,                      \\\n"
                        "    long long: true,                 \\\n"
                        "    float: is_intf(expr),            \\\n"
                        "    double: is_intd(expr))\n"
                        "\n"
                        "#define FMT(expr) _Generic((expr), \\\n"
                        "    short: \"%hd\\n\",                \\\n"
                        "    int: \"%d\\n\",                   \\\n"
                        "    long: \"%ld\\n\",                 \\\n"
                        "    long long: \"%lld\\n\",           \\\n"
                        "    float: \"%f\\n\",                 \\\n"
                        "    double: \"%f\\n\")\n"
                        "\n"
                        "#define PRINT(expr)                        \\\n"
                        "    if (ISINT(expr))                       \\\n"
                        "    {                                      \\\n"
                        "        long long res = (long long)(expr); \\\n"
                        "        printf(FMT(res), res);             \\\n"
                        "    }                                      \\\n"
                        "    else                                   \\\n"
                        "    {                                      \\\n"
                        "        printf(FMT(expr), (expr));         \\\n"
                        "    }\n";
    fputs(utils, outc_fp);
}

void write_main()
{
    char *main = "int main(int argc, char **argv)\n"
                 "{\n"
                 "\tml();\n"
                 "\treturn 0;\n"
                 "}";
    fputs(main, outc_fp);
}

bool is_func_define_line(char read[])
{
    // function definition line, must can be tokenized with separator " ", otherwise it is not a valid function define.
    char line[1024];
    strcpy(line, read);
    char *first_token = strtok(line, " ");
    if (first_token == NULL)
    {
        return false;
    }
    return strcmp(first_token, "function") == 0;
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

bool is_assignment_line(char read[])
{
    char line[1024];
    strcpy(line, read);
    char *trimed = trim(read);
    strcpy(line, trimed);
    char *pos = strstr(line, "<-");
    if (pos == NULL)
    {
        return false;
    }
    char *var = strtok(line, "<-");
    if (var == NULL)
    {
        LOGE("Assignment line is missing left side");
        TRANSLATE_PASS = false;
        return false;
    }
    if (!is_valid_identifier_name(var))
    {
        LOGE("Assignment line left side is not a valid identifier");
        TRANSLATE_PASS = false;
        return false;
    }
    Identifier *idn = find_identifier(var, VARIABLE);
    if (idn == NULL)
    {
        // new variable identifier
        Identifier new_var = {
            .id = g_symbols_count++,
            .name = malloc(strlen(var) + 1),
            .row_of_define = CUR_ML_ROW,
            .parent_id = g_func_id,
            .type = VARIABLE,
            .initialized = true,
        };
        strcpy(new_var.name, var);
        g_symbols_table[new_var.id] = new_var;
        LOGD("New variable [%s] defined, id: [%d], parent: %d", new_var.name, new_var.id, new_var.parent_id);
    }
    char *expr = strtok(NULL, "<-");
    if (expr == NULL)
    {
        LOGE("Assignment line is missing right side");
        TRANSLATE_PASS = false;
        return false;
    }
    return true;
}

void translate_assignment_line(char read[], char write[])
{
    char *pos = strstr(read, "<-");
    char translated[512];
    if (!parse_expr(pos + strlen("<-"), translated))
    {
        TRANSLATE_PASS = false;
        return;
    }
    char *left = strtok(read, "<-");
    // char *right = strtok(NULL, "<-");
    snprintf(write, 1024, "double %s = %s;\n", left, translated);
}

bool is_print_line(char read[])
{
    char line[512];
    char *trimed = trim(read);
    strcpy(line, trimed);
    char *token = strtok(line, " ");
    if (strcmp(token, "print") != 0)
    {
        LOGD("Line: [%s] Token [%s] is not print", line, token);
        return false;
    }
    return true;
}

void translate_print_line(char *read, char *write)
{
    char *pos = strstr(read, "print");
    char translated[512];
    if (!parse_expr(pos + strlen("print"), translated))
    {
        TRANSLATE_PASS = false;
        return;
    }
    snprintf(write, 1024, "\tPRINT(%s);\n", translated);
}

bool is_return_line(char *read)
{
    char line[1024];
    char *trimed = trim(read);
    strcpy(line, trimed);
    char *token = strtok(line, " ");
    if (token == NULL)
    {
        return false;
    }
    if (strcmp(token, "return") != 0)
    {
        return false;
    }
    return true;
}

void translate_return_line(char *read, char *write)
{
    char *pos = strstr(read, "return");
    char translated[512];
    if (!parse_expr(pos + strlen("return"), translated))
    {
        TRANSLATE_PASS = false;
        return;
    }
    snprintf(write, 1024, "\treturn %s;\n", translated);
}

void other_statement(char *read, char *write)
{
    char translated[512];
    if (!parse_expr(read, translated))
    {
        LOGE("Other statement [%s] is not a valid expression", read);
        strcpy(write, "\n");
        TRANSLATE_PASS = false;
        return;
    }
    snprintf(write, 1024, "%s;\n", translated);
}

/**
 * @return funciton id
 */
int process_func_define(char read[], char write[])
{
    char line[1024];
    strcpy(line, read);
    strcpy(write, "double ");
    char *token;
    token = strtok(line, " "); // function
    token = strtok(NULL, " "); // funcname
    if (token == NULL || token[0] == 0)
    {
        LOGE("Function name is missing");
        TRANSLATE_PASS = false;
        return MAIN;
    }
    if (!is_valid_identifier_name(token))
    {
        LOGE("Function name %s is not a valid identifier", token);
        TRANSLATE_PASS = false;
        return MAIN;
    }
    Identifier funcname = {
        .id = g_symbols_count++,
        .name = malloc(strlen(token) + 1),
        .row_of_define = CUR_ML_ROW,
        .parent_id = MAIN,
        .type = FUNCTION_NAME,
        .initialized = false,
    };
    strcpy(funcname.name, token);
    g_symbols_table[funcname.id] = funcname;

    strcat(write, token);
    strcat(write, "(");
    while (token != NULL) // para1 para2 ... paran
    {
        LOGD("Token: %s", token);
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            break;
        }
        Identifier para = {
            .id = g_symbols_count++,
            .name = malloc(strlen(token) + 1),
            .row_of_define = CUR_ML_ROW,
            .parent_id = funcname.id,
            .type = PARAMETER,
        };
        strcpy(para.name, token);
        g_symbols_table[para.id] = para;
        if (token != NULL)
        {
            strcat(write, "double ");
            strcat(write, token);
            strcat(write, ", ");
        }
    }
    // rm trailing ","
    write[strlen(write) - 2] = '\0';
    strcat(write, ")\n{\n");
    return funcname.id;
}

void parse_statement(char *read, char *write)
{
    rm_comment(read);
    if (is_assignment_line(read))
    {
        translate_assignment_line(read, write);
    }
    else if (is_print_line(read))
    {
        translate_print_line(read, write);
    }
    else if (is_return_line(read))
    {
        translate_return_line(read, write);
    }
    else
    {
        other_statement(read, write);
    }
}

/**
 * 1. Tokenize per line
 * 2. First token must be "function" keyword.
 * 3. Record line number of the start of the function body
 * 4. The following tokens identifiers which should be valid identifier. If not, LOGE, exit(EXIT_FAILURE)
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
    char read[1024] = {0};
    char write[1024] = {0};
    CUR_ML_ROW = 0;
    fseek(ml_fp, 0, SEEK_SET);
    bool processing_func = false;
    while (fgets(read, sizeof(read), ml_fp) != NULL)
    {
        CUR_ML_ROW++;
        if (is_blank_line(read) || is_comment_line(read))
        {
            continue;
        }
        rm_comment(read);
        if (!startwith_tab(read))
        {
            if (processing_func)
            {
                // end of function body
                strcpy(write, "};\n");
                fputs(write, outc_fp);
                processing_func = false;
            }
            if (is_assignment_line(read))
            {
                translate_assignment_line(read, write);
                fputs(write, outc_fp);
            }
            else if (is_func_define_line(read))
            {
                processing_func = true;
                g_func_id = process_func_define(read, write);
                fputs(write, outc_fp);
            } // else other global statements, handle in write_ml_executions()
        }
        else
        {
            if (processing_func)
            {
                parse_statement(read, write);
                fputs(write, outc_fp);
            }
            else
            {
                LOGE("Program lines commence at the left-hand margin (no indentation).");
            }
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
    char read[1024] = {0};
    char write[1024] = {0};
    fputs("void ml()\n{\n", outc_fp);
    CUR_ML_ROW = 0;
    g_func_id = MAIN;
    while (fgets(read, sizeof(read), ml_fp) != NULL)
    {
        CUR_ML_ROW++;
        if (is_blank_line(read) || is_comment_line(read))
            continue;
        if (!startwith_tab(read) && !is_assignment_line(read) && !is_func_define_line(read))
        {
            parse_statement(read, write);
            fputs(write, outc_fp);
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

int main(int argc, char **argv)
{
    check_args(argc, argv);
    translate();
    compile();
    execute();
#ifndef DEBUG_MODE
    clean();
#endif
    return 0;
}