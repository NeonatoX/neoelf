#include "dynamic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#define MAX_DYN_COUNT 1024

struct dyn_raw {
    uint64_t tag;
    uint64_t val;
};

static int read_dyn_entry(struct elf_ctx *ctx, uint64_t offset, struct dyn_raw *entry)
{
    unsigned char *map = ctx->map;

    if (offset + sizeof(Elf64_Dyn) > ctx->map_size)
        return 0;

    if (ctx->class == ELFCLASS32) {
        Elf32_Dyn *d = (Elf32_Dyn *)(map + offset);
        entry->tag = d->d_tag;
        entry->val = d->d_un.d_val;
    } else {
        Elf64_Dyn *d = (Elf64_Dyn *)(map + offset);
        entry->tag = d->d_tag;
        entry->val = d->d_un.d_val;
    }

    return 1;
}

static uint64_t dyn_get_val(struct dyn_raw *entry)
{
    return entry->val;
}

static const char *dyn_str(struct elf_ctx *ctx, uint64_t strtab_off, size_t strtab_sz, uint64_t str_idx)
{
    if (strtab_off + strtab_sz > ctx->map_size)
        return NULL;

    if (str_idx >= strtab_sz)
        return NULL;

    return (const char *)ctx->map + strtab_off + str_idx;
}

int dynamic_parse(struct elf_ctx *ctx, struct dyn_info *info)
{
    uint64_t dynoff = ctx->dynoff;
    size_t dynsz = ctx->dynsz;
    uint64_t strtab_off = 0;
    size_t strtab_sz = 0;
    

    memset(info, 0, sizeof(*info));

    if (dynsz == 0 || dynoff == 0)
        return DYN_OK;

    size_t max_needed = 64;
    info->needed = calloc(max_needed, sizeof(char *));
    if (!info->needed)
        return DYN_ERR;

    size_t entry_size = (ctx->class == ELFCLASS32) ? sizeof(Elf32_Dyn) : sizeof(Elf64_Dyn);
    size_t count = dynsz / entry_size;
    if (count > MAX_DYN_COUNT)
        count = MAX_DYN_COUNT;

    size_t dyn_count = 0;
    struct dyn_raw *dyn_entries = calloc(count, sizeof(struct dyn_raw));
    if (!dyn_entries) {
        free(info->needed);
        info->needed = NULL;
        return DYN_ERR;
    }

    for (size_t i = 0; i < count; i++) {
        uint64_t ent_off = dynoff + i * entry_size;
        struct dyn_raw entry;

        if (!read_dyn_entry(ctx, ent_off, &entry))
            break;

        if (entry.tag == DT_NULL) {
            dyn_entries[dyn_count++] = entry;
            break;
        }

        dyn_entries[dyn_count++] = entry;
    }

    for (size_t i = 0; i < dyn_count; i++) {
        if (dyn_entries[i].tag == DT_STRTAB)
            strtab_off = dyn_get_val(&dyn_entries[i]);
        if (dyn_entries[i].tag == DT_STRSZ)
            strtab_sz = (size_t)dyn_get_val(&dyn_entries[i]);
    }

    for (size_t i = 0; i < dyn_count; i++) {
        struct dyn_raw *e = &dyn_entries[i];

        switch (e->tag) {
        case DT_SONAME:
            info->soname = dyn_str(ctx, strtab_off, strtab_sz,
                                   dyn_get_val(e));
            break;

        case DT_NEEDED:
            if (info->needed_count < max_needed) {
                info->needed[info->needed_count] =
                    dyn_str(ctx, strtab_off, strtab_sz,
                            dyn_get_val(e));
                info->needed_count++;
            }
            break;

        case DT_RPATH:
            info->rpath = dyn_str(ctx, strtab_off, strtab_sz,
                                  dyn_get_val(e));
            break;

        case DT_RUNPATH:
            info->runpath = dyn_str(ctx, strtab_off, strtab_sz,
                                    dyn_get_val(e));
            break;

        default:
            break;
        }
    }

    free(dyn_entries);

    ctx->strtab = strtab_off;
    ctx->strsz = strtab_sz;

    return DYN_OK;
}

void dynamic_free(struct dyn_info *info)
{
    if (info->needed) {
        free(info->needed);
        info->needed = NULL;
    }
    info->needed_count = 0;
}