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

#include "prototype_addrconvert.h"


typedef struct {
    uint64_t pfn : 55;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;


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


int main() {
    uint64_t dummyMem = 0;
    uintptr_t va = (uintptr_t)&dummyMem;
    pid_t pid = getpid();

    dummyMem = 1;

    printf("dummyMem (virtual):  0x%x\n", va);
    printf("dummyMem (physical): 0x%x\n", virt_to_phys(va, pid));

    return EXIT_SUCCESS;
}