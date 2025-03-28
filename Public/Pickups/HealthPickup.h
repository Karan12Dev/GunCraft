// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();
	

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float HealAmount = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float HealingTime = 5.f;
};
