#ifndef _CSON_PARSE_H
#define _CSON_PARSE_H

#include "Cson.h"

CsonError Cson_parse(CsonParser parser);
Cson* Cson_read(char *filename);

// #define CSON_PARSE_IMPLEMENTATION // DEBUG
#ifdef CSON_PARSE_IMPLEMENTATION 

typedef struct{
    char * buffer;
    size_t index;
    size_t line;
    size_t column;
} CsonParser;

#define CSON_TRY(expr) do{ int _n = (expr); if (_n != CsonError_Success){cson_eprintln("%s", CsonErrorStrings[_n]); return _n;}} while (0);
#define CSON_TRY_PARSE(expr) do {int _n = (expr); if (_n != CsonError_Success) {}} while(0);
#define cson_get_c(parser) (parser->buffer[parser->index])

CsonError CsonParse_substr(char *buffer, size_t buffer_size, CsonParser *parser, size_t from, size_t to)
{
    if (buffer == NULL || parser == NULL) return CsonError_Param;
    if (buffer_size <= to-from){
        cson_eprintln("Buffer is to small %d/%d", buffer_size, to-from);
        return CsonError_Param;
    }
    for (size_t i=0; i<to-from; ++i){
        buffer[i] = parser->buffer[from+i];
    }
    return CsonError_Success;
}

CsonError Cson_parse_number(CsonParser *parser)
{
    size_t start_i = parser->index;
    size_t i = start_i;
    bool loop = true;
    while (loop){
        switch(parser->buffer[i]){
            case '.':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':{
                i++;
            }break;
            default:{
                loop = false;
            }
        }
    }
    size_t val_len = i-start_i;
    char val_buffer[val_len];  
    CSON_TRY(CsonParse_substr(val_buffer, val_len, parser, start_i, i));
    cson_debug("%s", val_buffer);
}

CsonError Cson_skip_space(CsonParser *parser)
{
    if (!parser || !parser->buffer) return CsonError_Param; 
    while (true){
        switch (parser->buffer[parser->index]){
            case '\n':{
                parser->line ++;
                parser->column = 0;
            } break;
            case ' ': break;
            case '\0':{
                return CsonError_EndOfString;
            }
            default:{
                return CsonError_Success;
            }
        }
        parser->index ++;
        parser->column ++;
    }
}

CsonError Cson_parse(CsonParser *parser)
{
    if (!parser) return CsonError_Param;
    CSON_TRY(Cson_skip_space(parser));
    switch (cson_get_c(parser)){
        case '{':{
            CSON_TRY(cson_parse_map());
        } break;
        case '[': {
            CSON_TRY(cson_parse_array());
        } break;
        case '"': {
            CSON_TRY(cson_parse_string());
        } break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':{
            CSON_TRY(cson_parse_number());
        } break;
        default:{
            return CsonError_InvalidValue;
        }
    }
    return CsonError_Success;
}

Cson* Cson_read(char* filename)
{
    uint64_t f_size = file_size(filename);
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;
    char *f_content = (char*) calloc(f_size+1, sizeof(*f_content));
    if (!f_content){
        cson_eprintln("failed to allocated file buffer!");
        fclose(file);
        return NULL;
    }
    uint64_t read = fread(f_content, sizeof(char), f_size, file);
    printf("%s\n", f_content);

    free(f_content);
    fclose(file);
    return NULL;
}

#endif // CSON_PARSE_IMPLEMENTATION

#endif // _CSON_PARSE_H