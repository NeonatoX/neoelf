#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"
#include "dynamic.h"
#include "output.h"

static void usage(const char *prog)
{
    fprintf(stderr, "neoelf 0.1 - Copyright (C) 2026 Carlos Sanchez\n");
    fprintf(stderr, "License GPLv3: GNU GPL version 3\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [OPTION] FILE\n", prog);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  --soname      Print SONAME\n");
    fprintf(stderr, "  --needed      Print NEEDED libraries\n");
    fprintf(stderr, "  --interp      Print interpreter\n");
    fprintf(stderr, "  --machine     Print machine type\n");
    fprintf(stderr, "  --type        Print ELF type\n");
    fprintf(stderr, "  --class       Print ELF class\n");
    fprintf(stderr, "  --rpath       Print RPATH\n");
    fprintf(stderr, "  --runpath     Print RUNPATH\n");
    fprintf(stderr, "  --export      Print machine-readable format\n");
    fprintf(stderr, "  --help        Show this help\n");
}

static void die(const char *prog, const char *path, int err)
{
    const char *msg;

    switch (err) {
    case ELF_ERR_OPEN:
        msg = "cannot open file";
        break;
    case ELF_ERR_READ:
        msg = "read error";
        break;
    case ELF_ERR_NOT_ELF:
        msg = "not an ELF file";
        break;
    case ELF_ERR_TRUNCATED:
        msg = "truncated ELF file";
        break;
    case ELF_ERR_NOCLASS:
        msg = "unsupported ELF class";
        break;
    default:
        msg = "unknown error";
        break;
    }

    fprintf(stderr, "%s: %s: %s\n", prog, path, msg);
}

int main(int argc, char **argv)
{
    enum output_mode mode = OUT_FULL;
    const char *path = NULL;

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--soname") == 0)
            mode = OUT_SONAME;
        else if (strcmp(argv[i], "--needed") == 0)
            mode = OUT_NEEDED;
        else if (strcmp(argv[i], "--interp") == 0)
            mode = OUT_INTERP;
        else if (strcmp(argv[i], "--machine") == 0)
            mode = OUT_MACHINE;
        else if (strcmp(argv[i], "--type") == 0)
            mode = OUT_TYPE;
        else if (strcmp(argv[i], "--class") == 0)
            mode = OUT_CLASS;
        else if (strcmp(argv[i], "--rpath") == 0)
            mode = OUT_RPATH;
        else if (strcmp(argv[i], "--runpath") == 0)
            mode = OUT_RUNPATH;
        else if (strcmp(argv[i], "--export") == 0)
            mode = OUT_EXPORT;
        else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
            return 1;
        } else {
            path = argv[i];
        }
    }

    if (!path) {
        fprintf(stderr, "%s: no file specified\n", argv[0]);
        return 1;
    }

    struct elf_ctx ctx;
    int rc = elf_open(&ctx, path);
    if (rc != ELF_OK) {
        die(argv[0], path, rc);
        return 1;
    }

    rc = elf_validate(&ctx);
    if (rc != ELF_OK) {
        die(argv[0], path, rc);
        elf_close(&ctx);
        return 1;
    }

    elf_read_headers(&ctx);

    struct dyn_info dyn;
    dynamic_parse(&ctx, &dyn);

    output_print(mode, &ctx, &dyn, path);

    dynamic_free(&dyn);
    elf_close(&ctx);

    return 0;
}