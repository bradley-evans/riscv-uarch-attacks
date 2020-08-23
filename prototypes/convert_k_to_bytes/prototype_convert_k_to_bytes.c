/**
 * @defgroup   PROTOTYPE_ADDRCONVERT prototype addrconvert
 *
 * @brief      This file implements prototype addrconvert.
 *
 * This solution based in part on the StackExchange answer "Is there any API for
 * determining the physical address from virtual address in Linux?"
 * https://stackoverflow.com/questions/5748492/is-there-any-api-for-determining-the-physical-address-from-virtual-address-in-li
 * 
 *
 * @author     Brad
 * @date       2020
 */

#include "prototype_convert_k_to_bytes.h"


int main(int argc, char* argv[]) {
    
    char *buff = malloc(100);

    if (argc < 2) {
        printf("usage ./prototype <numtoconvert>, eg ./prototype 32K.\n");
        exit(0);
    }

    buff = argv[1];
    int num = atoi(buff);
    if (strchr(buff,'K') != NULL) {
        num = num * 1024;
    } 

    printf("recieved: %s, result: %d\n", argv[1], num);

    return 0;

}