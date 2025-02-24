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
#include "Character/GunslingerAnimInstance.h"
#include "Weapon/Projectile.h"
#include "Weapon/Shotgun.h"


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
	DOREPLIFETIME(ThisClass, SecondaryWeapon)
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(ThisClass, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(ThisClass, CombatState);
	DOREPLIFETIME(ThisClass, Grenades);
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
	if (Character == nullptr || EquippedGun == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	if (Character->IsLocallyControlled() && EquippedGun->GetWeaponType() == EWeaponType::SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	 
	if (bFireButtonPressed)
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedGun == nullptr) return false;
	if (!EquippedGun->IsEmpty() && bCanFire && CombatState == ECombatState::Reloading && EquippedGun->GetWeaponType() == EWeaponType::Shotgun) return true;
	if (bLocallyReloading) return false;
	return !EquippedGun->IsEmpty() && bCanFire && CombatState == ECombatState::Unoccuiped;
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		if (EquippedGun)
		{
			bCanFire = false;
			CrosshairShootingFactor = 3.f;

			switch (EquippedGun->FireType)
			{
			case EFireType::HitScan:
				FireHitScanWeapon();
				break;

			case EFireType::Projectile:
				FireProjectileWeapon();
				break;

			case EFireType::Shotgun:
				FireShotgun();
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedGun)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedGun->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedGun)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedGun->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedGun == nullptr) return;
	if (Character && CombatState == ECombatState::Unoccuiped)
	{
		Character->PlayFireMontage(bAiming);
		EquippedGun->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedGun);
	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::Reloading || CombatState == ECombatState::Unoccuiped)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::Unoccuiped;
		bLocallyReloading = false;
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedGun && Character)
	{
		HitTarget = EquippedGun->bUseScatter ? EquippedGun->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedGun->FireDelay);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedGun && Character)
	{
		HitTarget = EquippedGun->bUseScatter ? EquippedGun->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedGun->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedGun);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedGun->FireDelay);
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

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] += AmmoAmmount;
		UpdateCarriedAmmoAndWeaponName();
	}
	if (EquippedGun && EquippedGun->IsEmpty() && EquippedGun->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	bool bJumToShotgunEnd = CombatState == ECombatState::Reloading && EquippedGun != nullptr && EquippedGun->GetWeaponType() == EWeaponType::Shotgun && CarriedAmmo == 0;
	if (bJumToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::Unoccuiped && EquippedGun && !EquippedGun->IsFull() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedGun == nullptr) return;
	CombatState = ECombatState::Reloading;
	if (!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false;
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

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::Unoccuiped;
	}
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	EquippedGun->SetWeaponState(EWeaponState::Equipped);
	AttachActorToSocket(EquippedGun, FName("RHSocket"));
	EquippedGun->SetHUDAmmo();
	UpdateCarriedAmmoAndWeaponName();
	if (EquippedGun->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedGun->EquipSound, Character->GetActorLocation());
	}
	SecondaryWeapon->SetWeaponState(EWeaponState::EquippedSecondary);
	AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
}

void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades <= 0) return;
	if (CombatState != ECombatState::Unoccuiped || EquippedGun == nullptr) return;
	CombatState = ECombatState::ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToSocket(EquippedGun, FName("LHSocket"));
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades <= 0) return;
	CombatState = ECombatState::ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToSocket(EquippedGun, FName("LHSocket"));
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->Controller) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerController->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::Unoccuiped;
	AttachActorToSocket(EquippedGun, FName("RHSocket"));
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParms;
		SpawnParms.Owner = Character;
		SpawnParms.Instigator = Character;
		if (UWorld* World = GetWorld())
		{
			World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParms);
		}
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
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
	EquippedGun->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedGun == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedGun->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedGun->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedGun->GetWeaponType()];
	}
	GunslingerController = GunslingerController == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : GunslingerController;
	if (GunslingerController)
	{
		GunslingerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedGun->AddAmmo(1);

	bCanFire = true;
	if (EquippedGun->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::Reloading:
		if (Character && !Character->IsLocallyControlled()) HandleReload();
		break;

	case ECombatState::Unoccuiped:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;

	case ECombatState::ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToSocket(EquippedGun, FName("LHSocket"));
			ShowAttachedGrenade(true);
		}
		break;

	case ECombatState::SwappingWeapon:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapWeaponMontage();
		}
		break;
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

void UCombatComponent::EquipPrimaryWeapon(AGun* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	if (EquippedGun)
	{
		EquippedGun->Dropped();
	}
	EquippedGun = WeaponToEquip;
	EquippedGun->SetWeaponState(EWeaponState::Equipped);
	AttachActorToSocket(EquippedGun, FName("RHSocket"));
	EquippedGun->SetOwner(Character);
	EquippedGun->SetHUDAmmo();
	UpdateCarriedAmmoAndWeaponName();
	if (EquippedGun->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedGun->EquipSound, Character->GetActorLocation());
	}
	if (EquippedGun->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::EquipSecondaryWeapon(AGun* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EquippedSecondary);
	AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
	SecondaryWeapon->SetOwner(Character);
	if (SecondaryWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SecondaryWeapon->EquipSound, Character->GetActorLocation());
	}
}

void UCombatComponent::EquipGun(AGun* GunToEquip)
{
	if (Character == nullptr || GunToEquip == nullptr) return;
	if (CombatState != ECombatState::Unoccuiped) return;
	if (EquippedGun != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(GunToEquip);
	}
	else
	{
		EquipPrimaryWeapon(GunToEquip);
	}
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedGun()
{
	if (EquippedGun && Character)
	{
		EquippedGun->SetWeaponState(EWeaponState::Equipped);
		AttachActorToSocket(EquippedGun, FName("RHSocket"));
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
		EquippedGun->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EquippedSecondary);
		AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
		if (SecondaryWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SecondaryWeapon->EquipSound, Character->GetActorLocation());
		}
	}
}

void UCombatComponent::AttachActorToSocket(AActor* ActorToAttach, FName SocketName)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedGun == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::Unoccuiped || Character == nullptr) return;

	Character->PlaySwapWeaponMontage();
	CombatState = ECombatState::SwappingWeapon;

	AGun* TempWeapon = EquippedGun;
	EquippedGun = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedGun != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::UpdateCarriedAmmoAndWeaponName()
{
	if (EquippedGun == nullptr) return;
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
}
