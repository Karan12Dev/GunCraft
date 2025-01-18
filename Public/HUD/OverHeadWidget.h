// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;


protected:
	virtual void NativeDestruct() override;


private:

};
