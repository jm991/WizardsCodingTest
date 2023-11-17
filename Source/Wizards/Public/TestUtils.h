// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#pragma once

#include "Engine.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

class FTestUtils
{
public:

	/** The flag used when defining tests. */
	static const int32 TestFlags =
		EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::ClientContext
		| EAutomationTestFlags::ProductFilter;

	/** Helper method for getting the game world. */
	static UWorld* GetWorld()
	{
		if (GEngine)
		{
			if (FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0))
			{
				return WorldContext->World();
			}
		}

		return nullptr;
	}

	/** Helper method for exiting the game. */
	static void Exit()
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(World, 0))
			{
				TargetPC->ConsoleCommand(TEXT("Exit"), true);
			}
		}
	}
};

