// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LayeredEffectDefinition.generated.h"

class ILayeredAttributes;

DECLARE_LOG_CATEGORY_EXTERN(LogLayeredEffects, Warning, All);

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
	Mana,
	Controller,
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
	BitwiseXor,
};
namespace EEffectOperationUtils
{
	/// <summary>
	/// Performs evaluation of the left and right hand operands, given the Operation.
	/// </summary>
	static int32 Evaluate(int32 LhsOperand, int32 RhsOperand, EEffectOperation Operation)
	{
		switch (Operation)
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

	/// <summary>
	/// Short string representation of Operation for debugging/printing.
	/// </summary>
	static FString OperatorToString(EEffectOperation Operation)
	{
		switch (Operation)
		{
			case EEffectOperation::Invalid:
				return TEXT("INVALID");

			case EEffectOperation::Set:
				return TEXT("=");
			case EEffectOperation::Add:
				return TEXT("+");
			case EEffectOperation::Subtract:
				return TEXT("-");
			case EEffectOperation::Multiply:
				return TEXT("*");
			case EEffectOperation::BitwiseOr:
				return TEXT("|");
			case EEffectOperation::BitwiseAnd:
				return TEXT("&");
			case EEffectOperation::BitwiseXor:
				return TEXT("^");

			default:
				checkNoEntry();
				return TEXT("");
		}
	}
}


/// <summary>
/// Temporary parameter struct used when an attribute has changed.
/// The Owner should create this struct any time an attribute is being modified,
/// so that attribute changes can be detected and broadcast to UI/etc.
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FOnAttributeChangedData
{
	GENERATED_BODY()

public:

	FOnAttributeChangedData() = default;

	FOnAttributeChangedData(
		UObject* InOwner,
		EAttributeKey InAttribute,
		int32 InOldValue);

	bool IsValid() const;

	ILayeredAttributes* GetOwner() const;


private:

	/// <summary>
	/// Who owns this attribute.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UObject* Owner = nullptr;

	/// <summary>
	/// Which attribute was affected.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EAttributeKey Attribute = EAttributeKey::Invalid;

	/// <summary>
	/// New/current value for the attribute.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 NewValue = 0;

	/// <summary>
	/// Old/previous value for the attribute.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 OldValue = 0;
};


/// <summary>
/// Parameter struct for AddLayeredEffect(...)
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FLayeredEffectDefinition
{
	GENERATED_BODY()

public:

	FLayeredEffectDefinition() = default;
	FLayeredEffectDefinition(
		EAttributeKey InAttribute,
		EEffectOperation InOperation,
		int32 InModification,
		int32 InLayer)
		: Attribute(InAttribute)
		, Operation(InOperation)
		, Modification(InModification)
		, Layer(InLayer)
	{ }

	EAttributeKey GetAttribute() const { return Attribute; };
	EEffectOperation GetOperation() const { return Operation; };
	int32 GetModification() const { return Modification; };
	int32 GetLayer() const { return Layer; };

	bool IsValid() const
	{
		return (GetAttribute() != EAttributeKey::Invalid
			&& GetOperation() != EEffectOperation::Invalid);
	}

	FString ToString() const;


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


/// <summary>
/// This handle is required for referring to a specific active FActiveEffectDefinition.
/// For example if a skill needs to create an active effect and then destroy that specific effect that it created,
/// it has to do so through a handle; a pointer or index into the active list is not sufficient.
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FActiveEffectHandle
{
	GENERATED_BODY()

	FActiveEffectHandle() = default;

	FActiveEffectHandle(int32 InHandle, EAttributeKey InAttribute)
		: Handle(InHandle)
		, Attribute(InAttribute)
	{ }

	static const FActiveEffectHandle kInvalid;

	/// <summary>
	/// Creates a new handle, will be set to successfully applied.
	/// </summary>
	static FActiveEffectHandle GenerateNewHandle(EAttributeKey Attribute);

	/// <summary>
	/// True if this is tracking an active ongoing effect.
	/// </summary>
	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%d"), Handle);
	}

	bool Equals(const FActiveEffectHandle& Other) const
	{
		return Handle == Other.Handle;
	}
	bool operator==(const FActiveEffectHandle& Other) const { return Equals(Other); }
	bool operator!=(const FActiveEffectHandle& Other) const { return !Equals(Other); }

	friend uint32 GetTypeHash(const FActiveEffectHandle& InHandle)
	{
		return InHandle.Handle;
	}

	void Invalidate()
	{
		Handle = INDEX_NONE;
		Attribute = EAttributeKey::Invalid;
	}

	EAttributeKey GetAttribute() const { return Attribute; }

private:

	/// <summary>
	/// Unique ID for this effect.
	/// </summary>
	UPROPERTY()
	int32 Handle = INDEX_NONE;

	/// <summary>
	/// Which attribute this effect modifies (for faster lookup on Owner).
	/// </summary>
	UPROPERTY()
	EAttributeKey Attribute = EAttributeKey::Invalid;
};


/// <summary>
/// Represents an active layered effect.
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FActiveEffectDefinition
{
	GENERATED_BODY()

public:

	FActiveEffectDefinition() = default;

	FActiveEffectDefinition(const UWorld* World, const FLayeredEffectDefinition& InDef)
		: Handle(FActiveEffectHandle::GenerateNewHandle(InDef.GetAttribute()))
		, StartServerWorldTime(World == nullptr ? -1.f : World->GetTimeSeconds())
		, Def(InDef)
	{ }

	bool IsValid() const
	{
		return (Handle.IsValid()
			&& StartServerWorldTime >= 0.f
			&& Def.IsValid());
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%s %.2f %s"),
			*Handle.ToString(),
			StartServerWorldTime,
			*Def.ToString());
	}

	const FActiveEffectHandle& GetHandle() const { return Handle; }

	float GetStartTime() const { return StartServerWorldTime; }

	const FLayeredEffectDefinition& GetEffectDefinition() const { return Def; }


private:

	/// <summary>
	/// Globally unique ID for identify this active effect.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, NotReplicated, meta = (AllowPrivateAccess = "true"))
	FActiveEffectHandle Handle = FActiveEffectHandle::kInvalid;

	/// <summary>
	/// Server timestamp when this effect was applied (in seconds).
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float StartServerWorldTime = 0.f;

	/// <summary>
	/// Effect definition. The static data that this spec points to.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FLayeredEffectDefinition Def = FLayeredEffectDefinition();
};


/// <summary>
/// Stores applied FLayeredEffectDefinition for a single attribute.
/// All operations maintain an increasing sorted order by FLayeredEffectDefinition::Layer for faster layered attribute calculation.
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FSortedEffectDefinitions
{
	GENERATED_BODY()

public:

	/// <summary>
	/// Applies a new layered effect to this object's attributes.
	/// </summary>
	/// <param name="World">World that this effect is being applied it, so that we can track time of application.</param>
	/// <param name="Effect">The new layered effect to apply.</param>
	/// <returns>The handle to the newly applied effect, so that it can be removed later.</returns>
	FActiveEffectHandle AddLayeredEffect(const UWorld* World, const FLayeredEffectDefinition& Effect);

	/// <summary>
	/// Removes an active layered effect.
	/// </summary>
	/// <param name="InHandle">Which active effect to remove.</param>
	/// <returns>True if the effect was successfully removed.</returns>
	bool RemoveLayeredEffect(const FActiveEffectHandle& InHandle);

	/// <summary>
	/// Removes all layered effects from this object. After this call,
	/// all current attributes will be equal to the base attributes.
	/// </summary>
	/// <returns>True if any effect was successfully removed.</returns>
	bool ClearLayeredEffects();

	/// <summary>
	/// Modifies the BaseValue by all active layered effects.
	/// </summary>
	/// <param name="BaseValue">The base value for the attribute, as a starting point to calculate from.</param>
	/// <returns>The current value of the attribute, accounting for all layered effects.</returns>
	int32 GetCurrentValue(const int32 BaseValue) const;

private:

	/// <summary>
	/// Sorted effects applied to an attribute.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FActiveEffectDefinition> SortedEffects;
};


/// <summary>
/// Exposing helper methods for layered effects to blueprint.
/// </summary>
UCLASS()
class WIZARDS_API ULayeredEffectBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "LayeredEffectDefinition")
	static bool IsValid(const FLayeredEffectDefinition& Effect) { return Effect.IsValid(); }

	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "->", BlueprintAutocast), Category = "LayeredEffectDefinition")
	static FString ToString(const FLayeredEffectDefinition& Effect) { return Effect.ToString(); }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToColor (int)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|Attributes")
	static FColor Conv_IntToColor(int32 Value);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToInt (color)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|Attributes")
	static int32 Conv_ColorToInt(FColor Value);
};


/// <summary>
/// Exposing enum helper methods to C++.
/// </summary>
UCLASS()
class WIZARDS_API UEnumLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Converts an enum value to an FName.
	 *
	 * E.g., for
	 * UStaticLibrary::GetEnumValueAsString(EMyEnum::OptionA),
	 * this will return "EMyEnum::OptionA".
	 *
	 * @tparam TEnum Type of the enum (e.g., <ENetRole>).
	 * @param Value Enum value to convert to an FName.
	 * @returns The enum value as an FName.
	 */
	template <typename TEnum>
	static FORCEINLINE FName GetEnumValueAsName(TEnum Value)
	{
		FName AsName = NAME_None;
		UEnum::GetValueAsName(Value, AsName);
		return AsName;
	}

	/**
	 * Converts an enum value to an FString.
	 *
	 * E.g., for
	 * UStaticLibrary::GetEnumValueAsString(EMyEnum::OptionA),
	 * this will return "EMyEnum::OptionA".
	 *
	 * @tparam TEnum Type of the enum (e.g., <ENetRole>).
	 * @param Value Enum value to convert to an FString.
	 * @returns The enum value as an FString.
	 */
	template <typename TEnum>
	static FORCEINLINE FString GetEnumValueAsString(TEnum Value)
	{
		return GetEnumValueAsName(Value).ToString();
	}

	/**
	 * Gets the enum pointer from the templated argument.
	 *
	 * @tparam TEnum Type of the enum (e.g., <ENetRole>).
	 * @returns	UEnum* Pointer to enum class.
	 */
	template <typename TEnum>
	static FORCEINLINE UEnum* GetEnumPtr()
	{
		static_assert(TIsEnum<TEnum>::Value, "Should only call this with enum types");
		UEnum* EnumClass = StaticEnum<TEnum>();
		check(EnumClass != nullptr);
		return EnumClass;
	}

	/**
	 * Gets the number of enum names in TEnum.
	 * Includes autogenerated _MAX entry.
	 *
	 * E.g., for
	 * UENUM(BlueprintType)
	 * enum class EMyEnum : uint8
	 * {
	 *     OptionA                                          = 0,
	 *     OptionB                                          = 1,
	 *     OptionC                                          = 4,
	 * };
	 * UEnumLibrary::GetEnumNumEntries<EMyEnum>(true),
	 * this will return "4" (OptionA, OptionB, OptionC, and autogenerated _MAX entry).
	 * UEnumLibrary::GetEnumNumEntries<EMyEnum>(false),
	 * will return "3" (excludes the autogenerated MAX)
	 *
	 * @tparam TEnum Type of the enum (e.g., <ENetRole>).
	 * @param bIncludeMax Whether the autogenerated _MAX entry should be included.
	 * @returns How many entries are in this enum.
	 */
	template <typename TEnum>
	static FORCEINLINE int32 GetEnumNumEntries(bool bIncludeMax = false)
	{
		return GetEnumNumEntries(GetEnumPtr<TEnum>());
	}
	static FORCEINLINE int32 GetEnumNumEntries(const UEnum* EnumClass, bool bIncludeMax = false)
	{
		if (EnumClass == nullptr)
		{
			return INDEX_NONE;
		}

		int32 NumEnums = EnumClass->NumEnums();

		if (!bIncludeMax && EnumClass->ContainsExistingMax())
		{
			NumEnums--;
		}

		return NumEnums;
	}

	/**
	 * Retrieve the values within an enum as an iterable array.
	 *
	 * @tparam	TEnum Type of the enum (e.g., <EMyEnum>).
	 * @param	StartingValue (inclusive) Only enum values greater or equal to this one will be included.
	 * @param	bIncludeMax Whether the autogenerated _MAX entry should be included.
	 * @returns	All of the requested enum values.
	 */
	template <typename TEnum>
	static TArray<TEnum> GetEnumEntries(TEnum StartingValue = (TEnum)0, bool bIncludeMax = false)
	{
		TArray<TEnum> EnumEntries;

		const UEnum* EnumPtr = GetEnumPtr<TEnum>();
		const int32 EnumNumEntries = UEnumLibrary::GetEnumNumEntries<TEnum>(bIncludeMax);
		for (uint8 i = 0; i < EnumNumEntries; i++)
		{
			TEnum CurEnumValue = (TEnum)EnumPtr->GetValueByIndex(i);
			if (CurEnumValue >= StartingValue)
			{
				EnumEntries.Add(CurEnumValue);
			}
		}

		return EnumEntries;
	}
};