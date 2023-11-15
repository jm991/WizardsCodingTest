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

	/// <summary>
	/// Set the base value for an attribute on this object. All base values
	/// default to 0 until set. Note that resetting a base attribute does not
	/// alter any existing layered effects.
	/// </summary>
	/// <param name="key">The attribute being set.</param>
	/// <param name="value">The new base value.</param>
	UFUNCTION(BlueprintCallable)
	virtual void SetBaseAttribute(EAttributeKey Key, int32 Value) = 0;

	/// <summary>
	/// Return the current value for an attribute on this object. Will
	/// be equal to the base value, modified by any applicable layered
	/// effects.
	/// </summary>
	/// <param name="key">The attribute being read.</param>
	/// <returns>The current value of the attribute, accounting for all layered effects.</returns>
	UFUNCTION(BlueprintCallable)
	virtual int32 GetCurrentAttribute(EAttributeKey Key) const = 0;

	/// <summary>
	/// Applies a new layered effect to this object's attributes. See
	/// LayeredEffectDefinition for details on how layered effects are
	/// applied. Note that any number of layered effects may be applied
	/// at any given time. Also note that layered effects are not necessarily
	/// applied in the same order they were added. (see FLayeredEffectDefinition::Layer)
	/// </summary>
	/// <param name="effect">The new layered effect to apply.</param>
	UFUNCTION(BlueprintCallable)
	virtual void AddLayeredEffect(FLayeredEffectDefinition Effect) = 0;

	/// <summary>
	/// Removes all layered effects from this object. After this call,
	/// all current attributes will be equal to the base attributes.
	/// </summary>
	UFUNCTION(BlueprintCallable)
	virtual void ClearLayeredEffects() = 0;

};
