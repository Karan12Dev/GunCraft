// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class GUNCRAFT_API ACasing : public AActor
{
	GENERATED_BODY()
	

public:	
	ACasing();


protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpluse, const FHitResult& Hit);


private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditDefaultsOnly)
	float ShellEjectionImpulse = 5.f;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* ShellSound;

	int32 bShellSoundCount = 0;
};
