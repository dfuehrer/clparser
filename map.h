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

struct MapNode_s;


// struct that holds other info helpful for this purpose like default value, type, other attrs like required, negation of other nodes, etc
typedef struct ArgData_s {
    const void * ptr;
    DataType type;
    bool required;
    struct MapNode_s * negation;
    struct ArgData_s * defaultData;
} ArgData;

typedef struct MapNode_s{
    const char * * names;
    int * nameLens;
    int namesLen;
    // TODO maybe go back to this being a void pointer and have this stuff be on the process.h specificity
    ArgData data;
    struct MapNode_s * next;
} MapNode;

typedef struct map_s{
    MapNode * ptrArray[MAP_ARR_LEN];
    int len;
} map_t;

static const bool flagFalse = false;
static const bool flagTrue  = true;

void initMap(map_t * map);
// fmt is a "format string" that specifies what types the variadic keys are
// %S is a StringView, %s is a char[] for the key, %d or %i is an int for the string length
// length must follow a string, otherwise it will error
MapNode * addMapMembers(map_t * map, void * data, DataType type, const char fmt[], ...);
MapNode * addMapMembers_fromList(map_t * map, void * data, DataType type, llist_t * head, int numKeys);

// TODO add check for if key in map
const void * setMapMemberData(map_t * map, const void * data, const char * key, int len);

bool hasNode(const map_t * map, const char * key, int len);
MapNode * getMapNode(const map_t * map, const char * key, int len);
const void * getMapMemberData(const map_t * map, const char * key, int len);
int  getMapMember_int (map_t * map, const char * key, int len);
bool getMapMember_bool(map_t * map, const char * key, int len);
char getMapMember_char(map_t * map, const char * key, int len);

//void delMembers(map_t map, ...);
void freeMap(map_t * map);

MapNode * popMapNode(map_t * map, const MapNode * refNode);

void printMap(map_t * map);
int printArgData(const ArgData * data, FILE * file);

typedef int (*MapIterFunc_t)(map_t *, MapNode *);
void iterMap      (map_t * map, MapIterFunc_t mapIterFunc);
void iterMapSingle(map_t * map, MapIterFunc_t mapIterFunc);




#endif  // MAP_H
