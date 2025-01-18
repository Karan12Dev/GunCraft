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

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);


protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedGun();

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();


private:
	UPROPERTY()
	class AGunslinger* Character;

	UPROPERTY()
	class AGunslingerPlayerController* GunslingerController;

	UPROPERTY()
	class AGunslingerHUD* GunslingerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedGun)
	AGun* EquippedGun;

	UPROPERTY(Replicated)
	bool bAiming;

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

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingARAmmo = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingRocketAmmo = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingPistolAmmo = 12;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingSMGAmmo = 24;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 StartingShotgunAmmo = 0;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::Unoccuiped;

	UFUNCTION()
	void OnRep_CombatState();
};