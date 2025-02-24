// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class GUNCRAFT_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickupMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "PickupFX")
	USoundBase* PickupSound;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* VFXComponent;

	UPROPERTY(EditDefaultsOnly, Category = "PickupFX")
	class UNiagaraSystem* PickupVFX;

	FTimerHandle BindOverlapTimer;
	void BindOverlapTimerFinished();
	float BindOverlapTime = 0.25f;
};
