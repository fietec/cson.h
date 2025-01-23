#ifndef _CSON_PARSER_H
#define _CSON_PARSER_H

#include "Cson.h"
#include "cleks.h"

typedef struct{
    CleksTokens *tokens;
    size_t index;
    // TODO: add line and column for error messages
} CsonParser;

Cson* Cson_parse(char *buffer, size_t buffer_size);
Cson* Cson_read(char *filename);

#define CSON_PARSER_IMPLEMENTATION // temp
#ifdef CSON_PARSER_IMPLEMENTATION

/*
    Definitions for the lexer
*/

enum CsonTokens{
    CSON_MAP_OPEN,
    CSON_MAP_CLOSE,
    CSON_ARRAY_OPEN,
    CSON_ARRAY_CLOSE,
    CSON_MAP_SEP,
    CSON_ITER_SEP,
    CSON_TRUE,
    CSON_FALSE,
    CSON_NULL
};

static CleksTokenConfig CsonTokenConfig[] = {
    [CSON_MAP_OPEN] = {"CsonMapOpen: '{'" , "", '{'},
    [CSON_MAP_CLOSE] = {"CsonMapClose: '}'", "", '}'},
    [CSON_ARRAY_OPEN] = {"CsonArrayOpen: '['", "", '['},
    [CSON_ARRAY_CLOSE] = {"CsonArrayClose: ']'", "", ']'},
    [CSON_MAP_SEP] = {"CsonMapSep: ':'", "", ':'},
    [CSON_ITER_SEP] = {"CsonIterSet: ','", "", ','},
    [CSON_TRUE] = {"CsonTrue: true", "true", '\0'},
    [CSON_FALSE] = {"CsonFalse: false", "false", '\0'},
    [CSON_NULL] = {"CsonNull: null", "null", '\0'}
};

CleksComment CsonComments[] = {};
CleksString CsonStrings[] = {
    {'"', '"'}
};

CleksConfig CsonConfig = {
    .default_tokens = CleksDefaultTokenConfig,
    .default_token_count = CLEKS_TOKEN_COUNT,
    .custom_tokens = CsonTokenConfig,
    .custom_token_count = CLEKS_ARR_LEN(CsonTokenConfig),
    .whitespaces = " \n",
    .strings = CsonStrings,
    .string_count = CLEKS_ARR_LEN(CsonStrings),
    .comments = CsonComments,
    .comment_count = CLEKS_ARR_LEN(CsonComments),
    .flags = CLEKS_FLAG_DEFAULT
};

/*
    Parser
*/

CleksToken *cson_parser_next(CsonParser *parser);
CsonArray*  cson_parse_array(CsonParser *parser);
CsonMap*    cson_parse_map(CsonParser *parser);

unsigned long long cson_file_size(const char* file_path){
    struct stat file;
    if (stat(file_path, &file) == -1){
        cson_eprintln("Failed to open file!");
        return 0;
    }
    return (unsigned long long) file.st_size;
}

Cson* Cson_parse(char *buffer, size_t buffer_size){
    if (!buffer || buffer_size == 0) return NULL;
    CleksTokens *tokens = Cleks_lex(buffer, buffer_size, CsonConfig);
    if (!tokens){
        cson_eprintln("Failed to lex file content!");
        return NULL;
    }
    CsonParser parser = {.tokens=tokens, .index=0};
    Cson *cson = NULL;

    CleksToken *token = cson_parser_next(&parser);
    switch (token->type){
        case CSON_ARRAY_OPEN:{
            CsonArray *c_array = cson_parse_array(&parser);
            if (!c_array) {goto cson_parse_exit;}
            cson = Cson_from_array(c_array);
        } break;
        case CSON_MAP_OPEN:{
            CsonMap *c_map = cson_parse_map(&parser);
            if (!c_map) {goto cson_parse_exit;}
            cson = Cson_from_map(c_map);
        } break;
        default:{
            cson_eprintln("[%s] Token: %u, Expected tokens [CSON_ARRAY_OPEN, CSON_MAP_OPEN], found %s!", CsonErrorStrings[CsonError_UnexpectedToken], parser.index, Cleks_token_to_string(token, CsonConfig));
            goto cson_parse_exit;
        }
    }
    if (parser.index < parser.tokens->size-1){
        #ifdef CSON_ALLOW_WARNINGS
        cson_warning("This section is unreachable: Token: %u!", parser.index);
        #else
        cson_eprintln("This section is unreachable: Token: %u!", parser.index);
        Cson_free(cson);
        cson = NULL;
        #endif
    }
    cson_parse_exit:
    Cleks_free_tokens(tokens);
    return cson;
}

Cson* Cson_read(char *filename){
    FILE *file = fopen(filename, "r");
    if (!file){
        cson_eprintln("Trying to read not-existing file: \"%s\"", filename);
        return NULL;
    }
    uint64_t file_size = cson_file_size(filename);
    char *file_content = (char*) calloc(file_size+1, sizeof(*file_content));
    if (!file_content){
        fclose(file);
        cson_eprintln("Out of memory!");
        exit(1);
    }
    fread(file_content, 1, file_size, file);
    fclose(file);
    Cson *cson = Cson_parse(file_content, file_size);
    free(file_content);
    return cson;
}

// private functions

CleksToken* cson_parser_next(CsonParser *parser)
{
    if (!parser || parser->index >= parser->tokens->size) return NULL;
    return parser->tokens->items[parser->index++];
}

CsonArray* cson_parse_array(CsonParser *parser)
{
    if (!parser) return NULL;
    CsonArray *array = CsonArray_create();
    bool sep_found = true;
    while (true){
        CleksToken *token = cson_parser_next(parser);
        if (!sep_found){
            switch (token->type){
                case CSON_ITER_SEP:{
                    sep_found = true;
                    continue;
                }
                case CSON_ARRAY_CLOSE:{
                    return array;
                }
                default:{
                    cson_eprintln("[%s] Token %u: Expected [CSON_ITER_SEP, CSON_ARRAY_CLOSE], found %s!", CsonErrorStrings[CsonError_UnexpectedToken], parser->index-1, Cleks_token_to_string(token, CsonConfig));
                    CsonArray_free(array);
                    return NULL;
                }
            }
        }
        switch (token->type){
            case CLEKS_INT: {
                CsonArray_append(array, Cson_from_int(atoi(token->value)));
            } break;
            case CLEKS_FLOAT: {
                CsonArray_append(array, Cson_from_double(atof(token->value)));
            } break;
            case CLEKS_STRING:{
                CsonArray_append(array, Cson_from_string(token->value));
            } break;
            case CSON_TRUE:{
                CsonArray_append(array, Cson_from_boolean(true));
            } break;
            case CSON_FALSE:{
                CsonArray_append(array, Cson_from_boolean(false));
            } break;
            case CSON_NULL:{
                CsonArray_append(array, Cson_from_null());
            } break;
            case CSON_ARRAY_OPEN:{
                CsonArray *c_array = cson_parse_array(parser);
                if (!c_array){
                    CsonArray_free(array);
                    return NULL;
                }
                CsonArray_append(array, Cson_from_array(c_array));
            } break;
            case CSON_MAP_OPEN:{
                CsonMap *c_map = cson_parse_map(parser);
                if (!c_map){
                    CsonArray_free(array);
                    return NULL;
                }
                CsonArray_append(array, Cson_from_map(c_map));
            } break;
            default:{
                cson_eprintln("[%s] Token: %u, Expected value, found %s!", CsonErrorStrings[CsonError_UnexpectedToken], parser->index-1, Cleks_token_to_string(token, CsonConfig));
                CsonArray_free(array);
                return NULL;
            }
        }
        sep_found = false;
    }
    return NULL;
}

CsonMap* cson_parse_map(CsonParser *parser)
{
    if (!parser) return NULL;
    CsonMap *map = CsonMap_create();
    while (true){
        CleksToken *token = cson_parser_next(parser);
        
    }
}

#endif // CSON_PARSER_IMPLEMENTATION
#endif // _CSON_PARSER_H