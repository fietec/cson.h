# cson.h
`cson.h` is a header-only C library for parsing and modifying json data.

The intent of this library is to provide a high-level implementation of a json-like data type with all the needed functionality, as well as the ability to automatically parse and write json data.
For this, `cson` uses the `Cson` struct which may contain any json base type or a `CsonArray`(a dynamic array of `Cson`) or `CsonMap`(a key-based hash map of `Cson`), which are themselves dynamically allocated data structures. All dynamic allocation is done via a custom arena implementation.

Therefore, if you just require a simple streaming json parser, you may be better off looking for something like [yajl](https://github.com/lloyd/yajl). But if you intent to be able to modify parsed json data while using the underlying data structures, `cson.h` may be for you.

## How to build
`cson.h` is an [stb-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) library, which means a single header file and no further dependencies. Define `CSON_IMPLEMENTATION` to access the function implementations, otherwise `cson.h` will act as a regular header-file.
Writing and parsing are for modularity's sake not included by default. To access them, define `CSON_WRITE` and `CSON_PARSE` respectively:
```c 
#define CSON_WRITE
#define CSON_PARSE
#define CSON_IMPLEMENTATION
#include "cson.h"
```

## How to use 
If you want to skip the boring documentation, take a look at the [examples](#examples).
### Dynamic allocation
All data structures are allocated by a custom arena implementation:
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

// simple wrappers for retreiving the stored value
int cson_get_int(Cson *cson);
double cson_get_float(Cson *cson);
bool cson_get_bool(Cson *cson);
CsonStr cson_get_string(Cson *cson);
CsonArray* cson_get_array(Cson *cson);
CsonMap* cson_get_map(Cson *cson);

Cson* cson_get(Cson *cson, (CsonArg)...); // macro
```
To extract data from a nested tree of `Cson` objects, use the `cson_get` function together with the `key` and `index` macros as follows:

*city.json*:
```json
{
	"houses":[
    	{name:"house1", stories:2},
        {name: "house2", stories:3}
    ],
    "name": "FancyCity"
}
```

```c
Cson *city = cson_read("city.json");
Cson *house2 = cson_get(city, key("houses"), index(1));
cson_print(house2);
```
> {name: "house2", stories:3}

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
CsonError cson_array_push(CsonArray *array, Cson *value);
CsonError cson_array_pop(CsonArray *array, size_t index);
Cson* cson_array_get(CsonArray *array, size_t index);
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
CsonError cson_map_insert(CsonMap *map, CsonStr key, Cson *value);
CsonError cson_map_remove(CsonMap *map, CsonStr key);
Cson* cson_map_get(CsonMap *map, CsonStr key);
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
uint32_t cson_str_hash(CsonStr str);
bool cson_str_equals(CsonStr a, CsonStr b);
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
To learn how to use the lexer, refer to [cleks2.h](https://github.com/fietec/cleks2.h), which is a general purpose version of  the `CsonLexer`.

### Examples
#### Parsing and modification
```c
#define CSON_IMPLEMENTATION
#define CSON_WRITE
#define CSON_PARSE
#include "cson.h"


int main(void)
{
	// parse a json file and create a Cson structure from it
    Cson *cson = cson_read("example.json"); 
    // retreive a Cson struct from the nested tree of values
    Cson *managers = cson_get(cson, key("company"), key("employees"), index(0), key("manager"));
    // extract the CsonMap from the Cson to have access to the CsonMap functions
    CsonMap *map = cson_get_map(managers);
    // print the current status of the map to the console
    cson_map_print(map);
    // remove the field with the key "id"
    cson_map_remove(map, cson_str("id"));
    // print the new status of the map
    cson_map_print(map);
    // write the root object to a file 
    cson_write(cson, "out.json");
    // free the memory of the default arena
    cson_free();
    return 0;
}

```