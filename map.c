
#include "map.h"

#include <stdarg.h>
#include <string.h>
#include <error.h>
#include <stdio.h>

uint8_t getHash(const char * key, int len);

// initialize map memory to all 0s
void initMap(map_t * map){
    //memset(map->ptrArray, 0, MAP_ARR_LEN * sizeof (MapNode *));
    //map->len = 0;
    memset(map, 0, sizeof *map);
}

// add arbitrary number of keys to map to a data ptr
void addMapMembers(map_t * map, void * data, const char fmt[], ...){
    va_list args;
    va_start(args, fmt);
    // key and len args for str (stringview same)
    const char * key = NULL;
    int len = 0;
    StringView sv = {.str = NULL, .len = 0};
    bool addedMember = false;

    // create node for this data, set data to data, next to null
    MapNode * node = (MapNode *) calloc(1, sizeof (MapNode));
    node->data = data;
    node->next = NULL;
    node->namesLen = 0;
    int fmtlen = strlen(fmt);
    node->names = (const char * *) calloc(fmtlen, sizeof (char *));
    node->nameLens = (int *) calloc(fmtlen, sizeof (int *));

    // loop over items in format to figure out what the keys are
    // format d/i for int len, s for char * str, S for string view
    // % optional (ignored)
    int ind = 0;
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
                key = va_arg(args, const char *);
                continue;
            default:
                return error(5, 0, "map member fmt str '%s' invalid: '%c' unrecognized should be s, i, d, or S", fmt, *fp);
        }

        // TODO prolly make len optional (or write wrapper func to only pass in strs and use strlen
        if(len <= 0){
            return error(6, 0, "map key length cannot be 0 (length should be specified after key)");
        }else if(!key){
            return error(6, 0, "map key cannot be NULL");
        }
        // set this key in the node
        node->names[ind] = key;
        node->nameLens[ind] = len;
        ++node->namesLen;
        // hash the str to get ind into array
        uint8_t hash = getHash(key, len);
        //printf("key: '%.*s', hash: %d\n", len, key, hash);

        // TODO consider modding (%) hash at the end to get it within the range of the size of the mapArray
        //  how things are set up now, hash cannot be longer so this step would either be ignored by the compiler or more likely waste computation
        //  maybe just bitmask to size since i used a power of 2?
        //  or just cazt to uint8_t in my case?

        // add node to map
        // if a node is already at this ind, set this node as the new head of th linked list
        if(map->ptrArray[hash]){
            node->next = map->ptrArray[hash];
        }
        map->ptrArray[hash] = node;

        ++ind;
        ++map->len;
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

MapNode * getMapNode(const map_t * map, const char * key, int len){
    uint8_t hash = getHash(key, len);
    //printf("key: '%.*s', hash: %d\n", len, key, hash);
    for(MapNode * node = map->ptrArray[hash]; node != NULL; node = node->next){
        for(int i = 0; i < node->namesLen; ++i){
            if(node->nameLens[i] == len && !strncmp(key, node->names[i], MIN(len, node->nameLens[i]))){
                return node;
            }
        }
    }
    return NULL;
}

const void * setMapMemberData(map_t * map, void * data_addr, const char * key, int len){
    MapNode * node = getMapNode(map, key, len);
    if(node == NULL){
        error(7, 0, "key '%.*s' does not exist in map", len, key);
        return NULL;
    }
    node->data = data_addr;

    return data_addr;
}

void * getMapMemberData(const map_t * map, const char * key, int len){
    MapNode * node = getMapNode(map, key, len);
    if(node != NULL){
        return node->data;
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

void printMap(map_t * map){
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        printf("map ind %d\n", i);
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = node->next){
            printf("node %p\n", node);
            printf("data %p\n", node->data);
            // pop off all the nodes so we dont try to free them
            for(int j = 0; j < node->namesLen; ++j){
                printf("node %d key %d %.*s\n", i, j, node->nameLens[j], node->names[j]);
            }
        }
    }
}


void freeMap(map_t * map){
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = map->ptrArray[i]){
            // pop off all the nodes so we dont try to free them
            for(int j = 0; j < node->namesLen; ++j){
                //printf("gonna pop node %d %.*s\n", i, node->nameLens[j], node->names[j]);
                popMapNode(map, node->names[j], node->nameLens[j]);
            }
            // free the node datas
            free(node->names);
            free(node->nameLens);
            free(node);
        }
    }
    printf("just freed the map nodes\n");
}

MapNode * popMapNode(map_t * map, const char * key, int len){
    uint8_t hash = getHash(key, len);
    //printf("key: '%.*s', hash: %d\n", len, key, hash);
    MapNode * node = NULL;
    MapNode * * prev_ptr = &map->ptrArray[hash];
    for(node = map->ptrArray[hash]; node != NULL; prev_ptr = &(*prev_ptr)->next, node = node->next){
        for(int i = 0; i < node->namesLen; ++i){
            if(node->nameLens[i] == len && !strncmp(key, node->names[i], MIN(len, node->nameLens[i]))){
                (*prev_ptr) = node->next;
                return node;
            }
        }
    }
    return NULL;
}


uint8_t getHash(const char * key, int len){
    uint8_t hash = *key;
    for(int i = 1; i < len; ++i){
        hash ^= key[i] << (i & 0x01);
    }
    return hash;
}
