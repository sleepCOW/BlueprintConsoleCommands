#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCowCheatManagerEditorModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnNewCheatManagerCreated(class UBlueprint* InBlueprint);
};
