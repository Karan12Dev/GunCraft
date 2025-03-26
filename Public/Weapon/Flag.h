// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Gun.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AFlag : public AGun
{
	GENERATED_BODY()
	

public:
	AFlag();
	virtual void Dropped() override;
	void ResetFlag();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastResetFlag();


protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;
	virtual void OnDropped() override;


private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;
};
