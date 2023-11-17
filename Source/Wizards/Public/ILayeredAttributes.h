// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LayeredEffectDefinition.h"

#include "ILayeredAttributes.generated.h"

// GOAL:
//   Create a class that implements the following interface. Use your best
//     judgement when it comes to design tradeoffs and implementation decisions.
//   You may create any number of other classes to support your implementation.
//   If you alter the given code, please maintain the intent of the original code
//     and document why your alterations were necessary.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeValueChangedEvent, const FOnAttributeChangedData&, Data);

// This class does not need to be modified.
UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class ULayeredAttributes : public UInterface
{
	GENERATED_BODY()
};

/// <summary>
/// Any object that implements this interface has a set of "base" attributes
/// that represent the default state of that object. However, the game engine
/// may apply one or more "layered effects" to modify those attributes. An
/// object's "current" attributes are always equal to the base attributes
/// with all layered effects applied, in the proper order. Any change to the
/// base attribute or layered effects should immediately be reflected in the
/// current attribute.
/// </summary>
class WIZARDS_API ILayeredAttributes
{
	GENERATED_BODY()

public:

	UObject* AsObject();

	UWorld* GetWorld();

	/// <summary>
	/// Set the base value for an attribute on this object. All base values
	/// default to 0 until set. Note that resetting a base attribute does not
	/// alter any existing layered effects.
	/// </summary>
	/// <param name="key">The attribute being set.</param>
	/// <param name="value">The new base value.</param>
	UFUNCTION(BlueprintCallable)
	virtual void SetBaseAttribute(EAttributeKey Key, int32 Value);

	UFUNCTION(BlueprintCallable)
	virtual int32 GetBaseAttribute(EAttributeKey Key) const;

	/// <summary>
	/// Return the current value for an attribute on this object. Will
	/// be equal to the base value, modified by any applicable layered
	/// effects.
	/// </summary>
	/// <param name="key">The attribute being read.</param>
	/// <returns>The current value of the attribute, accounting for all layered effects.</returns>
	UFUNCTION(BlueprintCallable)
	virtual int32 GetCurrentAttribute(EAttributeKey Key) const;

	UFUNCTION(BlueprintCallable)
	virtual FColor GetCurrentColor() const;

	UFUNCTION(BlueprintCallable)
	virtual ECreatureTypes GetCurrentTypes() const;

	UFUNCTION(BlueprintCallable)
	virtual ECreatureSubtypes GetCurrentSubtypes() const;

	UFUNCTION(BlueprintCallable)
	virtual ECreatureSupertypes GetCurrentSupertypes() const;

	/// <summary>
	/// Applies a new layered effect to this object's attributes. See
	/// LayeredEffectDefinition for details on how layered effects are
	/// applied. Note that any number of layered effects may be applied
	/// at any given time. Also note that layered effects are not necessarily
	/// applied in the same order they were added. (see ULayeredEffectDefinition::Layer)
	/// </summary>
	/// <param name="effect">The new layered effect to apply.</param>
	UFUNCTION(BlueprintCallable)
	virtual FActiveEffectHandle AddLayeredEffect(TSubclassOf<ULayeredEffectDefinition> Effect, bool& bSuccess);

	UFUNCTION(BlueprintCallable)
	virtual UPARAM(DisplayName = "bSuccess") bool RemoveLayeredEffect(const FActiveEffectHandle& InHandle);

	/// <summary>
	/// Removes all layered effects from this object. After this call,
	/// all current attributes will be equal to the base attributes.
	/// </summary>
	UFUNCTION(BlueprintCallable)
	virtual void ClearLayeredEffects();

	/// <summary>
	/// Delegate invoked when an attribute changes.
	/// </summary>
	virtual const FOnAttributeValueChangedEvent& GetOnAnyAttributeValueChanged() const = 0;


protected:

	virtual TMap<EAttributeKey, int32>& GetBaseAttributesMutable() = 0;
	virtual const TMap<EAttributeKey, int32>& GetBaseAttributes() const = 0;

	virtual TMap<EAttributeKey, FSortedEffectDefinitions>& GetActiveEffectsMutable() = 0;
	virtual const TMap<EAttributeKey, FSortedEffectDefinitions>& GetActiveEffects() const = 0;

};
