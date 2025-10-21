![](https://img.shields.io/badge/UE-5.4.4-green)
![](https://img.shields.io/badge/UE-5.5.4-green)
![](https://img.shields.io/badge/UE-5.6.1-green)

![Intro](Docs/Intro_addingCommands_optimized.gif)  

# About

An Unreal Engine plugin that provides an easy way to implement console commands in blueprints through custom `CheatManager` and `CheatManagerExtension` classes

if you like the plugin and want to support & encourage me you can purchase `Professional` license on Fab `LINK` or buy me a coffee

## Features:

- **Easy**: derive from `CowCheatManager` and add functions under "Cheat" category (can be changed via `CheatCategoryPrefix`), all in Blueprints.
- **Auto-complete**:
    - All commands within active cheat manager is autoregistered and added to Unreal`s console
    - Function `Description` is used as a console command help
    - All arguments automatically added to help
    ![Description](Docs/AutoComplete.png)
    - Support of Enums (both Blueprints and C++), primitive types, GameplayTags, Structures (however structs have very inconvinient syntax for console usage, e.g. `FVector` parameter should be passed as `(x=1.5,y=2.6,z=3.7)`, no spaces allowed, all members must be named explicitly so I recommend avoid using structs as console command parameters)
  ![Autowire](Docs/AutoComplete_Optimized.gif)
- **Custom colors**: Color of console commands can be changed in `Blueprint Console Commands Settings` via `MainCheatColor` or by using `OverrideCheatColor` in `CowCheatManagerExtension` if you want extension to have a specific color coding.
- **Modular**: exposed `CowCheatManagerExtension` for modular console commands, add with `AddCheatManagerExtensionClass`. The `Cheat` prefix can be changed via `CheatCategoryPrefix`.
- **Data Validation**: Warnings when console commands has no description (can be disabled via `bDisableMissingDescriptionWarnings`) and validation for conflicting names with existing console commands.
- **Cpp Support**: CheatManager's console commands can be added directly in code, so designers and programmers don't kill each other :P
  ```c++
    // Command description is here
    UFUNCTION(Category = "Cheat|Gameplay")
    void CppCommand(float MyFloat);
  ```

## How to

1. How to create & setup cheat manager
2. How to create a console command
3. How to create cheat manager extension
4. How to add cheat manager extension
5. Available settings
6. Cpp examples

## 
