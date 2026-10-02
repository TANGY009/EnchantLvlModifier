# EnchantLvlModifier

A Minecraft Bedrock mod that grants you the power to customize the minimum and maximum level of each enchantment.

## Limits

- The **minLevel** can be as low as **1** and as high as **32767**
- The **maxLevel** can be as low as **1** and as high as **32767**

## Why set limits?

- The absolute minimum is 1 because level 0 or negative-value enchantments cannot be applied using in-game tools such as commands, anvils, or the enchanting table

- The absolute maximum is **32767** because Minecraft Bedrock stores enchantment levels as a **16-bit signed integer**, whose maximum value is **32767**
- If you force the game to use a value greater than 32767, it will overflow into the negative range. For example, 32768 becomes -32768
- Negative-level enchantments also cannot be applied using in-game tools

## Features

- For example, if you set **Fortune maxLevel** to **5**, you will be able to do Fortune 3 + Fortune 3 = Fortune 4 and Fortune 4 + Fortune 4 = Fortune 5
- But doing 5 + 5 will not give you 6 until you raise Fortune maxLevel from 5 to 6
---
- For example, if you set **Unbreaking minLevel** to **5** and **maxLevel** to **10**, the enchantment can only be **generated between levels 5 and 10**
- This means **level 5** is the **lowest possible Unbreaking level**. Enchanting tables, villager trades, and loot chests will therefore **not generate Unbreaking 1–4**; the lowest level they can give is Unbreaking 5
---
- The enchanting table can also give these custom-level enchantments

- Villagers have a chance to sell these custom-level enchantments

- Enchantments also have a chance to generate in natural loot chests

- Mobs can also spawn with these custom-level enchantments on their equipment

## Requirements

[Ambient](https://play.google.com/store/apps/details?id=io.kitsuri.mayape) or [LeviLaunchroid](https://github.com/LiteLDev/LeviLaunchroid)

## Config

### Ambient
- The config.json is located at
```
Android/media/io.kitsuri.mayape/EnchantLvlModifier/config.json
```

### LeviLaunchroid
- Can be configured via the launcher Configure button in the Mod Details
- Manage Mods > EnchantLvlModifier > Configure

- The config.json is located at
```
Android/media/org.levimc.launcher/minecraft/com.mojang.minecraftpe/mods/EnchantLvlModifier/config/config.json
```
Or, if you are using a custom instance
```
Android/media/org.levimc.launcher/minecraft/<instance_name>/mods/EnchantLvlModifier/config/config.json
```

## License

- The project source code is licensed under GNU [LGPL v3.0](https://www.gnu.org/licenses/lgpl-3.0.html).
- See the [NOTICE](NOTICE) file for details.
- This project uses [jsmn](https://github.com/zserge/jsmn), which is licensed under the MIT License.
