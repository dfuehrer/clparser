
#include "map.h"

#include <stdarg.h>
#include <string.h>
#include <error.h>
#include <stdio.h>

uint8_t getHash(const char * key, int len);

void initMap(map_t * map){
    //memset(map->ptrArray, 0, MAP_ARR_LEN * sizeof (MapNode *));
    //map->len = 0;
    memset(map, 0, sizeof *map);
}

void addMapMembers(map_t * map, const void * data, const char fmt[], ...){
    va_list args;
    va_start(args, fmt);
    const void * * const data_ptr = (const void **) calloc(1, sizeof (void *));
    *data_ptr = data;
    char * key = NULL;
    int len = 0;
    StringView sv = {.str = NULL, .len = 0};
    bool addedMember = false;
    for(const char * fp = fmt; *fp; ++fp){
        switch(*fp){
            case '%':
                continue;
            case 'S':
                sv = va_arg(args, StringView);
                key = sv.str;
                len = sv.len;
                break;
            case 'i':
            case 'd':
                len = va_arg(args, int);
                break;
            case 's':
                key = va_arg(args, char *);
                continue;
            default:
                return error(5, 0, "map member fmt str '%s' invalid: '%c' unrecognized", fmt, *fp);
        }

        if(!len){
            return error(6, 0, "map key length cannot be 0 (length should be specified after key)");
        }else if(!key){
            return error(6, 0, "map key cannot be NULL");
        }
        uint8_t hash = getHash(key, len);
        //printf("key: '%.*s', hash: %d\n", len, key, hash);

        // TODO consider modding (%) hash at the end to get it within the range of the size of the mapArray
        //  how things are set up now, hash cannot be longer so this step would either be ignored by the compiler or more likely waste computation

        MapNode * node = (MapNode *) calloc(1, sizeof (MapNode));
        *node = (MapNode){.name=key, .nameLen=len, .data_ptr=data_ptr, .next=NULL};
        // is this a valid way to get around the const init issue?
        //MapNode n = {.name=key, .nameLen=len, .data_ptr=&data_ptr, .next=NULL};
        //memcpy(node, &n, sizeof n);

        if(map->ptrArray[hash]){
            node->next = map->ptrArray[hash];
        }
        map->ptrArray[hash] = node;

        map->len++;
        addedMember = true;
        len = 0;
        key = NULL;
        sv = (StringView){.str = NULL, .len = 0};
    }
    if(!addedMember){
        return error(6, 0, "map key needs at least 1 str,len pair to add a member");
    }

    va_end(args);
}

MapNode * getMapNode(map_t * map, const char * key, int len){
    uint8_t hash = getHash(key, len);
    //printf("key: '%.*s', hash: %d\n", len, key, hash);
    for(MapNode * node = map->ptrArray[hash]; node != NULL; node = node->next){
        if(node->nameLen == len && !strncmp(key, node->name, len)){
            return node;
        }
    }
    return NULL;
}

const void * setMapMemberData(map_t * map, const void * data_addr, const char * key, int len){
    MapNode * node = getMapNode(map, key, len);
    if(node == NULL){
        error(7, 0, "key '%.*s' does not exist in map", len, key);
        return NULL;
    }
    *(node->data_ptr) = data_addr;

    return data_addr;
}

const void * getMapMemberData(map_t * map, const char * key, int len){
    MapNode * node = getMapNode(map, key, len);
    if(node != NULL){
        return *(node->data_ptr);
    }
    error(7, 0, "key '%.*s' does not exist in map", len, key);
    return NULL;
}

int  getMapMember_int (map_t * map, const char * key, int len){
    const void * datas = getMapMemberData(map, key, len);
    // TODO add int type specificness with the DataType enum (will need to not use getMapMemberData)
    int data = *((const int *)datas);
    return data;
}

void freeMap(map_t * map){
    const void * const * * const ptrArray = (const void * const **)calloc(map->len + 1, sizeof (void **));
    //const void * const * * const ptrArray = (const void * const **)alloca((map->len + 1) * sizeof (void **));
    memset((void *) ptrArray, (long)NULL, (map->len + 1) * sizeof (void *));
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; ){
            const void * * data_ptr = (node->data_ptr);
            const void * const * * ptr;
            for(ptr = ptrArray; *ptr != NULL && *ptr != data_ptr; ++ptr);
            if(*ptr == NULL){
                *ptr = data_ptr;
            }
            MapNode * n = node;
            node = n->next;
            free(n);
        }
    }
    printf("just freed the map nodes\n");
    // TODO add a feature to go have a function to handle the data before freeing these pointers
    for(const void * const * * ptr = ptrArray; *ptr != NULL; ++ptr){
        free((void *)*ptr);
    }
    free(ptrArray);
}

void f(){
    map_t m;
    char * data = "asdf";
    addMapMembers(&m, (const void * const *)&data, "%S", (StringView){"hi", 2});
    freeMap(&m);
}

uint8_t getHash(const char * key, int len){
    uint8_t hash = *key;
    for(int i = 1; i < len; ++i){
        hash ^= key[i] << (i & 0x01);
    }
    return hash;
}
