// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "Gun.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Initial UMETA(DisplayName = "Initial State"),
	Equipped UMETA(DisplayName = "Equipped"),
	Dropped UMETA(DisplayName = "Dropped"),

	MAX UMETA(DisplayeName = "DefaultMAX")
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
	void Dropped();
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

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* EquipSound;


protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


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

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ACasing>	CasingClass;

	//Zoomed FOV while aiming
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditAnywhere, Category = "Weapon")
	int32 Ammo;

	void SpendRound();

	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 MagCapacity;

	UPROPERTY()
	class AGunslinger* OwnerCharacter;
	UPROPERTY()
	class AGunslingerPlayerController* OwnerController;

	UPROPERTY(EditDefaultsOnly)
	EWeaponType WeaponType;


public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USkeletalMeshComponent* GetGunMesh() const { return GunMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
