// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#include "Algo/ForEach.h"

#include "TestUtils.h"
#include "ILayeredAttributes.h"
#include "LayeredEffectDefinition.h"
#include "WizardsCharacter.h"

BEGIN_DEFINE_SPEC(AttributeTest, "Wizards.LayeredAttributes", FTestUtils::TestFlags)

UWorld* World = nullptr;
AWizardsCharacter* MyCharacter;
TArray<EAttributeKey> AllAttributes;

END_DEFINE_SPEC(AttributeTest)

void AttributeTest::Define()
{
	BeforeEach([this]()
	{
		// Open up the map to test in
		AutomationOpenMap("/Game/TopDown/Maps/TopDownMap");
		World = FTestUtils::GetWorld();

		// Spawn Character in the world
		if (World != nullptr)
		{
			if (!IsValid(MyCharacter))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				MyCharacter = World->SpawnActor<AWizardsCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			}
		}

		// Collect all the attributes
		AllAttributes = UStaticBlueprintLibrary::GetEnumEntries<EAttributeKey>();

		TestNotNull("Check if World is properly created", World);
		TestNotNull("Check if MyCharacter is properly created", MyCharacter);
		TestEqual("Check if attributes were populated", AllAttributes.Num(), UStaticBlueprintLibrary::GetEnumNumEntries<EAttributeKey>());
	});

	AfterEach([this]()
	{
		// Cleanup
		MyCharacter->Destroy();

		// Shut down the game
		FTestUtils::Exit();
	});

	Describe("LayeredAttributes", [this]()
	{
		It("Should initialize base attributes", [this]()
		{
			for (const EAttributeKey CurAttribute : AllAttributes)
			{
				TestEqual("All base values default to 0 until set", MyCharacter->GetCurrentAttribute(CurAttribute), 0);
			}

			for (int32 i = 0; i < AllAttributes.Num(); i++)
			{
				MyCharacter->SetBaseAttribute(AllAttributes[i], i);
			}

			for (int32 i = 0; i < AllAttributes.Num(); i++)
			{
				TestEqual("Set the base value for an attribute.", MyCharacter->GetCurrentAttribute(AllAttributes[i]), i);
			}
		});

		xIt("Changes to base attributes when layered effects are active should adjust current attribute", [this]()
		{
			TestEqual("Base attributes start at 0", MyCharacter->GetCurrentAttribute(EAttributeKey::Power), 0);

			MyCharacter->SetBaseAttribute(EAttributeKey::Power, 1);

			TestEqual("Base attribute initialized", MyCharacter->GetCurrentAttribute(EAttributeKey::Power), 1);

		});
	});
}