#ifndef _CSON_WRITE_H
#define _CSON_WRITE_H

#include "Cson.h"

CsonError Cson_write(Cson *json, char *filename);
void Cson_fprint_value(Cson *value, FILE *file, size_t indent);
#define Cson_print(cson) do {Cson_fprint_value(cson, stdout, 0); putchar('\n');} while (0);

#define CSON_WRITE_IMPLEMENTATION // DEBUG
#ifdef CSON_WRITE_IMPLEMENTATION

#define cson_print_indent(f, n)\
    for (int pi=0;pi<n;++pi){\
        fprintf(f, "%*c", CSON_DEF_INDENT, ' ');\
    }

void Cson_fprint_value(Cson *value, FILE *file, size_t indent)
{
    if (!value || !file) return;
    switch (value->type){
        case Cson_Integer:{
            fprintf(file, "%d", value->value.integer);
        }break;
        case Cson_Double:{
            fprintf(file, "%lf", value->value.floating);
        }break;
        case Cson_Boolean:{
            fprintf(file, "%s", value->value.boolean? "true":"false");
        }break;
        case Cson_String:{
            fprintf(file, "\"%s\"", value->value.string);
        }break;
        case Cson_Null:{
            fprintf(file, "null");
        }break;
        case Cson_Array:{
            fprintf(file, "[\n");
            CsonArray *array = value->value.array;
            for (size_t i=0; i<array->size; ++i){
                cson_print_indent(file, indent+1)
                Cson_fprint_value(array->values[i], file, indent+1);
                fprintf(file, "%c\n", (i+1 == array->size)? ' ':',');
            }
            cson_print_indent(file, indent);
            fputc(']', file);
        }break;
        case Cson_Map:{
            fprintf(file, "{\n");
            CsonMap *map = value->value.map;
            size_t size = map->size;
            for (size_t i=0; i<map->capacity; ++i){
                CsonMapValue *value = map->values[i];
                while (value){
                    cson_print_indent(file, indent+1);
                    fprintf(file, "\"%s\": ", value->key);
                    Cson_fprint_value(value->value, file, indent+1);
                    fprintf(file, "%c\n", (--size > 0)? ',':' ');
                    value = value->next;
                }
            }
            cson_print_indent(file, indent);
            fputc('}', file);
        }break;
        default:{
            cson_eprintln("Invalid Value! %d", value->type);
            return;
        }
    }
}

CsonError Cson_write(Cson *json, char *filename)
{
    if (!json || !filename) return CsonError_Param;
    FILE *file = fopen(filename, "w");
    if (!file) return CsonError_FileNotFound;
    Cson_fprint_value(json, file, 0);
    fclose(file);
    return CsonError_Success;
}


#endif // CSON_WRITE_IMPLEMENTATION

#endif // _CSON_WRITE_H