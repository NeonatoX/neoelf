#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "elf.h"

#define DYN_OK           0
#define DYN_ERR          1

struct dyn_info {
    const char *soname;
    const char **needed;
    size_t needed_count;
    const char *rpath;
    const char *runpath;
};

int dynamic_parse(struct elf_ctx *ctx, struct dyn_info *info);
void dynamic_free(struct dyn_info *info);

#endif