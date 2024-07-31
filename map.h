#ifndef MAP_H
#define MAP_H

#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAP_ARR_LEN UINT8_MAX

#define ARRAY_LENGTH(arr)   (sizeof arr / sizeof arr[0])
#define STRVIEW(string)     ((StringView) {.str=string, .len=(ARRAY_LENGTH(string) - 1)})
#define STR_LEN(string)     string, (ARRAY_LENGTH(string) - 1)
#define LEN_STR(string)     (ARRAY_LENGTH(string) - 1), string
#ifndef MIN
#   define MIN(x, y)   ((y < x) ? y : x)
#endif  // MIN

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

struct MapNode_s;
struct ArgData_s;

//typedef struct llist_s {
//    void * data;
//    struct llist_s * next;
//} llist_t;

typedef struct sllist_s {
    StringView sv;
    struct sllist_s * next;
} sllist_t;

typedef struct mnllist_s {
    struct MapData_s * data;
    struct mnllist_s * next;
} mnllist_t;

// TODO define errors in an enum somewhere


// struct that holds other info helpful for this purpose like default value, type, other attrs like required, negation of other nodes, etc
typedef struct ArgData_s {
    const void * ptr;
    DataType type;
    bool required;
    mnllist_t * negations;
    struct ArgData_s * defaultData;
} ArgData;

typedef struct MapData_s{
    const char * * names;
    int * nameLens;
    int namesLen;
    ArgData data;
} MapData;
typedef struct MapNode_s{
    MapData * data;
    struct MapNode_s * next;
} MapNode;

typedef struct map_s{
    MapNode * ptrArray[MAP_ARR_LEN];
    int numKeys;
    int numDatas;
} map_t;

static const bool flagFalse = false;
static const bool flagTrue  = true;

void initMap(map_t * map);
// fmt is a "format string" that specifies what types the variadic keys are
// %S is a StringView, %s is a char[] for the key, %d or %i is an int for the string length
// length must follow a string, otherwise it will error
MapData * addMapMembers(map_t * map, const void * data, DataType type, bool setDefault, const char fmt[], ...);
MapData * addMapMembers_fromList(map_t * map, const void * data, DataType type, sllist_t * head, int numKeys, bool setDefault);

// TODO maybe change from key, len to StringView
const void * setMapMemberData(map_t * map, const void * data, const char * key, int len);

bool hasNode(const map_t * map, const char * key, int len);
MapData * getMapNode(const map_t * map, const char * key, int len);
const void * getMapMemberData(const map_t * map, const char * key, int len);
int  getMapMember_int (map_t * map, const char * key, int len);
bool getMapMember_bool(map_t * map, const char * key, int len);
char getMapMember_char(map_t * map, const char * key, int len);
const void * getNodeData(MapData * node);
int  getNode_int (MapData * node);
bool getNode_bool(MapData * node);
char getNode_char(MapData * node);

void setNodeNegation(MapData * data, MapData * negative);

//void delMembers(map_t map, ...);
void freeMap(map_t * map);

void popMapNode(map_t * map, const MapData * data);
bool keyInNode(const MapData * data, const char * key, int len);

void printMap(map_t * map);
int printArgData(const ArgData * data, FILE * file);

typedef int (*MapIterFunc_t)(map_t *, MapData *, void *);
int iterMap      (map_t * map, MapIterFunc_t mapIterFunc, void * funcInput);
int iterMapSingle(map_t * map, MapIterFunc_t mapIterFunc, void * funcInput);




#endif  // MAP_H
