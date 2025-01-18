// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GunslingerAnimInstance.h"
#include "Character/Gunslinger.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Gun.h"
#include "GunslingerTypes/CombatState.h"


void UGunslingerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Gunslinger = Cast<AGunslinger>(TryGetPawnOwner());
}

void UGunslingerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Gunslinger == nullptr)
	{
		Gunslinger = Cast<AGunslinger>(TryGetPawnOwner());
	}
	if (Gunslinger == nullptr) return;	

	Speed = Gunslinger->GetVelocity().Size2D();
	bIsInAir = Gunslinger->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Gunslinger->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = Gunslinger->IsWeaponEquipped();
	EquippedGun = Gunslinger->GetEquippedGun();
	bIsCrouched = Gunslinger->bIsCrouched;
	bAiming = Gunslinger->IsAiming();
	TurningInPlace = Gunslinger->GetTurningInPlace();
	bRotateRootBone = Gunslinger->ShouldRotateRootBone();
	bElimmed = Gunslinger->IsElimmed();

	// Offset Yaw for Strafing
	FRotator AimRotation = Gunslinger->GetBaseAimRotation();
	FRotator MoveRotation = UKismetMathLibrary::MakeRotFromX(Gunslinger->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MoveRotation, AimRotation).Yaw;

	// Lean
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = Gunslinger->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = Gunslinger->GetAO_Yaw();
	AO_Pitch = Gunslinger->GetAO_Pitch();

	if (bWeaponEquipped && EquippedGun && EquippedGun->GetGunMesh() && Gunslinger->GetMesh())
	{
		LeftHandTransform = EquippedGun->GetGunMesh()->GetSocketTransform(FName("LH_Socket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		Gunslinger->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (Gunslinger->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = Gunslinger->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - Gunslinger->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}

	bUseFABRIK = Gunslinger->GetCombatState() != ECombatState::Reloading;
	bUseAimOffsets = Gunslinger->GetCombatState() != ECombatState::Reloading && !Gunslinger->bDisableGameplay;
	bTransformRightHand = Gunslinger->GetCombatState() != ECombatState::Reloading && !Gunslinger->bDisableGameplay;
}
