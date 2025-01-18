// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class GUNCRAFT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpluse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly)
	float Damage = 10.f;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* ImpactSound;

	UPROPERTY(VisibleDefaultsOnly)
	class UProjectileMovementComponent* ProjectileMovementComponent;


private:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;
};
