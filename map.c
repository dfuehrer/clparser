
#include "map.h"

#include <stdarg.h>
#include <string.h>
#include <error.h>
#include <stdio.h>

uint8_t getHash(const char * key, int len);
bool addMapKey(map_t * map, MapNode * node, StringView sv, int * ind_ptr);

// initialize map memory to all 0s
void initMap(map_t * map){
    //memset(map->ptrArray, 0, MAP_ARR_LEN * sizeof (MapNode *));
    //map->len = 0;
    memset(map, 0, sizeof *map);
}

MapNode * initNodeVals(MapNode * node, const void * data, DataType type, int len, bool setDefault){
    //// create node for this data, set data to data, next to null
    //MapNode * node = (MapNode *) calloc(1, sizeof (MapNode));
    node->data.ptr = data;
    node->data.type = type;
    node->data.required = false;        // default to optional cause thats pretty likely
    node->data.negations = NULL;        // default to no negations
    if(setDefault){
        // if setDefault then set this data as the default
        node->data.defaultData = (ArgData *) calloc(1, sizeof (ArgData));
        node->data.defaultData->ptr = data;
        node->data.defaultData->type = type;
    }else{
        node->data.defaultData = NULL; // default the default to NULL
    }

    node->next = NULL;
    node->namesLen = 0;
    node->names = (const char * *) calloc(len, sizeof (char *));
    node->nameLens = (int *) calloc(len, sizeof (int *));
    return node;
}

// TODO there is an issue where you could use the same name to add new values but it would jut add them over top and keep the old values which would be very confusin when popping and tuff
// add arbitrary number of keys to map to a data ptr
// TODO should this take an ArgData for the data arg
MapNode * addMapMembers(map_t * map, void * data, DataType type, bool setDefault, const char fmt[], ...){
    va_list args;
    va_start(args, fmt);
    // key and len args for str (stringview same)
    StringView sv = {.str = NULL, .len = 0};
    bool addedMember = false;
    int ind = 0;

    // create node for this data, set data to data, next to null
    MapNode * node = (MapNode *) calloc(1, sizeof (MapNode));
    initNodeVals(node, data, type, strlen(fmt), setDefault);

    // loop over items in format to figure out what the keys are
    // format d/i for int len, s for char * str, S for string view
    // % optional (ignored)
    for(const char * fp = fmt; *fp; ++fp){
        switch(*fp){
            case '%':
                continue;
            case 'S':
                sv = va_arg(args, StringView);
                break;
            case 'i':
            case 'd':
                sv.len = va_arg(args, int);
                break;
            case 's':
                sv.str = va_arg(args, char *);
                continue;
            default:
                error(5, 0, "map member fmt str '%s' invalid: '%c' unrecognized should be s, i, d, or S", fmt, *fp);
                return NULL;
        }

        // TODO allow strings without len
        addedMember |= addMapKey(map, node, sv, &ind);
        sv = (StringView){.str = NULL, .len = 0};
    }
    if(!addedMember){
        error(6, 0, "map key needs at least 1 str,len pair to add a member");
        return NULL;
    }

    va_end(args);
    return node;
}

// add arbitrary number of keys to map to a data ptr
MapNode * addMapMembers_fromList(map_t * map, void * data, DataType type, sllist_t * head, int numKeys, bool setDefault){
    // key and len args for str (stringview same)
    bool addedMember = false;
    int ind = 0;

    // create node for this data, set data to data, next to null
    MapNode * node = (MapNode *) calloc(1, sizeof (MapNode));
    initNodeVals(node, data, type, numKeys, setDefault);

    // loop over the llist, adding each key to this node
    for(sllist_t * keyNode = head; ind < numKeys && keyNode != NULL; keyNode = keyNode->next){
        addedMember |= addMapKey(map, node, keyNode->sv, &ind);
    }
    if(!addedMember){
        error(6, 0, "map key needs at least 1 str,len pair to add a member");
        return NULL;
    }
    return node;
}

bool addMapKey(map_t * map, MapNode * node, StringView sv, int * ind_ptr){
    // TODO prolly make len optional (or write wrapper func to only pass in strs and use strlen
    if(sv.len <= 0){
        error(6, 0, "map key length cannot be 0 (length should be specified after key)");
        return false;
    }else if(!sv.str){
        error(6, 0, "map key cannot be NULL");
        return false;
    }
    // set this key in the node
    node->names[*ind_ptr] = sv.str;
    node->nameLens[*ind_ptr] = sv.len;
    ++node->namesLen;
    //printf("adding map key '%.*s'\n", sv.len, sv.str);
    // hash the str to get ind into array
    uint8_t hash = getHash(sv.str, sv.len);
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

    ++(*ind_ptr);
    ++map->len;
    return true;
}

// NOTE probably just use getMapNode
bool hasNode(const map_t * map, const char * key, int len){
    return getMapNode(map, key, len) == NULL;
}

MapNode * getMapNode(const map_t * map, const char * key, int len){
    if(map == NULL){
        return NULL;
    }
    uint8_t hash = getHash(key, len);
    //printf("key: '%.*s', hash: %d\n", len, key, hash);
    for(MapNode * node = map->ptrArray[hash]; node != NULL; node = node->next){
        //printf("first key %.*s\n", node->nameLens[0], node->names[0]);
        for(int i = 0; i < node->namesLen; ++i){
            if(node->nameLens[i] == len && !strncmp(key, node->names[i], MIN(len, node->nameLens[i]))){
                return node;
            }
        }
    }
    //fprintf(stderr, "didnt find node for key '%.*s'\n", len, key);
    return NULL;
}

const void * setMapMemberData(map_t * map, const void * data, const char * key, int len){
    MapNode * node = getMapNode(map, key, len);
    if(node == NULL){
        error(7, 0, "key '%.*s' does not exist in map", len, key);
        return NULL;
    }
    node->data.ptr = data;

    return node->data.ptr;
}

const void * getMapMemberData(const map_t * map, const char * key, int len){
    MapNode * node = getMapNode(map, key, len);
    if(node != NULL){
        return node->data.ptr;
    }
    error(7, 0, "key '%.*s' does not exist in map", len, key);
    return NULL;
}

int  getMapMember_int (map_t * map, const char * key, int len){
    const void * datas = getMapMemberData(map, key, len);
    if(datas == NULL){
        //fprintf(stderr, "Did not find datas for %.*s\n", len, key);
        return -0;
    }
    // TODO add int type specificness with the DataType enum (will need to not use getMapMemberData)
    //  - probably just give a warning if its not an int
    int data = *((const int *)datas);
    return data;
}
// NOTE cant just use the int version because this doesnt store the size of the data so ints are too large
bool getMapMember_bool(map_t * map, const char * key, int len){
    const void * datas = getMapMemberData(map, key, len);
    if(datas == NULL){
        //fprintf(stderr, "Did not find datas for %.*s\n", len, key);
        return -0;
    }
    // TODO add int type specificness with the DataType enum (will need to not use getMapMemberData)
    bool data = *((const bool *)datas);
    return data;
}

int printArgData(const ArgData * data, FILE * file){
    if(data->ptr == NULL){
        return 0;
    }
    switch(data->type){
        case STR:
            return fprintf(file, "%s", (char *) data->ptr);
        case STRING_VIEW:
            return fprintf(file, "%.*s", ((StringView *) data->ptr)->len, ((StringView *) data->ptr)->str);
        case BOOL:
            return fprintf(file, "%s", (*(bool *) data->ptr) ? "true\0" : "false");
        case INT:
            return fprintf(file, "%d", *(int *) data->ptr);
        case CHAR:
            return fprintf(file, "%c", *(char *) data->ptr);
    }
    return 0;
}

void printMap(map_t * map){
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        printf("map ind %d\n", i);
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = node->next){
            printf("node %p\n", node);
            printf("data %p ", node->data.ptr);
            printArgData(&node->data, stdout);
            printf("\n");
            for(int j = 0; j < node->namesLen; ++j){
                printf("node %d key %d %.*s\n", i, j, node->nameLens[j], node->names[j]);
            }
        }
    }
}

void iterMap(map_t * map, MapIterFunc_t mapIterFunc){
    if(map == NULL || map->len == 0){
        return;
    }
    // TODO maybe have some way of indicating the first time or something (maybe just pass in i as well)
    int count = 0;
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = node->next){
            ++count;
            mapIterFunc(map, node);
        }
        if(count == map->len){
            break;
        }
    }
}
void iterMapSingle(map_t * map, MapIterFunc_t mapIterFunc){
    if(map == NULL || map->len == 0){
        return;
    }
    // allocating space for whole map in case its needed
    const MapNode * * node_ptrs = calloc(map->len - 1  , sizeof (const MapNode *));
    memset(node_ptrs, (long) NULL,      (map->len - 1) * sizeof (const MapNode *));
    int ptrsLen = 0;
    int count;
    // TODO maybe have some way of indicating the first time or something (maybe just pass in i as well)
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = node->next){
            ++count;
            bool skip = false;
            for(int j = 0; j < ptrsLen; ++j){
                if(node_ptrs[j] == node){
                    skip = true;
                    break;
                }
            }
            if(skip){
                continue;
            }
            node_ptrs[ptrsLen++] = node;
            mapIterFunc(map, node);
        }
        if(count == map->len){
            break;
        }
    }
    free(node_ptrs);
}

// internal version of pop node that can check my exact names keys array thing
MapNode * popMapNode(map_t * map, const MapNode * refNode){
    int i;
    MapNode * node = NULL;
    for(i = 0; i < refNode->namesLen; ++i){
        uint8_t hash = getHash(refNode->names[i], refNode->nameLens[i]);
        //printf("key: '%.*s', hash: %d\n", len, key, hash);
        node = NULL;
        MapNode * * prev_ptr = &map->ptrArray[hash];
        // TODO wouldnt need to loop like this if we just use doubly linked lists
        for(node = map->ptrArray[hash]; node != NULL; prev_ptr = &(*prev_ptr)->next, node = node->next){
            if(node == refNode){
                (*prev_ptr) = node->next;
                break;
            }
        }
    }
    return node;
}

void freeMap(map_t * map){
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = map->ptrArray[i]){
            // pop off all the nodes so we dont try to free them
            popMapNode(map, node);
            // free the node datas
            free(node->names);
            free(node->nameLens);
            free(node->data.defaultData);
            for(mnllist_t * mnlNode = node->data.negations; mnlNode != NULL; ){
                mnllist_t * n = mnlNode->next;
                free(mnlNode);
                mnlNode = n;
            }
            free(node);
        }
    }
}


uint8_t getHash(const char * key, int len){
    uint8_t hash = *key;
    for(int i = 1; i < len; ++i){
        hash ^= key[i] << (i & 0x01);
    }
    return hash;
}
