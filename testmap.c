
#include "map.h"


int main(){
    map_t map;
    initMap(&map);
    char str1[] = "test str";
    printf("going to add '%s' to map\n", str1);
    addMapMembers(&map, str1, "sdsdsd", "test1", 5, "other test", 5, "diff", 4);
    int i = 5;
    addMapMembers(&map, &i, "sdsd", "test2", 5, "another thing", 5, "i dunno", 4);

    printMap(&map);

    int j = getMapMember_int(&map, "another", 5);
    printf("j = %d (%.*s)\n", j, 5, "another");
    char * teststr = getMapMemberData(&map, "diff", 4);
    printf("teststr = '%s' (%.*s)\n", teststr, 4, "diff");

    printf("going to free map\n");
    freeMap(&map);

    puts("done");
    return 0;
}
