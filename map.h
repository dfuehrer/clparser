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
    struct MapNode_s * node;
    struct mnllist_s * next;
} mnllist_t;

// TODO define errors in an enum somewhere


// struct that holds other info helpful for this purpose like default value, type, other attrs like required, negation of other nodes, etc
typedef struct ArgData_s {
    const void * ptr;
    DataType type;
    bool required;
    // TODO make multiple negation nodes so it can negate multiple things
    mnllist_t * negations;
    struct ArgData_s * defaultData;
} ArgData;

typedef struct MapNode_s{
    const char * * names;
    int * nameLens;
    int namesLen;
    // TODO maybe go back to this being a void pointer and have this stuff be on the process.h specificity
    ArgData data;
    // TODO this currently has a bug where the node is duplicated across elements so they all share the same `next` value even though that doesn't make any sense (doesn't cause bugs, does increase collisions, decreasing performance)
    //  - just create a different structure that contains a pointer to one of these (without next) and a next node so the next is unique to the outer layer but the keys and data are shared
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
MapNode * addMapMembers(map_t * map, const void * data, DataType type, bool setDefault, const char fmt[], ...);
MapNode * addMapMembers_fromList(map_t * map, const void * data, DataType type, sllist_t * head, int numKeys, bool setDefault);

// TODO add check for if key in map
const void * setMapMemberData(map_t * map, const void * data, const char * key, int len);

bool hasNode(const map_t * map, const char * key, int len);
MapNode * getMapNode(const map_t * map, const char * key, int len);
const void * getMapMemberData(const map_t * map, const char * key, int len);
int  getMapMember_int (map_t * map, const char * key, int len);
bool getMapMember_bool(map_t * map, const char * key, int len);
char getMapMember_char(map_t * map, const char * key, int len);

void setNodeNegation(MapNode * node, MapNode * negative);

//void delMembers(map_t map, ...);
void freeMap(map_t * map);

MapNode * popMapNode(map_t * map, const MapNode * refNode);

void printMap(map_t * map);
int printArgData(const ArgData * data, FILE * file);

typedef int (*MapIterFunc_t)(map_t *, MapNode *, void *);
int iterMap      (map_t * map, MapIterFunc_t mapIterFunc, void * funcInput);
int iterMapSingle(map_t * map, MapIterFunc_t mapIterFunc, void * funcInput);




#endif  // MAP_H
