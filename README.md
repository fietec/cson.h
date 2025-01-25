# cson.h
Library for representing, parsing and writing json data in C. 

## Building
`cson.h` is a stb-style library, which means to use the libary simply include the header file.
To access the function implementations, define `CSON_IMPLEMENTATION`, like this:
```c 
#define CSON_IMPLEMENTATION
#include "cson.h"
```

### Writing
To enable building with the writing functionality of `cson.h`, define `CSON_WRITE` before including the file.

### Parsing
To enable building with the parsing functionality of `cson.h`, define `CSON_PARSE` before including the file.

## Usage
```c 
#include <stdio.h>
#define CSON_IMPLEMENTATION
#define CSON_WRITE
#define CSON_PARSE
#include "cson.h"


int main(void)
{
    Cson *cson = cson_read("example.json");
    CsonMap *map = cson_get_map(cson_get(cson, key("company"), key("employees"), index(0), key("manager")));
    cson_map_print(map);
    cson_map_remove(map, cson_str("id"));
    cson_map_print(map);
    cson_write(cson, "out.json");
    cson_free();
    return 0;
}
```