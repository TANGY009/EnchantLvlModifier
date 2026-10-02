#include "main.hpp"
#include "utils.hpp"

int64_t LevelController::getMinLevel(void* self) {
    if (!self) return 1;

    uint8_t ID = *(uint8_t*)((uintptr_t)self + 0x8);

    if (ID >= 42) return 1;

    return Config[ID].minLevel;
}

int64_t LevelController::getMaxLevel(void* self) {
    if (!self) return 1;

    uint8_t ID = *(uint8_t*)((uintptr_t)self + 0x8);

    if (ID >= 42) return 1;

    return Config[ID].maxLevel;
}

void HookLevels() {
    static const char* Vtables[] = {
        "17ProtectionEnchant",
        "11SwimEnchant",
        "18MeleeWeaponEnchant",
        "11LootEnchant",
        "14DiggingEnchant",
        "10BowEnchant",
        "14FishingEnchant",
        "18FrostWalkerEnchant",
        "14MendingEnchant",
        "19CurseBindingEnchant",
        "21CurseVanishingEnchant",
        "21TridentImpalerEnchant",
        "21TridentRiptideEnchant",
        "21TridentLoyaltyEnchant",
        "24TridentChannelingEnchant",
        "15CrossbowEnchant",
        "16SoulSpeedEnchant",
        "17SwiftSneakEnchant",
        "16WindBurstEnchant",
        "14DensityEnchant",
        "13BreachEnchant",
        "12LungeEnchant"
    };

    const size_t VtableCount = sizeof(Vtables) / sizeof(Vtables[0]);
    uintptr_t MinHook = (uintptr_t)&LevelController::getMinLevel;
    uintptr_t MaxHook = (uintptr_t)&LevelController::getMaxLevel;
    size_t Hooked{};

    for (const char* typeStr : Vtables) {
        void** Vtable = FindVtable(typeStr);

        if (!Vtable) {
            continue;
        }

        void** MinSlot = &Vtable[5];
        void** MaxSlot = &Vtable[6];

        if ((uintptr_t)*MinSlot != MinHook) {
            if (Unprotect((uintptr_t)MinSlot, sizeof(uintptr_t))) {
                *MinSlot = (void*)MinHook;
                Protect((uintptr_t)MinSlot, sizeof(uintptr_t));
            }
        }

        if ((uintptr_t)*MaxSlot != MaxHook) {
            if (Unprotect((uintptr_t)MaxSlot, sizeof(uintptr_t))) {
                *MaxSlot = (void*)MaxHook;
                Protect((uintptr_t)MaxSlot, sizeof(uintptr_t));
            }
        }

        if ((uintptr_t)*MinSlot == MinHook && (uintptr_t)*MaxSlot == MaxHook) {
            Hooked++;
        }
    }

    LOG("Hooked %zu/%zu vtables", Hooked, VtableCount);
}

__attribute__((constructor))
void Init() {
    if (!LoadConfig()) {
        LOG("failed to load config");
        return;
    }

    HookLevels();
}