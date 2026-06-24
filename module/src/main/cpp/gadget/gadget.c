#include <log.h>
#include <link.h>
#include <elf.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "gadget.h"

typedef struct _find_base_data {
    const char *name;
    void *base;
} FindBaseData;

typedef struct _find_path_data {
    const char *name;
    char *path;
} FindPathData;


static int dl_callback(struct dl_phdr_info *info, size_t size, void *data) {
    FindBaseData *d = (FindBaseData *) data;

    if (info->dlpi_name && strstr(info->dlpi_name, d->name) != NULL) {
        d->base = (void*)info->dlpi_addr;
        LOGD("Found base address of %s: %p", d->name, d->base);
        return 1;
    }
    return 0;
}

static void *find_module_base(const char *lib_name) {
    FindBaseData data = {
            .name = lib_name,
            .base = NULL,
    };
    dl_iterate_phdr(dl_callback, &data);

    if (data.base != NULL) {
        // found using the linker
        return data.base;
    }

    // fallback: parse /proc/self/maps
    FILE *f = fopen("/proc/self/maps", "r");
    if (f == NULL) {
        LOGE("Failed to open process mappings");
        return NULL;
    }

    char line[1024];
    void *base = NULL;
    while (fgets(line, sizeof(line), f) != NULL) {
        char *name = strstr(line, lib_name);
        if (name != NULL) {
            // search the first mapping of library that represents the base
            if (strstr(line, "r-xp") || (strstr(line, "r--p") && strstr(line, " 00000000 "))) {
                unsigned long addr;
                if (sscanf(line, "%lx", &addr) == 1) {
                    base = (void *)addr;
                    LOGD("Found base address of %s: %p (from /proc/self/maps)", lib_name, (void*)addr);
                    break;
                }
            }
        }
    }
    fclose(f);

    if (base == NULL) {
        LOGE("Failed to find base address of %s", lib_name);
        return NULL;
    }

    return base;
}

static int dl_path_callback(struct dl_phdr_info *info, size_t size, void *data) {
    FindPathData *d = (FindPathData *) data;

    if (info->dlpi_name && strlen(info->dlpi_name) > 0 && strstr(info->dlpi_name, d->name)) {
        d->path = strdup(info->dlpi_name);
        LOGD("Found library path: %s via dl_iterate_phdr", d->path);
        return 1;
    }

    return 0;
}

static void *find_module_path(const char *lib_name) {
    FindPathData data = {
            .name = lib_name,
            .path = NULL,
    };
    dl_iterate_phdr(dl_path_callback, &data);

    if (data.base != NULL) {
        // found using the linker
        return data.path;
    }

    // fallback: parse /proc/self/maps
    FILE *f = fopen("/proc/self/maps", "r");
    if (f == NULL) {
        LOGE("Failed to open process mappings");
        return NULL;
    }

    char line[1024];
    char *path = NULL;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strstr(line, lib_name)) {
            char *path_start = strrchr(line, '/');
            if (path_start != NULL) {
                char *path_end = strchr(path_start + 1, '\n');
                if (path_end) *path_end = '\0';

                char *full_path = strchr(line, '/');
                if (full_path) {
                    path = strdup(full_path);
                    LOGD("Found library path: %s via /proc/self/maps", result);
                    break;
                }
            }
        }
    }
    fclose(f);

    if (path == NULL) {
        LOGE("Failed to find base address of %s", lib_name);
        return NULL;
    }

    return path;
}

static void main() {
    const char *target_lib_name = "libc.so";

    void *target_lib_base = find_module(target_lib_name);
    if (target_lib_base == NULL) {
        LOGE("Failed to find library %s", target_lib_name);
        return;
    }


}

__attribute__((constructor))
void onLoad() {
    LOGD("Gotcha gadget loaded.");
    main();
}