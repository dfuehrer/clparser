
#include "map.h"


int main(){
    map_t map;
    initMap(&map);
    char str1[] = "test str";
    printf("going to add '%s' to map\n", str1);
    addMapMembers(&map, str1, "sdsdsd", "test1", 5, "other test", 5, "diff", 4);

    printMap(&map);

    printf("going to free map\n");
    freeMap(&map);

    puts("done");
    return 0;
}
