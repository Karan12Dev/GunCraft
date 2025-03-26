// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "GunslingerTypes/Team.h"
#include "Gun.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Initial UMETA(DisplayName = "Initial State"),
	Equipped UMETA(DisplayName = "Equipped"),
	EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	Dropped UMETA(DisplayName = "Dropped"),

	MAX UMETA(DisplayeName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	Projectile UMETA(DisplayName = "Projectile Weapon"),
	Shotgun UMETA (DisplayName = "Shotgun Weapon"),

	MAX UMETA(DisplayName = "Default MAX")
};

UCLASS()
class GUNCRAFT_API AGun : public AActor
{ 
	GENERATED_BODY()
	
public:	
	AGun();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();
	void AddAmmo(int32 AmmoToAdd);

	//Textures for the weapon crosshair
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	class UTexture2D* CrossHairsCenter;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrossHairsLeft;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrossHairsRight;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrossHairsTop;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrossHairsBottom;

	//AutomaticFire
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireDelay = .15f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAutomatic = true;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class USoundCue* EquipSound;

	//	Enable or Disable custom depth
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EFireType FireType;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bUseScatter = false;

	FVector TraceEndWithScatter(const FVector& HitTarget);


protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//	Trace end with scatter
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float SphereRadius = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HeadShotDamage = 50.f;

	UPROPERTY(Replicated ,EditDefaultsOnly, Category = "Weapon")
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class AGunslinger* OwnerCharacter;

	UPROPERTY()
	class AGunslingerPlayerController* OwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);


private:
	UPROPERTY(VisibleAnywhere, Category = "Properties")
	USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere, Category = "Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleInstanceOnly, Category = "Weapon")
	EWeaponState WeaponState;
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class ACasing>	CasingClass;

	//Zoomed FOV while aiming
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 MagCapacity;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "Properties")
	ETeam Team;

	//	The number of unprocessed server requests for Ammo.
	//	Incremented in SpendRound, decremented in ClientUpdateAmmo.
	int32 Sequence = 0;


public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USkeletalMeshComponent* GetGunMesh() const { return GunMesh; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE ETeam GetTeam() const { return Team; }
};
