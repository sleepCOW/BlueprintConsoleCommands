// MIT License

#pragma once

#include "CoreMinimal.h"
#include "Helpers.generated.h"

USTRUCT()
struct FCachedAutoCompleteCommand
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString Command;
	UPROPERTY()
	FString Desc;
};