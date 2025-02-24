// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class GUNCRAFT_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;


private:
	void SpawnPickup();

	UPROPERTY(EditAnywhere, Category = "Pickup Spawn")
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	UPROPERTY(EditAnywhere, Category = "Pickup Spawn")
	float MinSpawnTime = 6;

	UPROPERTY(EditAnywhere, Category = "Pickup Spawn")
	float MaxSpawnTime = 10;

	FTimerHandle SpawnPickupTimer;
	void SpawnPickupTimerFinished();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
};
