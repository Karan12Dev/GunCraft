// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Gun.h"
#include "Weapon/WeaponTypes.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AHitScanWeapon : public AGun
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;


protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);


private:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UParticleSystem* ImpactParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* HitSound;

	//	Trace end with scatter
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float SphereRadius = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bUseScatter = false;
};
