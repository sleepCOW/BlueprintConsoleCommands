//   LICENSE: zlib/libpng
//
//   Copyright (c) 2024-2025 Oleksandr Ozerov (@sleepCOW)
//
//   This software is provided "as-is", without any express or implied warranty. In no event
//   will the authors be held liable for any damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any purpose, including commercial
//   applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not claim that you
//     wrote the original software. If you use this software in a product, an acknowledgment
//     in the product documentation would be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not be misrepresented
//     as being the original software.
//
//     3. This notice may not be removed or altered from any source distribution.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CowCheatManagerDeveloperSettings.generated.h"

UCLASS(Config=Editor, DisplayName="Blueprint Console Commands Settings")
class COWCHEATMANAGER_API UCowCheatManagerDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * @brief Prefix used to categorize console commands specific to this project.
	 * 
	 * The `CheatCategoryPrefix` is essential to ensure that internal helper functions
	 * within the cheat manager are not exposed as public console commands. By using the
	 * project name as the primary category prefix, it helps in clearly distinguishing
	 * the project-specific console commands from the default ones provided by the engine.
	 * All console commands created in this project must start with this prefix.
	 * Additional sub-categories can be structured like "CheatCategoryPrefix.Gameplay.Stealth"
	 * to organize commands further under the main category.
	 */
	UPROPERTY(EditDefaultsOnly, Config, Category = "Config")
	FString CheatCategoryPrefix = "Cheat";

	/**
	 * Color that will be used for commands autocomplete in console
	 *
	 * @note CowCheatManagerExtension can override the color, so different modules have unique color
	 *		 it can be disabled by switching bForceCheatExtensionsUseMainColor to true
	 */
	UPROPERTY(EditDefaultsOnly, Config, Category = "Config|Visual")
	FColor MainCheatColor = FColor::Cyan;

	/**
	 * If true UCowCheatManagerExtension will use color defined by MainCheatColor instead of their override
	 */
	UPROPERTY(EditDefaultsOnly, Config, Category = "Config|Visual")
	bool bForceCheatExtensionsUseMainColor = false;

	/**
	 * If true Validation won't cry about missing comments for console commands
	 * But I beg you to spend additional 5 minutes to think about descriptive comment :pray:
	 */
	UPROPERTY(EditDefaultsOnly, Config, Category = "Config|Visual")
	bool bDisableMissingDescriptionWarnings = false;
	
	/**
	 * Whether to create default template console commands in newly created CowCheatManager/CowCheatManagerExtensions
	 * Helpful for beginners, might be redundant for somebody who get used to the plugin
	 */
	UPROPERTY(EditDefaultsOnly, Config, Category = "Config|Helpers")
	bool bCreateDefaultConsoleCommands = true;

	FORCEINLINE bool ShouldValidateMissingDescription() const { return !bDisableMissingDescriptionWarnings; }
};
