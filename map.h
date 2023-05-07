#ifndef MAP_H
#define MAP_H

#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#define MAP_ARR_LEN UINT8_MAX

enum DataType{
    INT,
    CHAR,
    STR,
    BOOL,
};


// TODO define errors in an enum somewhere



typedef struct MapNode_s{
    const char * name;
    int nameLen;
    // TODO figure out what this should actually hold and put that instead
    //  alternatively let it hold typed data in the void pointer and then store a type to know how to access it (use an enum or something for int, str, bool, etc)
    //const void * * const data_ptr;
    const void * * data_ptr;
    //DataType type;
    struct MapNode_s * next;
} MapNode;

typedef struct map_s{
    MapNode * ptrArray[MAP_ARR_LEN];
    int len;
} map_t;

typedef struct {
    char * str;
    int len;
} StringView;

void initMap(map_t * map);
// fmt is a "format string" that specifies what types the variadic keys are
// %S is a StringView, %s is a char[] for the key, %d or %i is an int for the string length
// length must follow a string, otherwise it will error
void addMapMembers            (map_t * map, const void * data_addr, const char fmt[], ...);
//void addMapMember             (map_t * map, const void * data_addr, StringView key);
void addMapMember             (map_t * map, const void * data_addr, const char * key, int len);

const void * setMapMemberData (map_t * map, const void * data_addr, const char * key, int len);

const void * getMapMemberData (map_t * map, const char * key, int len);
int          getMapMember_int (map_t * map, const char * key, int len);
bool         getMapMember_bool(map_t * map, const char * key, int len);
char         getMapMember_char(map_t * map, const char * key, int len);

//void delMembers(map_t map, ...);
void freeMap(map_t * map);









#endif  // MAP_H
