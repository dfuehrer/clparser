#ifndef MAP_H
#define MAP_H

#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAP_ARR_LEN UINT8_MAX

#define MIN(x, y)   ((y < x) ? y : x)

typedef enum DataType{
    INT,
    CHAR,
    STR,
    BOOL,
    STRING_VIEW,
} DataType;

typedef struct {
    char * str;
    int len;
} StringView;

typedef struct llist_s {
    StringView sv;
    struct llist_s * next;
} llist_t;

// TODO define errors in an enum somewhere



typedef struct MapNode_s{
    const char * * names;
    int * nameLens;
    int namesLen;
    // TODO figure out what this should actually hold and put that instead
    //  alternatively let it hold typed data in the void pointer and then store a type to know how to access it (use an enum or something for int, str, bool, etc)
    //const void * * data_ptr;
    void * data;
    // TODO add the type to this prolly
    DataType type;
    struct MapNode_s * next;
} MapNode;

typedef struct map_s{
    MapNode * ptrArray[MAP_ARR_LEN];
    int len;
} map_t;

const static bool flagFalse = false;
const static bool flagTrue  = true;

void initMap(map_t * map);
// fmt is a "format string" that specifies what types the variadic keys are
// %S is a StringView, %s is a char[] for the key, %d or %i is an int for the string length
// length must follow a string, otherwise it will error
void addMapMembers(map_t * map, void * data, DataType type, const char fmt[], ...);
void addMapMembers_fromList(map_t * map, void * data, DataType type, llist_t * head, int numKeys);

const void * setMapMemberData(map_t * map, void * data_addr, const char * key, int len);

MapNode * getMapNode(const map_t * map, const char * key, int len);
void * getMapMemberData(const map_t * map, const char * key, int len);
int  getMapMember_int (map_t * map, const char * key, int len);
bool getMapMember_bool(map_t * map, const char * key, int len);
char getMapMember_char(map_t * map, const char * key, int len);

//void delMembers(map_t map, ...);
void freeMap(map_t * map);

MapNode * popMapNode(map_t * map, const char * key, int len);

void printMap(map_t * map);


typedef int (*mapIterFuncType)(map_t *, MapNode *);
void iterMap(map_t * map, mapIterFuncType mapIterFunc);
void iterMapSingle(map_t * map, mapIterFuncType mapIterFunc);




#endif  // MAP_H
