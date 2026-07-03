#include "elf.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void *map_file(const char *path, int *fd, size_t *size)
{
    struct stat st;
    void *data;

    *fd = open(path, O_RDONLY);
    if (*fd < 0)
        return NULL;

    if (fstat(*fd, &st) < 0) {
        close(*fd);
        return NULL;
    }

    if (st.st_size == 0) {
        close(*fd);
        return NULL;
    }

    *size = (size_t)st.st_size;
    data = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, *fd, 0);
    if (data == MAP_FAILED) {
        close(*fd);
        return NULL;
    }

    return data;
}

int elf_open(struct elf_ctx *ctx, const char *path)
{
    size_t size;

    memset(ctx, 0, sizeof(*ctx));
    ctx->fd = -1;

    ctx->map = map_file(path, &ctx->fd, &size);
    if (!ctx->map)
        return ELF_ERR_OPEN;

    ctx->map_size = size;

    if (size < sizeof(Elf32_Ehdr)) {
        elf_close(ctx);
        return ELF_ERR_TRUNCATED;
    }

    unsigned char *e_ident = ctx->map;

    if (memcmp(e_ident, ELFMAG, SELFMAG) != 0) {
        elf_close(ctx);
        return ELF_ERR_NOT_ELF;
    }

    ctx->class = e_ident[EI_CLASS];

    if (ctx->class == ELFCLASS32) {
        Elf32_Ehdr *ehdr = ctx->map;
        ctx->endian = e_ident[EI_DATA];
        ctx->type = ehdr->e_type;
        ctx->machine = ehdr->e_machine;
        ctx->phoff = ehdr->e_phoff;
        ctx->phnum = ehdr->e_phnum;
        ctx->phentsize = ehdr->e_phentsize;
        ctx->shoff = ehdr->e_shoff;
        ctx->shnum = ehdr->e_shnum;
        ctx->shentsize = ehdr->e_shentsize;
        ctx->shstrndx = ehdr->e_shstrndx;

        if (ctx->phnum > 0 && ctx->phentsize < sizeof(Elf32_Phdr)) {
            elf_close(ctx);
            return ELF_ERR_TRUNCATED;
        }
    } else if (ctx->class == ELFCLASS64) {
        Elf64_Ehdr *ehdr = ctx->map;
        ctx->endian = e_ident[EI_DATA];
        ctx->type = ehdr->e_type;
        ctx->machine = ehdr->e_machine;
        ctx->phoff = ehdr->e_phoff;
        ctx->phnum = ehdr->e_phnum;
        ctx->phentsize = ehdr->e_phentsize;
        ctx->shoff = ehdr->e_shoff;
        ctx->shnum = ehdr->e_shnum;
        ctx->shentsize = ehdr->e_shentsize;
        ctx->shstrndx = ehdr->e_shstrndx;

        if (ctx->phnum > 0 && ctx->phentsize < sizeof(Elf64_Phdr)) {
            elf_close(ctx);
            return ELF_ERR_TRUNCATED;
        }
    } else {
        elf_close(ctx);
        return ELF_ERR_NOCLASS;
    }

    return ELF_OK;
}

void elf_close(struct elf_ctx *ctx)
{
    if (ctx->map) {
        munmap(ctx->map, ctx->map_size);
        ctx->map = NULL;
    }
    if (ctx->fd >= 0) {
        close(ctx->fd);
        ctx->fd = -1;
    }
}

int elf_validate(struct elf_ctx *ctx)
{
    if (ctx->class != ELFCLASS32 && ctx->class != ELFCLASS64)
        return ELF_ERR_NOCLASS;
    return ELF_OK;
}

int elf_read_headers(struct elf_ctx *ctx)
{
    uint64_t dynoff = 0;
    size_t dynsz = 0;
    elf_find_pt_dynamic(ctx, &dynoff, &dynsz);
    ctx->dynoff = dynoff;
    ctx->dynsz = dynsz;

    const char *interp_data = NULL;
    size_t interp_len = 0;
    elf_find_pt_interp(ctx, &interp_data, &interp_len);
    ctx->interp = interp_data;
    ctx->interp_len = interp_len;

    return ELF_OK;
}

static int check_bounds(struct elf_ctx *ctx, uint64_t offset, size_t len)
{
    if (offset > ctx->map_size)
        return 0;
    if (offset + len > ctx->map_size)
        return 0;
    return 1;
}

int elf_find_pt_dynamic(struct elf_ctx *ctx, uint64_t *off, size_t *sz)
{
    if (ctx->phnum == 0 || ctx->phoff == 0)
        return ELF_ERR_NOSEGMENT;

    if (ctx->class == ELFCLASS32) {
        uint64_t start = ctx->phoff;
        for (uint16_t i = 0; i < ctx->phnum; i++) {
            uint64_t phaddr = start + (uint64_t)i * ctx->phentsize;
            if (!check_bounds(ctx, phaddr, sizeof(Elf32_Phdr)))
                return ELF_ERR_TRUNCATED;

            Elf32_Phdr *ph = (Elf32_Phdr *)((char *)ctx->map + phaddr);
            if (ph->p_type == PT_DYNAMIC && ph->p_filesz > 0) {
                *off = ph->p_offset;
                *sz = (size_t)ph->p_filesz;
                return ELF_OK;
            }
        }
    } else if (ctx->class == ELFCLASS64) {
        uint64_t start = ctx->phoff;
        for (uint16_t i = 0; i < ctx->phnum; i++) {
            uint64_t phaddr = start + (uint64_t)i * ctx->phentsize;
            if (!check_bounds(ctx, phaddr, sizeof(Elf64_Phdr)))
                return ELF_ERR_TRUNCATED;

            Elf64_Phdr *ph = (Elf64_Phdr *)((char *)ctx->map + phaddr);
            if (ph->p_type == PT_DYNAMIC && ph->p_filesz > 0) {
                *off = ph->p_offset;
                *sz = (size_t)ph->p_filesz;
                return ELF_OK;
            }
        }
    }

    return ELF_ERR_NOSEGMENT;
}

int elf_find_pt_interp(struct elf_ctx *ctx, const char **data, size_t *len)
{
    if (ctx->phnum == 0 || ctx->phoff == 0)
        return ELF_ERR_NOSEGMENT;

    if (ctx->class == ELFCLASS32) {
        uint64_t start = ctx->phoff;
        for (uint16_t i = 0; i < ctx->phnum; i++) {
            uint64_t phaddr = start + (uint64_t)i * ctx->phentsize;
            if (!check_bounds(ctx, phaddr, sizeof(Elf32_Phdr)))
                return ELF_ERR_TRUNCATED;

            Elf32_Phdr *ph = (Elf32_Phdr *)((char *)ctx->map + phaddr);
            if (ph->p_type == PT_INTERP && ph->p_filesz > 1) {
                if (!check_bounds(ctx, ph->p_offset, ph->p_filesz))
                    return ELF_ERR_TRUNCATED;

                *data = (const char *)ctx->map + ph->p_offset;
                *len = (size_t)ph->p_filesz - 1;
                return ELF_OK;
            }
        }
    } else if (ctx->class == ELFCLASS64) {
        uint64_t start = ctx->phoff;
        for (uint16_t i = 0; i < ctx->phnum; i++) {
            uint64_t phaddr = start + (uint64_t)i * ctx->phentsize;
            if (!check_bounds(ctx, phaddr, sizeof(Elf64_Phdr)))
                return ELF_ERR_TRUNCATED;

            Elf64_Phdr *ph = (Elf64_Phdr *)((char *)ctx->map + phaddr);
            if (ph->p_type == PT_INTERP && ph->p_filesz > 1) {
                if (!check_bounds(ctx, ph->p_offset, ph->p_filesz))
                    return ELF_ERR_TRUNCATED;

                *data = (const char *)ctx->map + ph->p_offset;
                *len = (size_t)ph->p_filesz - 1;
                return ELF_OK;
            }
        }
    }

    return ELF_ERR_NOSEGMENT;
}

const char *elf_class_str(int class)
{
    switch (class) {
    case ELFCLASS32: return "ELF32";
    case ELFCLASS64: return "ELF64";
    default:         return "UNKNOWN";
    }
}

const char *elf_type_str(int type)
{
    switch (type) {
    case ET_EXEC: return "EXEC";
    case ET_DYN:  return "DYN";
    case ET_REL:  return "REL";
    case ET_CORE: return "CORE";
    default:      return "UNKNOWN";
    }
}

const char *elf_machine_str(int machine)
{
    switch (machine) {
    case EM_NONE:         return "NONE";
    case EM_386:          return "i386";
    case EM_ARM:          return "ARM";
    case EM_X86_64:       return "x86_64";
    case EM_AARCH64:      return "AArch64";
    case EM_MIPS:         return "MIPS";
    case EM_PPC:          return "PPC";
    case EM_PPC64:        return "PPC64";
    case EM_RISCV:        return "RISC-V";
    case EM_S390:         return "S390";
    default:              return "UNKNOWN";
    }
}