// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#include "LayeredEffectDefinition.h"

#include "Kismet\KismetMathLibrary.h"

#include "ILayeredAttributes.h"

DEFINE_LOG_CATEGORY(LogLayeredEffects);

#if WITH_EDITOR
void ULayeredEffectDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Synchronize the Modification property with the helper properties
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULayeredEffectDefinition, Modification))
	{
		ModificationColor = UStaticBlueprintLibrary::Conv_IntToColor(Modification);
		ModificationTypes = Modification;
		ModificationSubtypes = Modification;
		ModificationSupertypes = Modification;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULayeredEffectDefinition, ModificationColor))
	{
		Modification = UStaticBlueprintLibrary::Conv_ColorToInt(ModificationColor);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULayeredEffectDefinition, ModificationTypes))
	{
		Modification = ModificationTypes;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULayeredEffectDefinition, ModificationSubtypes))
	{
		Modification = ModificationSubtypes;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULayeredEffectDefinition, ModificationSupertypes))
	{
		Modification = ModificationSupertypes;
	}

	// Call the base class version
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void FSortedEffectDefinitions::AddLayeredEffect(const UWorld* World, const ULayeredEffectDefinition* Effect)
{
	const FActiveEffectDefinition NewEffect = FActiveEffectDefinition(World, Effect);
	if (World == nullptr || Effect == nullptr || !NewEffect.IsValid())
	{
		UE_LOG(LogLayeredEffects, Error, TEXT("Invalid effect '%s'"), *GetNameSafe(Effect));
		return;
	}

	const int32 NewEffectLayer = NewEffect.GetEffectDefinition()->GetLayer();
	const float NewEffectStartTime = NewEffect.GetStartTime();

	int32 IndexToInsert = 0;
	for (const FActiveEffectDefinition& CurEffect : SortedEffects)
	{
		if (CurEffect.IsValid())
		{
			const int32 CurEffectLayer = CurEffect.GetEffectDefinition()->GetLayer();
			const float CurEffectStartTime = CurEffect.GetStartTime();

			if (CurEffectLayer < NewEffectLayer
				|| CurEffectStartTime < NewEffectStartTime)
			{
				// 
				IndexToInsert++;
			}
			else
			{
				// We found the spot to insert
				break;
			}
		}
	}

	SortedEffects.Insert(NewEffect, IndexToInsert);
}

bool FSortedEffectDefinitions::ClearLayeredEffects()
{
	const bool bAnyEffectsCleared = (SortedEffects.Num() > 0);
	SortedEffects.Empty();
	return bAnyEffectsCleared;
}

int32 FSortedEffectDefinitions::GetCurrentValue(const int32 BaseValue) const
{
	int32 CurrentValue = BaseValue;

	for (const FActiveEffectDefinition& CurEffect : SortedEffects)
	{
		if (CurEffect.IsValid())
		{
			const int32 CurEffectMod = CurEffect.GetEffectDefinition()->GetModification();

			switch (CurEffect.GetEffectDefinition()->GetOperation())
			{
				case EEffectOperation::Set:
					CurrentValue = CurEffectMod;
					break;
				case EEffectOperation::Add:
					CurrentValue += CurEffectMod;
					break;
				case EEffectOperation::Subtract:
					CurrentValue -= CurEffectMod;
					break;
				case EEffectOperation::Multiply:
					CurrentValue *= CurEffectMod;
					break;
				case EEffectOperation::BitwiseOr:
					//CurrentValue = UKismetMathLibrary::Or_IntInt(CurrentValue, CurEffectMod);
					CurrentValue |= CurEffectMod;
					break;
				case EEffectOperation::BitwiseAnd:
					//CurrentValue = UKismetMathLibrary::And_IntInt(CurrentValue, CurEffectMod);
					CurrentValue &= CurEffectMod;
					break;
				case EEffectOperation::BitwiseXor:
					//CurrentValue = UKismetMathLibrary::Xor_IntInt(CurrentValue, CurEffectMod);
					CurrentValue ^= CurEffectMod;
					break;

				default:
					checkNoEntry();
					break;
			}
		}
	}

	return CurrentValue;
}

FOnAttributeChangedData::FOnAttributeChangedData(
	UObject* InOwner,
	EAttributeKey InAttribute,
	int32 InOldValue)
	: Owner(InOwner)
	, Attribute(InAttribute)
	, NewValue(0)
	, OldValue(InOldValue)
{
	if (const ILayeredAttributes* MyOwner = GetOwner())
	{
		NewValue = MyOwner->GetCurrentAttribute(InAttribute);
	}
}

FOnAttributeChangedData::~FOnAttributeChangedData()
{
	// When this struct is popped off the stack, we broadcast the event if it has valid data
	if (const ILayeredAttributes* MyOwner = GetOwner();
		MyOwner != nullptr && IsValid())
	{
		MyOwner->GetOnAnyAttributeValueChanged().Broadcast(*this);
	}
}

bool FOnAttributeChangedData::IsValid() const
{
	return (GetOwner() != nullptr
		&& Attribute != EAttributeKey::Invalid
		&& NewValue != OldValue);
}

ILayeredAttributes* FOnAttributeChangedData::GetOwner() const
{
	return Cast<ILayeredAttributes>(Owner);
}

FColor UStaticBlueprintLibrary::Conv_IntToColor(int32 Value)
{
	const uint32 UnsignedValue = std::make_unsigned_t<int32>(Value);
	return FColor(UnsignedValue);
}

int32 UStaticBlueprintLibrary::Conv_ColorToInt(FColor Value)
{
	const uint32 UnsignedValue = Value.ToPackedARGB();
	const int32 SignedValue = GetValueClampedToInt32(UnsignedValue);
	return SignedValue;
}

int32 UStaticBlueprintLibrary::GetValueClampedToInt32(int64 Value)
{
	UE_CLOG(!FMath::IsWithinInclusive(Value, static_cast<int64>(MIN_int32), static_cast<int64>(MAX_int32)),
		LogLayeredEffects, Error, TEXT("Clamping incoming 64-bit value %lld to [%d, %d]."), Value, MIN_int32, MAX_int32);
	return FMath::Clamp(Value, static_cast<int64>(MIN_int32), static_cast<int64>(MAX_int32));
}

int32 UStaticBlueprintLibrary::GetValueClampedToInt32(uint32 Value)
{
	UE_CLOG(!FMath::IsWithinInclusive(Value, static_cast<uint32>(0), static_cast<uint32>(MAX_int32)),
		LogLayeredEffects, Error, TEXT("Clamping incoming unsigned value %u to [%d, %d]."), Value, 0, MAX_int32);
	return FMath::Clamp(Value, static_cast<uint32>(0), static_cast<uint32>(MAX_int32));
}
