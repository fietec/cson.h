#include <stdio.h>
#define CSON_IMPLEMENTATION
#include "cson.h"


int main(void)
{
    Cson *cson = cson_read("example.json");
    Cson *map = cson_get(cson, key("company"), key("employees"), index(0), key("manager"));
    if (!cson_is_map(map)) return 1;
    cson_print(map);
    cson_map_remove(map, cson_str("id"));
    cson_print(map);
    int64_t manager_id;
    if (!cson_get_int(&manager_id, cson, key("company"), key("employees"), index(1), key("manager"), key("id"))) return 1;
    printf("The id of the second employee's manager is: %I64d\n", manager_id);
    cson_write(cson, "out.json");
    cson_free();
    return 0;
}
