#include <log.h>
#include <link.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include "gadget.h"

typedef struct _find_data {
    const char *name;
    void *base;
} FindData;

static int dl_callback(struct dl_phdr_info *info, size_t size, void *data) {
    FindData *d = (FindData *) data;

    if (info->dlpi_name && strstr(info->dlpi_name, d->name) != NULL) {
        d->base = (void*)info->dlpi_addr;
        LOGD("Found base address of %s: %p", d->name, d->base);
        return 1;
    }
    return 0; // 0 to continue, non-zero to stop
}

void *find_library(const char *lib_name) {
    FindData data = {
            .name = lib_name,
            .base = NULL,
    };
    dl_iterate_phdr(dl_callback, &data);

    if (data->base != NULL) {
        // found using the linker
        return data->base;
    }

    // fallback: parse /proc/self/maps
    FILE *f = fopen("/proc/self/maps", "r");

    

    return NULL;
}

__attribute__((constructor))
void onLoad() {
    LOGD("Gotcha gadget loaded.");
}