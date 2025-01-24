#ifndef _CSON_H
#define _CSON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>

#define CSON_DEF_ARRAY_CAPACITY   16
#define CSON_ARRAY_MUL_F           2
#define CSON_MAP_CAPACITY         16
#define CSON_DEF_INDENT            4
#define CSON_BYTE_SIZE        1024.0
#define CSON_REGION_CAPACITY  2*1024

#define CSON_ANSI_RGB(r, g, b) ("\e[38;2;" #r ";" #g ";" #b "m")
#define CSON_ANSI_END "\e[0m"

#define cson_args_len(...) sizeof((typeof(__VA_ARGS__)[]){__VA_ARGS__})/sizeof(typeof(__VA_ARGS__))
#define cson_args_array(...) (typeof(__VA_ARGS__)[]){__VA_ARGS__}, cson_args_len(__VA_ARGS__)

#define cson_info(msg, ...) (printf("%s%s:%d: " msg CSON_ANSI_END "\n", CSON_ANSI_RGB(196, 196, 196), __FILE__, __LINE__, ## __VA_ARGS__))
#define cson_warning(msg, ...) (fprintf(stderr, "%s%s:%d: [WARNING] " msg CSON_ANSI_END "\n", CSON_ANSI_RGB(196, 64, 0), __FILE__, __LINE__, ## __VA_ARGS__))
#define cson_error(error, msg, ...) (fprintf(stderr, "%s%s:%d [ERROR] (%s) " msg CSON_ANSI_END "\n", CSON_ANSI_RGB(196, 0, 0), __FILE__, __LINE__, (CsonErrorStrings[(error)]), ## __VA_ARGS__))

#define cson_assert(state, msg, ...) do{if (!(state)) {cson_error(0, msg, ##__VA_ARGS__); exit(1);}} while (0)
#define cson_assert_alloc(alloc) cson_assert((alloc)!=NULL, "Memory allocation failed! (need more RAM :/)")

#define cson_arr_len(arr) ((arr)!= NULL ? sizeof((arr))/sizeof((arr)[0]):0)

typedef struct Cson Cson;
typedef struct CsonArg CsonArg;
typedef struct CsonArray CsonArray;
typedef struct CsonMap CsonMap;
typedef struct CsonMapItem CsonMapItem;
typedef struct CsonArg CsonArg;
typedef struct CsonStr CsonStr;
typedef struct CsonArena CsonArena;
typedef struct CsonRegion CsonRegion;

typedef enum {
    Cson_Int,
    Cson_Float,
    Cson_Bool,
    Cson_Null,
    Cson_String,
    Cson_Array,
    Cson_Map,
    Cson__TypeCount
} CsonType;

const char* const CsonTypeStrings[] = {
    [Cson_Int] = "int",
    [Cson_Float] = "float",
    [Cson_Bool] = "bool",
    [Cson_Null] = "null",
    [Cson_String] = "String",
    [Cson_Array] = "Array",
    [Cson_Map] = "Map"
};

_Static_assert(Cson__TypeCount == cson_arr_len(CsonTypeStrings), "CsonType count has changed!");

typedef enum {
    CsonError_Success,
    CsonError_InvalidParam,
    CsonError_InvalidType,
    CsonError_Alloc,
    CsonError_FileNotFound,
    CsonError_UnexpectedToken,
    CsonError_EndOfBuffer,
    CsonError_Unimplemented,
    CsonError_UnclosedString,
    CsonError_Any,
    CsonError_None,
    Cson__ErrorCount
} CsonError;

const char* const CsonErrorStrings[] = {
    [CsonError_Success] = "Success",
    [CsonError_InvalidParam] = "InvalidArguments",
    [CsonError_FileNotFound] = "FileNotFound",
    [CsonError_InvalidType] = "InvalidType",
    [CsonError_Alloc] = "Allocation",
    [CsonError_UnexpectedToken] = "UnexpectedToken",
    [CsonError_UnclosedString] = "UnclosedString",
    [CsonError_EndOfBuffer] = "EndOfBuffer",
    [CsonError_Unimplemented] = "UNIMPLEMENTED",
    [CsonError_Any] = "Undefined",
    [CsonError_None] = ""
};

_Static_assert(Cson__ErrorCount == cson_arr_len(CsonErrorStrings), "CsonError count has changed!");

typedef enum {
    CsonArg_Key,
    CsonArg_Index,
    Cson__ArgCount
} CsonArgType;

const char* const CsonArgStrings[] = {
    [CsonArg_Key] = "Key",
    [CsonArg_Index] = "Index"
};

_Static_assert(Cson__ArgCount == cson_arr_len(CsonArgStrings), "CsonArgType count has changed!");

struct CsonStr{
    char *value;
    size_t len;
};

struct CsonArray{
    Cson **items;
    size_t size;
    size_t capacity;
};

struct CsonMap{
    CsonMapItem **items;
    size_t size;
    size_t capacity;
};

struct CsonMapItem{
    CsonStr key;
    Cson *value;
    CsonMapItem *next;
};

struct CsonArg{
    CsonArgType type;
    union{
        CsonStr key;
        size_t index;
    } value;
};

struct Cson{
    union{
        int integer;
        double floating;
        bool boolean;
        CsonStr string;
        CsonArray *array;
        CsonMap *map;
        void *null;
    } value;
    CsonType type;
};

struct CsonArena{
    CsonRegion *first, *last;
    size_t region_size;
};

struct CsonRegion{
    size_t size;
    size_t capacity;
    CsonRegion *next;
    uintptr_t data[];
};

static CsonArena cson_arena = {0};

#define key(kstr) ((CsonArg) {.value.key=cson_str(kstr), .type=CsonArg_Key})
#define index(istr) ((CsonArg) {.value.index=(size_t)(istr), .type=CsonArg_Index})
#define cson_get(cson, ...) cson__get(cson, cson_args_array(__VA_ARGS__))

Cson* cson__get(Cson *cson, CsonArg args[], size_t count);

Cson* cson_new(void);
Cson* cson_new_int(int value);
Cson* cson_new_float(double value);
Cson* cson_new_bool(bool value);
Cson* cson_new_string(CsonStr value);
Cson* cson_new_cstring(char *cstr);
Cson* cson_new_array(CsonArray *value);
Cson* cson_new_map(CsonMap *value);

int cson_get_int(Cson *cson);
double cson_get_float(Cson *cson);
bool cson_get_bool(Cson *cson);
CsonStr cson_get_string(Cson *cson);
CsonArray* cson_get_array(Cson *cson);
CsonMap* cson_get_map(Cson *cson);

CsonArray* cson_array_new(void);
CsonError cson_array_push(CsonArray *array, Cson *value);
Cson* cson_array_get(CsonArray *array, size_t index);

CsonMap* cson_map_new(void);
CsonError cson_map_insert(CsonMap *map, CsonStr key, Cson *value);
Cson* cson_map_get(CsonMap *map, CsonStr key);

CsonRegion* cson__new_region(size_t capacity);
void* cson_alloc(CsonArena *arena, size_t size);
void* cson_realloc(CsonArena *arena, void *old_ptr, size_t old_size, size_t new_size);
void cson_free();

#define cson_str(string) ((CsonStr){.value=(string), .len=strlen(string)})
CsonStr cson_str_new(char *cstr);
uint32_t cson_str_hash(CsonStr str);
bool cson_str_equals(CsonStr a, CsonStr b);

#ifdef CSON_WRITE
#define CSON_PRINT_INDENT 4
CsonError cson_write(Cson *json, char *filename);
void cson_fprint(Cson *value, FILE *file, size_t indent);
#define cson_print(cson) do {if (cson!=NULL){cson_fprint(cson, stdout, 0); putchar('\n');}else{printf("-null-\n");}} while (0)
#endif // CSON_WRITE

#ifdef CSON_PARSE

/* Lexer */
#define cson_lex_is_whitespace(c) ((c == ' ' || c == '\n' || c == '\t'))
#define cson_lex_check_line(lexer, c) do{if (c == '\n'){(lexer)->loc.row++; (lexer)->loc.column=1;}else{lexer->loc.column++;}}while(0)
#define cson_lex_inc(lexer) do{lexer->index++; lexer->loc.column++;}while(0)
#define cson_lex_get_char(lexer) (lexer->buffer[lexer->index])
#define cson_lex_get_pointer(lexer) (lexer->buffer + lexer->index)
#define cson_loc_expand(loc) (loc).filename, (loc).row, (loc).column
#define cson_token_args_array(...) (CsonTokenType[]){__VA_ARGS__}, cson_args_len(__VA_ARGS__)

#define CSON_LOC_FMT "%s:%d:%d"

typedef enum{
    CsonToken_MapOpen,
    CsonToken_MapClose,
    CsonToken_ArrayOpen,
    CsonToken_ArrayClose,
    CsonToken_Sep,
    CsonToken_MapSep,
    CsonToken_String,
    CsonToken_Int,
    CsonToken_Float,
    CsonToken_True,
    CsonToken_False,
    CsonToken_Null,
    CsonToken_Invalid,
    CsonToken_End,
    CsonToken__Count
} CsonTokenType;

const char* const CsonTokenTypeNames[] = {
    [CsonToken_MapOpen] = "MapOpen",
    [CsonToken_MapClose] = "MapClose",
    [CsonToken_ArrayOpen] = "ArrayOpen",
    [CsonToken_ArrayClose] = "ArrayClose",
    [CsonToken_Sep] = "Sep",
    [CsonToken_MapSep] = "MapSep",
    [CsonToken_String] = "Literal: String",
    [CsonToken_Int] = "Literal: Int",
    [CsonToken_Float] = "Literal: Float",
    [CsonToken_True] = "Literal: True",
    [CsonToken_False] = "Literal: False",
    [CsonToken_Null] = "Literal: Null",
    [CsonToken_Invalid] = "Invalid",
    [CsonToken_End] = "--END--"
};

_Static_assert(CsonToken__Count == cson_arr_len(CsonTokenTypeNames), "CsonTokenType count has changed!");

typedef struct{
    char *filename;
    size_t row;
    size_t column;
} CsonLoc;

typedef struct{
    CsonTokenType type;
    char *t_start;
    char *t_end;
    CsonLoc loc;
} CsonToken;

typedef struct{
    char *buffer;
    size_t buffer_size;
    size_t index;
    CsonLoc loc;
} CsonLexer;

CsonLexer cson_lex_init(char *buffer, size_t buffer_size, char *filename);
bool cson_lex_next(CsonLexer *lexer, CsonToken *token);
bool cson__lex_expect(CsonLexer *lexer, CsonToken *token, CsonTokenType types[], size_t count);
bool cson_lex_extract(CsonToken *token, char *buffer, size_t buffer_size);
void cson_lex_trim_left(CsonLexer *lexer);
bool cson_lex_find(CsonLexer *lexer, char c);
void cson_lex_set_token(CsonToken *token, CsonTokenType type, char *t_start, char *t_end, CsonLoc loc);
bool cson_lex_is_delimeter(char c);
bool cson_lex_is_int(char *s, char *e);
bool cson_lex_is_float(char *s, char *e);

#define cson_lex_expect(lexer, token, ...) cson__lex_expect(lexer, token, cson_token_args_array(__VA_ARGS__))

/* Parser */
#define cson_parse(buffer, buffer_size) cson_parse_buffer(buffer, buffer_size, "")
Cson* cson_parse_buffer(char *buffer, size_t buffer_size, char *filename);
Cson* cson_read(char *filename);
bool cson__parse_map(CsonMap *map, CsonLexer *lexer);
bool cson__parse_array(CsonArray *map, CsonLexer *lexer);
#endif // CSON_PARSE

#endif // _CSON_H

#ifdef CSON_IMPLEMENTATION

Cson* cson__get(Cson *cson, CsonArg args[], size_t count)
{
    if (cson == NULL) return NULL;
    Cson *next = cson;
    for (size_t i=0; i<count; ++i){
        CsonArg arg = args[i];
        if (arg.type == CsonArg_Key && next->type == Cson_Map){
            next = cson_map_get(cson->value.map, arg.value.key);
            if (next == NULL) return NULL;
            continue;
        }
        else if (arg.type == CsonArg_Index && next->type == Cson_Array){
            next = cson_array_get(next->value.array, arg.value.index);
            if (next == NULL) return NULL;
            continue;
        }
        else{
            cson_error(CsonError_InvalidType, "Cannot access %s via %s!", CsonTypeStrings[next->type], CsonArgStrings[arg.type]);
        }
        return NULL;
    }
    return next;
}

/* Cson constructors */

Cson* cson_new(void)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    return cson;
}

Cson* cson_new_int(int value)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_Int;
    cson->value.integer = value;
    return cson;
}

Cson* cson_new_float(double value)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_Float;
    cson->value.floating = value;
    return cson;
}

Cson* cson_new_bool(bool value)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_Bool;
    cson->value.boolean = value;
    return cson;
}

Cson* cson_new_string(CsonStr value)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_String;
    cson->value.string = value;
    return cson;
}

Cson* cson_new_cstring(char *cstr)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_String;
    cson->value.string = cson_str(cstr);
    return cson;
}

Cson* cson_new_array(CsonArray *value)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_Array;
    cson->value.array = value;
    return cson;
}

Cson* cson_new_map(CsonMap *value)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_Map;
    cson->value.map = value;
    return cson;
}

Cson* cson_new_null(void)
{
    Cson *cson = cson_alloc(&cson_arena, sizeof(*cson));
    cson_assert_alloc(cson);
    cson->type = Cson_Null;
    cson->value.null = NULL;
    return cson;
}

int cson_get_int(Cson *cson)
{
    if (cson->type != Cson_Int) {
        cson_error(CsonError_InvalidType, "Cannot convert %s to %s!", CsonTypeStrings[cson->type], CsonTypeStrings[Cson_Int]);
        return 0;
    }
    return cson->value.integer;
}

double cson_get_float(Cson *cson)
{
    if (cson->type != Cson_Float) {
        cson_error(CsonError_InvalidType, "Cannot convert %s to %s!", CsonTypeStrings[cson->type], CsonTypeStrings[Cson_Float]);
        return 0.0;
    }
    return cson->value.floating;
}

bool cson_get_bool(Cson *cson)
{
    if (cson->type != Cson_Bool) {
        cson_error(CsonError_InvalidType, "Cannot convert %s to %s!", CsonTypeStrings[cson->type], CsonTypeStrings[Cson_Bool]);
        return false;
    }
    return cson->value.boolean;
}

CsonStr cson_get_string(Cson *cson)
{
    if (cson->type != Cson_String) {
        cson_error(CsonError_InvalidType, "Cannot convert %s to %s!", CsonTypeStrings[cson->type], CsonTypeStrings[Cson_String]);
        return (CsonStr){0};
    }
    return cson->value.string;
}

CsonArray* cson_get_array(Cson *cson)
{
    if (cson->type != Cson_Array) {
        cson_error(CsonError_InvalidType, "Cannot convert %s to %s!", CsonTypeStrings[cson->type], CsonTypeStrings[Cson_Array]);
        return NULL;
    }
    return cson->value.array;
}

CsonMap* cson_get_map(Cson *cson)
{
    if (cson->type != Cson_Map) {
        cson_error(CsonError_InvalidType, "Cannot convert %s to %s!", CsonTypeStrings[cson->type], CsonTypeStrings[Cson_Map]);
        return NULL;
    }
    return cson->value.map;
}


/* Array implementation */

CsonArray* cson_array_new(void)
{
    CsonArray *array = cson_alloc(&cson_arena, sizeof(*array) + CSON_DEF_ARRAY_CAPACITY*sizeof(Cson*));
    cson_assert_alloc(array);
    array->size = 0;
    array->capacity = CSON_DEF_ARRAY_CAPACITY;
    array->items = (Cson**) (array+1);
    return array;
}

CsonError cson_array_push(CsonArray *array, Cson *value)
{
    if (array == NULL || value == NULL) return CsonError_InvalidParam;
    if (array->size >= array->capacity){
        size_t new_capacity = array->capacity * CSON_ARRAY_MUL_F;
        array->items = cson_realloc(&cson_arena, array->items, array->capacity*sizeof(Cson*), new_capacity*sizeof(Cson));
        array->capacity = new_capacity;
    }
    array->items[array->size++] = value;
    return CsonError_Success;
}

Cson* cson_array_get(CsonArray *array, size_t index)
{
    if (array == NULL || index >= array->size) return NULL;
    return array->items[index];
}

/* Map implementation */

CsonMap* cson_map_new(void)
{
    CsonMap *map = cson_alloc(&cson_arena, sizeof(*map) + CSON_MAP_CAPACITY*sizeof(CsonMapItem*));
    cson_assert_alloc(map);
    map->size = 0;
    map->capacity = CSON_MAP_CAPACITY;
    map->items = (CsonMapItem**) (map+1);
    return map;
}

CsonMapItem* cson_map_item_new(CsonStr key, Cson *value)
{
    CsonMapItem *item = cson_alloc(&cson_arena, sizeof(*item));
    cson_assert_alloc(item);
    item->key = key;
    item->value = value;
    item->next = NULL;
    return item;
}

CsonError cson_map_insert(CsonMap *map, CsonStr key, Cson *value)
{
    if (map == NULL || key.value == NULL || value == NULL) return CsonError_InvalidParam;
    size_t index = cson_str_hash(key) % map->capacity;
    CsonMapItem *item = map->items[index];
    if (item != NULL){
        do {
            if (cson_str_equals(item->key, key)){
                item->value = value;
                return CsonError_Success;
            }
            if (item->next == NULL) break;
            item = item->next;
        }while(true);
        item->next = cson_map_item_new(key, value);
    }
    else{
        map->items[index] = cson_map_item_new(key, value);
    }
    map->size++;
    return CsonError_Success;
}

Cson* cson_map_get(CsonMap *map, CsonStr key)
{
    if (map == NULL || key.value == NULL) return NULL;
    size_t index = cson_str_hash(key) % map->capacity;
    CsonMapItem *item = map->items[index];
    while (item != NULL){
        if (cson_str_equals(item->key, key)) return item->value;
        item = item->next;
    }
    return NULL;
}

/* Memory management */

CsonRegion* cson__new_region(size_t capacity)
{
    size_t size = sizeof(CsonRegion) + sizeof(uintptr_t)*capacity;
    CsonRegion *region = malloc(size);
    cson_assert(region != NULL, "Failed to allocate new region!");
    region->size = 0;
    region->capacity = capacity;
    region->next = NULL;
    return region;
}

void cson__free(CsonArena *arena)
{
    if (arena == NULL) return;
    CsonRegion *next = arena->first;
    while (next != NULL){
        CsonRegion *temp = next;
        next = temp->next;
        free(temp);
    }
    arena->first = NULL;
    arena->last = NULL;
}

void* cson_alloc(CsonArena *arena, size_t size)
{
    if (arena == NULL) return NULL;
    size_t all_size = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
    size_t default_capacity = (arena->region_size != 0)? arena->region_size:CSON_REGION_CAPACITY;
    if (arena->first == NULL){
        cson_assert(arena->last == NULL, "Invalid arena state!: first:%p, last:%p", arena->first, arena->last);
        size_t capacity = (all_size > default_capacity)? all_size:default_capacity;
        CsonRegion *region = cson__new_region(capacity);
        arena->first = region;
        arena->last = region;
    }
    CsonRegion *last = arena->last;
    if (last->size + all_size > last->capacity){
        cson_assert(arena->last == NULL, "Invalid arena state!: size:%zu, capacity:%zu, next:%p", last->size, last->capacity, last->next);
        size_t capacity = (all_size > default_capacity)? all_size:default_capacity;
        last->next = cson__new_region(capacity);
        arena->last = last->next;
    }
    void *result = &arena->last->data[arena->last->size];
    arena->last->size += all_size;
    return result;
}

void* cson_realloc(CsonArena *arena, void *old_ptr, size_t old_size, size_t new_size)
{
    if (arena == NULL) return NULL;
    if (old_size >= new_size) return old_ptr;
    void *new_ptr = cson_alloc(arena, new_size);
    cson_assert_alloc(new_ptr);
    char *rc = (char*) old_ptr;
    char *wc = (char*) new_ptr;
    for (size_t i=0; i<old_size; ++i){
        *wc++ = *rc++;
    }
    return new_ptr;
}

void* cson_dup(CsonArena *arena, void *old_ptr, size_t old_size, size_t new_size)
{
    if (arena == NULL) return NULL;
    void *new_ptr = cson_alloc(arena, new_size);
    cson_assert_alloc(new_ptr);
    char *rc = (char*) old_ptr;
    char *wc = (char*) new_ptr;
    for (size_t i=0; i<new_size; ++i){
        *wc++ = (i<old_size)?*rc++:'\0';
    }
    return new_ptr;
}

void cson_free()
{
    cson__free(&cson_arena);
}

/* Further utilities */
uint32_t cson_hash(void *p, size_t n)
{
    // DJB2 hashing
    uint32_t hash = 5381;
    while (n-- > 0){
        hash = ((hash << 5) + hash) + *((char*)p++);
    }
    return hash;
}

uint64_t cson_file_size(const char* filename){
    struct stat file;
    if (stat(filename, &file) == -1){
        cson_error(CsonError_FileNotFound, "Failed to read file: \"%s\"", filename);
        return 0;
    }
    return (uint64_t) file.st_size;
}

/* String Utilities */

CsonStr cson_str_new(char *cstr)
{
    size_t len = strlen(cstr);
    char *value = cson_dup(&cson_arena, cstr, len, len+1);
    return (CsonStr) {.value=value, .len=len};
}

uint32_t cson_str_hash(CsonStr str)
{
    return cson_hash(str.value, str.len);
}

bool cson_str_equals(CsonStr a, CsonStr b)
{
    if (a.len != b.len) return false;
	char *pa = a.value;
	char *pb = b.value;
    size_t len = a.len;
	while (len-- > 0){
		if (*pa++ != *pb++) return false;
	}
	return true;
}

#ifdef CSON_WRITE

#define cson_print_indent(file, indent) (fprintf((file), "%*s", (indent)*CSON_PRINT_INDENT, ""))

void cson_fprint(Cson *value, FILE *file, size_t indent)
{
    if (value == NULL || file == NULL) return;
    switch (value->type){
        case Cson_Int:{
            fprintf(file, "%d", value->value.integer);
        }break;
        case Cson_Float:{
            fprintf(file, "%lf", value->value.floating);
        }break;
        case Cson_Bool:{
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
                cson_print_indent(file, indent+1);
                cson_fprint(array->items[i], file, indent+1);
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
                CsonMapItem *value = map->items[i];
                while (value){
                    cson_print_indent(file, indent+1);
                    fprintf(file, "\"%s\": ", value->key);
                    cson_fprint(value->value, file, indent+1);
                    fprintf(file, "%c\n", (--size > 0)? ',':' ');
                    value = value->next;
                }
            }
            cson_print_indent(file, indent);
            fputc('}', file);
        }break;
        default:{
            cson_error(CsonError_InvalidType, "Invalid value type: %s", CsonErrorStrings[value->type]);
            return;
        }
    }
}

CsonError cson_write(Cson *json, char *filename)
{
    if (json == NULL || filename == NULL) return CsonError_InvalidParam;
    FILE *file = fopen(filename, "w");
    if (file == NULL) return CsonError_FileNotFound;
    cson_fprint(json, file, 0);
    fclose(file);
    return CsonError_Success;
}

#endif // CSON_WRITE

#ifdef CSON_PARSE

CsonLexer cson_lex_init(char *buffer, size_t buffer_size, char *filename)
{
    return (CsonLexer) {.buffer=buffer, .buffer_size=buffer_size, .index=0, .loc=(CsonLoc){.filename=filename, .row=1, .column=1}};
}

bool cson_lex_next(CsonLexer *lexer, CsonToken *token)
{
    if (lexer == NULL || token == NULL || lexer->index > lexer->buffer_size) return false;
    cson_lex_trim_left(lexer);
    char *t_start = cson_lex_get_pointer(lexer);
    CsonLoc t_loc = lexer->loc;
    switch (cson_lex_get_char(lexer)){
        case '{':{
            cson_lex_set_token(token, CsonToken_MapOpen, t_start, t_start+1, t_loc);
            break;
        }
        case '}':{
            cson_lex_set_token(token, CsonToken_MapClose, t_start, t_start+1, t_loc);
            break;
        }
        case '[':{
            cson_lex_set_token(token, CsonToken_ArrayOpen, t_start, t_start+1, t_loc);
            break;
        }
        case ']':{
            cson_lex_set_token(token, CsonToken_ArrayClose, t_start, t_start+1, t_loc);
            break;
        }
        case ',':{
            cson_lex_set_token(token, CsonToken_Sep, t_start, t_start+1, t_loc);
            break;
        }
        case ':':{
            cson_lex_set_token(token, CsonToken_MapSep, t_start, t_start+1, t_loc);
            break;
        }
        case '"':{
            // lex string
            cson_lex_inc(lexer);
            char *s_start = cson_lex_get_pointer(lexer);
            if (!cson_lex_find(lexer, '"')){
                cson_error(CsonError_UnclosedString, "Missing closing delimeter for '\"' at " CSON_LOC_FMT "\n", cson_loc_expand(t_loc));
                return false;
            }
            char *s_end = cson_lex_get_pointer(lexer);
            cson_lex_set_token(token, CsonToken_String, s_start, s_end, t_loc);
            break;
        }
        case '\0':
        case EOF:{
            cson_lex_set_token(token, CsonToken_End, t_start, t_start+1, t_loc);
            cson_lex_inc(lexer);
            return false;
        }
        default:{
            // multi-character literal
            // find end of literal
            char c;
            while (lexer->index < lexer->buffer_size && !cson_lex_is_delimeter(c = cson_lex_get_char(lexer))){
                cson_lex_check_line(lexer, c);
                lexer->index++;
            }
            char *t_end = cson_lex_get_pointer(lexer);
            size_t t_len = t_end-t_start;
            // check for known literals
            if (memcmp(t_start, "true", t_len) == 0){
                cson_lex_set_token(token, CsonToken_True, t_start, t_end, t_loc);
                return true;
            }
            if (memcmp(t_start, "false", t_len) == 0){
                cson_lex_set_token(token, CsonToken_False, t_start, t_end, t_loc);
                return true;
            }
            if (memcmp(t_start, "null", t_len) == 0){
                cson_lex_set_token(token, CsonToken_Null, t_start, t_end, t_loc);
                return true;
            }
            if (cson_lex_is_int(t_start, t_end)){
                cson_lex_set_token(token, CsonToken_Int, t_start, t_end, t_loc);
                return true;
            }
            if (cson_lex_is_float(t_start, t_end)){
                cson_lex_set_token(token, CsonToken_Int, t_start, t_end, t_loc);
                return true;
            }
            cson_error(CsonError_InvalidType, "Invalid literal \"%.*s\" at "CSON_LOC_FMT, t_end, t_start, t_loc);
            cson_lex_set_token(token, CsonToken_Invalid, t_start, t_end, t_loc);
            return false;
        }
    }
    cson_lex_inc(lexer);
    return true;
}

bool cson__lex_expect(CsonLexer *lexer, CsonToken *token, CsonTokenType types[], size_t count)
{
    if (lexer == NULL || token == NULL) return false;
    cson_lex_next(lexer, token);
    for (size_t i=0; i<count; ++i){
        if (token->type == types[i]) return true;
    }
    cson_error(CsonError_UnexpectedToken, CSON_LOC_FMT " Expected token of type [", cson_loc_expand(token->loc));
    size_t i;
    for (i=0; i<count-1; ++i){
        printf("\b%s, ", CsonTokenTypeNames[types[i]]);
    }
    printf("%s] but got %s!\n", CsonTokenTypeNames[types[i]], CsonTokenTypeNames[token->type]);
    return false;  
}

bool cson_lex_extract(CsonToken *token, char *buffer, size_t buffer_size)
{
    if (token == NULL || buffer == NULL || buffer_size == 0) return false;
    size_t t_len = token->t_end-token->t_start;
    if (t_len >= buffer_size) return false;
    if (token->type == CsonToken_String){
        char temp_buffer[t_len+1];
        memset(temp_buffer, 0, t_len+1);
        char *r = token->t_start;
        char *w = temp_buffer;
        while (r != token->t_end){
            if (*r == '\\'){
                switch(*++r){
                    case '\'': *w = 0x27; break;
					case '"':  *w = 0x22; break;
					case '?':  *w = 0x3f; break;
					case '\\': *w = 0x5c; break;
					case 'a':  *w = 0x07; break;
					case 'b':  *w = 0x08; break;
					case 'f':  *w = 0x0c; break;
					case 'n':  *w = 0x0a; break;
					case 'r':  *w = 0x0d; break;
					case 't':  *w = 0x09; break;
					case 'v':  *w = 0x0b; break;
                    default:{
                        *w++ = '\\';
                        *w = *r;
                    }
                }
            }
            else{
                *w = *r;
            }
            r++;
            w++;
        }
        sprintf(buffer, "%.*s\0", w-temp_buffer+1, temp_buffer);
    }
    else{
        sprintf(buffer, "%.*s\0", t_len, token->t_start);
    }
    return true;
}

void cson_lex_print(CsonToken token)
{
    cson_info(CSON_LOC_FMT": %s: '%.*s'\n", cson_loc_expand(token.loc), CsonTokenTypeNames[token.type], token.t_end-token.t_start, token.t_start);
}

void cson_lex_set_token(CsonToken *token, CsonTokenType type, char *t_start, char *t_end, CsonLoc loc)
{
    if (token == NULL) return;
    token->type = type;
    token->t_start = t_start;
    token->t_end = t_end,
    token->loc = loc;
}

bool cson_lex_find(CsonLexer *lexer, char c)
{
    char rc;
    while (lexer->index < lexer->buffer_size){
        if ((rc = cson_lex_get_char(lexer)) == c) return true;
        cson_lex_check_line(lexer, rc);
        lexer->index++;
    }
    return false;
}

void cson_lex_trim_left(CsonLexer *lexer)
{
    char c;
    while (lexer->index <= lexer->buffer_size && cson_lex_is_whitespace((c = cson_lex_get_char(lexer)))){
        cson_lex_check_line(lexer, c);
        lexer->index++;
    }
}

bool cson_lex_is_delimeter(char c)
{
    switch (c){
        case '{':
        case '}':
        case '[':
        case ']':
        case ',':
        case ':':
        case ' ':
        case '\n':
        case '\t':{
            return true;
        }
        default: return false;
    }
}

bool cson_lex_is_int(char *s, char *e)
{
	if (!s || !e || e-s < 1) return false;
	if (*s == '-' || *s == '+') s++;
	while (s < e){
		if (!isdigit(*s++)) return false;
	}
	return true;
}

bool cson_lex_is_float(char *s, char *e)
{
    char* ep = NULL;
    strtod(s, &ep);
    return (ep && ep == e);
}

/* Parser implementation */

bool cson__parse_map(CsonMap *map, CsonLexer *lexer)
{
    cson_error(CsonError_Unimplemented, "cson__parse_map");
    return false;
}

bool cson__parse_array(CsonArray *array, CsonLexer *lexer)
{
    cson_error(CsonError_Unimplemented, "cson__parse_array");
    return false;
}

#define cson_parse(buffer, buffer_size) cson_parse_buffer(buffer, buffer_size, "")

Cson* cson_parse_buffer(char *buffer, size_t buffer_size, char *filename)
{
    if (buffer == NULL || buffer_size == 0) return NULL;
    CsonLexer lexer = cson_lex_init(buffer, buffer_size, filename);
    CsonToken token;
    if (!cson_lex_next(&lexer, &token)){
        if (token.type == CsonToken_End){
            cson_error(CsonError_EndOfBuffer, "file is empty: \"%s\"", filename);
        }
        return NULL;
    }
    Cson *cson = NULL;
    switch(token.type){
        case CsonToken_ArrayOpen:{
            CsonArray *array = cson_array_new();
            if (cson__parse_array(array, &lexer)){
                cson = cson_new_array(array);
            }
        }break;
        case CsonToken_MapOpen:{
            CsonMap *map = cson_map_new();
            if (cson__parse_map(map, &lexer)){
                cson = cson_new_map(map);
            }
        }break;
        default:{
            cson_error(CsonError_UnexpectedToken, CSON_LOC_FMT": json object may only start with [%s, %s] and not ", cson_loc_expand(token.loc), CsonTokenTypeNames[CsonToken_ArrayOpen], CsonTokenTypeNames[CsonToken_MapOpen], CsonTokenTypeNames[token.type]);
            return NULL;
        }
    }
    if (cson != NULL && !cson_lex_expect(&lexer, &token, CsonToken_End)){
        cson_error(CsonError_UnexpectedToken, "json object may not have trailing values after closing of parent %s!", CsonTypeStrings[cson->type]);
        return NULL;
    }
    return cson;
}

Cson* cson_read(char *filename){
    FILE *file = fopen(filename, "r");
    if (file == NULL){
        cson_error(CsonError_FileNotFound, "Could not find file: \"%s\"", filename);
        return NULL;
    }
    uint64_t file_size = cson_file_size(filename);
    char *file_content = (char*) calloc(file_size+1, sizeof(*file_content));
    if (!file_content){
        fclose(file);
        cson_error(CsonError_Alloc, "Out of memory!");
        return NULL;
    }
    fread(file_content, 1, file_size, file);
    fclose(file);
    Cson *cson = cson_parse_buffer(file_content, file_size, filename);
    free(file_content);
    return cson;
}
#endif // CSON_PARSE

#endif // CSON_IMPLEMENTATION