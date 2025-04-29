#include <stdio.h>
#define CSON_IMPLEMENTATION
#define CSON_WRITE
#define CSON_PARSE
#include "cson2.h"


int main(void)
{
    Cson *cson = cson_read("example.json");
    Cson *map = cson_get(cson, key("company"), key("employees"), index(0), key("manager"));
    cson_print(map);
    cson_map_remove(map, cson_str("id"));
    cson_print(map);
    cson_write(cson, "out.json");
    cson_free();
    return 0;
}
