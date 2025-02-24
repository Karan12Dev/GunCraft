// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/GunslingerHUD.h"
#include "Weapon/WeaponTypes.h"
#include "GunslingerTypes/CombatState.h"
#include "CombatComponent.generated.h"


class AGun;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GUNCRAFT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	friend class AGunslinger;
	void EquipGun(AGun* GunToEquip);
	void SwapWeapons();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmmount);

	bool bLocallyReloading = false;


protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedGun();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();


private:
	void ThrowGrenade();
	void AttachActorToSocket(AActor* ActorToAttach, FName SocketName);
	void UpdateCarriedAmmoAndWeaponName();
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AGun* WeaponToEquip);
	void EquipSecondaryWeapon(AGun* WeaponToEquip);

	UPROPERTY()
	class AGunslinger* Character;

	UPROPERTY()
	class AGunslingerPlayerController* GunslingerController;

	UPROPERTY()
	class AGunslingerHUD* GunslingerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedGun)
	AGun* EquippedGun;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AGun* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float BaseWalkSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AimWalkSpeed = 180.f;

	bool bFireButtonPressed;

	//HUD and Crosshairs
	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	//Aiming and FOV
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	//Automatic Fire
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	void Reload();
	int32 AmountToReload();	
	void UpdateAmmoValue();
	void UpdateShotgunAmmoValues();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingARAmmo = 60;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingRocketAmmo = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingPistolAmmo = 40;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingSMGAmmo = 80;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingShotgunAmmo = 24;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingSniperRifleAmmo = 12;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingGrenadeLauncherAmmo = 10;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::Unoccuiped;

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AProjectile> GrenadeClass;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();


public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapons();
};