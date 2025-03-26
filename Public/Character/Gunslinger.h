// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GunslingerTypes/TurningInPlace.h"
#include "Interfaces/CrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "GunslingerTypes/CombatState.h"
#include "GunslingerTypes/Team.h"
#include "Gunslinger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class UInputAction;
class UAnimMontage;
class UBoxComponent;

UCLASS()
class GUNCRAFT_API AGunslinger : public ACharacter, public ICrosshairsInterface
{
	GENERATED_BODY()

public:
	AGunslinger();
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override; //Only called on Server
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapWeaponMontage();
	void UpdateHUDHealth();
	void UpdateHUDSheild();
	void UpdateHUDAmmo();

	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

	UPROPERTY(Replicated, EditDefaultsOnly)
	float MinRunningSpeed = 250;

	UPROPERTY(Replicated, EditDefaultsOnly)
	float MaxRunningSpeed = 700;

	float InitialMaxRunningSpeed = 700;


protected:
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	virtual void Jump() override;
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void PollInit();

	//	Hit Boxes used for server side rewind
	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* head;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* pelvis;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* spine_02;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* spine_03;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* upperarm_r;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* upperarm_l;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* hand_r;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* hand_l;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* backpack;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* blanket;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* thigh_r;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* thigh_l;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* calf_r;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* calf_l;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* foot_r;

	UPROPERTY(EditDefaultsOnly, Category = "Hit Box")
	UBoxComponent* foot_l;


private:
	float CalculateSpeed();
	void SimProxiesTurn();
	void PlayElimMontage();
	void HideCharacterIfCameraClose();
	void PlayHitReactMontage();
	void InitializeInput();
	void RotateInPlace(float DeltaTime);

	UPROPERTY()
	class AGunslingerPlayerController* GunslingerPlayerController;

	UPROPERTY()
	class AGunslingerPlayerState* GunslingerPlayerState;

	// Gunslinger Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;
	//\\

	UPROPERTY()
	class AGunslingerGameMode* GunslingerGameMode;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GunslingerContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MovementAction;
	void Move(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RunAction;
	void Run();
	void StopRun();

	UFUNCTION(Server, Reliable)
	void ClientRun();
	UFUNCTION(Server, Reliable)
	void ClientStopRun();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* EquipAction;
	void Equip();

	UFUNCTION(Server, Reliable)
	void ServerEquip();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CrouchAction;
	void StartCrouch();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* AimAction;
	void Aim();
	void Disengage();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FireAction;
	void Fire();
	void StopFire();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ReloadAction;
	void Reload();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ThrowAction;
	void Throw();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
	class UWidgetComponent* OverHeadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingGun)
	class AGun* OverlappingGun;
	UFUNCTION()
	void OnRep_OverlappingGun(AGun* LastGun);

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category = "Combat Anim")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Anim")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Anim")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Anim")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Anim")
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Anim")
	UAnimMontage* SwapWeaponMontage;

	UPROPERTY(EditDefaultsOnly)
	float CameraThreshold = 100.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	//	Health
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 0.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	//	Sheild
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxSheild = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Sheild, VisibleAnywhere, Category = "Player Stats")
	float Sheild = 0.f;

	UFUNCTION()
	void OnRep_Sheild(float LastSheild);

	//	Elimination
	bool bElimmed = false;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	FTimerHandle ElimTimer;
	void ElimTimerFinished();

	bool bLeftGame = false;

	//	Dissolve Effect
	UPROPERTY(VisibleDefaultsOnly, Category = "Elim")
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissloveValue);
	void StartDissolve();

	UPROPERTY(VisibleDefaultsOnly, Category = "Elim")
	UMaterialInstanceDynamic* DynamicDissloveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UMaterialInstance* DissloveMaterialInstance;

	//	Team Colors
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UMaterialInstance* OriginalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UMaterialInstance* RedDissloveMatInst;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UMaterialInstance* BlueDissloveMatInst;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UMaterialInstance* BlueMaterial;

	//	Elim Effect
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	class USoundBase* ElimBotSound;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* AttachedGrenade;

	//	Default Weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AGun> DefaultWeaponClass;

	void SpawnDefaultWeapon();

	FTimerHandle UpdateHUDTimer;
	void UpdateHUD();


public:
	void SetOverlappingGun(AGun* Gun);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AGun* GetEquippedGun();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone;	}
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float HealAmount) { Health = FMath::Clamp(Health + HealAmount, 0, MaxHealth); }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetSheild() const { return Sheild; }
	FORCEINLINE void SetSheild(float ReplenishAmmount) { Sheild = FMath::Clamp(Sheild + ReplenishAmmount, 0, MaxSheild); }
	FORCEINLINE float GetMaxSheild() const { return MaxSheild; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	bool IsHoldingTheFlag() const;
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);
};
