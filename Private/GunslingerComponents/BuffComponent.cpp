// Fill out your copyright notice in the Description page of Project Settings.


#include "GunslingerComponents/BuffComponent.h"
#include "Character/Gunslinger.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	SheildRampUp(DeltaTime);
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;
	
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(HealThisFrame);
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::SheildRampUp(float DeltaTime)
{
	if (!bReplenishingSheild || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = SheildReplenishRate * DeltaTime;
	Character->SetSheild(ReplenishThisFrame);
	Character->UpdateHUDSheild();
	SheildReplenishAmmount -= ReplenishThisFrame;
	if (SheildReplenishAmmount <= 0.f || Character->GetSheild() >= Character->GetMaxSheild())
	{
		bReplenishingSheild = false;
		SheildReplenishAmmount = 0.f;
	}
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishSheild(float SheildAmount, float ReplenishTime)
{
	bReplenishingSheild = true;
	SheildReplenishRate = SheildAmount / ReplenishTime;
	SheildReplenishAmmount += SheildAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &ThisClass::ResetSpeeds, BuffTime);
	MulticastSpeedBuff(BuffBaseSpeed, Character->MinRunningSpeed ,BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	MulticastSpeedBuff(Character->InitialMaxRunningSpeed ,Character->MinRunningSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseMaxSpeed, float BaseMinSpeed, float CrouchSpeed)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->MaxRunningSpeed = BaseMaxSpeed;
	Character->MinRunningSpeed = BaseMinSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &ThisClass::ResetVelocity, BuffTime);
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::ResetVelocity()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
}
