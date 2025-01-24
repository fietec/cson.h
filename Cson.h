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
    CsonError_Any,
    CsonError_None,
    Cson__ErrorCount
} CsonError;

const char* const CsonErrorStrings[] = {
    [CsonError_Success] = "Success",
    [CsonError_InvalidParam] = "InvalidArguments",
    [CsonError_FileNotFound] = "FileNotFound",
    [CsonError_Alloc] = "Allocation",
    [CsonError_UnexpectedToken] = "UnexpectedToken",
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
#define cson_parse(buffer, buffer_size) cson_parse_buffer(buffer, buffer_size, "")
Cson* cson_parse_buffer(char *buffer, size_t buffer_size, char *filename);
Cson* cson_read(char *filename);
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
            cson_error(CsonError_InvalidType, "Cannot %s via %s!", CsonTypeStrings[next->type], CsonArgStrings[arg.type]);
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

/* CsonCleks implementation */
#define CSON_CLEKS_VALUE_FORMAT "%.*s"
#define CSON_CLEKS_LOC_EXPAND(loc) (loc).filename, (loc).row, (loc).column

#define CSON_CLEKS_FLAGS_ALL_NUMS 0xf           // enable all number parsing
#define CSON_CLEKS_FLAGS_INTEGERS 0x1           // enable parsing of CSON_CLEKS_INTEGER
#define CSON_CLEKS_FLAGS_FLOATS 0x2             // enable parsing of CSON_CLEKS_FLOAT
#define CSON_CLEKS_FLAGS_HEX 0x4                // enable parsing of CSON_CLEKS_HEX
#define CSON_CLEKS_FLAGS_BIN 0x8                // enable parsing of CSON_CLEKS_BIN
#define CSON_CLEKS_FLAGS_KEEP_UNKNOWN 0x10      // do not mark CSON_CLEKS_UNKNOWN when printing using cson_cleks_print_default
#define CSON_CLEKS_FLAGS_DISABLE_UNKNOWN 0x20   // do not allow CSON_CLEKS_UNKNOWN, throw error instead

/* Helper macros */
#define cson_clekser_inc(cson_clekser) do{(cson_clekser)->index++; (cson_clekser)->loc.column++;}while(0);
#define cson_clekser__get_char(cson_clekser) (cson_clekser)->buffer[(cson_clekser)->index]
#define cson_clekser__get_pointer(cson_clekser) (cson_clekser)->buffer + (cson_clekser)->index
#define cson_clekser__check_line(cson_clekser) do{if (cson_clekser__get_char((cson_clekser)) == '\n'){(cson_clekser)->loc.row++; (cson_clekser)->loc.column=1;}else{cson_clekser->loc.column++;}}while(0);

#define cson_cleks__is_special(c) ((c) == '\0' || (c) == EOF)

/* User macros */
// extract the CsonCleksTokenType from the id of a token
#define cson_cleks_token_type(id) ((CsonCleksTokenType) (((CsonCleksTokenID)(id)) >> 32))
// get the string name of a CsonCleksTokenType
#define cson_cleks_token_type_name(type) (CsonCleksTokenTypeNames[(CsonCleksTokenID)(type)])
// extract the CsonCleksTokenIndex from the id of a token
#define cson_cleks_token_index(id) (CsonCleksTokenIndex)((id) & 0xFFFFFFFF)
// buid a CsonCleksTokenID from a type and index
#define cson_cleks_token_id(type, index) ((CsonCleksTokenID) ((CsonCleksTokenID) (type) << 32) | ((CsonCleksTokenIndex)(index)))
// just a shorthand for accesing the string value of a token
#define cson_cleks_token_value(token) (token).start
// calculate the string length of a token in the original buffer
#define cson_cleks_token_value_length(token) ((token).end - (token).start)

// used with cson_cleks_token_id(type, index) if no index is supposed to be defined
#define CSON_CLEKS_ANY_INDEX 0xFFFFFFFF

/* Type definitions */
typedef const char CsonCleksSymbol;     // CSON_CLEKS_SYMBOL 
typedef const char* CsonCleksWord;      // CSON_CLEKS_WORD
typedef const char CsonCleksWhitespace; // token delimeters to be ignored

// the overall type of a token
typedef enum{
	CSON_CLEKS_WORD,             // word literals
	CSON_CLEKS_SYMBOL,           // character literals
	CSON_CLEKS_STRING,           // region within character delimeters
    CSON_CLEKS_FIELD,            // region within string delimeters
	CSON_CLEKS_INTEGER,          // decimal integers
	CSON_CLEKS_FLOAT,            // floating pointer numbers
	CSON_CLEKS_HEX,              // heximal integers
	CSON_CLEKS_BIN,              // binary integers
	CSON_CLEKS_UNKNOWN,          // unknown literals not within CSON_CLEKS_WORD
	CSON_CLEKS_TOKEN_TYPE_COUNT  // not a type, the amount of types
} CsonCleksTokenType;

// Printing names for each TokenType
const char* CsonCleksTokenTypeNames[] = {
	[CSON_CLEKS_WORD] = "Word",
	[CSON_CLEKS_SYMBOL] "Symbol",
	[CSON_CLEKS_STRING] = "String",
    [CSON_CLEKS_FIELD] = "Field",
	[CSON_CLEKS_INTEGER] = "Int",
	[CSON_CLEKS_FLOAT] = "Float",
	[CSON_CLEKS_HEX] = "Hex",
	[CSON_CLEKS_BIN] = "Bin",
	[CSON_CLEKS_UNKNOWN] = "Unknown"
};

_Static_assert(CSON_CLEKS_TOKEN_TYPE_COUNT == cson_arr_len(CsonCleksTokenTypeNames), "CsonCleksTokenTypeNames out of sync !");

// the original location of a token in the buffer
typedef struct{
	size_t row;
	size_t column;
	char *filename;
} CsonCleksLoc;

// the index of a token within a type config
typedef uint32_t CsonCleksTokenIndex;
// a mask containing the CsonCleksTokenType and CsonCleksTokenIndex
typedef uint64_t CsonCleksTokenID;

// an extracted token
typedef struct{
	CsonCleksTokenID id; // the type and index of a token
	CsonCleksLoc loc;    // the token's location in the buffer
	char *start;     // pointer to the start of the token within the buffer
	char *end;       // pointer to the end of the token within the buffer
} CsonCleksToken;

// a function used for printing a token representation
typedef void (*CsonCleksPrintFn) (CsonCleksToken);

// a region within which everything is ignored
typedef struct{
	char *start_del;  // start delimeter 
	char *end_del;    // end delimeter
} CsonCleksComment;

// a character string within char delimeters
typedef struct{
	char start_del;   // start delimeter
	char end_del;     // end delimeter
} CsonCleksString;

// a character string within string delimeters
typedef struct{
    char *prefix;
    char *suffix;
} CsonCleksField;

// config structure containing all custom sub-type definitions
typedef struct{
	CsonCleksWord *words;             // definitions of CSON_CLEKS_WORD
	size_t word_count;          
	CsonCleksSymbol *symbols;         // definitions of CSON_CLEKS_SYMBOL
	size_t symbol_count;
	CsonCleksString *strings;         // definitions of CSON_CLEKS_STRING
	size_t string_count;
    CsonCleksField *fields;           // definitions of CSON_CLEKS_FIELD
    size_t field_count;
	CsonCleksComment *comments;       // definitions of ignored regions
	size_t comment_count;
	CsonCleksWhitespace *whitespaces; // definitiosn of ignored characters
	size_t whitespace_count;
	uint8_t flags;                // additional lexing rules
} CsonCleksConfig;

// the lexing structure containing runtime lexing information
typedef struct{
	char *buffer;
	size_t buffer_size;
	CsonCleksLoc loc;
	size_t index;
	CsonCleksConfig config;
    CsonCleksPrintFn print_fn;
} CsonClekser;

/* Function declarations */

// 'public' functions
// initialization of a CsonClekser structure
CsonClekser cson_cleks_create(char *buffer, size_t buffer_size, CsonCleksConfig config, char *filename, CsonCleksPrintFn print_fn);
// retreive the next token, returns `true` on success
bool cson_cleks_next(CsonClekser *cson_clekser, CsonCleksToken *token);
// retreive the next token and fail when not of specified type  
bool cson_cleks_expect(CsonClekser *cson_clekser, CsonCleksToken *token, CsonCleksTokenID id);
// extract the content of a token into seperate buffer
bool cson_cleks_extract(CsonCleksToken *token, char *buffer, size_t buffer_size);
// use the print dialog associated with the CsonClekser to print a token
void cson_cleks_print(CsonClekser cson_clekser, CsonCleksToken token);
// the default print dialog
void cson_cleks_print_default(CsonClekser cson_clekser, CsonCleksToken token);

// 'private' functions
void cson_cleks__trim_left(CsonClekser *cson_clekser);
void cson_cleks__skip_string(CsonClekser *cson_clekser, char *del);
bool cson_cleks__try_skip_comments(CsonClekser *cson_clekser, bool *result);
bool cson_cleks__find_string(CsonClekser *cson_clekser, char *del);
bool cson_cleks__find_char(CsonClekser *cson_clekser, char del);
bool cson_cleks__try_find_string(CsonClekser *cson_clekser, char *del);
bool cson_cleks__try_find_char(CsonClekser *cson_clekser, char del);
void cson_cleks__set_token(CsonCleksToken *token, uint32_t type, uint32_t id, CsonCleksLoc loc, char *start, char *end);
bool cson_cleks__is_symbol(CsonClekser *cson_clekser, char symbol);
bool cson_cleks__is_whitespace(CsonClekser *cson_clekser, char c);
bool cson_cleks__starts_with(CsonClekser *cson_clekser, char *str);
bool cson_cleks__str_is_float(char *s, char *e);
bool cson_cleks__str_is_int(char *s, char *e);
bool cson_cleks__str_is_hex(char *s, char *e);
bool cson_cleks__str_is_bin(char *s, char *e);

CsonClekser cson_cleks_create(char *buffer, size_t buffer_size, CsonCleksConfig config, char *filename, CsonCleksPrintFn print_fn)
{
	cson_assert(buffer != NULL, "Invalid parameter buffer:%p", buffer);
	return (CsonClekser) {.buffer = buffer, .buffer_size=buffer_size, .loc=(CsonCleksLoc){1, 1, filename}, .index=0, .config=config, .print_fn=print_fn};
}

bool cson_cleks_next(CsonClekser *cson_clekser, CsonCleksToken *token)
{
	cson_assert(cson_clekser != NULL && token != NULL, "Invalid arguments cson_clekser:%p, token:%p", cson_clekser, token);

	while (true){
		if (cson_clekser->index >= cson_clekser->buffer_size) return false;
		// skip spaces
		cson_cleks__trim_left(cson_clekser);
		if (cson_clekser->index >= cson_clekser->buffer_size) return false;
		// skip comments
		bool comment_found = false;
		if (!cson_cleks__try_skip_comments(cson_clekser, &comment_found)) return false;
		if (!comment_found) break;
	}
	// zero-initialize the token
	memset(token, 0, sizeof(*token));
	// try to lex string
	for (size_t i=0; i<cson_clekser->config.string_count; ++i){
		CsonCleksString string = cson_clekser->config.strings[i];
		if (cson_clekser__get_char(cson_clekser) == string.start_del){
			CsonCleksLoc start_loc = cson_clekser->loc;
			cson_clekser->index++;
			cson_clekser->loc.column++;
			char *p_start = cson_clekser__get_pointer(cson_clekser);
			if (!cson_cleks__try_find_char(cson_clekser, string.end_del)){
                cson_error(CsonError_None, "Expected matching '%c' after string delimeter '%c' at %s:%d:%d", string.end_del, string.start_del, CSON_CLEKS_LOC_EXPAND(start_loc));
                return false;
            }
			char *p_end = cson_clekser__get_pointer(cson_clekser);
			cson_cleks__set_token(token, CSON_CLEKS_STRING, i, start_loc, p_start, p_end); 
			cson_clekser_inc(cson_clekser);
			return true;
		}
	}
	// try to lex symbols
	for (size_t i=0; i<cson_clekser->config.symbol_count; ++i){
		if (cson_clekser__get_char(cson_clekser) == cson_clekser->config.symbols[i]){
			char *p_start = cson_clekser__get_pointer(cson_clekser);
			cson_cleks__set_token(token, CSON_CLEKS_SYMBOL, i, cson_clekser->loc, p_start, p_start+1);
			cson_clekser_inc(cson_clekser);
			return true;
		}
	}
    
    // lex fields
    for (size_t i=0; i<cson_clekser->config.field_count; ++i){
        CsonCleksField field = cson_clekser->config.fields[i];
        if (cson_cleks__starts_with(cson_clekser, field.prefix)){
            cson_cleks__skip_string(cson_clekser, field.prefix);
            char *p_start = cson_clekser__get_pointer(cson_clekser);
            CsonCleksLoc loc = cson_clekser->loc;
            if (!cson_cleks__try_find_string(cson_clekser, field.suffix)){
                cson_error(CsonError_None, "Expected \"%s\" for matching field prefix \"%s\" at %s:%d:%d!", field.suffix, field.prefix, CSON_CLEKS_LOC_EXPAND(loc));
                return false;
            }
            char *p_end = cson_clekser__get_pointer(cson_clekser);
            cson_cleks__skip_string(cson_clekser, field.suffix);
            cson_cleks__set_token(token, CSON_CLEKS_FIELD, i, loc, p_start, p_end);
            return true;
        }
    }
    
	char *p_start = cson_clekser__get_pointer(cson_clekser);
	CsonCleksLoc start_loc = cson_clekser->loc;
	char c;
	while (true){
		c = cson_clekser__get_char(cson_clekser);
		if (cson_cleks__is_symbol(cson_clekser, c) || cson_cleks__is_whitespace(cson_clekser, c) || cson_cleks__is_special(c)) break;
		bool delimeter_found = false;
		for (size_t i=0; i<cson_clekser->config.string_count; ++i){
			if (c == cson_clekser->config.strings[i].start_del){
				delimeter_found = true;
				break;
			}
		}
		if (delimeter_found) break;
		for (size_t i=0; i<cson_clekser->config.comment_count; ++i){
			if (cson_cleks__starts_with(cson_clekser, cson_clekser->config.comments[i].start_del)){
				delimeter_found = true;
				break;
			}
		}
		if (delimeter_found) break;
        for (size_t i=0; i<cson_clekser->config.field_count; ++i){
            if (cson_cleks__starts_with(cson_clekser, cson_clekser->config.fields[i].prefix)){
                delimeter_found = true;
                break;
            }
        }
        if (delimeter_found) break;
		cson_clekser_inc(cson_clekser);
	}
	char *p_end = cson_clekser__get_pointer(cson_clekser);
	for (size_t i=0; i<cson_clekser->config.word_count; ++i){
		if (memcmp(p_start, cson_clekser->config.words[i], p_end-p_start) == 0){
			cson_cleks__set_token(token, CSON_CLEKS_WORD, i, start_loc, p_start, p_end);
			return true;
		}
	}
	// no matching words found
	if ((cson_clekser->config.flags & CSON_CLEKS_FLAGS_INTEGERS) && cson_cleks__str_is_int(p_start, p_end)){
		cson_cleks__set_token(token, CSON_CLEKS_INTEGER, 0, start_loc, p_start, p_end);
		return true;
	}
	if ((cson_clekser->config.flags & CSON_CLEKS_FLAGS_FLOATS)&& cson_cleks__str_is_float(p_start, p_end)){
		cson_cleks__set_token(token, CSON_CLEKS_FLOAT, 0, start_loc, p_start, p_end);
		return true;
	}
    if ((cson_clekser->config.flags & CSON_CLEKS_FLAGS_HEX) && cson_cleks__str_is_hex(p_start, p_end)){
        cson_cleks__set_token(token, CSON_CLEKS_HEX, 0, start_loc, p_start, p_end);
        return true;
    }
    if ((cson_clekser->config.flags & CSON_CLEKS_FLAGS_BIN) && cson_cleks__str_is_bin(p_start, p_end)){
        cson_cleks__set_token(token, CSON_CLEKS_BIN, 0, start_loc, p_start, p_end);
        return true;
    }
	if ((cson_clekser->config.flags & CSON_CLEKS_FLAGS_DISABLE_UNKNOWN) == 0){
		cson_cleks__set_token(token, CSON_CLEKS_UNKNOWN, (cson_clekser->config.flags & CSON_CLEKS_FLAGS_KEEP_UNKNOWN)? 1:0, start_loc, p_start, p_end);
		return true;
	}
	cson_error(CsonError_None, "Unknown word found (flags: %d) at %s:%d:%d \"%.*s\"\n", cson_clekser->config.flags, start_loc.filename, start_loc.row, start_loc.column, p_end-p_start, p_start);
	return false;
}

bool cson_cleks_expect(CsonClekser *cson_clekser, CsonCleksToken *token, CsonCleksTokenID id)
{
	CsonCleksToken t_token;
	if (!cson_cleks_next(cson_clekser, &t_token)) return false;
    CsonCleksTokenType type = cson_cleks_token_type(id);
    CsonCleksTokenIndex index = cson_cleks_token_index(id);
	if (cson_cleks_token_type(t_token.id) != type || (index != CSON_CLEKS_ANY_INDEX && cson_cleks_token_index(t_token.id) != index)){
		cson_error(CsonError_UnexpectedToken, "Expected: %s:%d, but got %s:%d!", cson_cleks_token_type_name(type), (index == CSON_CLEKS_ANY_INDEX ? 0 : index), cson_cleks_token_type_name(cson_cleks_token_type(t_token.id)), cson_cleks_token_index(t_token.id));
		return false;
	}
	memcpy(token, &t_token, sizeof(CsonCleksToken));
	return true;
}

#define cson_cleks_expect_multiple(clekser, token, ...) cson__cleks_expect_multiple(clekser, token, cson_args_array(__VA_ARGS__))

bool cson__cleks_expect_multiple(CsonClekser *clekser, CsonCleksToken *token, CsonCleksTokenID ids[], size_t count)
{
    CsonCleksToken t_token;
    if (!cson_cleks_next(clekser, &t_token)){
        cson_error(CsonError_EndOfBuffer, "Expected tokens but reached end of buffer!");
        return false;
    }
    CsonCleksTokenType t_type = cson_cleks_token_type(t_token.id);
    CsonCleksTokenIndex t_index = cson_cleks_token_index(t_token.id);
    for (size_t i=0; i<count; ++i){
        CsonCleksTokenID id = ids[i];
        CsonCleksTokenType type = cson_cleks_token_type(id);
        CsonCleksTokenIndex index = cson_cleks_token_index(id);
        if (type == t_type && (index == CSON_CLEKS_ANY_INDEX || index == t_index)){
            memcpy(token, &t_token, sizeof(CsonCleksToken));
            return true;
        }
    }
    cson_error(CsonError_UnexpectedToken, "unexpected token at %s:%d:%d: %s:%d", CSON_CLEKS_LOC_EXPAND(t_token.loc), cson_cleks_token_type_name(t_type), t_index);
    return false;
}

bool cson_cleks_extract(CsonCleksToken *token, char *buffer, size_t buffer_size)
{
	if (token == NULL || buffer == NULL) return false;
	size_t value_len = token->end - token->start;
	if (value_len >= buffer_size) return false;
	if (cson_cleks_token_type(token->id) == CSON_CLEKS_STRING){
		char temp_buffer[value_len+1];
		memset(temp_buffer, 0, value_len+1);
		char *pr = token->start;
		char *pw = temp_buffer;
		while (pr != token->end){
			if (*pr == '\\'){
				switch(*++pr){
					case '\'': *pw = 0x27; break;
					case '"':  *pw = 0x22; break;
					case '?':  *pw = 0x3f; break;
					case '\\': *pw = 0x5c; break;
					case 'a':  *pw = 0x07; break;
					case 'b':  *pw = 0x08; break;
					case 'f':  *pw = 0x0c; break;
					case 'n':  *pw = 0x0a; break;
					case 'r':  *pw = 0x0d; break;
					case 't':  *pw = 0x09; break;
					case 'v':  *pw = 0x0b; break;
					default:{
						*pw++ = '\\';
						*pw = *pr;
					}
				}
			}
			else{
				*pw = *pr;
			}
			pr++;   
			pw++;
		}
		snprintf(buffer, pw-temp_buffer+2, "%s", temp_buffer);
	}
	else{
		sprintf(buffer, "%.*s\0", value_len, token->start);
	}
	return true;
}

void cson_cleks_print_default(CsonClekser cson_clekser, CsonCleksToken token)
{
	// TODO: probably best to do this with string builders instead
    if (token.loc.filename != NULL) printf("%s:", token.loc.filename);
	CsonCleksTokenType type = cson_cleks_token_type(token.id);
    CsonCleksTokenIndex index = cson_cleks_token_index(token.id);
    bool keep_unknown = (type == CSON_CLEKS_UNKNOWN && index == 1);
	printf("%d:%d %s: ", token.loc.row, token.loc.column, (keep_unknown)? "":cson_cleks_token_type_name(type));
	cson_assert(type < CSON_CLEKS_TOKEN_TYPE_COUNT, "Invalid token type: %u!", type);
	switch(type){
		case CSON_CLEKS_WORD:
		case CSON_CLEKS_SYMBOL: printf("'%.*s'", token.end-token.start, token.start); break;
        case CSON_CLEKS_FIELD: printf("<%s '%.*s' %s>", cson_clekser.config.fields[index].prefix, token.end-token.start, token.start, cson_clekser.config.fields[index].suffix); break;
		case CSON_CLEKS_STRING: printf("\"%.*s\"", token.end-token.start, token.start); break;
		case CSON_CLEKS_INTEGER:
		case CSON_CLEKS_FLOAT: 
		case CSON_CLEKS_HEX: 
		case CSON_CLEKS_BIN: printf("%.*s", token.end-token.start, token.start); break;
		case CSON_CLEKS_UNKNOWN: {
            if (keep_unknown){printf("%.*s", token.end-token.start, token.start);}
            else {printf("`%.*s`", token.end-token.start, token.start);}
        } break;
		default: cson_error(CsonError_None, "Uninplemented type in print: %s", cson_cleks_token_type_name(type)); exit(1);
	}
    putchar('\n');
}

void cson_cleks_print(CsonClekser cson_clekser, CsonCleksToken token)
{
	if (cson_clekser.print_fn != NULL){
        cson_clekser.print_fn(token);
    }
    else{
        cson_cleks_print_default(cson_clekser, token);
    }
}

void cson_cleks__trim_left(CsonClekser *cson_clekser)
{
	cson_assert(cson_clekser != NULL, "Invalid argument cson_clekser:%p", cson_clekser);
	char c;
	while (cson_cleks__is_whitespace(cson_clekser, (c = cson_clekser__get_char(cson_clekser))) || cson_cleks__is_special(c)){
		cson_clekser__check_line(cson_clekser);
		cson_clekser->index++;
	}
}

void cson_cleks__skip_string(CsonClekser *cson_clekser, char *str)
{
	cson_assert(cson_clekser != NULL && str != NULL, "Invalid arguments cson_clekser:%p, str:%p", cson_clekser, str);
	while (cson_clekser__get_char(cson_clekser) == *str++){
		cson_clekser__check_line(cson_clekser);
		cson_clekser->index++;
	}
}

bool cson_cleks__find_string(CsonClekser *cson_clekser, char *del)
{
	cson_assert(cson_clekser != NULL && del != NULL, "Invalid arguments cson_clekser:%p, del:%p", cson_clekser, del);
	while (!cson_cleks__starts_with(cson_clekser, del)){
		if (cson_clekser->index >= cson_clekser->buffer_size) return false;
		cson_clekser__check_line(cson_clekser);
		cson_clekser->index++;
	}
    return true;
}

bool cson_cleks__find_char(CsonClekser *cson_clekser, char del)
{
	cson_assert(cson_clekser != NULL, "Invalid argument cson_clekser:%p", cson_clekser);
	while (cson_clekser__get_char(cson_clekser) != del){
		if (cson_clekser->index >= cson_clekser->buffer_size) return false;
		cson_clekser__check_line(cson_clekser);
		cson_clekser->index++;
	}
    return true;
}

bool cson_cleks__try_find_string(CsonClekser *cson_clekser, char *del)
{
    cson_assert(cson_clekser != NULL && del != NULL, "Invalid argumens cson_clekser:%p, del:%p", cson_clekser, del);
    while (!cson_cleks__starts_with(cson_clekser, del)){
        if (cson_clekser->index >= cson_clekser->buffer_size) return false;
        bool comment_found = false;
        if (!cson_cleks__try_skip_comments(cson_clekser, &comment_found)) return false;
        if (comment_found) continue;
        cson_clekser__check_line(cson_clekser);
        cson_clekser->index++;
    }
    return true;
}

bool cson_cleks__try_find_char(CsonClekser *cson_clekser, char del)
{
    cson_assert(cson_clekser != NULL, "Invalid arguments cson_clekser:%p", cson_clekser);
    while (cson_clekser__get_char(cson_clekser) != del){
        if (cson_clekser->index >= cson_clekser->buffer_size) return false;
        bool comments_found = false;
        if (!cson_cleks__try_skip_comments(cson_clekser, &comments_found)) return false;
        if (comments_found) continue;
        cson_clekser__check_line(cson_clekser);
        cson_clekser->index++;
    }
    return true;
}

bool cson_cleks__try_skip_comments(CsonClekser *cson_clekser, bool *result)
{
    for (size_t i=0; i<cson_clekser->config.comment_count; ++i){
        CsonCleksComment comment = cson_clekser->config.comments[i];
        if (cson_cleks__starts_with(cson_clekser, comment.start_del)){
            CsonCleksLoc loc = cson_clekser->loc;
            cson_cleks__skip_string(cson_clekser, comment.start_del);
            if (!cson_cleks__find_string(cson_clekser, comment.end_del)){
                cson_error(CsonError_UnexpectedToken, "Expected \"%s\" for matching comment delimeter \"%s\" at %s:%d:%d!", comment.end_del, comment.start_del, CSON_CLEKS_LOC_EXPAND(loc));
                return false;
            }
            cson_cleks__skip_string(cson_clekser, comment.end_del);
            *result = true;
            break;
        }
    }
    return true;
}

void cson_cleks__set_token(CsonCleksToken *token, uint32_t type, uint32_t index, CsonCleksLoc loc, char *start, char *end)
{
	cson_assert(token != NULL, "Invalid argument token:%p", token);
	token->id = cson_cleks_token_id(type, index);
	token->loc = loc;
	token->start = start;
	token->end = end;
}

bool cson_cleks__is_whitespace(CsonClekser *cson_clekser, char c)
{
	cson_assert(cson_clekser != NULL, "Invalid argument cson_clekser:%p", cson_clekser);
	for (size_t i=0; i<cson_clekser->config.whitespace_count; ++i){
		if (cson_clekser->config.whitespaces[i] == c) return true;
	}
	return false;
}

bool cson_cleks__is_symbol(CsonClekser *cson_clekser, char c)
{
	cson_assert(cson_clekser != NULL, "Invalid argument cson_clekser:%p", cson_clekser);
	for (size_t i=0; i<cson_clekser->config.symbol_count; ++i){
		if (c == cson_clekser->config.symbols[i]) return true;
	}
	return false;
}

bool cson_cleks__starts_with(CsonClekser *cson_clekser, char *str)
{
	cson_assert(cson_clekser != NULL, "Invalid arguments cson_clekser:%p, str:%p", cson_clekser, str);
	char *curr = cson_clekser->buffer + cson_clekser->index;
	for (size_t i=0, n=strlen(str); i<n; ++i, ++curr){
		if (cson_clekser->index + i >= cson_clekser->buffer_size || *curr != *(str+i)) return false;
	}
	return true;
}

bool cson_cleks__str_is_int(char *s, char *e)
{
	if (!s || !e || e-s < 1) return false;
	if (*s == '-' || *s == '+') s++;
	while (s < e){
		if (!isdigit(*s++)) return false;
	}
	return true;
}

bool cson_cleks__str_is_float(char *s, char *e)
{
    char* ep = NULL;
    strtod(s, &ep);
    return (ep && ep == e);
}

bool cson_cleks__str_is_hex(char *s, char *e)
{
	if (s == NULL || e == NULL || e-s < 3) return false;
	if (*s++ != '0' || *s++ != 'x') return false;
	while (s < e){
		if (!isxdigit(*s++)) return false;
	}
	return true;
}

bool cson_cleks__str_is_bin(char *s, char *e)
{
	if (s == NULL || e == NULL || e-s < 3) return false;
	if (*s++ != '0' || *s++ != 'b') return false;
	while (s < e){
		char c = *s++;
		if (c != '0' && c != '1') return false;
	}
	return true;
}

/* CsonCleks configuration */
typedef enum{
	CSON_JSON_TRUE,
	CSON_JSON_FALSE,
	CSON_JSON_NULL,
} CsonJsonWord;

typedef enum{
	CSON_JSON_MAP_OPEN,
	CSON_JSON_MAP_CLOSE,
	CSON_JSON_ARRAY_OPEN,
	CSON_JSON_ARRAY_CLOSE,
	CSON_JSON_SEP,
	CSON_JSON_MAP_SEP
} CsonJsonSymbol;

CsonCleksWord CsonJsonWords[] = {
	[CSON_JSON_TRUE] = "true",
	[CSON_JSON_FALSE] = "false",
	[CSON_JSON_NULL] = "null"
};

char* const CsonJsonWordNames[] = {
	[CSON_JSON_TRUE] = "True",
	[CSON_JSON_FALSE] = "False",
	[CSON_JSON_NULL] = "Null"
};

CsonCleksSymbol CsonJsonSymbols[] = {
	[CSON_JSON_MAP_OPEN] = '{',
	[CSON_JSON_MAP_CLOSE] = '}',
	[CSON_JSON_MAP_SEP] = ':',
	[CSON_JSON_ARRAY_OPEN] = '[',
	[CSON_JSON_ARRAY_CLOSE] = ']',
	[CSON_JSON_SEP] = ','
};

char* const CsonJsonSymbolNames[] = {
	[CSON_JSON_MAP_OPEN] = "MapOpen",
	[CSON_JSON_MAP_CLOSE] = "MapClose",
	[CSON_JSON_MAP_SEP] = "MapSep",
	[CSON_JSON_ARRAY_OPEN] = "ArrayOpen",
	[CSON_JSON_ARRAY_CLOSE] = "ArrayClose",
	[CSON_JSON_SEP] = "Sep"
};

CsonCleksComment CsonJsonComments[] = {};

CsonCleksString CsonJsonStrings[] = {
	{'"', '"'}
};

CsonCleksWhitespace CsonJsonWhitespaces[] = {
	' ', '\n'
};

CsonCleksConfig CsonJsonConfig = {
	.words = CsonJsonWords,
	.word_count = cson_arr_len(CsonJsonWords),
	.symbols = CsonJsonSymbols,
	.symbol_count = cson_arr_len(CsonJsonSymbols),
	.strings = CsonJsonStrings,
	.string_count = cson_arr_len(CsonJsonStrings),
	.comments = CsonJsonComments,
	.comment_count = cson_arr_len(CsonJsonComments),
	.whitespaces = CsonJsonWhitespaces,
	.whitespace_count = cson_arr_len(CsonJsonWhitespaces),
	.flags = CSON_CLEKS_FLAGS_ALL_NUMS
};

/* Parser implementation */

bool cson__parse_map(CsonMap *map, CsonClekser *clekser);
bool cson__parse_array(CsonArray *map, CsonClekser *clekser);

bool cson__parse_map(CsonMap *map, CsonClekser *clekser)
{
    cson_error(CsonError_Unimplemented, "cson__parse_map");
    return false;
}

bool cson__parse_array(CsonArray *array, CsonClekser *clekser)
{
    cson_error(CsonError_Unimplemented, "cson__parse_array");
    return false;
}

#define cson_parse(buffer, buffer_size) cson_parse_buffer(buffer, buffer_size, "")

Cson* cson_parse_buffer(char *buffer, size_t buffer_size, char *filename)
{
    if (buffer == NULL) return NULL;
    CsonClekser clekser = cson_cleks_create(buffer, buffer_size, CsonJsonConfig, filename, NULL);
    CsonCleksToken token;
    CsonCleksTokenID map_open_id = cson_cleks_token_id(CSON_CLEKS_SYMBOL, CSON_JSON_MAP_OPEN);
    CsonCleksTokenID array_open_id = cson_cleks_token_id(CSON_CLEKS_SYMBOL, CSON_JSON_ARRAY_OPEN);
    if (!cson_cleks_expect_multiple(&clekser, &token, map_open_id, array_open_id)) return NULL;
    Cson *cson = NULL;
    if (token.id == map_open_id){
        CsonMap *map = cson_map_new();
        if (cson__parse_map(map, &clekser)){
            cson = cson_new_map(map);
        }
    }
    else if (token.id == array_open_id){
        CsonArray *array = cson_array_new();
        if (cson__parse_array(array, &clekser)){
            cson = cson_new_array(array);
        }
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