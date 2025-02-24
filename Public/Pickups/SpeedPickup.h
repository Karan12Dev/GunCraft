// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
	

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float CrouchSpeedBuff = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float SpeedBuffTime = 20.f;
};
