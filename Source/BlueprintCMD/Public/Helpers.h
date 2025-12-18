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
#include "ConsoleSettings.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
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
	FColor Color = FColor::Cyan;
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
