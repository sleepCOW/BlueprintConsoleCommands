// MIT Licencse

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "Helpers.h"
#include <Engine/Console.h>
#include "CowCheatManager.generated.h"

/**
 *  A cheat manager extensions can extend the main cheat manager in a modular way,
 *	being enabled or disabled when the system associated with the cheats is enabled or disabled
 *
 *	See AddCheatManagerExtension and RemoveCheatManagerExtension
 *
 * Nice-to-have #TODO:
 * - Validate the function name doesn't overlap with UCowCheatManager
 */
UCLASS()
class COWCHEATMANAGER_API UCowCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Config|Visual")
	FColor OverrideCheatColor = FColor::Cyan;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;	
#endif
	
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;
    virtual void Serialize(FArchive& Ar) override;

	UPROPERTY()
	TArray<FCachedAutoCompleteCommand> CachedAutoCompleteCommands;
};

/**
 * #TODO list:
 * - Think about exposing autocomplete interface for the cheat manager, so commands like StartQuest could be made with autocomplete help
 * - Check whether CheatManagerExtensions being cut correctly in SHIPPED package
 * - When cheat manager class is created automatically make basic setup
 *		Example cheat command
 *		Comment block in event graph that say to look at ExampleCheatCommand and how to disable those default functions being created
 *		Comment block in ExampleCheatCommand that asks to fill description and says the category is all you need and I recommend to rename it to project name it's cool and intuitive
 *		Option to disable those helpers
 * - Add Icon
 * - Check Gameplay Tags are supported as param of command
 * 
 * - Add description of how to use
 *		Write docs on github
 * - Check C++ cheat commands can be created and work the same way as blueprint ones
 * - Check C++ cheat manager extensions could be created and work the same way as blueprint ones
 *
 * BUGS:
 * - UMainCheatManagerExtension::CppExtension isn't available in packaged game, debug cooking
 *
 * Features:
 *
 * - Simple to use blueprint console commands
 *		Just add a function with a category
 * - Automatic tooltip generation for function arguments, even ENUM support
 * - Validation of missing function descriptions and incorrect naming
 * - Exposed CheatManagerExtensions to blueprints, see AddCheatManagerExtension and RemoveCheatManagerExtension
 *		Versions AddCheatManagerExtensionClass and RemoveCheatManagerExtensionClass as QoL for easier setup
 * - Works in the editor and in the builds (Excluding SHIPPING)
 *
 * Nice-to-have #TODO:
 * - When CheatCategoryPrefix is changed automatically rename category in the function and event graph for event commands
 * - Add support for commands in event graph
 */
UCLASS()
class COWCHEATMANAGER_API UCowCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** Registers a pre-created cheat manager extension with this cheat manager */
	UFUNCTION(BlueprintCallable, Category = "Extension", meta = (DisplayName = "Add Cheat Manager Extension"))
	void BP_AddCheatManagerExtension(UCowCheatManagerExtension* CheatObject);

	/** Removes a cheat manager extension from this cheat manager */
	UFUNCTION(BlueprintCallable, Category = "Extension", meta = (DisplayName = "Remove Cheat Manager Extension"))
	void BP_RemoveCheatManagerExtension(UCowCheatManagerExtension* CheatObject);

	/** Creates and register a cheat manager extension with this cheat manager */
	UFUNCTION(BlueprintCallable, Category = "Extension", meta = (DisplayName = "Add Cheat Manager Extension Class"))
	void BP_AddCheatManagerExtensionClass(TSubclassOf<UCowCheatManagerExtension> CheatClass);

	/**
	 * Removes a cheat manager extension by class from this cheat manager
	 * @note removes first occurrence
	 */
	UFUNCTION(BlueprintCallable, Category = "Extension", meta = (DisplayName = "Remove Cheat Manager Extension Class"))
	void BP_RemoveCheatManagerExtensionClass(TSubclassOf<UCowCheatManagerExtension> CheatClass);
	
	/** Finds a previously registered cheat manager extension of the specified class */
	UFUNCTION(BlueprintCallable, Category = "Extension", meta = (DisplayName = "Find Cheat Manager Extension"))
	UCheatManagerExtension* BP_FindCheatManagerExtension(const UClass* InClass) const;
	
	virtual void InitCheatManager() override;
	virtual void BeginDestroy() override;
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	/**
	 * 
	 * @param InClass class for which generate executable cheat manager commands
	 * @param InclusiveStopClass class when to stop gathering ufunctions
	 * @return 
	 */
	static TArray<FAutoCompleteCommand> GenerateAutoCompleteCommands(UClass* InClass, const UClass* InclusiveStopClass, const FString& CheatPrefix, const FColor& CommandColor);
	
	// Rules:
	// 1. Verify functions in cheat manager doesn't contain white spaces
	// 2. Verify functions have description for tooltip
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;

    // Transforms function name into console command by merging function's categories
	// example "CategoryA.CategoryB.FuncName"
    static FString FormatFunctionNameToConsoleCommand(const UFunction* Function);
	static bool IsFunctionFirstCategoryEqualTo(const FString& CategoryToCheck, const UFunction* Function);
	static EDataValidationResult ValidateCheatFunctions(const TArray<FName>& FunctionNames, const UClass* OwningClass, class FDataValidationContext& Context);
#endif

	static TArray<FName> GetAllFunctionFromClass(UClass* InClass, const UClass* InclusiveStopClass);
	
protected:
	UPROPERTY()
	TArray<FCachedAutoCompleteCommand> CachedAutoCompleteCommands;

	void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand>& Commands) const;
};