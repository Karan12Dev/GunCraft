// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
	

public:
	virtual void Fire(const FVector& HitTarget) override;


private:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	uint32 NumberOfPellets = 10;
};
