#include <stdio.h>
#define CSON_IMPLEMENTATION
#define CSON_WRITE
#define CSON_PARSE
#include "cson.h"


int main(void)
{
    Cson *cson = cson_read("test.json");
    CsonMap *map = cson_get_map(cson_get(cson, key("company"), key("employees"), index(0), key("manager")));
    cson_map_print(map);
    cson_map_remove(map, cson_str("id"));
    cson_map_print(map);
    cson_write(cson, "out.json");
    cson_free();
    return 0;
}
