// Copyright 2025, Oleksandr 'sleepCOW' Ozerov

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"

class FCowCheatManagerEditorModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnNewCheatManagerCreated(class UBlueprint* InBlueprint);

private:
	TSharedPtr<FSlateStyleSet> CheatManagerStyle;
};
