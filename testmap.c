
#include "map.h"


int main(){
    map_t map;
    initMap(&map);
    char str1[] = "test str";
    printf("going to add '%s' to map\n", str1);
    addMapMembers(&map, str1, STR, false, "SsdS", STRVIEW("test1"), "other test"   , 5, STRVIEW("diff"   ));
    int i = 5;
    addMapMembers(&map,   &i, INT, true , "SsdS", STRVIEW("test2"), "another thing", 5, STRVIEW("i dunno"));

    printMap(&map);

    int j = getMapMember_int(&map, "another", 5);
    printf("j = %d (%.*s)\n", j, 5, "another");
    const char * teststr = getMapMemberData(&map, "diff", 4);
    printf("teststr = '%s' (%.*s)\n", teststr, 4, "diff");

    printf("going to free map\n");
    freeMap(&map);

    puts("done");
    return 0;
}
