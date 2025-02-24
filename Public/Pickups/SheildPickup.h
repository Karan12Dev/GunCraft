// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "SheildPickup.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API ASheildPickup : public APickup
{
	GENERATED_BODY()
	

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float SheildReplenishAmount = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float SheildReplenishTime = 5.f;
};
