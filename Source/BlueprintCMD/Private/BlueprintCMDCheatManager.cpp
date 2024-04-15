// MIT License


#include "BlueprintCMDCheatManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCheatManager, Log, All);

static int32 NumAutoCompletesRegistered = 0;

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
	
	if (CheatTooltip.IsEmpty())
	{
		UE_LOG(LogCheatManager, Warning, TEXT("Cheat function %s has no description! Please consider adding it!"), *Func->GetName());
		Desc += "Please, provide description in function details panel!";
	}

	return Desc;
}

TArray<FAutoCompleteCommand> UBlueprintCMDCheatManager::GenerateAutoCompleteCommands() const
{
	TArray<FAutoCompleteCommand> ReturnValue;

	// Find any functions with the Cheat meta (haven't bothered doing any duplicate detection or other validation)
	TArray<FName> FunctionNames;
	GetClass()->GenerateFunctionList(FunctionNames);
	for (const FName& Name : FunctionNames)
	{
		const UFunction* Func = GetClass()->FindFunctionByName(Name);

		FString Category = Func->GetMetaData(TEXT("Category"));
		TArray<FString> FuncCategories;
		Category.ParseIntoArray(FuncCategories, TEXT("|"), true);

		// Make sure Cheat is under correct category prefix!
		if (FuncCategories.IsEmpty() || FuncCategories[0] != CheatCategoryPrefix)
		{
			continue;
		}

		// Append to the categories command name
		// So it will look like "CheatCategoryPrefix.Category.FuncName"
		FuncCategories.Add(Name.ToString());
		const FString CommandName = FString::Join(FuncCategories, TEXT("."));

		FAutoCompleteCommand Entry;

		Entry.Command = CommandName;
		Entry.Desc = BuildCommandDescription(Func);
		Entry.Color = CheatColor;
		
		ReturnValue.Add(Entry);
	}
	return ReturnValue;
}
#endif


void UBlueprintCMDCheatManager::Serialize(FArchive& Ar)
{
#if WITH_EDITOR
	if (Ar.IsSaving())
	{
		CachedAutoCompleteCommands.Empty();

		TArray<FAutoCompleteCommand> Commands = GenerateAutoCompleteCommands();
		for (FAutoCompleteCommand& Command : Commands)
		{
			FCachedAutoCompleteCommand SavedCommand;
			SavedCommand.Command = Command.Command;
			SavedCommand.Desc = Command.Desc;
			CachedAutoCompleteCommands.Add(SavedCommand);
		}
	}
#endif
	Super::Serialize(Ar);
}

void UBlueprintCMDCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

	if ( NumAutoCompletesRegistered <= 0 )
	{
		UE_LOG( LogCheatManager, Verbose, TEXT( "Registering cheat manager for autocomplete callbacks: %s" ), *GetName() );
		UConsole::RegisterConsoleAutoCompleteEntries.AddUObject( this, &UBlueprintCMDCheatManager::RegisterAutoCompleteEntries );
		NumAutoCompletesRegistered += 1;
	}
	else
	{
		UE_LOG( LogCheatManager, Verbose, TEXT( "Didn't need to register cheat manager for autocomplete callbacks: %s" ), *GetName() );
	}
}

void UBlueprintCMDCheatManager::BeginDestroy()
{
	const int32 NumRemoved = UConsole::RegisterConsoleAutoCompleteEntries.RemoveAll( this );
	NumAutoCompletesRegistered -= NumRemoved;
	Super::BeginDestroy();
}

bool UBlueprintCMDCheatManager::ProcessConsoleExec( const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor )
{
	FString OriginalCmd = Cmd;

	FString CommandName;
	if (FParse::Token(Cmd, CommandName, true) && CommandName.StartsWith(CheatCategoryPrefix))
	{
		TArray<FString> FuncCategories;
		CommandName.ParseIntoArray(FuncCategories, TEXT("."), false);

		// Make sure Cheat is under correct category prefix!
		if (!FuncCategories.IsEmpty() && FuncCategories[0] == CheatCategoryPrefix)
		{
			// Cmd will now just be the arguments since we parsed the command name out of it
			const FString CmdString = FuncCategories.Last() + Cmd;
			return CallFunctionByNameWithArguments(*CmdString, Ar, Executor, /* bForceCallWithNonExec */ true);
		}
	}

	// Call function that are marked FUNC_Exec
	return CallFunctionByNameWithArguments( *OriginalCmd, Ar, Executor );
}

void UBlueprintCMDCheatManager::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand>& Commands) const
{
#if WITH_EDITOR
	// Always generate new commands while in editor, we dont bother to properly generate and propagate changes inside engine compile routine 
	const TArray<FAutoCompleteCommand> NewCommands = GenerateAutoCompleteCommands();
	Commands.Append(NewCommands);
#else
	UBlueprintCMDCheatManager* CDO = GetClass()->GetDefaultObject<UBlueprintCMDCheatManager>();
	for (int32 i = 0; i < CDO->CachedAutoCompleteCommands.Num(); ++i)
	{
		FAutoCompleteCommand AsEngineCommand;
		AsEngineCommand.Command = CDO->CachedAutoCompleteCommands[i].Command;
		
		AsEngineCommand.Desc = CDO->CachedAutoCompleteCommands[i].Desc;
		AsEngineCommand.Color = CDO->CheatColor;
		Commands.Add(AsEngineCommand);
	}
#endif
}
