#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>

#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct EnchantmentLevel {
    int minLevel;
    int maxLevel;
};

extern EnchantmentLevel Config[42];

bool LoadConfig();

const char* GetConfigPath();
uintptr_t GetLibSection(const char* libname, const char* section_name, size_t* out_size);
bool SetMemoryPermission(uintptr_t addr, size_t len, int prot);
bool Unprotect(uintptr_t addr, size_t len);
bool Protect(uintptr_t addr, size_t len);
void** FindVtable(const char* typeStr);