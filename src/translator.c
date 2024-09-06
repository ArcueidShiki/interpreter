#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Can not openfile");
        exit(EXIT_FAILURE);
    }

    // get the size of the file
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // allocate buffer to store the content of the file
    char *buffer = malloc(size + 1);
    if (buffer)
    {
        fread(buffer, 1, size, file);
        buffer[size] = '\0';
    }

    fclose(file);
    return buffer;
}

typedef enum
{
    IDENTIFIER,
    NUMBER,
    ASSIGN,
    PRINT,
    RETURN,
    PLUS,
    MINUS,
    MULTIPLICATION,
    DIVISION,
    LBRACKET,
    RBRACKET,
    COMMA,
    SEMICOLON,
    COMMENT,
    UNKNOWN,
    TOKEN_EOF,
} TokenType;

typedef struct
{
    TokenType type;
    char *text;
} Token;

Token *create_token(TokenType type, const char *start, int length)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->text = (char *)malloc(length + 1);
    strncpy(token->text, start, length);
    token->text[length] = '\0';
    return token;
}
Token **tokenize(const char *code, int *token_count)
{
    const char *start = code;
    Token **tokens = NULL;
    *token_count = 0;

    while (*start)
    {
        if (isspace(*start))
        {
            start++;
            continue;
        }

        TokenType type = UNKNOWN;
        int length = 0;

        if (isalpha(*start))
        {
            const char *id_start = start;
            while (isalnum(*start))
            {
                start++;
            }
            length = start - id_start;
            type = IDENTIFIER;
        }
        else if (*start == '<' && *(start + 1) == '-')
        {
            length = 2;
            type = ASSIGN;
        }
        else if (*start == '(')
        {
            length = 1;
            type = LBRACKET;
        }
        else if (*start == ')')
        {
            length = 1;
            type = RBRACKET;
        }

        if (type != UNKNOWN)
        {
            tokens = realloc(tokens, sizeof(Token *) * (*token_count + 1));
            tokens[*token_count] = create_token(type, start, length);
            (*token_count)++;
        }

        start += length;
    }

    tokens = realloc(tokens, sizeof(Token *) * (*token_count + 1));
    tokens[*token_count] = create_token(TOKEN_EOF, "", 0);
    (*token_count)++;

    return tokens;
}