// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "ILayeredAttributes.h"

#include "WizardsCharacter.generated.h"

/// <summary>
/// Character that implements layered attributes.
/// </summary>
UCLASS(Blueprintable)
class AWizardsCharacter : public ACharacter, public ILayeredAttributes
{
	GENERATED_BODY()

public:

	AWizardsCharacter();

	virtual void BeginPlay() override;

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void HandleOnAnyAttributeValueChanged(const FOnAttributeChangedData& Data);

	// "ILayeredAttributes" interface methods
	virtual TMap<EAttributeKey, int32>& GetBaseAttributesMutable() override { return BaseAttributes; }
	virtual const TMap<EAttributeKey, int32>& GetBaseAttributes() const override { return BaseAttributes; }
	virtual TMap<EAttributeKey, FSortedEffectDefinitions>& GetActiveEffectsMutable() override { return ActiveEffects; }
	virtual const TMap<EAttributeKey, FSortedEffectDefinitions>& GetActiveEffects() const override { return ActiveEffects; }
	virtual const FOnAttributeValueChangedEvent& GetOnAnyAttributeValueChanged() const override { return OnAnyAttributeValueChanged; }


public:

	UPROPERTY(BlueprintAssignable, Category = Attributes)
	FOnAttributeValueChangedEvent OnAnyAttributeValueChanged;

private:

	/// <summary>
	/// Top down camera
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/// <summary>
	/// Camera boom positioning the camera above the character
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/// <summary>
	/// Base attributes for this character
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"))
	TMap<EAttributeKey, int32> BaseAttributes;

	/// <summary>
	/// Active effects modifying attributes for this character
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"))
	TMap<EAttributeKey, FSortedEffectDefinitions> ActiveEffects;

};

