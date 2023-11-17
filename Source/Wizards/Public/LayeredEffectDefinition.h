// Copyright 2023 John McElmurray (johnmcelmurray.com). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LayeredEffectDefinition.generated.h"

class ILayeredAttributes;

DECLARE_LOG_CATEGORY_EXTERN(LogLayeredEffects, Warning, All);

/// <summary>
/// These would be better suited as FGameplayTag, so that they could be in a tree structure, and not limited by the number of bits
/// </summary>
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum ECreatureTypes
{
	CreatureTypes_None					= 0				UMETA(DisplayName = "None"),
	CreatureTypes_Aberration			= (1 << 0)		UMETA(DisplayName = "Aberration"),
	CreatureTypes_Beast					= (1 << 1)		UMETA(DisplayName = "Beast"),
	CreatureTypes_Celestial				= (1 << 2)		UMETA(DisplayName = "Celestial"),
	CreatureTypes_Construct				= (1 << 3)		UMETA(DisplayName = "Construct"),
	CreatureTypes_Dragon				= (1 << 4)		UMETA(DisplayName = "Dragon"),
	CreatureTypes_Elemental				= (1 << 5)		UMETA(DisplayName = "Elemental"),
	CreatureTypes_Fey					= (1 << 6)		UMETA(DisplayName = "Fey"),
	CreatureTypes_Fiend					= (1 << 7)		UMETA(DisplayName = "Fiend"),
	CreatureTypes_Giant					= (1 << 8)		UMETA(DisplayName = "Giant"),
	CreatureTypes_Humanoid				= (1 << 9)		UMETA(DisplayName = "Humanoid"),
	CreatureTypes_Monstrosity			= (1 << 10)		UMETA(DisplayName = "Monstrosity"),
	CreatureTypes_Ooze					= (1 << 11)		UMETA(DisplayName = "Ooze"),
	CreatureTypes_Plant					= (1 << 12)		UMETA(DisplayName = "Plant"),
	CreatureTypes_Undead				= (1 << 13)		UMETA(DisplayName = "Undead"),
};
ENUM_CLASS_FLAGS(ECreatureTypes);


/// <summary>
/// These would be better suited as FGameplayTag, so that they could be in a tree structure, and not limited by the number of bits
/// </summary>
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum ECreatureSubtypes
{
	CreatureSubtypes_None				= 0				UMETA(DisplayName = "None"),
	CreatureSubtypes_Air				= (1 << 0)		UMETA(DisplayName = "Air"),
	CreatureSubtypes_Aquatic			= (1 << 1)		UMETA(DisplayName = "Aquatic"),
	CreatureSubtypes_Chaotic			= (1 << 2)		UMETA(DisplayName = "Chaotic"),
	CreatureSubtypes_Cold				= (1 << 3)		UMETA(DisplayName = "Cold"),
	CreatureSubtypes_Earth				= (1 << 4)		UMETA(DisplayName = "Earth"),
	CreatureSubtypes_Electricity		= (1 << 5)		UMETA(DisplayName = "Electricity"),
	CreatureSubtypes_Evil				= (1 << 6)		UMETA(DisplayName = "Evil"),
	CreatureSubtypes_Incorporeal		= (1 << 7)		UMETA(DisplayName = "Incorporeal"),
	CreatureSubtypes_Fire				= (1 << 8)		UMETA(DisplayName = "Fire"),
	CreatureSubtypes_Good				= (1 << 9)		UMETA(DisplayName = "Good"),
	CreatureSubtypes_Lawful				= (1 << 10)		UMETA(DisplayName = "Lawful"),
	CreatureSubtypes_Reptilian			= (1 << 11)		UMETA(DisplayName = "Reptilian"),
	CreatureSubtypes_Water				= (1 << 12)		UMETA(DisplayName = "Water"),
};
ENUM_CLASS_FLAGS(ECreatureSubtypes);


/// <summary>
/// These would be better suited as FGameplayTag, so that they could be in a tree structure, and not limited by the number of bits
/// </summary>
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum ECreatureSupertypes
{
	CreatureSupertypes_None					= 0				UMETA(DisplayName = "None"),
	CreatureSupertypes_Basic				= (1 << 0)		UMETA(DisplayName = "Basic"),
	CreatureSupertypes_Legendary			= (1 << 1)		UMETA(DisplayName = "Legendary"),
};
ENUM_CLASS_FLAGS(ECreatureSupertypes);


UENUM(BlueprintType)
enum class EAttributeKey : uint8
{
	Invalid = 0,

	Power			UMETA(Tooltip = "@TODO: fill out"),
	Toughness		UMETA(Tooltip = "@TODO: fill out"),
	Loyalty			UMETA(Tooltip = "@TODO: fill out"),
	Mana			UMETA(Tooltip = "@TODO: fill out"),

	Color			UMETA(Tooltip = "@TODO: fill out"),

	Types			UMETA(Tooltip = "@TODO: fill out"),
	Subtypes		UMETA(Tooltip = "@TODO: fill out"),
	Supertypes		UMETA(Tooltip = "@TODO: fill out"),

	Controller		UMETA(Tooltip = "@TODO: fill out")
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
	/// Perform a bitwise "or" operation. (Add flag)
	/// </summary>
	BitwiseOr,

	/// <summary>
	/// Perform a bitwise "and" operation.
	/// </summary>
	BitwiseAnd,

	/// <summary>
	/// Perform a bitwise "exclusive or" operation. (Toggle flag)
	/// </summary>
	BitwiseXor
};


/// <summary>
/// Temporary parameter struct used when an attribute has changed.
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UObject* Owner = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EAttributeKey Attribute = EAttributeKey::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 NewValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 OldValue = 0;
};


/// <summary>
/// Parameter struct for AddLayeredEffect(...)
/// </summary>
UCLASS(BlueprintType, Blueprintable)
class WIZARDS_API ULayeredEffectDefinition : public UObject
{
	GENERATED_BODY()

public:

	EAttributeKey GetAttribute() const { return Attribute; };
	EEffectOperation GetOperation() const { return Operation; };
	int32 GetModification() const { return Modification; };
	int32 GetLayer() const { return Layer; };

	UFUNCTION(BlueprintPure)
	bool IsValid() const
	{
		return (GetAttribute() != EAttributeKey::Invalid
			&& GetOperation() != EEffectOperation::Invalid);
	}

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif

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

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FColor ModificationColor = FColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", Bitmask, BitmaskEnum = ECreatureTypes))
	int32 ModificationTypes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", Bitmask, BitmaskEnum = ECreatureSubtypes))
	int32 ModificationSubtypes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", Bitmask, BitmaskEnum = ECreatureSupertypes))
	int32 ModificationSupertypes = 0;
#endif

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
/// For example if a skill needs to create an active effect and then destroy that specific effect that it created, it has to do so
/// through a handle.a pointer or index into the active list is not sufficient.
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

	/** True if this is tracking an active ongoing effect */
	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	/** Creates a new handle, will be set to successfully applied */
	static FActiveEffectHandle GenerateNewHandle(EAttributeKey Attribute);

	bool operator==(const FActiveEffectHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FActiveEffectHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	friend uint32 GetTypeHash(const FActiveEffectHandle& InHandle)
	{
		return InHandle.Handle;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%d"), Handle);
	}

	void Invalidate()
	{
		Handle = INDEX_NONE;
	}

	EAttributeKey GetAttribute() const { return Attribute; }

private:

	UPROPERTY()
	int32 Handle = INDEX_NONE;

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

	FActiveEffectDefinition(const UWorld* World, TSubclassOf<ULayeredEffectDefinition> InDef)
		: Handle(FActiveEffectHandle::GenerateNewHandle(InDef == nullptr ? EAttributeKey::Invalid : InDef.GetDefaultObject()->GetAttribute()))
		, StartServerWorldTime(World == nullptr ? -1.f : World->GetTimeSeconds())
		, Def(InDef == nullptr ? nullptr : InDef.GetDefaultObject())
	{ }

	bool IsValid() const
	{
		return (Handle.IsValid()
			&& StartServerWorldTime >= 0.f
			&& Def != nullptr
			&& Def->IsValid());
	}

	const FActiveEffectHandle& GetHandle() const { return Handle; }

	float GetStartTime() const { return StartServerWorldTime; }

	const ULayeredEffectDefinition* GetEffectDefinition() const { return Def; }


private:

	/// <summary>
	/// Globally unique ID for identify this active effect. Not networked since it's created from a static var.
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
	const ULayeredEffectDefinition* Def = nullptr;
};


/// <summary>
/// Stores applied FLayeredEffectDefinition.
/// All operations maintain an increasing sorted order by FLayeredEffectDefinition::Layer.
/// </summary>
USTRUCT(BlueprintType)
struct WIZARDS_API FSortedEffectDefinitions
{
	GENERATED_BODY()

public:

	FActiveEffectHandle AddLayeredEffect(const UWorld* World, TSubclassOf<ULayeredEffectDefinition> Effect);

	bool RemoveLayeredEffect(const FActiveEffectHandle& InHandle);

	bool ClearLayeredEffects();

	int32 GetCurrentValue(const int32 BaseValue) const;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FActiveEffectDefinition> SortedEffects;
};


/// <summary>
/// Exposing helper methods to blueprint.
/// </summary>
UCLASS()
class WIZARDS_API UStaticBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToColor (int)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|Attributes")
	static FColor Conv_IntToColor(int32 Value);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToInt (color)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|Attributes")
	static int32 Conv_ColorToInt(FColor Value);

	static int32 GetValueClampedToInt32(int64 Value);

	static int32 GetValueClampedToInt32(uint32 Value);

	/** Uses StringFunction to transform every member of TargetArray to an FString. */
	template <typename T, typename TAllocator = FDefaultAllocator, typename StringFn>
	static FString ArrayAsString(const TArray<T, TAllocator>& TargetArray, StringFn StringFunction, const FString& Separator = TEXT(", "))
	{
		FString Result;
		bool    First = true;
		for (const auto& Element : TargetArray)
		{
			if (First)
			{
				First = false;
			}
			else
			{
				Result += Separator;
			}

			Result += StringFunction(Element);
		}

		return Result;
	}

	template <typename TAllocator = FDefaultAllocator>
	static FString NameArrayAsString(const TArray<FName, TAllocator>& NameArray, const FString& Separator = TEXT(", "))
	{
		auto NameToString = [](const FName& Name) -> FString {
			return Name.ToString();
		};

		return ArrayAsString(NameArray, NameToString, Separator);
	}

	/**
	 * Converts a bitmask value to an FString.
	 *
	 * E.g., for
	 * GetBitmaskValueAsString(EMyMask::MyMask_OptionA | EMyMask::MyMask_OptionB),
	 * this will return "EMyMask::MyMask_OptionA | EMyMask::MyMask_OptionB".
	 *
	 * @tparam TEnum Type of the bitmask (e.g., <EMyMask>).
	 * @param MaskValue Bitmask value to convert to an FString.
	 * @returns The bitmask value as an FString.
	 */
	template <typename TEnum>
	static FString GetBitmaskValueAsString(TEnum MaskValue)
	{
		UEnum* EnumPtr = GetEnumPtr<TEnum>();
		TArray<FName> MaskValueEnumEntryNames;

		for (int32 i = 0; i < GetEnumNumEntries<TEnum>(/*bIncludeMax*/ false); i++)
		{
			const TEnum CurEnumEntryValue = static_cast<TEnum>(EnumPtr->GetValueByIndex(i));
			const FName CurEnumEntryName = EnumPtr->GetNameByIndex(i);

			if (EnumHasAnyFlags(MaskValue, CurEnumEntryValue))
			{
				MaskValueEnumEntryNames.Add(CurEnumEntryName);
			}
		}

		return NameArrayAsString(MaskValueEnumEntryNames, TEXT(" | "));
	}

};