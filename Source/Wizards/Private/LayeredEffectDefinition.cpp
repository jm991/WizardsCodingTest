// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#include "LayeredEffectDefinition.h"

#include "ILayeredAttributes.h"

DEFINE_LOG_CATEGORY(LogLayeredEffects);

const FActiveEffectHandle FActiveEffectHandle::kInvalid = FActiveEffectHandle();

int32 FSortedEffectDefinitions::Evaluate(int32 LhsOperand, int32 RhsOperand, EEffectOperation Operator)
{
	switch (Operator)
	{
		case EEffectOperation::Set:
			return RhsOperand;
		case EEffectOperation::Add:
			return (LhsOperand + RhsOperand);
		case EEffectOperation::Subtract:
			return (LhsOperand - RhsOperand);
		case EEffectOperation::Multiply:
			return (LhsOperand * RhsOperand);
		case EEffectOperation::BitwiseOr:
			return (LhsOperand | RhsOperand);
		case EEffectOperation::BitwiseAnd:
			return (LhsOperand & RhsOperand);
		case EEffectOperation::BitwiseXor:
			return (LhsOperand ^ RhsOperand);

		default:
			checkNoEntry();
			return LhsOperand;
	}
}

int32 FSortedEffectDefinitions::GetCurrentValue(const int32 BaseValue) const
{
	int32 CurrentValue = BaseValue;

	for (const FActiveEffectDefinition& CurEffect : SortedEffects)
	{
		if (CurEffect.IsValid())
		{
			const int32 CurEffectMod = CurEffect.GetEffectDefinition().GetModification();
			const EEffectOperation CurEffectOp = CurEffect.GetEffectDefinition().GetOperation();

			CurrentValue = Evaluate(CurrentValue, CurEffectMod, CurEffectOp);
		}
	}

	return CurrentValue;
}

FActiveEffectHandle FSortedEffectDefinitions::AddLayeredEffect(const UWorld* World, const FLayeredEffectDefinition& Effect)
{
	if (World == nullptr)
	{
		UE_LOG(LogLayeredEffects, Error, TEXT("Invalid world"));
		return FActiveEffectHandle::kInvalid;
	}

	if (!Effect.IsValid())
	{
		UE_LOG(LogLayeredEffects, Error, TEXT("Invalid effect '%s'"), *Effect.ToString());
		return FActiveEffectHandle::kInvalid;
	}

	const FActiveEffectDefinition NewActiveEffect = FActiveEffectDefinition(World, Effect);
	if (!NewActiveEffect.IsValid())
	{
		UE_LOG(LogLayeredEffects, Error, TEXT("Invalid active effect created from '%s'"), *NewActiveEffect.ToString());
		return FActiveEffectHandle::kInvalid;
	}

	const int32 NewEffectLayer = NewActiveEffect.GetEffectDefinition().GetLayer();
	const float NewEffectStartTime = NewActiveEffect.GetStartTime();

	int32 IndexToInsert = 0;
	for (const FActiveEffectDefinition& CurEffect : SortedEffects)
	{
		if (CurEffect.IsValid())
		{
			const int32 CurEffectLayer = CurEffect.GetEffectDefinition().GetLayer();
			const float CurEffectStartTime = CurEffect.GetStartTime();

			if (NewEffectLayer > CurEffectLayer // Smaller numbered layers get applied first
				|| (NewEffectLayer == CurEffectLayer // Effects with the same layer get applied in the order that they were added (timestamp order)
					&& NewEffectStartTime >= CurEffectStartTime))
			{
				// This effect should be at a higher layer - continue
				IndexToInsert++;
			}
			else
			{
				// We found the spot to insert
				break;
			}
		}
	}

	SortedEffects.Insert(NewActiveEffect, IndexToInsert);

	// Create a new handle for this effect
	return NewActiveEffect.GetHandle();
}

bool FSortedEffectDefinitions::RemoveLayeredEffect(const FActiveEffectHandle& InHandle)
{
	const int32 NumRemovedEffects = SortedEffects.RemoveAll([InHandle](const FActiveEffectDefinition& CurActiveEffect) {
		return CurActiveEffect.GetHandle() == InHandle;
	});
	return (NumRemovedEffects > 0);
}

bool FSortedEffectDefinitions::ClearLayeredEffects()
{
	const bool bAnyEffectsCleared = (SortedEffects.Num() > 0);
	SortedEffects.Empty();
	return bAnyEffectsCleared;
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

	// When this struct is created, we broadcast the event if it has valid data
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
	FColor ColorValue = FColor(UnsignedValue);
	ColorValue.A = 255;
	return ColorValue;
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

FActiveEffectHandle FActiveEffectHandle::GenerateNewHandle(EAttributeKey Attribute)
{
	static int32 GHandleID = 0;
	FActiveEffectHandle NewHandle(GHandleID++, Attribute);
	return NewHandle;
}
