
#include "map.h"

#include <stdarg.h>
#include <string.h>
#include <error.h>
#include <stdio.h>

uint8_t getHash(const char * key, int len);
bool addMapKey(map_t * map, MapData * node, StringView sv, int * ind_ptr);

// initialize map memory to all 0s
void initMap(map_t * map){
    //memset(map->ptrArray, 0, MAP_ARR_LEN * sizeof (MapNode *));
    //map->len = 0;
    memset(map, 0, sizeof *map);
}

MapData * initNodeVals(MapData * node, const void * data, DataType type, int len, bool setDefault){
    node->data.ptr = data;
    node->data.type = type;
    node->data.required = !setDefault && type != BOOL;  // default to optional if set default or BOOLean type
    node->data.negations = NULL;        // default to no negations
    if(setDefault){
        // if setDefault then set this data as the default
        node->data.defaultData = (ArgData *) calloc(1, sizeof (ArgData));
        node->data.defaultData->ptr = data;
        node->data.defaultData->type = type;
    }else{
        node->data.defaultData = NULL; // default the default to NULL
    }

    node->namesLen = 0;
    node->names = (const char * *) calloc(len, sizeof (char *));
    node->nameLens = (int *) calloc(len, sizeof (int *));
    return node;
}

// TODO there is an issue where you could use the same name to add new values but it would jut add them over top and keep the old values which would be very confusin when popping and tuff
// add arbitrary number of keys to map to a data ptr
// TODO should this take an ArgData for the data arg
MapData * addMapMembers(map_t * map, const void * data, DataType type, bool setDefault, const char fmt[], ...){
    va_list args;
    va_start(args, fmt);
    // key and len args for str (stringview same)
    StringView sv = {.str = NULL, .len = 0};
    bool addedMember = false;
    int ind = 0;

    // create node for this data, set data to data, next to null
    MapData * nodeData = (MapData *) calloc(1, sizeof (MapData));
    initNodeVals(nodeData, data, type, strlen(fmt), setDefault);

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
                break;
            default:
                error(5, 0, "map member fmt str '%s' invalid: '%c' unrecognized should be s, i, d, or S", fmt, *fp);
                return NULL;
        }
        // TODO allow strings without len
        addedMember |= addMapKey(map, nodeData, sv, &ind);
        sv = (StringView){.str = NULL, .len = 0};
    }
    if(!addedMember){
        error(6, 0, "map key needs at least 1 str,len pair to add a member");
        return NULL;
    }

    va_end(args);
    return nodeData;
}

// add arbitrary number of keys to map to a data ptr
MapData * addMapMembers_fromList(map_t * map, const void * data, DataType type, sllist_t * head, int numKeys, bool setDefault){
    // key and len args for str (stringview same)
    bool addedMember = false;
    int ind = 0;

    // create node for this data, set data to data, next to null
    MapData * nodeData = (MapData *) calloc(1, sizeof (MapData));
    initNodeVals(nodeData, data, type, numKeys, setDefault);

    // loop over the llist, adding each key to this node
    for(sllist_t * keyNode = head; ind < numKeys && keyNode != NULL; keyNode = keyNode->next){
        addedMember |= addMapKey(map, nodeData, keyNode->sv, &ind);
    }
    if(!addedMember){
        error(6, 0, "map key needs at least 1 str,len pair to add a member");
        return NULL;
    }
    return nodeData;
}

bool addMapKey(map_t * map, MapData * data, StringView sv, int * ind_ptr){
    // TODO prolly make len optional (or write wrapper func to only pass in strs and use strlen
    if(sv.len <= 0){
        error(6, 0, "map key length cannot be 0 (length should be specified after key)");
        return false;
    }else if(!sv.str){
        error(6, 0, "map key cannot be NULL");
        return false;
    }
    // set this key in the node
    data->names[*ind_ptr] = sv.str;
    data->nameLens[*ind_ptr] = sv.len;
    ++data->namesLen;
    //printf("adding map key '%.*s'\n", sv.len, sv.str);
    // hash the str to get ind into array
    uint8_t hash = getHash(sv.str, sv.len);
    //printf("key: '%.*s', hash: %d\n", len, key, hash);

    MapNode * node = (MapNode *) calloc(1, sizeof (MapNode));
    node->data = data;
    node->next = NULL;

    // TODO consider modding (%) hash at the end to get it within the range of the size of the mapArray
    //  how things are set up now, hash cannot be longer so this step would either be ignored by the compiler or more likely waste computation
    //  maybe just bitmask to size since i used a power of 2?
    //  or just cazt to uint8_t in my case?

    // add node to map
    // if a node is already at this ind, set this node as the new head of th linked list
    if(map->ptrArray[hash] != NULL){
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

MapData * getMapNode(const map_t * map, const char * key, int len){
    if(map == NULL){
        return NULL;
    }
    uint8_t hash = getHash(key, len);
    //printf("key: '%.*s', hash: %d\n", len, key, hash);
    for(MapNode * node = map->ptrArray[hash]; node != NULL; node = node->next){
        //printf("first key %.*s\n", node->nameLens[0], node->names[0]);
        MapData * data = node->data;
        for(int i = 0; i < data->namesLen; ++i){
            if(data->nameLens[i] == len && !strncmp(key, data->names[i], MIN(len, data->nameLens[i]))){
                return data;
            }
        }
    }
    //fprintf(stderr, "didnt find node for key '%.*s'\n", len, key);
    return NULL;
}

const void * setMapMemberData(map_t * map, const void * data, const char * key, int len){
    MapData * nodeData = getMapNode(map, key, len);
    if(nodeData == NULL){
        error(7, 0, "key '%.*s' does not exist in map", len, key);
        return NULL;
    }
    nodeData->data.ptr = data;

    return nodeData->data.ptr;
}

const void * getMapMemberData(const map_t * map, const char * key, int len){
    MapData * data = getMapNode(map, key, len);
    if(data != NULL){
        return data->data.ptr;
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
        return false;
    }
    // TODO add int type specificness with the DataType enum (will need to not use getMapMemberData)
    bool data = *((const bool *)datas);
    return data;
}

void setNodeNegation(MapData * data, MapData * negative){
    mnllist_t * mnlNode = (mnllist_t *) malloc(sizeof (mnllist_t));
    mnlNode->data = data;
    mnlNode->next = negative->data.negations;
    negative->data.negations = mnlNode;
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
            MapData * data = node->data;
            printf("node %p\n", (void *)node);
            printf("data %p ", (void *)data);
            printf("data ptr %p ", data->data.ptr);
            printArgData(&data->data, stdout);
            printf("\n");
            for(int j = 0; j < data->namesLen; ++j){
                printf("node %d key %d %.*s\n", i, j, data->nameLens[j], data->names[j]);
            }
        }
    }
}

int iterMap(map_t * map, MapIterFunc_t mapIterFunc, void * funcInput){
    if(map == NULL || map->len == 0){
        return 0;
    }
    // TODO maybe have some way of indicating the first time or something (maybe just pass in i as well)
    int total = 0;
    int count = 0;
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = node->next){
            ++count;
            total += mapIterFunc(map, node->data, funcInput);
        }
        if(count == map->len){
            break;
        }
    }
    return total;
}
int iterMapSingle(map_t * map, MapIterFunc_t mapIterFunc, void * funcInput){
    if(map == NULL || map->len == 0){
        return 0;
    }
    // allocating space for whole map in case its needed
    const MapData * * data_ptrs = calloc(map->len  , sizeof (const MapData *));
    memset(data_ptrs, (long) NULL,      (map->len) * sizeof (const MapData *));
    int ptrsLen = 0;
    int total = 0;
    int count = 0;
    // TODO maybe have some way of indicating the first time or something (maybe just pass in i as well)
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = node->next){
            ++count;
            bool skip = false;
            for(int j = 0; j < ptrsLen; ++j){
                if(data_ptrs[j] == node->data){
                    skip = true;
                    break;
                }
            }
            if(skip){
                continue;
            }
            data_ptrs[ptrsLen++] = node->data;
            total += mapIterFunc(map, node->data, funcInput);
        }
        if(count == map->len){
            break;
        }
    }
    free(data_ptrs);
    return total;
}

// pop (all keys' nodes) of data in map
void popMapNode(map_t * map, const MapData * data){
    int i;
    MapNode * node = NULL;
    for(i = 0; i < data->namesLen; ++i){
        uint8_t hash = getHash(data->names[i], data->nameLens[i]);
        //printf("key: '%.*s', hash: %d\n", len, key, hash);
        node = NULL;
        MapNode * * prev_ptr = &map->ptrArray[hash];
        // TODO wouldnt need to loop like this if we just use doubly linked lists
        for(node = map->ptrArray[hash]; node != NULL; ){
            if(node->data == data){
                (*prev_ptr) = node->next;
                break;
            }
            // TODO im not sure about actually freeing in here but it doesnt make any sense to not remove all nodes
            // really you want this node freed cause its kinda useless, the data is in the data struct
            MapNode * n = node;
            prev_ptr = &(*prev_ptr)->next;
            node = node->next;
            free(n);
        }
    }
}

void freeMap(map_t * map){
    for(int i = 0; i < MAP_ARR_LEN; ++i){
        for(MapNode * node = map->ptrArray[i]; node != NULL; node = map->ptrArray[i]){
            MapData * data = node->data;
            // pop off all the nodes so we dont try to free them
            popMapNode(map, data);
            // free the node datas
            free(data->names);
            free(data->nameLens);
            free(data->data.defaultData);
            for(mnllist_t * mnlNode = data->data.negations; mnlNode != NULL; ){
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
