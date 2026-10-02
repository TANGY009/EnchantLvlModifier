#include "main.hpp"
#include "utils.hpp"

#define JSMN_STATIC
#define JSMN_STRICT
#include "jsmn.h"

static const unsigned char DefaultConfig[] = {
#include "config.json.h"
};

EnchantmentLevel Config[42]{};

const char* GetConfigPath() {
    static char Path[1024];
    char Launcher[256]{};
    Dl_info Info{};

    FILE* File = fopen("/proc/self/cmdline", "rb");

    if (!File) {
        LOG("Failed to open /proc/self/cmdline");
        return nullptr;
    }

    size_t Size = fread(Launcher, 1, sizeof(Launcher) - 1, File);
    fclose(File);

    if (!Size) {
        LOG("Failed to read launcher package");
        return nullptr;
    }

    if (!dladdr((void*)&GetConfigPath, &Info) || !Info.dli_fname) {
        LOG("Failed to get mod path");
        return nullptr;
    }

    if (!strcmp(Launcher, "io.kitsuri.mayape")) {
        snprintf(Path, sizeof(Path), "/storage/emulated/0/Android/media/%s/EnchantLvlModifier/config.json", Launcher);
        LOG("Ambient detected");
        LOG("Config file: %s", Path);
        return Path;
    }

    if (!strcmp(Launcher, "org.levimc.launcher")) {
        const char* Marker = "/native_mods/";
        const char* Start = strstr(Info.dli_fname, Marker);

        if (!Start) {
            LOG("Failed to find native_mods in mod path");
            return nullptr;
        }

        Start += strlen(Marker);

        const char* End = strchr(Start, '/');

        if (!End) {
            LOG("Failed to find Minecraft package in mod path");
            return nullptr;
        }

        char MinecraftPackage[256]{};
        size_t Length = End - Start;

        if (!Length || Length >= sizeof(MinecraftPackage)) {
            LOG("Invalid Minecraft package");
            return nullptr;
        }

        memcpy(MinecraftPackage, Start, Length);

        snprintf(Path, sizeof(Path), "/storage/emulated/0/Android/media/%s/minecraft/%s/mods/EnchantLvlModifier/config/config.json", Launcher, MinecraftPackage);
        LOG("LeviLaunchroid detected");
        LOG("Config file: %s", Path);
        return Path;
    }

    LOG("Unknown launcher: %s", Launcher);
    return nullptr;
}

bool LoadConfig() {
    const char* Path = GetConfigPath();

    if (!Path) {
        LOG("Failed to find config path");
        return false;
    }

    char ConfigDir[1024];
    strncpy(ConfigDir, Path, sizeof(ConfigDir) - 1);
    ConfigDir[sizeof(ConfigDir) - 1] = '\0';

    char* Slash = strrchr(ConfigDir, '/');

    if (!Slash) {
        LOG("Invalid config path");
        return false;
    }

    *Slash = '\0';

    if (access(Path, F_OK) != 0) {
        mkdir(ConfigDir, 0755);

        FILE* File = fopen(Path, "wb");

        if (!File) {
            LOG("Failed to create config: %s", Path);
            return false;
        }

        fwrite(DefaultConfig, 1, sizeof(DefaultConfig) - 1, File);
        fclose(File);

        LOG("Created config: %s", Path);
    }

    FILE* File = fopen(Path, "rb");

    if (!File) {
        LOG("Failed to open config: %s", Path);
        return false;
    }

    fseek(File, 0, SEEK_END);
    long Size = ftell(File);
    fseek(File, 0, SEEK_SET);

    if (Size <= 0) {
        fclose(File);
        LOG("Config is empty");
        return false;
    }

    char* JSON = (char*)malloc(Size + 1);

    if (!JSON) {
        fclose(File);
        LOG("Failed to allocate config");
        return false;
    }

    fread(JSON, 1, Size, File);
    fclose(File);

    jsmn_parser Parser;
    jsmntok_t Tokens[256];

    jsmn_init(&Parser);

    int TokenCount = jsmn_parse(&Parser, JSON, Size, Tokens, 256);

    if (TokenCount < 1 || Tokens[0].type != JSMN_OBJECT) {
        free(JSON);
        LOG("Invalid config JSON");
        return false;
    }

    memset(Config, 0, sizeof(Config));

    struct Correction {
        int Start;
        int End;
        int Value;
    };

    Correction Corrections[84];
    int CorrectionCount{};

    int Token = 1;
    int ID{};

    while (Token < TokenCount && ID < 42) {
        jsmntok_t* Name = &Tokens[Token];
        jsmntok_t* Value = &Tokens[Token + 1];

        if (Name->type != JSMN_STRING) {
            free(JSON);
            LOG("Invalid config entry");
            return false;
        }

        if (Value->type != JSMN_OBJECT) {
            free(JSON);
            LOG("Invalid enchantment entry");
            return false;
        }

        int MinLevel{};
        int MaxLevel{};
        bool HasMinLevel{};
        bool HasMaxLevel{};
        int MinStart{};
        int MinEnd{};
        int MaxStart{};
        int MaxEnd{};

        int Child = Token + 2;

        for (int i{}; i < Value->size; i++) {
            jsmntok_t* Key = &Tokens[Child];
            jsmntok_t* Number = &Tokens[Child + 1];

            if (Key->type != JSMN_STRING || Number->type != JSMN_PRIMITIVE) {
                free(JSON);
                LOG("Invalid enchantment value");
                return false;
            }

            int NumberValue{};
            int Start = Number->start;
            bool Negative{};

            if (JSON[Start] == '-') {
                Negative = true;
                Start++;
            }

            for (int j = Start; j < Number->end; j++) {
                if (JSON[j] == '.') {
                    break;
                }

                if (JSON[j] < '0' || JSON[j] > '9') {
                    free(JSON);
                    LOG("Invalid enchantment level");
                    return false;
                }

                if (NumberValue < 32768) {
                    NumberValue = NumberValue * 10 + (JSON[j] - '0');

                    if (NumberValue > 32768) {
                        NumberValue = 32768;
                    }
                }
            }

            if (Negative) {
                NumberValue = -NumberValue;
            }

            if (Key->end - Key->start == 8 && !strncmp(JSON + Key->start, "minLevel", 8)) {
                MinLevel = NumberValue;
                HasMinLevel = true;
                MinStart = Number->start;
                MinEnd = Number->end;
            }
            else if (Key->end - Key->start == 8 && !strncmp(JSON + Key->start, "maxLevel", 8)) {
                MaxLevel = NumberValue;
                HasMaxLevel = true;
                MaxStart = Number->start;
                MaxEnd = Number->end;
            }

            Child += 2;
        }

        if (!HasMinLevel || !HasMaxLevel) {
            free(JSON);
            LOG("Missing level in enchantment %d", ID);
            return false;
        }

        if (MinLevel < 1) {
            MinLevel = 1;

            Corrections[CorrectionCount++] = {
                MinStart,
                MinEnd,
                MinLevel
            };
        }

        if (MaxLevel > 32767) {
            MaxLevel = 32767;

            Corrections[CorrectionCount++] = {
                MaxStart,
                MaxEnd,
                MaxLevel
            };
        }

        Config[ID] = {MinLevel, MaxLevel};

        ID++;
        Token = Child;
    }

    if (ID != 42) {
        free(JSON);
        LOG("Expected 42 enchantments, got %d", ID);
        return false;
    }

    if (CorrectionCount > 0) {
        char* FixedJSON = (char*)malloc(Size + 1);

        if (!FixedJSON) {
            free(JSON);
            LOG("Failed to allocate corrected config");
            return false;
        }

        size_t OutputSize{};
        int Position{};

        for (int i{}; i < CorrectionCount; i++) {
            memcpy(FixedJSON + OutputSize, JSON + Position, Corrections[i].Start - Position);
            OutputSize += Corrections[i].Start - Position;

            char Number[16];
            int NumberSize = snprintf(Number, sizeof(Number), "%d", Corrections[i].Value);

            memcpy(FixedJSON + OutputSize, Number, NumberSize);
            OutputSize += NumberSize;

            Position = Corrections[i].End;
        }

        memcpy(FixedJSON + OutputSize, JSON + Position, Size - Position);
        OutputSize += Size - Position;

        File = fopen(Path, "wb");

        if (!File) {
            free(FixedJSON);
            free(JSON);
            LOG("Failed to rewrite config: %s", Path);
            return false;
        }

        fwrite(FixedJSON, 1, OutputSize, File);
        fclose(File);

        free(FixedJSON);
    }

    free(JSON);

    LOG("Loaded %d enchantments from config", ID);
    return true;
}

uintptr_t GetLibSection(const char* libname, const char* section_name, size_t* out_size) {
    if (!libname) return 0;
    if (!section_name) section_name = ".text";

    uintptr_t base_addr{};
    char lib_path[512] = {0};

    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return 0;

    char line[512];

    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, libname)) {
            char path[256] = {0};

            if (sscanf(line, "%llx-%*x %*s %*x %*s %*d %255s", (unsigned long long*)&base_addr, path) >= 1) {
                if (path[0] == '/') {
                    strncpy(lib_path, path, sizeof(lib_path) - 1);
                    break;
                }
            }
        }
    }

    fclose(maps);

    if (lib_path[0] == '\0' || base_addr == 0) return 0;

    int fd = open(lib_path, O_RDONLY);
    if (fd < 0) return 0;

    struct stat st;

    if (fstat(fd, &st) < 0) {
        close(fd);
        return 0;
    }

    void* map_base = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (map_base == MAP_FAILED) return 0;

    uintptr_t section_runtime_addr{};

    ElfW(Ehdr)* ehdr = (ElfW(Ehdr)*)map_base;

    if (ehdr->e_ident[EI_MAG0] == ELFMAG0 && ehdr->e_ident[EI_MAG1] == ELFMAG1 && ehdr->e_ident[EI_MAG2] == ELFMAG2 && ehdr->e_ident[EI_MAG3] == ELFMAG3) {
        ElfW(Shdr)* shdr = (ElfW(Shdr)*)((uintptr_t)map_base + ehdr->e_shoff);
        const char* shstrtab = (const char*)((uintptr_t)map_base + shdr[ehdr->e_shstrndx].sh_offset);

        for (int i{}; i < ehdr->e_shnum; i++) {
            const char* current_section_name = shstrtab + shdr[i].sh_name;

            if (strcasecmp(current_section_name, section_name) == 0) {
                section_runtime_addr = base_addr + shdr[i].sh_addr;

                if (out_size) {
                    *out_size = shdr[i].sh_size;
                }

                break;
            }
        }
    }

    munmap(map_base, st.st_size);

    return section_runtime_addr;
}

bool SetMemoryPermission(uintptr_t addr, size_t len, int prot) {
    if (!addr || !len) return false;

    size_t pagesize = sysconf(_SC_PAGESIZE);
    uintptr_t aligned_addr = addr & ~(pagesize - 1);
    size_t aligned_len = ((addr + len + pagesize - 1) & ~(pagesize - 1)) - aligned_addr;

    return mprotect((void*)aligned_addr, aligned_len, prot) == 0;
}

bool Unprotect(uintptr_t addr, size_t len) {
    return SetMemoryPermission(addr, len, PROT_READ | PROT_WRITE);
}

bool Protect(uintptr_t addr, size_t len) {
    return SetMemoryPermission(addr, len, PROT_READ);
}

void** FindVtable(const char* typeStr) {
    static uintptr_t rodata{}, drr{};
    static size_t rodataSize{}, drrSize{};

    if (!rodata) {
        const char* module = "libminecraftpe.so";

        rodata = GetLibSection(module, ".rodata", &rodataSize);
        drr = GetLibSection(module, ".data.rel.ro", &drrSize);
    }

    if (!rodata || !drr) return nullptr;

    char* ztsPtr = nullptr;
    size_t classLen = strlen(typeStr);
    size_t offset{};

    while (offset < rodataSize) {
        char* match = (char*)memmem((void*)(rodata + offset), rodataSize - offset, typeStr, classLen + 1);

        if (!match) break;

        if (match == (char*)rodata || *(match - 1) == '\0') {
            ztsPtr = match;
            break;
        }

        offset = (uintptr_t)match - rodata + 1;
    }

    if (!ztsPtr) return nullptr;

    uintptr_t zts = (uintptr_t)ztsPtr;
    uintptr_t zti{};

    for (size_t i{}; i < drrSize; i += sizeof(uintptr_t)) {
        if (*(uintptr_t*)(drr + i) == zts) {
            zti = drr + i - sizeof(uintptr_t);
            break;
        }
    }

    if (!zti) return nullptr;

    uintptr_t vtable{};

    for (size_t i{}; i < drrSize; i += sizeof(uintptr_t)) {
        if (*(uintptr_t*)(drr + i) == zti) {
            uintptr_t potential_vtable = drr + i + sizeof(uintptr_t);

            if (i >= sizeof(uintptr_t) && *(uintptr_t*)(drr + i - sizeof(uintptr_t)) == 0) {
                vtable = potential_vtable;
                break;
            }

            if (!vtable) {
                vtable = potential_vtable;
            }
        }
    }

    if (!vtable) return nullptr;

    return (void**)vtable;
}