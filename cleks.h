#ifndef _CLEKS_H
#define _CLEKS_H

/*

          _      _          _     
         | |    | |        | |    
      ___| | ___| | _____  | |__  
     / __| |/ _ \ |/ / __| | '_ \ 
    | (__| |  __/   <\__ \_| | | |
     \___|_|\___|_|\_\___(_)_| |_|    (by Constantijn de Meer)
                                  
                              
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

/* Lexing flags */
#define CLEKS_FLAG_DEFAULT     0x0 // all default features enabled
#define CLEKS_FLAG_NO_INTEGERS 0x1 // integers are not recognized by the lexer
#define CLEKS_FLAG_NO_FLOATS   0x2 // floats are not recognized by the lexer

/* Default tokens */
typedef enum{
    CLEKS_TOKEN_COUNT = 4,                   // no token, but an indicator of the number of default tokens
    CLEKS_STRING = -CLEKS_TOKEN_COUNT,       // everthing within string delimeters
    CLEKS_WORD,                              // un unknown word
    CLEKS_INT,                               // an integer value
    CLEKS_FLOAT,                             // an floating point value (catches integers when CLEKS_INT is disabled)
} CleksDefaultToken;

/* Token configs */
typedef struct{
    char* print_string; // the string to print for Cleks_print_tokens
    char* word;         // the string defining a word, "" for non-words
    char  symbol;       // the character defining a symbol, '\0' non-symbols
} CleksTokenConfig;

static CleksTokenConfig CleksDefaultTokenConfig[] = {
    [CLEKS_STRING+CLEKS_TOKEN_COUNT] = {"String", "", '\0'},
    [CLEKS_WORD+CLEKS_TOKEN_COUNT] = {"Word", "", '\0'},
    [CLEKS_INT+CLEKS_TOKEN_COUNT] = {"Integer", "", '\0'},
    [CLEKS_FLOAT+CLEKS_TOKEN_COUNT] = {"Float", "", '\0'}
};

/* Customization */
typedef struct{
    const char* const start_del;
    const char* const end_del;
} CleksComment;

typedef struct{
    const char const start_del;
    const char const end_del;
} CleksString;

/* Configuration of the lexer and description of input format*/
typedef struct{
    CleksTokenConfig *default_tokens;      // it is highly encouraged to use CleksDefaultTokenConfig for this, overwise the lexing may fail.
    size_t default_token_count;            // when using CleksDefaultTokenConfig, provide CLEKS_TOKEN_COUNT
    CleksTokenConfig *custom_tokens;       // provided your custom tokens
    size_t custom_token_count;             // the number of custom tokens (tip: use CLEKS_ARR_LEN)
    const char* const  whitespaces;        // a string with each character defining a whitespace
    CleksString *strings;                  // an array of string definitions, each with its start and end char delimeter
    size_t string_count;                   // the number of strings (tip: use CLEKS_ARR_LEN)
    CleksComment *comments;                // an array of comment definitions, each with its start and end char* delimeter
    size_t comment_count;                  // the number of comments (tip: use CLEKS_ARR_LEN)
    uint8_t flags;                         // a mask for customizing the lexing behavior (so far: CLEKS_NO_INTEGERS, CLEKS_NO_FLOATS). The default is CLEKS_DEFAULT
} CleksConfig;    


/* Output */
typedef int CleksTokenType;
typedef struct{
    CleksTokenType type;
    char *value;
} CleksToken;

typedef struct{
    CleksToken **items;
    size_t size;
    size_t capacity;
    CleksConfig config;
} CleksTokens;

/* Internal lexing state*/
typedef struct{
    char *buffer;
    size_t buffer_size;
    size_t index;
} Clekser;

/* Macros */

#define CLEKS_NOT_FOUND -1
#define CLEKS_TOKENS_RESIZE_FACTOR 2

#define CLEKS_TOKEN_IS_VALID(token) ((token) != NULL && (token->type) >= -CLEKS_TOKEN_COUNT) // Checks whether a token is valid (does not check upper bound for custom tokens)
#define CLEKS_TOKEN_IS_CUSTOM(token) ((token)->type >= 0) // Checks whether a token is custom

#define CLEKS_ARR_LEN(arr) (arr != NULL ? (sizeof((arr))/sizeof((arr)[0])) : 0) // get the length of an array
#define CLEKS_ANSI_END "\e[0m" // reset ansi color
#define CLEKS_ANSI_RGB(r, g, b) ("\e[38;2;" #r ";" #g ";" #b "m") // set ansi color to rgb value

/* Debugging */
#define cleks_info(msg, ...) (printf("%s%s:%d: " msg CLEKS_ANSI_END "\n", CLEKS_ANSI_RGB(255, 255, 255), __FILE__, __LINE__, ## __VA_ARGS__))
#define cleks_debug(msg, ...) (printf("%s%s:%d: [DEBUG] " msg CLEKS_ANSI_END "\n", CLEKS_ANSI_RGB(255, 124, 0), __FILE__, __LINE__, ## __VA_ARGS__))
#define cleks_error(msg, ...) (fprintf(stderr, "%s%s:%d: [ERROR] " msg CLEKS_ANSI_END "\n", CLEKS_ANSI_RGB(255, 0, 0), __FILE__, __LINE__, ## __VA_ARGS__))

#define cleks_assert(statement, msg, ...) do{if (!(statement)) {cleks_error(msg, ##__VA_ARGS__); exit(1);}} while (0);

/* Function definitions and implementations */

void cleks_print_token(CleksToken *token, CleksConfig config);
static char* cleks_strndup(char *s, size_t n);
static bool cleks_str_is_int(char *s);
static bool cleks_str_is_float(char *s);

CleksTokens* Cleks_create_tokens(size_t capacity, CleksConfig config)
{
    CleksTokens *tokens = (CleksTokens*) calloc(1, sizeof(*tokens));
    cleks_assert(tokens != NULL, "Failed to allocate CleksTokens!");
    tokens->items = (CleksToken**) calloc(capacity, sizeof(CleksToken*));
    if (tokens->items == NULL){
        free(tokens);
        cleks_error("Failed to allocate CleksTokens items!");
        exit(1);
    }
    tokens->capacity = capacity;
    memcpy(&tokens->config, &config, sizeof(CleksConfig));
    return tokens;
}

void Cleks_free_tokens(CleksTokens *tokens)
{
    if (tokens != NULL){
        if (tokens->items != NULL){
            for (size_t i=0; i<tokens->size; ++i){
                CleksToken *token = tokens->items[i];
                if (token != NULL){
                    free(token->value);
                    free(token);
                }
            }
            free(tokens->items);
        }
        free(tokens);
    }
}

void Cleks_append_token(CleksTokens *tokens, CleksTokenType token_type, char *token_value)
{
    if (!tokens){
        cleks_error("Invalid CleksTokens pointer!");
        exit(1);
    } 
    if (tokens->size >= tokens->capacity){
        size_t new_capacity = tokens->capacity * CLEKS_TOKENS_RESIZE_FACTOR;
        CleksToken **new_items = (CleksToken**) realloc(tokens->items, new_capacity*sizeof(CleksToken*));
        cleks_assert(new_items != NULL, "Failed to resize CleksTokens!");
        tokens->items = new_items;
        tokens->capacity = new_capacity;
    }
    CleksToken *token = calloc(1, sizeof(CleksToken));
    cleks_assert(token != NULL, "Failed to allocate new CleksToken!");
    token->type = token_type;
    token->value = token_value; 
    tokens->items[tokens->size++] = token;
}

int Cleks_char_to_token(char c, CleksTokenConfig *tokens, size_t size)
{
    for (size_t i=0; i<size; ++i){
        if (c == tokens[i].symbol) return i;
    }
    return CLEKS_NOT_FOUND;
}

int Cleks_char_in_string_dels(char c, CleksString *string_dels, size_t size)
{
    for (size_t i=0; i<size; ++i){
        if (c == string_dels[i].start_del) return 0;
    }
    return 1;
}

int Cleks_char_in_string(char c, const char *array)
{
    for (size_t i=0; i<strlen(array); ++i){
        if (c == array[i]) return 0;
    }
    return 1;
}

int Cleks_lex_word(Clekser *clekser, CleksTokens *tokens, CleksConfig config)
{
    cleks_assert(clekser != NULL && tokens != NULL, "Invalid Arguments: clekser=%p, tokens=%p", clekser, tokens);
    size_t word_start = clekser->index;
    while (clekser->index < clekser->buffer_size){
        char c = clekser->buffer[clekser->index];
        if (Cleks_char_in_string_dels(c, config.strings, config.string_count)==0 || Cleks_char_in_string(c, config.whitespaces)==0 || Cleks_char_to_token(c, config.custom_tokens, config.custom_token_count) != CLEKS_NOT_FOUND) break;
        clekser->index += 1;
    }
    for (size_t i=0; i<config.custom_token_count; ++i){
        if (strncmp(clekser->buffer + word_start, config.custom_tokens[i].word, clekser->index - word_start) == 0){
            // words match
            Cleks_append_token(tokens, (CleksTokenType) i, NULL);
            return 0;
        }
    }
    char *word_value = cleks_strndup(clekser->buffer + word_start, clekser->index - word_start);
    if (word_value == NULL){
        cleks_error("Failed to allocate word value!");
        return 1;
    }
    if ((config.flags & CLEKS_FLAG_NO_INTEGERS) == 0 && cleks_str_is_int(word_value)){
        Cleks_append_token(tokens, CLEKS_INT, word_value);
        return 0;
    }
    else if ((config.flags & CLEKS_FLAG_NO_FLOATS) == 0 && cleks_str_is_float(word_value)){
        Cleks_append_token(tokens, CLEKS_FLOAT, word_value);
        return 0;
    }
    Cleks_append_token(tokens, CLEKS_WORD, word_value);
    return 0;
}

int Cleks_lex_string(Clekser *clekser, CleksTokens *tokens, CleksConfig config, char str_end_del)
{
    cleks_assert(clekser != NULL && tokens != NULL, "Invalid Arguments: clekser=%p, tokens=%p", clekser, tokens);
    size_t str_start = clekser->index;
    char c;
    while (clekser->index < clekser->buffer_size){
        c = clekser->buffer[++(clekser->index)];
        cleks_assert(c != '\0', "[PARSING] Unclosed string delimeters at index %u!", str_start);
        if (c == str_end_del) break;
    }

    size_t str_len = clekser->index - str_start;
    char *str_value = (char*) calloc(str_len+1, sizeof(char));
    if (str_value == NULL){
        cleks_error("Failed to allocate string value!");
        return 1;
    }
    
    // handle escape codes
    char *buff_start = clekser->buffer + str_start;
    char *iB = buff_start;
    char *iS = str_value;
    while (iB - buff_start < str_len){
        if (*iB == '\\'){
            switch(*++iB){
                case '\'': *iS = 0x27; break;
                case '"':  *iS = 0x22; break;
                case '?':  *iS = 0x3f; break;
                case '\\': *iS = 0x5c; break;
                case 'a':  *iS = 0x07; break;
                case 'b':  *iS = 0x08; break;
                case 'f':  *iS = 0x0c; break;
                case 'n':  *iS = 0x0a; break;
                case 'r':  *iS = 0x0d; break;
                case 't':  *iS = 0x09; break;
                case 'v':  *iS = 0x0b; break;
            }
        }
        else{
            *iS = *iB;
        }
        iB++;   
        iS++;
    }
    Cleks_append_token(tokens, CLEKS_STRING, str_value);
    return 0;
}

int Cleks_lex_comment(Clekser *clekser, CleksConfig config)
{
    cleks_assert(clekser != NULL, "Invalid arguments: clekser: %p!", clekser);
    for (size_t i=0; i<config.comment_count; ++i){
        const char *start_del = config.comments[i].start_del;
        bool equals = true;
        size_t temp_index = clekser->index;
        for (size_t j=0; j<strlen(start_del); ++j, ++temp_index){
            if (clekser->buffer[temp_index] != start_del[j]){
                equals = false;
                break;
            }
        }
        // skip until end_del
        if (!equals) continue;
        const char* end_del = config.comments[i].end_del;
        size_t del_size = strlen(end_del);
        while (temp_index < clekser->buffer_size){
            bool matches = true;
            for (size_t j=0; j<del_size; ++j){
                size_t ti = temp_index + j;
                if (ti >= clekser->buffer_size){
                    clekser->index = ti;
                    return 0;
                }
                if (clekser->buffer[ti] != end_del[j]){
                    matches = false;
                    break;
                }
            }
            if (matches){
                clekser->index = temp_index+del_size-1;
                return 0;
            }
            temp_index += 1;
        }
        return 1;
    }
    return 1;
}

CleksTokens* Cleks_lex(char *buffer, size_t buffer_size, CleksConfig config)
{
    cleks_assert(buffer != NULL && buffer_size != 0, "Invalid arguments!"); 
    Clekser clekser = {.buffer=buffer, .buffer_size=buffer_size, .index=0};
    CleksTokens *tokens = Cleks_create_tokens(16, config);
    char c;
    while (clekser.index <= buffer_size && (c = buffer[clekser.index]) != '\0'){
        int int_token;
        // try to lex string
        bool string_found = false;
        for (size_t i=0; i<config.string_count; ++i){
            if (c == config.strings[i].start_del){
                clekser.index += 1;
                if (Cleks_lex_string(&clekser, tokens, config, config.strings[i].end_del) == 1){
                    Cleks_free_tokens(tokens);
                    return NULL;
                }
                string_found = true;
                clekser.index += 1;
            }
        }
        if (string_found) continue;
        // try to lex custom symbol
        if ((int_token = Cleks_char_to_token(c, config.custom_tokens, config.custom_token_count)) != CLEKS_NOT_FOUND){
            Cleks_append_token(tokens, (CleksTokenType) int_token, NULL);
        }
        // try to lex whitespace
        else if (Cleks_char_in_string(c, config.whitespaces) == 0){}
        // try to lex comment
        else if (Cleks_lex_comment(&clekser, config) == 0){}
        // try to lex word
        else{
            if (Cleks_lex_word(&clekser, tokens, config) == 0){
                continue;
            }
            else{
                Cleks_free_tokens(tokens);
                return NULL;
            }
        }
        clekser.index += 1;
    }
    return tokens;
}

char* Cleks_token_to_string(CleksToken *token, CleksConfig config)
{
    if (token == NULL || !CLEKS_TOKEN_IS_VALID(token)) return NULL;
    if (CLEKS_TOKEN_IS_CUSTOM(token) && token->type < config.custom_token_count){
        return config.custom_tokens[token->type].print_string;
    }
    return config.default_tokens[token->type+config.default_token_count].print_string;
}

void cleks_print_token(CleksToken *token, CleksConfig config)
{
    if (!CLEKS_TOKEN_IS_VALID(token) || token->type >= (int) config.custom_token_count){
        cleks_error("Invalid token: token: %p, type: %d", token, (int) token->type);
        return;
    }
    printf("Token % 3d: %s", token->type, Cleks_token_to_string(token, config));
    if (token->value != NULL){
        printf(" \"%s\"", token->value);
    }
    putchar('\n');
}

void Cleks_print_tokens(CleksTokens *tokens)
{
    if (tokens == NULL){
        cleks_error("Invalid tokens given!");
        return;
    }
    printf("Token count: %u\n  ", tokens->size);
    for (size_t i=0; i<tokens->size; ++i){
        cleks_print_token(tokens->items[i], tokens->config);
        printf("  ");
    }
    putchar('\n');
}

/* String helper functions */

static char* cleks_strndup(char *s, size_t n)
{
    if (s == NULL) return NULL;
    char *n_str = (char*) calloc(n+1, sizeof(char));
    if (n_str != NULL){
        strncpy(n_str, s, n);
    }
    return n_str;
}

static bool cleks_str_is_int(char* s)
{
    if (!s) return false;
    char c;
    if (*s == '+' || *s == '-') s++;
    if (*s == '\0') return false;
    while ((c = *s++)){
        if (!isdigit(c)) return false;
    }
    return true;
}

static bool cleks_str_is_float(char* s)
{
    char* ep = NULL;
    strtod(s, &ep);
    return (ep && !*ep);
}

#endif // _CLEKS_H