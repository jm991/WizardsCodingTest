// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "LayeredEffectDefinition.generated.h"

UENUM(BlueprintType)
enum class EAttributeKey : uint8
{
	Invalid = 0,
	Power,
	Toughness,
	Loyalty,
	Color,
	Types,
	Subtypes,
	Supertypes,
	ConvertedManaCost,
	Controller
};


UENUM(BlueprintType)
enum class EEffectOperation : uint8
{
	Invalid = 0,

	/// <summary>
	/// Set a value, discarding any prior value.
	/// </summary>
	Set,

	/// <summary>
	/// Add to the prior value.
	/// </summary>
	Add,

	/// <summary>
	/// Subtract from the prior value.
	/// </summary>
	Subtract,

	/// <summary>
	/// Multiply the prior value by the layered effect's Modification.
	/// </summary>
	Multiply,

	/// <summary>
	/// Perform a bitwise "or" operation.
	/// </summary>
	BitwiseOr,

	/// <summary>
	/// Perform a bitwise "and" operation.
	/// </summary>
	BitwiseAnd,

	/// <summary>
	/// Perform a bitwise "exclusive or" operation.
	/// </summary>
	BitwiseXor
};


/// <summary>
/// Parameter struct for AddLayeredEffect(...)
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FLayeredEffectDefinition
{
	GENERATED_BODY()

public:

	EAttributeKey GetAttribute() const { return Attribute; };
	EEffectOperation GetOperation() const { return Operation; };
	int32 GetModification() const { return Modification; };
	int32 GetLayer() const { return Layer; };

private:

	/// <summary>
	/// Which attribute this layered effect applies to.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EAttributeKey Attribute = EAttributeKey::Invalid;

	/// <summary>
	/// What mathematical or bitwise operation this layer performs.
	/// See EffectOperation for details.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EEffectOperation Operation = EEffectOperation::Invalid;

	/// <summary>
	/// The operand used for this layered effect's Operation.
	/// For example, if Operation is EffectOperation.Add, this is the
	/// amount that is added.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 Modification = 0;

	/// <summary>
	/// Which layer to apply this effect in. Smaller numbered layers
	/// get applied first. Layered effects with the same layer get applied
	/// in the order that they were added. (timestamp order)
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 Layer = 0;
};
