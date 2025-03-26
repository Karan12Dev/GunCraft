// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GUNCRAFT_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AGunslinger;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Heal(float HealAmount, float HealingTime);
	void ReplenishSheild(float SheildAmount, float ReplenishTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);


protected:
	virtual void BeginPlay() override;


private:
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
	void HealRampUp(float DeltaTime);
	void SheildRampUp(float DeltaTime);

	UPROPERTY()
	AGunslinger* Character;

	//	Heal Buff
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	//	Sheild Buff
	bool bReplenishingSheild = false;
	float SheildReplenishRate = 0.f;
	float SheildReplenishAmmount = 0.f;

	//	Speed Buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseMaxSpeed, float BaseMinSpeed ,float CrouchSpeed);

	//	Jump Buff
	FTimerHandle JumpBuffTimer;
	void ResetVelocity();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
};
