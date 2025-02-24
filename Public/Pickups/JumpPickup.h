// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AJumpPickup : public APickup
{
	GENERATED_BODY()
	

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float JumpZVelocityBuff = 4000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float JumpBuffTime = 15.f;
};
