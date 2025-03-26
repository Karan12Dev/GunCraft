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

	// Used with server side rewind
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Projectile")
	float InitialProjectileSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Projectile")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Projectile")
	float HeadShotDamage = 20.f;


protected:
	virtual void BeginPlay() override;
	void SpawnTrailSystem();
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpluse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DamageInnerRadius = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DamageOuterRadius = 600.f;


private:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DestroyTime = 2.5f;
};
