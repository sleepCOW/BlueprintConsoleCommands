// MIT Licencse

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "Helpers.h"
#include <Engine/Console.h>
#include "BlueprintCMDCheatManager.generated.h"

/**
 * #TODO Add description of how to use
 */
UCLASS()
class BLUEPRINTCMD_API UBlueprintCMDCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Config|Visual")
	FColor CheatColor = FColor::Cyan;

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
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FString CheatCategoryPrefix = "Cheat";
	
	virtual void InitCheatManager() override;
	virtual void BeginDestroy() override;
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	TArray<FAutoCompleteCommand> GenerateAutoCompleteCommands() const;
#endif
	
protected:
	UPROPERTY()
	TArray<FCachedAutoCompleteCommand> CachedAutoCompleteCommands;

	void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand>& Commands) const;
};