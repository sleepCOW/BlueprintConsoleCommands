// MIT License

#pragma once

#include "CoreMinimal.h"
#include "ConsoleSettings.h"
#include "Engine/Console.h"
#include "Helpers.generated.h"

USTRUCT()
struct FCachedAutoCompleteCommand
{
	GENERATED_BODY()

	FCachedAutoCompleteCommand() = default;
	
	FCachedAutoCompleteCommand(const FAutoCompleteCommand& InAutoComplete) :
		Command(InAutoComplete.Command),
		Desc(InAutoComplete.Desc),
		Color(InAutoComplete.Color)
	{}

	operator FAutoCompleteCommand() const
	{
		FAutoCompleteCommand Return;
		Return.Command = Command;
		Return.Desc = Desc;
		Return.Color = Color;
		return Return;
	}
	
	UPROPERTY()
	FString Command;
	UPROPERTY()
	FString Desc;
	UPROPERTY()
	FColor Color;
};

// Well, it invalidates console autocomplete
inline void InvalidateConsoleAutocomplete()
{
#if !UE_BUILD_SHIPPING
	if (UConsole* ViewportConsole = (GEngine->GameViewport != nullptr) ? GEngine->GameViewport->ViewportConsole : nullptr) 
	{
		ViewportConsole->InvalidateAutocomplete();
	}
#endif
}
