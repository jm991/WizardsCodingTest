// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "ILayeredAttributes.h"

#include "WizardsCharacter.generated.h"

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

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Base attributes for this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"))
	TMap<EAttributeKey, int32> BaseAttributes;

	/** Active effects modifying attributes for this character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"))
	TMap<EAttributeKey, FSortedEffectDefinitions> ActiveEffects;

};

