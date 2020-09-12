/**
 * @defgroup   ADDR address
 * 
 * Operations performed upon memory addresses.
 *
 * @file       addr.c
 * @ingroup    ADDR address
 * @brief      Implementation of functions that operate on memory addresses.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */

#include "addr.h"


/**
 * @brief      Calculates the number offset bits.
 * @ingroup    ADDR address
 *
 * @param[in]  cache  The cache
 *
 * @return     The number offset bits.
 */
int calculateNumOffsetBits(struct cache_t cache) {
    return log2(cache.linesize);
}


/**
 * @brief      Calculates the number index bits.
 * @ingroup    ADDR address
 *
 * @param[in]  cache  The cache
 *
 * @return     The number index bits.
 */
int calculateNumIndexBits(struct cache_t cache) {
    return log2(cache.sets);
}


/**
 * @brief      Gets the address size.
 * @ingroup    ADDR address
 *
 * @param      var   The variable
 *
 * @return     The address size.
 */
int get_AddressSize(int *var) {
    return sizeof(var)*8;
}


/**
 * @brief      Gets the offset.
 * @ingroup    ADDR address
 *
 * @param[in]  addr           The address
 * @param[in]  numOffsetBits  The number offset bits
 *
 * @return     The offset.
 */
unsigned int get_Offset(long long addr, int numOffsetBits) {
    long long mask = pow(2,numOffsetBits)-1;
    return mask & addr;
}


/**
 * @brief      Gets the index.
 * @ingroup    ADDR address
 *
 * @param[in]  addr           The address
 * @param[in]  numOffsetBits  The number offset bits
 * @param[in]  numIndexBits   The number index bits
 *
 * @return     The index.
 */
unsigned int get_Index(long long addr, int numOffsetBits, int numIndexBits) {
    long long mask = (int)(pow(2,numIndexBits)-1) << numOffsetBits;
    return (mask & addr) >> numOffsetBits;
}


/**
 * @brief      Generates a tag from an address.
 * @ingroup    ADDR address
 *
 * @param[in]  addr           The address
 * @param[in]  numOffsetBits  The number of offset bits
 * @param[in]  numIndexBits   The number of index bits
 *
 * @return     The tag portion of the address.
 */
unsigned int get_Tag(long long addr, int numOffsetBits, int numIndexBits) {
    long long mask = pow(2,numIndexBits+numOffsetBits)-1;
    mask = ~mask;
    return (mask & addr) >> (numOffsetBits + numIndexBits);
}


/**
 * @brief      Generates an address_t struct from cache information
 * and a variable passed by reference.
 * @ingroup    ADDR address
 *
 * @param[in]  cache  A cache_t struct.
 * @param      var    A variable, passed by reference, to generate an address_t from.
 *
 * @return     A struct describing the different fields of the address.
 */
struct address_t get_Address(struct cache_t cache, int *var) {
    struct address_t address;
    char *msg = malloc(100);

    sprintf(msg, "Recieved address for processing: 0x%llx", var);
    debug_msg(msg);


    /* this seems to consistently offset by 0x74 (116_10) bits. why? */
    address.addr = (uint64_t)var;
    address.bitsize = get_AddressSize(var);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);
    int numTagBits = address.bitsize - numOffsetBits - numIndexBits;

    address.offset = get_Offset(address.addr, numOffsetBits);
    address.set = get_Index(address.addr, numOffsetBits, numIndexBits);
    address.tag = get_Tag(address.addr, numOffsetBits, numIndexBits);

    return address;
}


/**
 * @brief      Gets a pagemap entry. Supports VA->PA conversion.
 * @ingroup    ADDR address
 * 
 * This function was taken from a StackExchange answer, "Is there any API for
 * determining the physical address from virtual address in Linux?".
 * https://stackoverflow.com/a/45128487/4107537
 * 
 * @param      entry       The entry
 * @param[in]  pagemap_fd  The pagemap fd
 * @param[in]  vaddr       The vaddr
 *
 * @return     { description_of_the_return_value }
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr) {
    size_t nread;
    ssize_t ret;
    uint64_t data;
    uintptr_t vpn;

    vpn = vaddr / sysconf(_SC_PAGE_SIZE);
    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(pagemap_fd, ((uint8_t*)&data) + nread, sizeof(data) - nread,
                vpn * sizeof(data) + nread);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->pfn = data & (((uint64_t)1 << 55) - 1);
    entry->soft_dirty = (data >> 55) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;
    return 0;
}


/**
 * @brief      Function to convert virtual addresses running within a process
 * to physical addresses.
 * @ingroup    ADDR address
 * 
 * This function was taken from a StackExchange answer, "Is there any API for
 * determining the physical address from virtual address in Linux?".
 * https://stackoverflow.com/a/45128487/4107537
 *
 * @param[in]  vaddr  The virtual address
 * @param[in]  pid    The pid associated with the virtual address
 *
 * @return     The physical address
 */
uint64_t virt_to_phys(uint64_t vaddr, pid_t pid) {
    char pagemap_file[BUFSIZ];
    int pagemap_fd;
    uintptr_t *paddr = malloc(sizeof(uintptr_t));

    snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
    pagemap_fd = open(pagemap_file, O_RDONLY);
    if (pagemap_fd < 0) {
        return 1;
    }
    PagemapEntry entry;
    if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
        return 1;
    }
    close(pagemap_fd);
    *paddr = (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));

    return (uint64_t)paddr;
}


/**
 * @brief      Creates a candidate address in the same set as a "victim" address.
 * In theory, this address could evict the victim address by occupying the same
 * cache line, but this doesn't work. It will instead segmentation fault, more
 * often than not.
 * @ingroup    ADDR address
 *
 * @param[in]  cache   Cache parameters.
 * @param[in]  victim  Victim address we would like to evict.
 *
 * @return     A candidate "evictor" address.
 */
int * generate_Evictor(struct cache_t cache, struct address_t victim) {
    char *msg = malloc(100);

    sprintf(msg, "Address of victim: \t\t0x%llx", victim.addr);
    debug_msg(msg);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);
    unsigned long long int add_to_tag = 0x1 << (numOffsetBits + numIndexBits + 1);
    unsigned long long int evictor_addr = victim.addr + add_to_tag;
    sprintf(msg, "Candidate evictor address: \t0x%llx", evictor_addr);
    debug_msg(msg);

    const void * evictor = (void *)evictor_addr;

    sprintf(msg, "Evictor address: \t\t0x%llx", evictor);
    debug_msg(msg);


    return (int *)evictor;
}