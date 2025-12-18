#include "CowCheatManager.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "CowCheatManagerDeveloperSettings.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/DataValidation.h"

DEFINE_LOG_CATEGORY_STATIC(LogCowCheatManager, Log, All);

static int32 NumAutoCompletesRegistered = 0;

#define LOCTEXT_NAMESPACE "CowCheatManager"

static FORCEINLINE bool IsArrayEmpty(const TArray<FString>& InArray)
{
#if ENGINE_MAJOR_VERSION == 4
	return InArray.Num() == 0;
#else ENGINE_MAJOR_VERSION == 5
	return InArray.IsEmpty();
#endif
}

#if WITH_EDITOR
static const UEnum* IsEnumProperty(const FProperty* Property)
{
	if (const FByteProperty* ByteProp = CastField<FByteProperty>(Property))
	{
		return ByteProp->GetIntPropertyEnum();
	}
	else if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
	{
		return EnumProp->GetEnum();
	}
	return nullptr;
}

static FString FormatEnumDescription(const UEnum* Enum)
{
	FString Description;
	Description += "[";

	// Subtract one to avoid automatically generated E_MAX value
	const int32 EnumSize = Enum->NumEnums() - 1;

	for (int32 Index = 0; Index < EnumSize; ++Index)
	{
		const FString Delim = Index == EnumSize - 1 ? "" : ", ";
		FString EnumName = Enum->GetDisplayNameTextByIndex(Index).ToString();
		Description += EnumName + Delim;
	}
	Description += "] ";
	return Description;
}

static FString BuildCommandDescription(const UFunction* Func)
{
	FString Desc;

	// build a help string
	// append each property (and it's type) to the help string
	for (TFieldIterator<FProperty> PropIt(Func); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
	{
		FProperty* Property = *PropIt;
		
		if (const UEnum* Enum = IsEnumProperty(Property))
		{
			Desc += Property->GetName();
			Desc += FormatEnumDescription(Enum);
		}
		else
		{
			Desc += FString::Printf(TEXT("%s[%s] "), *Property->GetName(), *Property->GetCPPType());
		}
	}
	
	FString CheatTooltip = Func->GetMetaData( TEXT("Tooltip") );
	// Append function tooltip to the description
	Desc += CheatTooltip;

	const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();
	if (Settings->ShouldValidateMissingDescription() && CheatTooltip.IsEmpty())
	{
		UE_LOG(LogCowCheatManager, Warning, TEXT("Cheat function %s has no description! Please consider adding it!"), *Func->GetName());
		Desc += "Please, provide description in function details panel!";
	}

	return Desc;
}

TArray<FAutoCompleteCommand> UCowCheatManager::GenerateAutoCompleteCommands(UClass* InClass,
	const UClass* InclusiveStopClass, const FString& CheatPrefix, const FColor& CommandColor)
{
	check(InClass);
	TArray<FAutoCompleteCommand> ReturnValue;

	// Find any functions with the Cheat meta (haven't bothered doing any duplicate detection or other validation)

	// Generate function for whole hierarchy
	TArray<FName> FunctionNames = GetAllFunctionFromClass(InClass, InclusiveStopClass);
	
	for (const FName& Name : FunctionNames)
	{
		const UFunction* Func = InClass->FindFunctionByName(Name);

		// Make sure Cheat is under correct category prefix!
		if (!IsFunctionFirstCategoryEqualTo(CheatPrefix, Func))
		{
			continue;
		}
		
        const FString CommandName = FormatFunctionNameToConsoleCommand(Func);

		FAutoCompleteCommand Entry;

		Entry.Command = CommandName;
		Entry.Desc = BuildCommandDescription(Func);
		Entry.Color = CommandColor;
	
		ReturnValue.Add(Entry);
	}
	return ReturnValue;
}

EDataValidationResult UCowCheatManager::IsDataValid(FDataValidationContext& Context) const
{
	// #TODO: Add automatic rename
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);
	TArray<FName> FunctionNames = GetAllFunctionFromClass(GetClass(), UCowCheatManager::StaticClass());
	
	ValidationResult = CombineDataValidationResults(ValidateCheatFunctions(FunctionNames, GetClass(),Context), ValidationResult);
		
	return ValidationResult;
}

FString UCowCheatManager::FormatFunctionNameToConsoleCommand(const UFunction* Function)
{
    check(Function);
		
    const FString& Category = Function->GetMetaData(TEXT("Category"));
    TArray<FString> FuncCategories;
    Category.ParseIntoArray(FuncCategories, TEXT("|"), true);

    // Append to the categories command name
    // So it will look like "CategoryA.CategoryB.FuncName"
    FuncCategories.Add(Function->GetName());
    return FString::Join(FuncCategories, TEXT("."));
}

bool UCowCheatManager::IsFunctionFirstCategoryEqualTo(const FString& CategoryToCheck, const UFunction* Function)
{
	check(Function);
	const FString& Category = Function->GetMetaData(TEXT("Category"));
	TArray<FString> FuncCategories;
	Category.ParseIntoArray(FuncCategories, TEXT("|"), true);
	
	if (IsArrayEmpty(FuncCategories) || FuncCategories[0] != CategoryToCheck)
	{
		return false;
	}
	else
	{
		return true;
	}
}

EDataValidationResult UCowCheatManager::ValidateCheatFunctions(const TArray<FName>& FunctionNames,
                                                              const UClass* OwningClass, FDataValidationContext& Context)
{
	check(OwningClass);
	// Validate BP function names in bps don't have any white spaces
	EDataValidationResult ValidationResult =  EDataValidationResult::Valid;
	const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();
	
	for (const FName& Name : FunctionNames)
	{
        const FString& NameStr = Name.ToString();

		// Check no whitespaces in function names, otherwise it will break function calling
		if (NameStr.Find(" ") != INDEX_NONE)
		{
			ValidationResult = EDataValidationResult::Invalid;
			Context.AddError(
						FText::Format(
							LOCTEXT("Error_InvalidFunctionName", "Function \"{0}\" contains white spaces, please rename without spaces."),
							FText::FromString(NameStr)
						)
					);
		}

        if (UFunction* Func = OwningClass->FindFunctionByName(Name))
		{
            if (IsFunctionFirstCategoryEqualTo(Settings->CheatCategoryPrefix, Func))
			{
                // Check whether we have a conflict with existing console objects
                const FString ConsoleCommandStr = UCowCheatManager::FormatFunctionNameToConsoleCommand(Func);
                if (IConsoleObject* ExistingConsoleObj = IConsoleManager::Get().FindConsoleObject(*ConsoleCommandStr))
                {
                    ValidationResult = EDataValidationResult::Invalid;
                    Context.AddError(
                                FText::Format(
                                    LOCTEXT("Error_NameConflict", "Function \"{0}\" conflicts with existing console command \"{1}\"."),
                                    FText::FromString(NameStr), FText::FromString(IConsoleManager::Get().FindConsoleObjectName(ExistingConsoleObj))
                                )
                            );
                }

                // Check every exposed cheat function has description for proper tooltip generation
                if (Settings->ShouldValidateMissingDescription())
				{
					const FString& CommandDescription = Func->GetMetaData(TEXT("Tooltip"));
		
					if (CommandDescription.IsEmpty())
					{
						Context.AddWarning(
							FText::Format(
								LOCTEXT("Warning_MissingCheatDescription", "Cheat function \"{0}\" has no description! Please consider adding it, for console tooltip!"),
								FText::FromString(NameStr)
							)
						);
					}
				}
			}
		}
	}

	return ValidationResult;
}

TArray<FName> UCowCheatManager::GetAllFunctionFromClass(UClass* InClass, const UClass* InclusiveStopClass)
{
	TArray<FName> FunctionNames;
	UClass* CurrentClass = InClass;
	do
	{
		TArray<FName> TempFunctionNames;
		CurrentClass->GenerateFunctionList(TempFunctionNames);
		FunctionNames.Append(MoveTemp(TempFunctionNames));
		CurrentClass = CurrentClass->GetSuperClass();
	}
	while (CurrentClass != InclusiveStopClass);
	return FunctionNames;
}
#endif


void UCowCheatManager::Serialize(FArchive& Ar)
{
#if WITH_EDITOR
	if (Ar.IsCooking())
	{
		CachedAutoCompleteCommands.Empty();

		const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();
		TArray<FAutoCompleteCommand> Commands = GenerateAutoCompleteCommands(GetClass(), UCowCheatManager::StaticClass(), Settings->CheatCategoryPrefix, Settings->MainCheatColor);
		for (FAutoCompleteCommand& Command : Commands)
		{
			CachedAutoCompleteCommands.Add(FCachedAutoCompleteCommand(Command));
		}
	}
#endif
	Super::Serialize(Ar);
}

void UCowCheatManager::CowAddCheatManagerExtension(UCowCheatManagerExtension* CheatObject)
{
	if (!IsValid(CheatObject))
	{
		return;
	}
	UE_LOG(LogCowCheatManager, Log, TEXT("Added cheat extension %s"), *GetNameSafe(CheatObject->GetClass()));
	AddCheatManagerExtension(CheatObject);
}

void UCowCheatManager::CowRemoveCheatManagerExtension(UCowCheatManagerExtension* CheatObject)
{
	if (!IsValid(CheatObject))
    {
    	return;
    }
	UE_LOG(LogCowCheatManager, Log, TEXT("Removed cheat extension %s"), *GetNameSafe(CheatObject->GetClass()));
	RemoveCheatManagerExtension(CheatObject);
}

void UCowCheatManager::AddCheatManagerExtensionClass(TSubclassOf<UCowCheatManagerExtension> CheatClass)
{
	if (!IsValid(CheatClass))
	{
		return;
	}
	UCowCheatManagerExtension* NewExtension = NewObject<UCowCheatManagerExtension>(this, CheatClass);
	CowAddCheatManagerExtension(NewExtension);
}

void UCowCheatManager::RemoveCheatManagerExtensionClass(TSubclassOf<UCowCheatManagerExtension> CheatClass)
{
	if (!IsValid(CheatClass))
	{
		return;
	}
	
	int32 CheatExtensionIdx = CheatManagerExtensions.IndexOfByPredicate([CheatClass](const TObjectPtr<UCheatManagerExtension>& Item)
	{
		return Item && Item->GetClass() == CheatClass;
	});
	if (CheatExtensionIdx != INDEX_NONE)
	{
		CowRemoveCheatManagerExtension(Cast<UCowCheatManagerExtension>(CheatManagerExtensions[CheatExtensionIdx]));
	}
}

void UCowCheatManager::AsyncAddCheatManagerExtensionSoftClass(TSoftClassPtr<UCowCheatManagerExtension> SoftCheatClass)
{
    TSubclassOf<UCowCheatManagerExtension> LoadedClass = SoftCheatClass.Get();
    if (LoadedClass)
    {
        AddCheatManagerExtensionClass(LoadedClass);
    }
    else if (!SoftCheatClass.IsNull())
    {
        auto Delegate = FStreamableDelegate::CreateUObject(this, &UCowCheatManager::AsyncAddCheatManagerExtensionSoftClass, SoftCheatClass);

        UAssetManager::Get().GetStreamableManager().RequestAsyncLoad({SoftCheatClass.ToSoftObjectPath()}, Delegate);
    }
}

void UCowCheatManager::RemoveCheatManagerExtensionSoftClass(TSoftClassPtr<UCowCheatManagerExtension> CheatClass)
{
    // If our class isn't loaded it couldn't be among existing extensions and RemoveCheatManagerExtensionClass will early return
    RemoveCheatManagerExtensionClass(CheatClass.Get());
}

UCheatManagerExtension* UCowCheatManager::BP_FindCheatManagerExtension(const UClass* InClass) const
{
	return FindCheatManagerExtension(InClass);
}

void UCowCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

	if ( NumAutoCompletesRegistered <= 0 )
	{
		UE_LOG( LogCowCheatManager, Verbose, TEXT( "Registering cheat manager for autocomplete callbacks: %s" ), *GetName() );
		UConsole::RegisterConsoleAutoCompleteEntries.AddUObject( this, &UCowCheatManager::RegisterAutoCompleteEntries );
		NumAutoCompletesRegistered += 1;
	}
	else
	{
		UE_LOG( LogCowCheatManager, Verbose, TEXT( "Didn't need to register cheat manager for autocomplete callbacks: %s" ), *GetName() );
	}
}

void UCowCheatManager::BeginDestroy()
{
	const int32 NumRemoved = UConsole::RegisterConsoleAutoCompleteEntries.RemoveAll( this );
	NumAutoCompletesRegistered -= NumRemoved;
	Super::BeginDestroy();
}

bool UCowCheatManager::ProcessConsoleExec( const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor )
{
	const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();
	FString OriginalCmd = Cmd;

	FString CommandName;
	if (FParse::Token(Cmd, CommandName, true) && CommandName.StartsWith(Settings->CheatCategoryPrefix))
	{
        TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*FString::Printf(TEXT("UCowCheatManager::ProcessConsoleExec %s"), *CommandName));
		TArray<FString> FuncCategories;
		CommandName.ParseIntoArray(FuncCategories, TEXT("."), false);

		// Make sure Cheat is under correct category prefix!
		if (!IsArrayEmpty(FuncCategories) && FuncCategories[0] == Settings->CheatCategoryPrefix)
		{
			// Cmd will now just be the arguments since we parsed the command name out of it
			const FString CmdString = FuncCategories.Last() + " " + Cmd;
			bool bExecuted = CallFunctionByNameWithArguments(*CmdString, Ar, Executor, /* bForceCallWithNonExec */ true);
			if (bExecuted)
			{
				return true;
			}
			
			for (auto& Extension : CheatManagerExtensions)
			{
				bExecuted = Extension->CallFunctionByNameWithArguments(*CmdString, Ar, Executor, /* bForceCallWithNonExec */ true);
				if (bExecuted)
				{
					return true;
				}
			}
		}
	}

	// Call function that are marked FUNC_Exec
	return CallFunctionByNameWithArguments( *OriginalCmd, Ar, Executor );
}

void UCowCheatManager::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand>& Commands) const
{
#if WITH_EDITOR
	// Always generate new commands while in editor, we dont bother to properly generate and propagate changes inside engine compile routine
	const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();

	// Gather command from CheatManager
	{
		TArray<FAutoCompleteCommand> NewCommands = GenerateAutoCompleteCommands(GetClass(), UCowCheatManager::StaticClass(), Settings->CheatCategoryPrefix, Settings->MainCheatColor);
		Commands.Append(MoveTemp(NewCommands));
	}

	// Gather commands from extensions
	for (auto& Extension : CheatManagerExtensions)
	{
		if (auto CowCheatExtension = Cast<UCowCheatManagerExtension>(Extension))
		{
			const FColor& CheatColor = Settings->bForceCheatExtensionsUseMainColor ? Settings->MainCheatColor : CowCheatExtension->OverrideCheatColor;
			TArray<FAutoCompleteCommand> NewCommands = GenerateAutoCompleteCommands(CowCheatExtension->GetClass(), UCowCheatManagerExtension::StaticClass(), Settings->CheatCategoryPrefix, CheatColor);
			Commands.Append(MoveTemp(NewCommands));
		}
	}
#else
	UCowCheatManager* CDO = GetClass()->GetDefaultObject<UCowCheatManager>();
	for (const FCachedAutoCompleteCommand& CachedCommand : CDO->CachedAutoCompleteCommands)
	{
		Commands.Add(static_cast<FAutoCompleteCommand>(CachedCommand));
	}

	// Gather command from active extensions
	for (auto& Extension : CheatManagerExtensions)
	{
		if (auto CowCheatExtension = Cast<UCowCheatManagerExtension>(Extension))
		{
			auto ExtensionCDO = CowCheatExtension->GetClass()->GetDefaultObject<UCowCheatManagerExtension>();
			for (const FCachedAutoCompleteCommand& CachedCommand : ExtensionCDO->CachedAutoCompleteCommands)
			{
				Commands.Add(static_cast<FAutoCompleteCommand>(CachedCommand));
			}
		}
	}
#endif
}

#if WITH_EDITOR
EDataValidationResult UCowCheatManagerExtension::IsDataValid(FDataValidationContext& Context) const
{
	// See UCowCheatManager::IsDataValid for more implementation details and general description
	
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);
	TArray<FName> FunctionNames = UCowCheatManager::GetAllFunctionFromClass(GetClass(), UCowCheatManagerExtension::StaticClass());
	
	ValidationResult = CombineDataValidationResults(UCowCheatManager::ValidateCheatFunctions(FunctionNames, GetClass(), Context), ValidationResult);
		
	return ValidationResult;
}
#endif

void UCowCheatManagerExtension::AddedToCheatManager_Implementation()
{
	InvalidateConsoleAutocomplete();
	Super::AddedToCheatManager_Implementation();
}

void UCowCheatManagerExtension::RemovedFromCheatManager_Implementation()
{
	InvalidateConsoleAutocomplete();
	Super::RemovedFromCheatManager_Implementation();
}

void UCowCheatManagerExtension::Serialize(FArchive& Ar)
{
#if WITH_EDITOR
	if (Ar.IsCooking())
	{
		CachedAutoCompleteCommands.Empty();

		const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();
		const FColor& CheatColor = Settings->bForceCheatExtensionsUseMainColor ? Settings->MainCheatColor : OverrideCheatColor;
		TArray<FAutoCompleteCommand> Commands = UCowCheatManager::GenerateAutoCompleteCommands(GetClass(), UCowCheatManagerExtension::StaticClass(), Settings->CheatCategoryPrefix, CheatColor);
		for (FAutoCompleteCommand& Command : Commands)
		{
			CachedAutoCompleteCommands.Add(FCachedAutoCompleteCommand(Command));
		}
	}
#endif
	Super::Serialize(Ar);
}

#undef LOCTEXT_NAMESPACE