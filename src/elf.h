#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>
#include <elf.h>

#define ELF_OK              0
#define ELF_ERR_OPEN        1
#define ELF_ERR_READ        2
#define ELF_ERR_NOT_ELF     3
#define ELF_ERR_TRUNCATED   4
#define ELF_ERR_NOCLASS     5
#define ELF_ERR_NOARCH      6
#define ELF_ERR_NOSEGMENT   7

struct elf_ctx {
    int fd;
    void *map;
    size_t map_size;
    int class;
    int endian;
    int type;
    int machine;
    uint64_t phoff;
    uint16_t phnum;
    uint16_t phentsize;
    uint64_t shoff;
    uint16_t shnum;
    uint16_t shentsize;
    uint16_t shstrndx;
    const char *interp;
    size_t interp_len;
    uint64_t dynoff;
    size_t dynsz;
    uint64_t strtab;
    uint64_t strsz;
};

int elf_open(struct elf_ctx *ctx, const char *path);
void elf_close(struct elf_ctx *ctx);
int elf_validate(struct elf_ctx *ctx);
int elf_read_headers(struct elf_ctx *ctx);
int elf_find_pt_dynamic(struct elf_ctx *ctx, uint64_t *off, size_t *sz);
int elf_find_pt_interp(struct elf_ctx *ctx, const char **data, size_t *len);

const char *elf_class_str(int class);
const char *elf_type_str(int type);
const char *elf_machine_str(int machine);

#endif