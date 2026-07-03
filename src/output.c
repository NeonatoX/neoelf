#include "output.h"
#include <stdio.h>

static void print_full(enum output_mode mode, struct elf_ctx *ctx, struct dyn_info *dyn, const char *path)
{
    if (mode == OUT_EXPORT) {
        printf("CLASS=%s\n", elf_class_str(ctx->class));
        printf("TYPE=%s\n", elf_type_str(ctx->type));
        printf("MACHINE=%s\n", elf_machine_str(ctx->machine));
        printf("INTERP=%s\n", ctx->interp ? ctx->interp : "");
        printf("SONAME=%s\n", dyn->soname ? dyn->soname : "");
        for (size_t i = 0; i < dyn->needed_count; i++)
            printf("NEEDED=%s\n", dyn->needed[i]);
        printf("RUNPATH=%s\n", dyn->runpath ? dyn->runpath : "");
        printf("RPATH=%s\n", dyn->rpath ? dyn->rpath : "");
        return;
    }

    printf("File: %s\n", path);
    printf("\n");

    printf("Class:\n");
    printf("    %s\n", elf_class_str(ctx->class));
    printf("\n");

    printf("Machine:\n");
    printf("    %s\n", elf_machine_str(ctx->machine));
    printf("\n");

    printf("Type:\n");
    printf("    %s\n", elf_type_str(ctx->type));
    printf("\n");

    printf("Interpreter:\n");
    printf("    %s\n", ctx->interp ? ctx->interp : "(none)");
    printf("\n");

    printf("SONAME:\n");
    printf("    %s\n", dyn->soname ? dyn->soname : "(none)");
    printf("\n");

    printf("NEEDED:\n");
    if (dyn->needed_count == 0) {
        printf("    (none)\n");
    } else {
        for (size_t i = 0; i < dyn->needed_count; i++)
            printf("    %s\n", dyn->needed[i]);
    }
    printf("\n");

    printf("RUNPATH:\n");
    printf("    %s\n", dyn->runpath ? dyn->runpath : "(none)");
    printf("\n");

    printf("RPATH:\n");
    printf("    %s\n", dyn->rpath ? dyn->rpath : "(none)");
}

void output_print(enum output_mode mode, struct elf_ctx *ctx, struct dyn_info *dyn, const char *path)
{
    switch (mode) {
    case OUT_SONAME:
        printf("%s\n", dyn->soname ? dyn->soname : "(none)");
        break;
    case OUT_NEEDED:
        for (size_t i = 0; i < dyn->needed_count; i++)
            printf("%s\n", dyn->needed[i]);
        break;
    case OUT_INTERP:
        printf("%s\n", ctx->interp ? ctx->interp : "(none)");
        break;
    case OUT_MACHINE:
        printf("%s\n", elf_machine_str(ctx->machine));
        break;
    case OUT_TYPE:
        printf("%s\n", elf_type_str(ctx->type));
        break;
    case OUT_CLASS:
        printf("%s\n", elf_class_str(ctx->class));
        break;
    case OUT_RPATH:
        printf("%s\n", dyn->rpath ? dyn->rpath : "(none)");
        break;
    case OUT_RUNPATH:
        printf("%s\n", dyn->runpath ? dyn->runpath : "(none)");
        break;
    case OUT_EXPORT:
    case OUT_FULL:
    default:
        print_full(mode, ctx, dyn, path);
        break;
    }
}