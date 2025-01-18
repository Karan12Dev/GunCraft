// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Gun.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AProjectileWeapon : public AGun
{
	GENERATED_BODY()
	

public:
	virtual void Fire(const FVector& HitTarget) override;


private:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AProjectile> ProjectileClass;
};
