#ifndef OUTPUT_H
#define OUTPUT_H

#include "elf.h"
#include "dynamic.h"

enum output_mode {
    OUT_FULL,
    OUT_SONAME,
    OUT_NEEDED,
    OUT_INTERP,
    OUT_MACHINE,
    OUT_TYPE,
    OUT_CLASS,
    OUT_RPATH,
    OUT_RUNPATH,
    OUT_EXPORT
};

void output_print(enum output_mode mode, struct elf_ctx *ctx, struct dyn_info *dyn, const char *path);

#endif