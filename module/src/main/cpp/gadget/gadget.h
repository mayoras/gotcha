#ifndef GOTCHA_GADGET_H
#define GOTCHA_GADGET_H

#include <log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#if GOTCHA_DEBUG
#define LOG_TAG "[GotchaGadgetDebug]"
#else
#define LOG_TAG "[GotchaGadget]"
#endif

typedef struct _gotcha_module_st {
    const char *name;
    void *base;
    const char *path;
} Module;

typedef struct _gotcha_elf_section_st {
    char name[64];
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t type;
} ElfSection;

typedef struct _gotcha_elf_section_list_st {
    ElfSection *sections;
    size_t count;
} ElfSectionList;

static void *find_module_base(const char *lib_name);
static char *find_module_path(const char *lib_name);

static ElfSectionList *read_elf_sections(const char *path) {
    LOGE("Func %s not implemented", __func__);
}

#endif //GOTCHA_GADGET_H
