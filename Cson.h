#ifndef _CSON_H
#define _CSON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "CsonUtils.h"

#define CSON_DEF_ARRAY_CAPACITY 16
#define CSON_DEF_ARRAY_MF        2
#define CSON_MAP_CAPACITY       16
#define CSON_DEF_INDENT          4

#define cson_debug(msg, ...) (printf("%s%s:%d: " msg ANSI_END "\n", ANSI_RGB(0, 255, 0),__FILE__, __LINE__, ## __VA_ARGS__))
#define cson_eprintln(msg, ...) (fprintf(stderr, "%s%s:%d: [ERROR] " msg ANSI_END "\n", ANSI_RGB(255, 0, 0), __FILE__, __LINE__, ## __VA_ARGS__))

typedef enum{
    Cson_Integer,
    Cson_Double,
    Cson_Boolean,
    Cson_String,
    Cson_Null,
    Cson_Array,
    Cson_Map
} CsonType;

const char* const CsonTypeStrs[] = {
    [Cson_Integer] = "int",
    [Cson_Double]  = "double",
    [Cson_Boolean] = "bool",
    [Cson_String]  = "String",
    [Cson_Null]    = "Null",
    [Cson_Array]   = "Array",
    [Cson_Map]     = "Map"
};

typedef enum{
    CsonError_Success,
    // parsing
    // run-time
    CsonError_ItemNotFound,
    CsonError_FileNotFound,
    CsonError_Alloc,
    CsonError_Param
} CsonError;

typedef enum{
    CsonArg_Index,
    CsonArg_Key
} CsonArgType;

typedef struct Cson Cson;
typedef struct CsonArg CsonArg;
typedef struct CsonArray CsonArray;
typedef struct CsonMap CsonMap;
typedef struct CsonMapValue CsonMapValue;

struct Cson{
    union{
        void *null;
        int integer;
        double floating;
        bool boolean;
        char *string;
        CsonArray *array;
        CsonMap *map;
    } value;
    CsonType type;
};

struct CsonArray{
    Cson **values;
    size_t capacity;
    size_t size;
};

struct CsonMap{
    CsonMapValue **values;
    size_t capacity;
    size_t size;
};

struct CsonMapValue{
    Cson *value;
    char *key;
    CsonMapValue *next;
};

struct CsonArg{
    CsonArgType type;
    union{
        char *key;
        size_t index;
    } value;
    CsonArg *next;
};

/**
 * Creates an empty Cson object.
 *
 * @return A pointer to the newly created empty Cson object.
 */
Cson* Cson_create(void);

/**
 * Frees the memory allocated for a Cson object.
 *
 * @param json A pointer to the Cson object to free.
 */
void Cson_free(Cson *json);

/**
 * Returns the size of the Cson object in memory.
 *
 * @param json A pointer to the Cson object.
 * @return The size of the Cson object in bytes
 */
size_t Cson_size(Cson *json);

/**
 * Retrieves a nested value from a Cson object.
 *
 * @param json A pointer to the Cson object.
 * @param arg_count The number of arguments to specify the path (e.g., array indices or map keys).
 * @param ... A variable number of CsonArg pointers specifying the path.
 * @return A pointer to the nested Cson object, or NULL if not found.
 */
Cson* Cson_get(Cson *json, size_t arg_count, ...);

/**
 * Duplicates a Cson object, creating a deep copy.
 *
 * @param json A pointer to the Cson object to duplicate.
 * @return A pointer to the newly created Cson object that is a deep copy of the original.
 */
Cson* Cson_dup(Cson *json);

/**
 * Creates a CsonArg object, used for specifying arguments in Cson_get().
 *
 * @param type The type of the argument (e.g., index or key).
 * @param value The value of the argument.
 * @return A pointer to the newly created CsonArg object.
 */
CsonArg* CsonArg_create(CsonArgType type, void* value);

/**
 * Macro to create a CsonArg for use in Cson_get() without compiler warnings.
 *
 * @param type The type of the argument (e.g., CsonArg_Index, CsonArg_Key).
 * @param value The value of the argument (e.g., index value or key string).
 * @return A pointer to the newly created CsonArg object.
 */
#define Arg(type, value) (CsonArg_create((type), (void*) (value)))

/**
 * Macro to create a CsonArg representing an index, for use in Cson_get().
 *
 * @param ind The index value.
 * @return A pointer to the newly created CsonArg object representing the index.
 */
#define Index(ind) (CsonArg_create(CsonArg_Index, (ind)))

/**
 * Macro to create a CsonArg representing a key, for use in Cson_get().
 *
 * @param key The key string.
 * @return A pointer to the newly created CsonArg object representing the key.
 */
#define Key(key) (CsonArg_create(CsonArg_Key, (key)))

/**
 * Creates a Cson object from an integer value.
 *
 * @param value The integer value to convert.
 * @return A pointer to the newly created Cson object representing the integer.
 */
Cson* Cson_from_int(int value);

/**
 * Creates a Cson object from a double value.
 *
 * @param value The double value to convert.
 * @return A pointer to the newly created Cson object representing the double.
 */
Cson* Cson_from_double(double value);

/**
 * Creates a Cson object from a boolean value.
 *
 * @param value The boolean value to convert.
 * @return A pointer to the newly created Cson object representing the boolean.
 */
Cson* Cson_from_boolean(bool value);

/**
 * Creates a Cson object from a string value.
 *
 * @param value The string value to convert.
 * @return A pointer to the newly created Cson object representing the string.
 */
Cson* Cson_from_string(char* value);

/**
 * Creates a Cson object representing a null value.
 *
 * @return A pointer to the newly created Cson object representing null.
 */
Cson* Cson_from_null(void);

/**
 * Creates a Cson object from an array.
 *
 * @param array A pointer to the CsonArray to convert.
 * @return A pointer to the newly created Cson object representing the array.
 */
Cson* Cson_from_array(CsonArray* array);

/**
 * Creates a Cson object from a map.
 *
 * @param map A pointer to the CsonMap to convert.
 * @return A pointer to the newly created Cson object representing the map.
 */
Cson* Cson_from_map(CsonMap* map);

/**
 * Extracts an integer value from a Cson object.
 *
 * @param value A pointer to the Cson object.
 * @return The integer value if the Cson object is of type Cson_Integer; otherwise, 0.
 */
int Cson_to_int(Cson *value);

/**
 * Extracts a double value from a Cson object.
 *
 * @param value A pointer to the Cson object.
 * @return The double value if the Cson object is of type Cson_Double; otherwise, 0.0.
 */
double Cson_to_double(Cson *value);

/**
 * Extracts a boolean value from a Cson object.
 *
 * @param value A pointer to the Cson object.
 * @return The boolean value if the Cson object is of type Cson_Boolean; otherwise, false.
 */
bool Cson_to_boolean(Cson *value);

/**
 * Extracts a string value from a Cson object.
 *
 * @param value A pointer to the Cson object.
 * @return The string value if the Cson object is of type Cson_String; otherwise, NULL.
 */
char* Cson_to_string(Cson *value);

/**
 * Extracts an array value from a Cson object.
 *
 * @param value A pointer to the Cson object.
 * @return A pointer to the CsonArray if the Cson object is of type Cson_Array; otherwise, NULL.
 */
CsonArray* Cson_to_array(Cson *value);

/**
 * Extracts a map value from a Cson object.
 *
 * @param value A pointer to the Cson object.
 * @return A pointer to the CsonMap if the Cson object is of type Cson_Map; otherwise, NULL.
 */
CsonMap* Cson_to_map(Cson *value);


/**
 * Creates an empty CsonArray object.
 *
 * @return A pointer to the newly created empty CsonArray object.
 */
CsonArray* CsonArray_create(void);

/**
 * Frees the memory allocated for a CsonArray object, including its elements.
 *
 * @param array A pointer to the CsonArray object to free.
 */
void CsonArray_free(CsonArray *array);

/**
 * Appends a Cson value to the end of a CsonArray.
 *
 * @param array A pointer to the CsonArray object.
 * @param value A pointer to the Cson object to append.
 * @return A CsonError indicating success or failure.
 */
CsonError CsonArray_append(CsonArray *array, Cson *value);

/**
 * Retrieves the Cson object at a specified index in a CsonArray.
 *
 * @param array A pointer to the CsonArray object.
 * @param index The index of the element to retrieve.
 * @return A pointer to the Cson object at the specified index, or NULL if the index is out of bounds.
 */
Cson* CsonArray_get(CsonArray *array, size_t index);

/**
 * Sets the value of a Cson object at a specified index in a CsonArray.
 *
 * @param array A pointer to the CsonArray object.
 * @param index The index where the value should be set.
 * @param value A pointer to the Cson object to set.
 * @return A CsonError indicating success or failure.
 */
CsonError CsonArray_set(CsonArray *array, size_t index, Cson *value);

/**
 * Removes a Cson object at a specified index from a CsonArray.
 *
 * @param array A pointer to the CsonArray object.
 * @param index The index of the element to remove.
 * @return A CsonError indicating success or failure.
 */
CsonError CsonArray_remove(CsonArray *array, size_t index);

/**
 * Duplicates a CsonArray object, creating a deep copy of the array and its elements.
 *
 * @param array A pointer to the CsonArray object to duplicate.
 * @return A pointer to the newly created CsonArray object that is a deep copy of the original.
 */
CsonArray* CsonArray_dup(CsonArray *array);

/**
 * Returns the size of a CsonArray object in memory.
 *
 * @param array A pointer to the CsonArray object.
 * @return The size of the CsonArray in bytes.
 */
size_t CsonArray_size(CsonArray *array);

/**
 * Creates an empty CsonMap object.
 *
 * @return A pointer to the newly created empty CsonMap object.
 */
CsonMap* CsonMap_create(void);

/**
 * Frees the memory allocated for a CsonMap object, including its elements.
 *
 * @param map A pointer to the CsonMap object to free.
 */
void CsonMap_free(CsonMap *map);

/**
 * Retrieves the Cson object associated with a specified key in a CsonMap.
 *
 * @param map A pointer to the CsonMap object.
 * @param key The key associated with the value to retrieve.
 * @return A pointer to the Cson object associated with the key, or NULL if the key does not exist.
 */
Cson* CsonMap_get(CsonMap *map, char *key);

/**
 * Inserts a key-value pair into a CsonMap. Values with the same key are overwritten.
 *
 * @param map A pointer to the CsonMap object.
 * @param key The key to insert or update.
 * @param value A pointer to the Cson object to associate with the key.
 * @return A CsonError indicating success or failure.
 */
CsonError CsonMap_insert(CsonMap *map, char *key, Cson *value);

/**
 * Removes a key-value pair from a CsonMap by key.
 *
 * @param map A pointer to the CsonMap object.
 * @param key The key of the element to remove.
 * @return A CsonError indicating success or failure.
 */
CsonError CsonMap_remove(CsonMap *map, char *key);

/**
 * Duplicates a CsonMap object, creating a deep copy of the map and its elements.
 *
 * @param map A pointer to the CsonMap object to duplicate.
 * @return A pointer to the newly created CsonMap object that is a deep copy of the original.
 */
CsonMap* CsonMap_dup(CsonMap *map);

/**
 * Returns the size of a CsonMap object in memory.
 *
 * @param map A pointer to the CsonMap object.
 * @return The size of the CsonMap in bytes.
 */
size_t CsonMap_size(CsonMap *map);


#define CSON_IMPLEMENTATION // DEBUG
#ifdef CSON_IMPLEMENTATION
#define STRLIB_IMPLEMENTATION

CsonArray* CsonArray_create(void)
{
    CsonArray* array = (CsonArray*) malloc(sizeof(*array));
    if (array){
        Cson** values = (Cson**) calloc(CSON_DEF_ARRAY_CAPACITY, sizeof(*values));
        if (!values){
            free(array);
            return NULL;
        }
        array->values = values;
        array->capacity = CSON_DEF_ARRAY_CAPACITY;
        array->size = 0;
    }
    return array;
}

void CsonArray_free(CsonArray *array)
{
    if (array){
        if (array->values){
            for (size_t i=0; i<array->size; ++i){
                Cson_free(array->values[i]);
            }
            free(array->values);
        }
        free(array);
    }
}

CsonError CsonArray_append(CsonArray *array, Cson *value)
{
    if (!array || !value) return CsonError_Param;
    if (array->size >= array->capacity){
        size_t new_capacity = array->capacity*CSON_DEF_ARRAY_MF;
        Cson **new_values = (Cson**) realloc(array->values, sizeof(*new_values)*new_capacity);
        if (!new_values) return CsonError_Alloc;
        array->values = new_values;
        array->capacity = new_capacity;
    }
    array->values[array->size++] = value;
    return CsonError_Success;
}

Cson* CsonArray_get(CsonArray *array, size_t index)
{
    if (!array || index >= array->size) return NULL;
    return array->values[index];
}

CsonError CsonArray_set(CsonArray *array, size_t index, Cson *value)
{
    if (!array || !value || index >= array->size) return CsonError_Param;
    Cson_free(array->values[index]);
    array->values[index] = value;
    return CsonError_Success;
}

CsonError CsonArray_remove(CsonArray *array, size_t index)
{
    if (!array || index >= array->size) return CsonError_Param;
    Cson_free(array->values[index]);
    for (size_t i=index; i<array->size-1; ++i){
        array->values[i] = array->values[i+1];
    }
    array->values[array->size-1] = NULL;
    return CsonError_Success;
}

size_t CsonArray_size(CsonArray *array)
{
    if (!array) return 0;
    size_t size = sizeof(CsonArray);
    if (array->values){
        for (size_t i=0; i<array->size; ++i){
            size += Cson_size(array->values[i]);
        }
    }
    return size;
}

CsonArray* CsonArray_dup(CsonArray *array)
{
    if (!array) return NULL;
    CsonArray *new_array = (CsonArray*) malloc(sizeof(*new_array));
    if (new_array){
        Cson **new_values = (Cson**) calloc(array->capacity, sizeof(*new_values));
        if (!new_values){
            free(new_array);
            return NULL;
        }
        new_array->values = new_values;
        new_array->capacity = array->capacity;
        new_array->size = array->size;
        for (size_t i=0; i<array->size; ++i){
            Cson *dup_value = Cson_dup(array->values[i]);  
            if (!dup_value){
                // TODO: handle error                
            }      
            new_array->values[i] = dup_value;
        }
    }
    return new_array;
}

unsigned long CsonMap_hash(char *key)
{
    // DJB2 hashing
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)){
        hash = ((hash << 5) + hash) + c;
    }
    return hash % CSON_MAP_CAPACITY;
}

CsonMapValue* CsonMapValue_create(char *key, Cson *value)
{
    CsonMapValue *map_value = (CsonMapValue*) malloc(sizeof(*value));
    if (map_value){
        char *new_key = str_dup(key);
        if (!new_key){
            free(map_value);
            return NULL;
        }
        map_value->key = new_key;
        map_value->value = value;
        map_value->next = NULL;
    }
    return map_value;
}

void CsonMapValue_free(CsonMapValue *value)
{
    CsonMapValue *next;
    while (value){
        next = value->next;
        Cson_free(value->value);
        free(value->key);
        free(value);
        value = next;
    }
}

CsonMapValue* CsonMapValue_dup(CsonMapValue *value)
{
    if (!value) return NULL;
    CsonMapValue *new_value = (CsonMapValue*) malloc(sizeof(*new_value));
    if (new_value){
        char *new_key = str_dup(value->key);
        if (!new_key){
            free(new_value);
            return NULL;
        }
        new_value->key = new_key;
        new_value->value = Cson_dup(value->value);
        new_value->next = CsonMapValue_dup(value->next);
    }
    return new_value;
}

CsonMap* CsonMap_create(void)
{
    CsonMap *map = (CsonMap*) malloc(sizeof(*map));
    if (map){
        CsonMapValue **values = (CsonMapValue**) calloc(CSON_MAP_CAPACITY, sizeof(CsonMapValue*));
        if (!values){
            free(map);
            return NULL;
        }
        for (size_t i=0; i<CSON_MAP_CAPACITY; ++i){
            values[i] = NULL;
        }
        map->capacity = CSON_MAP_CAPACITY;
        map->size = 0;
        map->values = values;
    }
    return map;
}

void CsonMap_free(CsonMap *map)
{
    if (map){
        if (map->values){
            for (size_t i=0; i<map->capacity; ++i){
                CsonMapValue_free(map->values[i]);
            }
        }
    }
}

CsonError CsonMap_insert(CsonMap *map, char *key, Cson *value)
{
    if (!map || !map->values || !key || !value) return CsonError_Param;
    size_t index = (size_t) CsonMap_hash(key);
    CsonMapValue *curr_value = map->values[index];
    CsonMapValue *new_value = CsonMapValue_create(key, value);
    if (!new_value) return CsonError_Alloc;
    if (curr_value != NULL){
        if (strcmp(curr_value->key, key) == 0){
            CsonMapValue_free(curr_value);
            map->values[index] = new_value;
            return CsonError_Success;
        }
        while (curr_value->next){
            if (strcmp(curr_value->next->key, key) == 0){
                new_value->next = curr_value->next->next;
                CsonMapValue_free(curr_value->next);
                curr_value->next = new_value;
                return CsonError_Success;
            }
            curr_value = curr_value->next;
        }
        curr_value->next = new_value;
    }
    else{
        map->values[index] = new_value;
    }
    map->size ++;
    return CsonError_Success;
}

Cson* CsonMap_get(CsonMap *map, char *key)
{
    if (!map || !map->values || !key) return NULL;
    CsonMapValue *value = map->values[CsonMap_hash(key)];
    while (value){
        if (strcmp(value->key, key) == 0){
            return value->value;
        }
        value = value->next;
    }
    return NULL;
}

CsonError CsonMap_remove(CsonMap *map, char *key)
{
    if (!map || !map->values || !key) return CsonError_Param;
    size_t index = CsonMap_hash(key);
    CsonMapValue *curr_value = map->values[index];
    if (!curr_value) return CsonError_ItemNotFound;
    if (strcmp(curr_value->key, key) == 0){
        map->values[index] = curr_value->next;
        CsonMapValue_free(curr_value);
        map->size --;
        return CsonError_Success;
    }
    CsonMapValue *last_value = curr_value;
    curr_value = curr_value->next;
    while (curr_value){
        if (strcmp(curr_value->key, key) == 0){
            last_value->next = curr_value->next;
            CsonMapValue_free(curr_value);
            map->size --;
            return CsonError_Success;
        }
        last_value = curr_value;
        curr_value = curr_value->next;
    }
    return CsonError_ItemNotFound;
}

size_t CsonMap_size(CsonMap *map)
{
    if (!map) return 0;
    size_t size = sizeof(*map);
    CsonMapValue *curr_value;
    for (size_t i=0; i<map->capacity; ++i){
        curr_value = map->values[i];
        while (curr_value){
            size += (sizeof(*curr_value) + Cson_size(curr_value->value) + str_size(curr_value->key));
            curr_value = curr_value->next;
        }
    }
    return size;
}

CsonMap* CsonMap_dup(CsonMap *map)
{
    if (!map) return NULL;
    CsonMap *new_map = (CsonMap*) malloc(sizeof(*new_map));
    if (new_map){
        CsonMapValue **new_values = (CsonMapValue**) calloc(map->capacity, sizeof(*new_values));
        if (!new_values){
            free(new_map);
            return NULL;
        }
        new_map->values = new_values;
        new_map->capacity = map->capacity;
        new_map->size = map->size;
        for (size_t i=0; i<map->capacity; ++i){
            CsonMapValue *new_value = CsonMapValue_dup(map->values[i]);
            new_map->values[i] = new_value;
        }
    }
    return new_map;
}

Cson* Cson_create()
{
    Cson *json = (Cson*) malloc(sizeof(*json));
    if (json){
        json->type = Cson_Null;
        json->value.null = NULL;
    }
    return json;
}

void Cson_free(Cson *json)
{
    if (json){
        switch (json->type){
            case Cson_Array:{
                CsonArray_free(json->value.array);
            }break;
            case Cson_Map:{
                CsonMap_free(json->value.map);
            }break;
            case Cson_String:{
                free(json->value.string);
            } break;
        }
        free(json);
    }
}

Cson* Cson_from_int(int value) 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_Integer;
        json->value.integer = value;
    }
    return json;
}

Cson* Cson_from_double(double value) 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_Double;
        json->value.floating = value;
    }
    return json;
}

Cson* Cson_from_boolean(bool value) 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_Boolean;
        json->value.boolean = value;  
    }
    return json;
}

Cson* Cson_from_string(char* value) 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_String;
        json->value.string = str_dup(value); 
    }
    return json;
}

Cson* Cson_from_null() 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_Null;
    }
    return json;
}

Cson* Cson_from_array(CsonArray* array) 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_Array;
        json->value.array = array;  
    }
    return json;
}

Cson* Cson_from_map(CsonMap* map) 
{
    Cson* json = (Cson*) malloc(sizeof(*json));
    if (json) {
        json->type = Cson_Map;
        json->value.map = map;
    }
    return json;
}

int Cson_to_int(Cson *value)
{
    if (!value) return 0;
    if (value->type != Cson_Integer) {
        cson_eprintln("Could not extract integer from Cson of type %s!", CsonTypeStrs[Cson_Integer]);
        return 0;
    }
    return value->value.integer;
}

double Cson_to_double(Cson *value)
{
    if (!value) return 0.0;
    if (value->type != Cson_Double) {
        cson_eprintln("Could not extract double from Cson of type %s!", CsonTypeStrs[Cson_Double]);
        return 0.0;
    }
    return value->value.floating;
}

bool Cson_to_boolean(Cson *value)
{
    if (!value) return false;
    if (value->type != Cson_Boolean) {
        cson_eprintln("Could not extract boolean from Cson of type %s!", CsonTypeStrs[Cson_Boolean]);
        return false;
    }
    return value->value.boolean;
}

char* Cson_to_string(Cson *value)
{
    if (!value) return NULL;
    if (value->type != Cson_String) {
        cson_eprintln("Could not extract string from Cson of type %s!", CsonTypeStrs[Cson_String]);
        return NULL;
    }
    return value->value.string;
}

CsonArray* Cson_to_array(Cson *value)
{
    if (!value) return NULL;
    if (value->type != Cson_Array) {
        cson_eprintln("Could not extract array from Cson of type %s!", CsonTypeStrs[Cson_Array]);
        return NULL;
    }
    return value->value.array;
}

CsonMap* Cson_to_map(Cson *value)
{
    if (!value) return NULL;
    if (value->type != Cson_Map) {
        cson_eprintln("Could not extract map from Cson of type %s!", CsonTypeStrs[Cson_Map]);
        return NULL;
    }
    return value->value.map;
}

CsonArg* CsonArg_create(CsonArgType type, void *value)
{
    CsonArg *arg = (CsonArg*) malloc(sizeof(*arg));
    if (arg){
        arg->type = type;
        arg->next = NULL;
        if (type == CsonArg_Key){
            char *arg_key = str_dup((char*) value);
            if (!arg_key){
                free(arg);
                return NULL;
            }
            arg->value.key = arg_key;
        }
        else{
            arg->value.index = (size_t) value;
        }
    }
    return arg;
}

Cson* CsonArg_get(Cson *value, CsonArg *arg)
{
    if (!value || !arg) return NULL;
    if (value->type == Cson_Map && arg->type == CsonArg_Key){
        Cson *next = CsonMap_get(value->value.map, arg->value.key);
        if (!next) return NULL;
        if (arg->next) return CsonArg_get(next, arg->next);
        return next;
    }
    else if (value->type == Cson_Map && arg->type == CsonArg_Key){
        Cson *next = CsonArray_get(value->value.array, arg->value.index);
        if (!next) return NULL;
        if (arg->next) return CsonArg_get(next, arg->next);
        return next;
    }
    return NULL;
}

void CsonArg_free(CsonArg *arg)
{
    CsonArg *next;
    while (arg){
        next = arg->next;
        if (arg->type == CsonArg_Key){
            free(arg->value.key);
        }
        free(arg);
        arg = next;
    }
}

Cson* Cson_get(Cson *json, size_t arg_count, ...)
{
    if (!json || arg_count == 0) return NULL;
    va_list args;
    va_start(args, arg_count);
    CsonArg *first_arg;
    CsonArg *curr_arg;
    CsonArg *new_arg;
    for (size_t i=0; i<arg_count; ++i){
        new_arg = va_arg(args, CsonArg*);
        if (!new_arg) return NULL;
        if (i==0) first_arg = new_arg;
        else curr_arg->next = new_arg;
        curr_arg = new_arg;
    }
    va_end(args);
    Cson *value = CsonArg_get(json, first_arg);
    if (!value) cson_eprintln("Could not find any value with this path!");
    CsonArg_free(first_arg);
    return value;
}

Cson* Cson_dup(Cson *json)
{
    if (!json) return NULL;
    Cson *new_json = (Cson*) malloc(sizeof(*json));
    if (new_json){
        switch (json->type){
            case Cson_Array: {
                CsonArray *new_array = CsonArray_dup(json->value.array);
                if (!new_array){
                    free(new_json);
                    return NULL;
                }
                new_json->value.array = new_array;
            }break;
            case Cson_Map:{
                CsonMap *new_map = CsonMap_dup(json->value.map);
                if (!new_map){
                    free(new_json);
                    return NULL;
                }   
                new_json->value.map = new_map;
            }break;
            case Cson_String:{
                char *new_string = str_dup(json->value.string);
                if (!new_string){
                    free(new_json);
                    return NULL;
                }
                new_json->value.string = new_string;
            }break;
            default:{
                new_json->value = json->value;
            }
        }
        new_json->type = json->type;
    }
    return new_json;
}

size_t Cson_size(Cson *json)
{
    if (!json) return 0;
    size_t size = sizeof(*json);
    switch (json->type){
        case Cson_Array:{
            size += CsonArray_size(json->value.array);
        }break;
        case Cson_Map:{
            size += CsonMap_size(json->value.map);
        }break;
        case Cson_String:{
            size += str_size(json->value.string);
        }break;
    }
    return size;
}

#endif // CSON_IMPLEMENTATION

#endif // _CSON_H   