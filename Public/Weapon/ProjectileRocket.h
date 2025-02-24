// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
	

public:
	AProjectileRocket();
	virtual void Destroyed() override;


protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpluse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Rocket")
	class USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Rocket")
	USoundAttenuation* LoopingSoundAttenuation;
};
