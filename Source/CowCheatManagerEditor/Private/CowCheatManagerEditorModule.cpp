#include "CowCheatManagerEditorModule.h"

#include "BlueprintCMD/Public/CowCheatManager.h"
#include "Kismet2/KismetEditorUtilities.h"

IMPLEMENT_MODULE(FCowCheatManagerEditorModule, CowCheatManagerEditor);

void FCowCheatManagerEditorModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();

	FKismetEditorUtilities::RegisterOnBlueprintCreatedCallback(this, UCowCheatManager::StaticClass(),
				FKismetEditorUtilities::FOnBlueprintCreated::CreateRaw(this, &FCowCheatManagerEditorModule::OnNewCheatManagerCreated));

	FKismetEditorUtilities::RegisterOnBlueprintCreatedCallback(this, UCowCheatManagerExtension::StaticClass(),
				FKismetEditorUtilities::FOnBlueprintCreated::CreateRaw(this, &FCowCheatManagerEditorModule::OnNewCheatManagerCreated));
}

void FCowCheatManagerEditorModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
	
	FKismetEditorUtilities::UnregisterAutoBlueprintNodeCreation(this);
}

void FCowCheatManagerEditorModule::OnNewCheatManagerCreated(class UBlueprint* InBlueprint)
{
	// I'm naturally suspicious and LOVE to bloat CPU branch predictor cache
	if (!InBlueprint)
	{
		return;
	}

	// TODO: Add a default cheat function example and nested category function example
}
