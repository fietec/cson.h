#ifndef _CSON_PARSE_H
#define _CSON_PARSE_H

#include "Cson.h"

Cson* Cson_parse(char *json_string);
Cson* Cson_read(char *filename);

#define CSON_PARSE_IMPLEMENTATION // DEBUG
#ifdef CSON_PARSE_IMPLEMENTATION 

struct CsonParser{
    size_t line;
    size_t column;
};

Cson* Cson_parse(char *json_string)
{
    return NULL;
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