// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#include "ILayeredAttributes.h"

UObject* ILayeredAttributes::AsObject()
{
	return Cast<UObject>(this);
}

UWorld* ILayeredAttributes::GetWorld()
{
	return AsObject()->GetWorld();
}

void ILayeredAttributes::SetBaseAttribute(EAttributeKey Key, int32 Value)
{
	// Capture the current attribute value
	const int32 OldValue = GetCurrentAttribute(Key);

	// Set the base attribute to the new Value
	TMap<EAttributeKey, int32>& BaseAttributes = GetBaseAttributesMutable();
	BaseAttributes.Add(Key, Value);

	// If there's a change, broadcast it
	FOnAttributeChangedData(AsObject(), Key, OldValue);
}

int32 ILayeredAttributes::GetBaseAttribute(EAttributeKey Key) const
{
	const TMap<EAttributeKey, int32>& BaseAttributes = GetBaseAttributes();
	return BaseAttributes.FindRef(Key);
}

int32 ILayeredAttributes::GetCurrentAttribute(EAttributeKey Key) const
{
	const int32 BaseValueForAttribute = GetBaseAttribute(Key);

	if (const TMap<EAttributeKey, FSortedEffectDefinitions>& ActiveEffects = GetActiveEffects();
		const FSortedEffectDefinitions* ActiveEffectsForAttribute = ActiveEffects.Find(Key))
	{
		return ActiveEffectsForAttribute->GetCurrentValue(BaseValueForAttribute);
	}

	return BaseValueForAttribute;
}

FColor ILayeredAttributes::GetCurrentColor() const
{
	const int32 CurrentColorInt = GetCurrentAttribute(EAttributeKey::Color);
	const FColor CurrentColor = UStaticBlueprintLibrary::Conv_IntToColor(CurrentColorInt);
	return CurrentColor;
}

ECreatureTypes ILayeredAttributes::GetCurrentTypes() const
{
	const int32 CurrentTypesInt = GetCurrentAttribute(EAttributeKey::Types);
	return static_cast<ECreatureTypes>(CurrentTypesInt);
}

ECreatureSubtypes ILayeredAttributes::GetCurrentSubtypes() const
{
	const int32 CurrentSubtypesInt = GetCurrentAttribute(EAttributeKey::Subtypes);
	return static_cast<ECreatureSubtypes>(CurrentSubtypesInt);
}

ECreatureSupertypes ILayeredAttributes::GetCurrentSupertypes() const
{
	const int32 CurrentSupertypesInt = GetCurrentAttribute(EAttributeKey::Supertypes);
	return static_cast<ECreatureSupertypes>(CurrentSupertypesInt);
}

FActiveEffectHandle ILayeredAttributes::AddLayeredEffect(FLayeredEffectDefinition Effect, bool& bSuccess)
{
	if (!Effect.IsValid())
	{
		bSuccess = false;
		return FActiveEffectHandle::kInvalid;
	}

	// Capture the current attribute value
	const EAttributeKey Key = Effect.GetAttribute();
	const int32 OldValue = GetCurrentAttribute(Key);

	// Add the new layered effect
	FSortedEffectDefinitions& ActiveEffects = GetActiveEffectsMutable().FindOrAdd(Key);
	const FActiveEffectHandle NewEffect = ActiveEffects.AddLayeredEffect(GetWorld(), Effect);

	// If there's a change, broadcast it
	FOnAttributeChangedData(AsObject(), Key, OldValue);

	bSuccess = NewEffect.IsValid();
	return NewEffect;
}

bool ILayeredAttributes::RemoveLayeredEffect(const FActiveEffectHandle& InHandle)
{
	if (!InHandle.IsValid())
	{
		return false;
	}

	if (const EAttributeKey Key = InHandle.GetAttribute();
		FSortedEffectDefinitions* ActiveEffectsForAttribute = GetActiveEffectsMutable().Find(Key))
	{
		// Capture the current attribute value
		const int32 OldValue = GetCurrentAttribute(Key);

		// See if any effects were removed
		if (ActiveEffectsForAttribute->RemoveLayeredEffect(InHandle))
		{
			// If there's a change, broadcast it
			FOnAttributeChangedData(AsObject(), Key, OldValue);
			return true;
		}
	}

	return false;
}

void ILayeredAttributes::ClearLayeredEffects()
{
	for (TPair<EAttributeKey, FSortedEffectDefinitions>& CurActiveEffects : GetActiveEffectsMutable())
	{
		const EAttributeKey CurAttribute = CurActiveEffects.Key;
		const int32 CurOldValue = GetCurrentAttribute(CurAttribute);

		if (CurActiveEffects.Value.ClearLayeredEffects())
		{
			// If there's a change, broadcast it
			FOnAttributeChangedData(AsObject(), CurAttribute, CurOldValue);
		}
	}
}
