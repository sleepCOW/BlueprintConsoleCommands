#include "CowCheatManagerEditorModule.h"

#include "CowCheatManagerDeveloperSettings.h"
#include "EdGraphNode_Comment.h"
#include "BlueprintCMD/Public/CowCheatManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
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

UEdGraphNode_Comment* CreateCommentNode(class UEdGraph* ParentGraph, const FVector2D& Location, const FString& NodeComment, bool bSelectNewNode = false)
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(ParentGraph);
	
	UEdGraphNode_Comment* NewNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, Location, bSelectNewNode);
	NewNode->NodeComment = NodeComment;

	if (!NodeComment.IsEmpty())
	{
		// Set proper node size
		// and adjust spawn location
		TArray<FString> Lines;
		constexpr bool bCullEmpty = false;
		NodeComment.ParseIntoArray(Lines, TEXT("\n"), bCullEmpty);
		Algo::SortBy(Lines, [](const FString& Line)
		{
			return Line.Len();
		});

		constexpr double FontSize = 18. * 1.7;
		double Width = Lines.Last().Len() * FontSize * 0.5;
		double Height = Lines.Num() * FontSize;
		FVector2D TopLeft = Location; // Adjust only height because it will move the node on top the function entry
		TopLeft.Y -= Height;
		FSlateRect Bounds = FSlateRect::FromPointAndExtent(TopLeft, {Width, Height});
		NewNode->SetBounds(Bounds);
	}
	
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

	return NewNode;
}

#if 1
UEdGraph* AddFunction(UBlueprint* BlueprintObj, FName FunctionName, const FString& Category = FString(""))
{
	check(BlueprintObj && BlueprintObj->SkeletonGeneratedClass);
	
	// Implement the function graph
	UEdGraph* const NewGraph = FBlueprintEditorUtils::CreateNewGraph(BlueprintObj, FunctionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	NewGraph->Modify();
	
	FBlueprintEditorUtils::AddFunctionGraph<UClass>(BlueprintObj, NewGraph, true, nullptr);

	if (!Category.IsEmpty())
	{
		FBlueprintEditorUtils::SetBlueprintFunctionOrMacroCategory(NewGraph, FText::FromString(Category));	
	}

	return NewGraph;
}
#endif

void FCowCheatManagerEditorModule::OnNewCheatManagerCreated(class UBlueprint* InBlueprint)
{
	// I'm naturally suspicious and LOVE to bloat CPU branch predictor cache
	if (!InBlueprint)
	{
		return;
	}
	
	const UCowCheatManagerDeveloperSettings* Settings = GetDefault<UCowCheatManagerDeveloperSettings>();
	if (!Settings || !Settings->bCreateDefaultConsoleCommands)
	{
		return;
	}

	if (!InBlueprint->UbergraphPages.IsEmpty())
	{
		FString NodeComment =
			"Hello, thank you for giving this plugin a go, I really appreciate it!\n\n"
			"Getting started:\n"
		    "  1. Go to your PlayerController and selected your newly created " + GetNameSafe(InBlueprint) + " under CheatClass\n"
		    "  2. To create a new console command you simply need to create a function, see \"ExampleCheatCommand\", it contains another helpful comment\n"
		    "  3. That's it, hit play, test, iterate, enjoy :)\n\n"
			"NOTE: You can disable those hints in new CowCheatManager/CowCheatManagerExtension by toggling bCreateDefaultConsoleCommands in Project Settings -> Editor -> Blueprint Console Commands Settings\n"
			"#TODO: Write about other settings\n\n"
		    "  If you like the plugin I will appreciate if you buy it on FAB but no pressure we all have been through tough times and sometimes it's not an option, I understand :)";
		CreateCommentNode(InBlueprint->UbergraphPages[0], {1920 / 2, 1080 / 2}, NodeComment,true);
	}

	// #TODO: Add PrintString "Hello world!"
	// #TODO: Add Duration as parameter to ConsoleCommand with default value
	if (auto* ExampleFuncGraph = AddFunction(InBlueprint, TEXT("ExampleCheatCommand"), Settings->CheatCategoryPrefix))
	{
		FString NodeComment =
			"Regular blueprint function as this one is all you need to use it as console command!\n"
		    "Just hook anything you want to run and that's it :)\n\n"
		    "However, few things to note:\n"
		    "  1. For function to be treated as console command it must be under correct category defined by \"CheatCategoryPrefix\" in \"Blueprint Console Commands Settings\", currently \"" + Settings->CheatCategoryPrefix + "\"\n"
		    "       I highly recommend to rename it to your project name, so it's clear for all your team members that's your cheat commands, project specific\n"
		    "  2. Function could be under nested Categories, see \"NestedCategoryExample\", it should be used for clarity but you can omit it if you want to.\n"
		    "       To use nested categories use \"|\", e.x. \"Cheat|Gameplay|Weapons\" it will be translated into \"Cheat.Gameplay.Weapons\" in console\n"
		    "  3. If your cheat command requires some latent actions like \"Delay\" or any Async node, you should to create an CustomEvent in EventGraph and just call it from your Function\n"
		    "  4. Consider adding \"Description\" for cheat command, it will be displayed in console autocomplete and will help people to understand what your command does\n"
		    "	    If you hate generated warnings for missing descriptions you can toggle \"bDisableMissingDescriptionWarnings\" in  \"Blueprint Console Commands Settings\"\n"
		    "  5. You can pass arguments to Commands, just add arguments to the function they will be automatically displayed in autocomplete";
		CreateCommentNode(ExampleFuncGraph, {-50, -50}, NodeComment,true);
	}
	AddFunction(InBlueprint, TEXT("NestedCategoryExample"), Settings->CheatCategoryPrefix + FString("|Gameplay"));
#if 0
	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(this, UEdGraphSchema_K2::GN_EventGraph, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	NewGraph->bAllowDeletion = false;

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(this);
	FBlueprintEditorUtils::AddUbergraphPage(this, NewGraph);
	LastEditedDocuments.AddUnique(NewGraph);
#endif

	// TODO: Add a default cheat function example and nested category function example
}
