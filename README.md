# cson.h
`cson.h` is a header-only C library for parsing and modifying json data.

The intent of this library is to provide a high-level implementation of a json-like data type with all the needed functionality, as well as the ability to automatically parse and write json data.

**Warning**: This library is still in development and may yet experience critical changes

```c
#define CSON_IMPLEMENTATION
#define CSON_WRITE
#define CSON_PARSE
#include "cson.h"


int main(void)
{
    // parse a json file and create a Cson structure
    Cson *cson = cson_read("example.json"); 
    // retreive a Cson struct from the nested tree of values
    Cson *managers = cson_get(cson, key("company"), key("employees"), index(0), key("manager"));
    // print the current status of the map to the console
    cson_print(managers);
    // remove the field with the key "id"
    cson_map_remove(managers, cson_str("id"));
    // print the new status of the map
    cson_print(managers);
    // write the root object to a file 
    cson_write(cson, "out.json");
    // free the memory of the default arena
    cson_free();
    return 0;
}
```

## How to build
`cson.h` is an [stb-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) library, which means a single header file and no further dependencies. Define `CSON_IMPLEMENTATION` to access the function implementations, otherwise `cson.h` will act as a regular header-file.
Writing and parsing are for modularity's sake not included by default. To access them, define `CSON_WRITE` and `CSON_PARSE` respectively:
```c 
#define CSON_WRITE
#define CSON_PARSE
#define CSON_IMPLEMENTATION
#include "cson.h"
```

## Documentation
### Dynamic allocation
All data structures are allocated by a custom arena implementation (inspired by Tsoding's [arena](https://github.com/tsoding/arena)):
```c
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
```
Functions:
```c
void* cson_alloc(CsonArena *arena, size_t size);
void* cson_realloc(CsonArena *arena, void *old_ptr, size_t old_size, size_t new_size);
void cson_free(); // free the default arena
void cson__free(Cson Arena *arena);
void cson_swap_arena(CsonArena *arena);
void cson_swap_and_free_arena(CsonArena *arena);
```
By default, `cson.h` always uses the `cson_default_arena` to allocate its memory. To change the currently active `CsonArena`, swap it to a custom defined arena, using `cson_swap_arena` or to also free the previous `cson_swap_and_free_arena`.
### Data Structures
#### Cson
```c 
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
```
This is the underlying data structure for storing a single json value. It contains a `type` field defining, well, the type of the value stored in the union `value`.
Functions:
```c
// allocate new cson with accordinly set value and type
Cson* cson_new(void);
Cson* cson_new_int(int value);
Cson* cson_new_float(double value);
Cson* cson_new_bool(bool value);
Cson* cson_new_string(CsonStr value);
Cson* cson_new_cstring(char *cstr);
Cson* cson_new_array(CsonArray *value);
Cson* cson_new_map(CsonMap *value);

// type checking
bool cson_is_int(Cson *cson);
bool cson_is_float(Cson *cson);
bool cson_is_bool(Cson *cson);
bool cson_is_string(Cson *cson);
bool cson_is_array(Cson *cson);
bool cson_is_map(Cson *cson);
bool cson_is_null(Cson *cson);

// simple wrappers for retreiving the stored value (macros), va-args can be empty
int cson_get_int(Cson *cson, (CsonArg) ...);
double cson_get_float(Cson *cson, (CsonArg) ...);
bool cson_get_bool(Cson *cson, (CsonArg) ...);
CsonStr cson_get_string(Cson *cson, (CsonArg) ...);
CsonArray* cson_get_array(Cson *cson, (CsonArg) ...);
CsonMap* cson_get_map(Cson *cson, (CsonArg) ...);

Cson* cson_get(Cson *cson, (CsonArg)...); // macro

// calculate the amount of dynamic memory used for a nested structure
size_t cson_memsize(Cson *cson);
// get the number of arguments stored in the current level (works only for Cson_Array and Cson_Map)
size_t cson_len(Cson *cson);

```
To extract data from a nested tree of `Cson` objects, use the `cson_get` function together with the `key` and `index` macros as follows:

*city.json*:
```json
{
    "houses":[
        {"name":"house1", "stories":2},
        {"name": "house2", "stories":3}
    ],
    "name": "FancyCity"
}
```

```c
Cson *city = cson_read("city.json");
Cson *house2 = cson_get(city, key("houses"), index(1));
cson_print(house2);
```
> {"name": "house2", "stories":3}

or using the `cson_get_<type>` wrappers to access the underlying data types:
```c 
Cson *city = cson_read("city.json");
int stories = cson_get_int(city, key("houses"), index(1), key("stories"));
printf("%d\n", stories);
```
> 3

#### CsonArray
```c 
struct CsonArray{
    Cson **items;
    size_t size;
    size_t capacity;
};
```
A simple dynamic array of `Cson` values.

Functions:
```c 
CsonArray* cson_array_new(void);
CsonError cson_array_push(Cson *array, Cson *value);
CsonError cson_array_pop(Cson *array, size_t index);
Cson* cson_array_get(Cson *array, size_t index);
Cson* cson_array_get_last(Cson *array);

size_t cson_array_memsize(Cson *array);
```

#### CsonMap
```c
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
```
A simple hash map of `CsonStr` - `Cson` value-key pairs.

Functions:
```c
CsonMap* cson_map_new(void);
CsonError cson_map_insert(Cson *map, CsonStr key, Cson *value);
CsonError cson_map_remove(Cson *map, CsonStr key);
Cson* cson_map_get(Cson *map, CsonStr key);
Cson* cson_map_keys(Cson *map); // returns CsonArray of keys

size_t cson_map_memsize(Cson *map);
```

#### CsonStr
```c
struct CsonStr{
    char *value;
    size_t len;
};
```
A string wrapper for storing strings. (yet to decide if this is a good idea)

Functions:
```c
CsonStr cson_str(char *cstr);
CsonStr cson_str_new(char *cstr); // allocates new string
CsonStr cson_str_dup(CsonStr str); // allocates new string
uint32_t cson_str_hash(CsonStr str);
bool cson_str_equals(CsonStr a, CsonStr b);

size_t cson_str_memsize(CsonStr str);
```

### Writing
To enable the writing functionality, define `CSON_WRITE`, as shown above.

Functions:
```c
void cson_print(Cson *cson);  // (macro)
void cson_array_print(CsonArray *array); // (macro) 
void cson_map_print(CsonMap *map); // (macro)
void cson_fprint(Cson *value, FILE *file, size_t indent);
void cson_array_fprint(CsonArray *array, FILE *file, size_t indent);
void cson_map_fprint(CsonMap *map, FILE *file, size_t indent);
bool cson_write(Cson *json, char *filename);
```

### Parsing
To enable the parsing functionality, define `CSON_PARSE`, as shown above.

Functions:
```c
Cson* cson_parse(char *buffer, size_t buffer_size); // (macro)
Cson* cson_parse_buffer(char *buffer, size_t buffer_size, char *filename);
Cson* cson_read(char *filename);
```
#### Lexing
Parsing is achieved via a custom json lexer (`CsonLexer`), which may be used on its own.
The necessary data strucures are:
```c
typedef struct{
    CsonTokenType type;
    char *t_start;
    char *t_end;
    size_t len;
    CsonLoc loc;
} CsonToken;

typedef struct{
    char *buffer;
    size_t buffer_size;
    size_t index;
    CsonLoc loc;
} CsonLexer;
```
The `CsonLexer` is an intermediate structure holding important runtime information of the lexing process. A `CsonToken` is the representation of a json token and contains information about the `type` and the location in the buffer.
Valid `CsonTokenTypes` are:
```c
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
CsonToken_Invalid, // an invalid token, lexer will throw error on occurance 
CsonToken_End // the end of the buffer
```

Functions:
```c
CsonLexer cson_lex_init(char *buffer, size_t buffer_size, char *filename);
bool cson_lex_next(CsonLexer *lexer, CsonToken *token);
bool cson_lex_expect(CsonLexer *lexer, CsonToken *token, (CsonTokenType) ...); // (macro)
bool cson_lex_extract(CsonToken *token, char *buffer, size_t buffer_size);
```
To learn how to use the lexer, refer to [jexc.h](https://github.com/fietec/jexc.h), which is a standalone version of the `CsonLexer`.

## License
This project is licensed under the MIT License. View the `LICENSE` file for details.