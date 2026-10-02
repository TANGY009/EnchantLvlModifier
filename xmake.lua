set_project("EnchantLvlModifier")

set_languages("cxx23")

set_plat("android")
set_arch("arm64-v8a")

add_rules("mode.release")

add_cxflags("-Oz", "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections", "-flto", "-w")
add_ldflags("-Wl,--gc-sections", "-Wl,--strip-all", "-s")

target("EnchantLvlModifier")
    set_kind("shared")
    add_rules("utils.bin2c", {extensions = {".json"}})
    add_files("src/*.cpp")
    add_files("config/config.json")
    add_includedirs("src")
    add_links("android", "log")