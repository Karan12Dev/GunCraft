// Fill out your copyright notice in the Description page of Project Settings.


#include "GunslingerComponents/CombatComponent.h"
#include "Weapon/Gun.h"
#include "Character/Gunslinger.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/GunslingerPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character && Character->GetCamera())
	{
		DefaultFOV = Character->GetCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;

		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedGun);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(ThisClass, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(ThisClass, CombatState);
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->Controller) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GunslingerController->GetHUD()) : GunslingerHUD;
		if (GunslingerHUD)
		{
			if (EquippedGun)
			{
				HUDPackage.CrosshairsCenter = EquippedGun->CrossHairsCenter;
				HUDPackage.CrosshairsLeft = EquippedGun->CrossHairsLeft;
				HUDPackage.CrosshairsRight = EquippedGun->CrossHairsRight;
				HUDPackage.CrosshairsTop = EquippedGun->CrossHairsTop;
				HUDPackage.CrosshairsBottom = EquippedGun->CrossHairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			float Velocity = Character->GetVelocity().Size2D();
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity);

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 3.f, DeltaTime, 1.5f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0, DeltaTime, 20.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 30.f);
				CrosshairVelocityFactor = 0.f;
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, 20.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			GunslingerHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedGun == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedGun->GetZoomedFOV(), DeltaTime, EquippedGun->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetCamera())
	{
		Character->GetCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	ServerSetAiming(bIsAiming);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	 
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedGun == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, EquippedGun->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if (EquippedGun == nullptr) return;

	if (bFireButtonPressed && EquippedGun->bAutomatic)
	{
		Fire();
	}
	if (EquippedGun->IsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedGun == nullptr) return false;
	return !EquippedGun->IsEmpty() && bCanFire && CombatState == ECombatState::Unoccuiped;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::Shotgun, StartingShotgunAmmo);
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedGun == nullptr) return;
	CombatState = ECombatState::Reloading;
	HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::Unoccuiped;
		UpdateAmmoValue();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedGun == nullptr) return 0;
	
	int32 RoomInMag = EquippedGun->GetMagCapacity() - EquippedGun->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedGun->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedGun->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::UpdateAmmoValue()
{
	if (Character == nullptr || EquippedGun == nullptr) return;

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedGun->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedGun->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedGun->GetWeaponType()];
	}
	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedGun->AddAmmo(-ReloadAmount);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::Reloading:
		HandleReload();
		break;

	case ECombatState::Unoccuiped:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())	
	{
		ServerFire(HitTarget);
		if (EquippedGun)
		{
			bCanFire = false;
			CrosshairShootingFactor = 3.f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedGun == nullptr) return;

	if (Character && CombatState == ECombatState::Unoccuiped)
	{
		Character->PlayFireMontage(bAiming);
		EquippedGun->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2D CrosshairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::EquipGun(AGun* GunToEquip)
{
	if (Character == nullptr || GunToEquip == nullptr) return;
	if (EquippedGun)
	{
		EquippedGun->Dropped();
	}
	EquippedGun = GunToEquip;
	EquippedGun->SetWeaponState(EWeaponState::Equipped);
	const USkeletalMeshSocket* HandSocket =  Character->GetMesh()->GetSocketByName(FName("RHSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedGun, Character->GetMesh());
	}
	EquippedGun->SetOwner(Character);
	EquippedGun->SetHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedGun->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedGun->GetWeaponType()];
	}
	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerController->SetHUDCarriedAmmo(CarriedAmmo);
		FString WeaponTypeName = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)EquippedGun->GetWeaponType());
		GunslingerController->SetHUDWeaponName(*WeaponTypeName);
	}
	if (EquippedGun->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedGun->EquipSound, Character->GetActorLocation());
	}
	if (EquippedGun->IsEmpty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedGun()
{
	if (EquippedGun && Character)
	{
		EquippedGun->SetWeaponState(EWeaponState::Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RHSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedGun, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		if (EquippedGun->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedGun->EquipSound, Character->GetActorLocation());
		}
		GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : GunslingerController;
		if (GunslingerController)
		{
			FString WeaponTypeName = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)EquippedGun->GetWeaponType());
			GunslingerController->SetHUDWeaponName(*WeaponTypeName);
		}
	}
}