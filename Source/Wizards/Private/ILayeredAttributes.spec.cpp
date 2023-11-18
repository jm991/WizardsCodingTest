// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#include "Algo/Accumulate.h"
#include "Algo/ForEach.h"
#include "Algo/Transform.h"

#include "TestUtils.h"
#include "ILayeredAttributes.h"
#include "LayeredEffectDefinition.h"
#include "WizardsCharacter.h"

/// <summary>
/// Small struct that can be added to a TArray to test layering effects.
/// </summary>
struct LayeredEffectTest
{
	EEffectOperation Operation;
	int32 Modification;
	int32 ExpectedValue;
};

BEGIN_DEFINE_SPEC(AttributeTest, "Wizards.LayeredAttributes", FTestUtils::TestFlags)

UWorld* World = nullptr;
AWizardsCharacter* MyCharacter = nullptr;
TArray<EAttributeKey> AllAttributes = { };
TArray<EEffectOperation> AllOperations = { };

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
			if (MyCharacter == nullptr || !MyCharacter->IsValidLowLevel())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				MyCharacter = World->SpawnActor<AWizardsCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			}
		}

		// Collect all the attributes
		AllAttributes = UStaticBlueprintLibrary::GetEnumEntries<EAttributeKey>();
		AllOperations = UStaticBlueprintLibrary::GetEnumEntries<EEffectOperation>(static_cast<EEffectOperation>(1)); // skip the EEffectOperation::Invalid

		TestNotNull("Check if World is properly created", World);
		TestNotNull("Check if MyCharacter is properly created", MyCharacter);
		TestTrue("Check if MyCharacter memory is valid", MyCharacter->IsValidLowLevel());
		TestEqual("Check if attributes were populated", AllAttributes.Num(), UStaticBlueprintLibrary::GetEnumNumEntries<EAttributeKey>());
	});

	AfterEach([this]()
	{
		// Cleanup
		MyCharacter->Destroy();
		MyCharacter = nullptr;

		// Shut down the game
		FTestUtils::Exit();
	});

	Describe("LayeredAttributes", [this]()
	{
		It("Can initialize base attributes", [this]()
		{
			for (int32 i = 0; i < AllAttributes.Num(); i++)
			{
				TestEqual("All base values default to 0 until set", MyCharacter->GetCurrentAttribute(AllAttributes[i]), 0);
				MyCharacter->SetBaseAttribute(AllAttributes[i], i);
				TestEqual("Get the base value for an attribute", MyCharacter->GetBaseAttribute(AllAttributes[i]), i);
				TestEqual("Setting the base value for an attribute also adjusts the current attribute", MyCharacter->GetCurrentAttribute(AllAttributes[i]), i);
				TestEqual("Current and base attributes should be equal if no layered effects are active", MyCharacter->GetCurrentAttribute(AllAttributes[i]), MyCharacter->GetBaseAttribute(AllAttributes[i]));
			}
		});

		It("Invalid effects should not be applied", [this]()
		{
			{
				const FLayeredEffectDefinition InvalidAttributeEffect = FLayeredEffectDefinition(EAttributeKey::Invalid, EEffectOperation::Add, 0, 0);
				bool bInvalidAttributeEffectApplied = false;
				MyCharacter->AddLayeredEffect(InvalidAttributeEffect, bInvalidAttributeEffectApplied);
				TestFalse("Invalid attribute effect not applied", bInvalidAttributeEffectApplied);
			}

			{
				const FLayeredEffectDefinition InvalidOperationEffect = FLayeredEffectDefinition(EAttributeKey::Power, EEffectOperation::Invalid, 0, 0);
				bool bInvalidOperationEffectApplied = false;
				MyCharacter->AddLayeredEffect(InvalidOperationEffect, bInvalidOperationEffectApplied);
				TestFalse("Invalid operation effect not applied", bInvalidOperationEffectApplied);
			}
		});

		It("Changes to base attributes when layered effects are active should adjust current attribute", [this]()
		{
			const EAttributeKey Attribute = EAttributeKey::Power;
			const int32 BaseValueA = 1;
			const int32 BaseValueB = 10;
			const int32 AddAmt = 5;
			const int32 MultiplyAmt = 2;
			TArray<LayeredEffectTest> OrderedOperations =
			{
				LayeredEffectTest{ EEffectOperation::Add , AddAmt, (BaseValueA + AddAmt) },
				LayeredEffectTest{ EEffectOperation::Multiply , MultiplyAmt, ((BaseValueA + AddAmt) * MultiplyAmt) },
			};

			TestEqual("Base attributes start at 0", MyCharacter->GetCurrentAttribute(Attribute), 0);
			MyCharacter->SetBaseAttribute(Attribute, BaseValueA);
			TestEqual("First base attribute initialized", MyCharacter->GetCurrentAttribute(Attribute), BaseValueA);

			for (int32 Layer = 0; Layer < OrderedOperations.Num(); Layer++)
			{
				const LayeredEffectTest& CurTest = OrderedOperations[Layer];
				bool bSuccess = false;
				MyCharacter->AddLayeredEffect(FLayeredEffectDefinition(Attribute, CurTest.Operation, CurTest.Modification, Layer), bSuccess);
				TestTrue("Current layered effect was successfully applied", bSuccess);
				TestEqual("Layered effect value was correct", MyCharacter->GetCurrentAttribute(Attribute), CurTest.ExpectedValue);
			}

			MyCharacter->SetBaseAttribute(Attribute, BaseValueB);
			TestEqual("Changing to second base attribute modifies current value", MyCharacter->GetCurrentAttribute(Attribute), ((BaseValueB + AddAmt) * MultiplyAmt));
		});

		It("Removing a layered effect properly adjust current value", [this]()
		{
			const EAttributeKey Attribute = EAttributeKey::Power;
			const int32 BaseValue = 4;
			const int32 AddAmt = 5;
			const int32 SubtractAmt = 2;
			const int32 MultiplyAmt = 10;
			TArray<LayeredEffectTest> OrderedOperations =
			{
				LayeredEffectTest{ EEffectOperation::Add , AddAmt, (BaseValue + AddAmt) },
				LayeredEffectTest{ EEffectOperation::Subtract , SubtractAmt, ((BaseValue + AddAmt) - SubtractAmt) },
				LayeredEffectTest{ EEffectOperation::Multiply , MultiplyAmt, (((BaseValue + AddAmt) - SubtractAmt) * MultiplyAmt) },
			};
			TArray<FActiveEffectHandle> OrderedEffectHandles = { };

			MyCharacter->SetBaseAttribute(Attribute, BaseValue);
			TestEqual("First base attribute initialized", MyCharacter->GetCurrentAttribute(Attribute), BaseValue);

			for (int32 Layer = 0; Layer < OrderedOperations.Num(); Layer++)
			{
				const LayeredEffectTest& CurTest = OrderedOperations[Layer];
				bool bSuccess = false;
				OrderedEffectHandles.Add(MyCharacter->AddLayeredEffect(FLayeredEffectDefinition(Attribute, CurTest.Operation, CurTest.Modification, Layer), bSuccess));
				TestTrue("Current layered effect was successfully applied", bSuccess);
				TestEqual("Layered effect value was correct", MyCharacter->GetCurrentAttribute(Attribute), CurTest.ExpectedValue);
			}

			const int32 SubtractiveEffectIndex = 1;
			TestTrue("Subtractive effect handle exists", OrderedEffectHandles.IsValidIndex(SubtractiveEffectIndex));
			TestTrue("Subtractive effect was removed", MyCharacter->RemoveLayeredEffect(OrderedEffectHandles[SubtractiveEffectIndex]));
			TestEqual("Layered effect value was updated after subtractive effect was removed", MyCharacter->GetCurrentAttribute(Attribute), ((BaseValue + AddAmt) * MultiplyAmt));
		});

		It("Testing evaluation of EEffectOperation", [this]()
		{
			const int32 BaseValue = 1;
			const int32 Modification = 256;

			TArray<TPair<LayeredEffectTest, int32>> TestOperationsAndExpectedValues =
			{
				MakeTuple(LayeredEffectTest{ EEffectOperation::Set, Modification, Modification }, 256),
				MakeTuple(LayeredEffectTest{ EEffectOperation::Add, Modification, (BaseValue + Modification) }, 257),
				MakeTuple(LayeredEffectTest{ EEffectOperation::Subtract, Modification, (BaseValue - Modification) }, -255),
				MakeTuple(LayeredEffectTest{ EEffectOperation::Multiply, Modification, (BaseValue * Modification) }, 256),
				MakeTuple(LayeredEffectTest{ EEffectOperation::BitwiseOr, Modification, (BaseValue | Modification) }, 257),
				MakeTuple(LayeredEffectTest{ EEffectOperation::BitwiseAnd, Modification, (BaseValue & Modification) }, 0),
				MakeTuple(LayeredEffectTest{ EEffectOperation::BitwiseXor, Modification, (BaseValue ^ Modification) }, 257),
			};

			for (const TPair<LayeredEffectTest, int32>& CurTestOperationAndExpectedValue : TestOperationsAndExpectedValues)
			{
				const int32 CurActualValue = FSortedEffectDefinitions::Evaluate(BaseValue, CurTestOperationAndExpectedValue.Key.Modification, CurTestOperationAndExpectedValue.Key.Operation);
				TestEqual("Evaluated value matches expected hand-calculated value", CurActualValue, CurTestOperationAndExpectedValue.Value);
				TestEqual("Evaluated value matches expected code-calculated value", CurActualValue, CurTestOperationAndExpectedValue.Key.ExpectedValue);
			}
		});

		It("Layered effects with the same layer get applied in the order they were added (timestamp order)", [this]()
		{
			const EAttributeKey Attribute = EAttributeKey::Power;
			const int32 BaseValue = -1;
			const int32 Layer = 1;
			const int32 AddAmt = 2;
			const int32 MultiplyAmt = -5;

			// These same-layer effects are intentionally authored so that their order of application matters
			const TArray<TPair<FLayeredEffectDefinition, int32>> LayeredEffectsAndExpectedValuesA =
			{
				//																				Mod				Layer
				MakeTuple(FLayeredEffectDefinition(Attribute,	EEffectOperation::Multiply,		MultiplyAmt,	Layer), (BaseValue * MultiplyAmt)),
				MakeTuple(FLayeredEffectDefinition(Attribute,	EEffectOperation::Add,			AddAmt,			Layer), ((BaseValue * MultiplyAmt) + AddAmt)),
			};
			const TArray<TPair<FLayeredEffectDefinition, int32>> LayeredEffectsAndExpectedValuesB =
			{
				//																				Mod				Layer
				MakeTuple(FLayeredEffectDefinition(Attribute,	EEffectOperation::Add,			AddAmt,			Layer), (BaseValue + AddAmt)),
				MakeTuple(FLayeredEffectDefinition(Attribute,	EEffectOperation::Multiply,		MultiplyAmt,	Layer), ((BaseValue + AddAmt) * MultiplyAmt)),
			};

			auto TestLayeredEffectsFn = [this, Attribute](const TArray<TPair<FLayeredEffectDefinition, int32>>& LayeredEffectsAndExpectedValues)
			{
				for (const TPair<FLayeredEffectDefinition, int32>& CurLayeredEffectAndExpectedVal : LayeredEffectsAndExpectedValues)
				{
					bool bSuccess = false;
					MyCharacter->AddLayeredEffect(CurLayeredEffectAndExpectedVal.Key, bSuccess);
					TestTrue("Current layered effect was successfully applied", bSuccess);
					const int32 CurActual = MyCharacter->GetCurrentAttribute(Attribute);
					const int32 CurExpected = CurLayeredEffectAndExpectedVal.Value;
					TestEqual("Layered effect value was correct", CurActual, CurExpected);
				}
			};

			MyCharacter->SetBaseAttribute(Attribute, BaseValue);
			TestEqual("First base attribute initialized", MyCharacter->GetCurrentAttribute(Attribute), BaseValue);

			TestLayeredEffectsFn(LayeredEffectsAndExpectedValuesA);
			MyCharacter->ClearLayeredEffects();
			TestLayeredEffectsFn(LayeredEffectsAndExpectedValuesB);
		});

		It("Smaller numbered layers get applied first - layered effects with the same layer get applied in the order that they were added (timestamp order)", [this]()
		{
			const EAttributeKey Attribute = EAttributeKey::Power;
			const int32 BaseValue = -15;

			// Test a complex layering of effects, to make sure operations are applied correctly and the effects are sorted correctly within the same layer
			const TArray<FLayeredEffectDefinition> LayeredEffects =
			{
				//																	Mod		Layer
				FLayeredEffectDefinition(Attribute,	EEffectOperation::Add,			2,		3),
				FLayeredEffectDefinition(Attribute,	EEffectOperation::Subtract,		-5,		0),
				FLayeredEffectDefinition(Attribute,	EEffectOperation::Multiply,		-2,		1),
				FLayeredEffectDefinition(Attribute,	EEffectOperation::BitwiseOr,	1,		4),
				FLayeredEffectDefinition(Attribute,	EEffectOperation::BitwiseAnd,	4,		2),
				FLayeredEffectDefinition(Attribute,	EEffectOperation::BitwiseXor,	8,		2),
				FLayeredEffectDefinition(Attribute,	EEffectOperation::Set,			3,		2),
			};

			TArray<FLayeredEffectDefinition> SortedLayerEffects;
			TArray<TPair<FLayeredEffectDefinition, int32>> LayeredEffectsAndExpectedValues;
			Algo::Transform(LayeredEffects, LayeredEffectsAndExpectedValues, [this, &SortedLayerEffects](const FLayeredEffectDefinition& CurLayeredEffect)
			{
				SortedLayerEffects.Add(CurLayeredEffect);

				// Sort lowest layers to the front of the array, must be StableSort since effects on the same layer are applied in the order they were added
				SortedLayerEffects.StableSort([](const FLayeredEffectDefinition& A, const FLayeredEffectDefinition& B) {
					return A.GetLayer() < B.GetLayer();
				});

				// Accumulate the layered effect expected value with the current stack of effects applied to BaseValue
				const int32 CurExpectedValue = Algo::Accumulate(SortedLayerEffects, BaseValue, [](int32 Accum, const FLayeredEffectDefinition& CurSortedLayeredEffect)
				{
					return FSortedEffectDefinitions::Evaluate(Accum, CurSortedLayeredEffect.GetModification(), CurSortedLayeredEffect.GetOperation());
				});

				return MakeTuple(CurLayeredEffect, CurExpectedValue);
			});

			MyCharacter->SetBaseAttribute(Attribute, BaseValue);
			TestEqual("First base attribute initialized", MyCharacter->GetCurrentAttribute(Attribute), BaseValue);

			for (const TPair<FLayeredEffectDefinition, int32>& CurLayeredEffectAndExpectedVal : LayeredEffectsAndExpectedValues)
			{
				bool bSuccess = false;
				MyCharacter->AddLayeredEffect(CurLayeredEffectAndExpectedVal.Key, bSuccess);
				TestTrue("Current layered effect was successfully applied", bSuccess);
				const int32 CurActual = MyCharacter->GetCurrentAttribute(Attribute);
				const int32 CurExpected = CurLayeredEffectAndExpectedVal.Value;
				TestEqual("Layered effect value was correct", CurActual, CurExpected);
			}
		});

		It("Clearing attributes removes all layered effects from this object - after this call, all current attributes will be equal to the base attributes", [this]()
		{
			for (int32 i = 0; i < AllAttributes.Num(); i++)
			{
				TestEqual("All base values default to 0 until set", MyCharacter->GetCurrentAttribute(AllAttributes[i]), 0);
				MyCharacter->SetBaseAttribute(AllAttributes[i], i);
				TestEqual("Set the base value for an attribute", MyCharacter->GetCurrentAttribute(AllAttributes[i]), i);
			}

			for (int32 i = 1; i < AllAttributes.Num(); i++)
			{
				// Skip the EAttributeKey::Invalid, since it can't be applied
				if (const EAttributeKey CurAttribute = AllAttributes[i];
					CurAttribute != EAttributeKey::Invalid)
				{
					bool bSuccess = false;
					FLayeredEffectDefinition CurLayeredEffect = FLayeredEffectDefinition(CurAttribute, EEffectOperation::Add, i, i);
					MyCharacter->AddLayeredEffect(CurLayeredEffect, bSuccess);
					TestTrue("Layered effect was successfully applied", bSuccess);
					TestEqual("Layered effect value", MyCharacter->GetCurrentAttribute(CurAttribute), (MyCharacter->GetBaseAttribute(CurAttribute) + i));
				}
			}

			MyCharacter->ClearLayeredEffects();

			for (int32 i = 0; i < AllAttributes.Num(); i++)
			{
				TestEqual("After clearing attributes, all current attributes should equal base attributes", MyCharacter->GetCurrentAttribute(AllAttributes[i]), MyCharacter->GetBaseAttribute(AllAttributes[i]));
			}
		});
	});
}