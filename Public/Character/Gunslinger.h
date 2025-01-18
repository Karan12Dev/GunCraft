// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GunslingerTypes/TurningInPlace.h"
#include "Interfaces/CrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "GunslingerTypes/CombatState.h"
#include "Gunslinger.generated.h"

class UInputAction;
class UAnimMontage;

UCLASS()
class GUNCRAFT_API AGunslinger : public ACharacter, public ICrosshairsInterface
{
	GENERATED_BODY()

public:
	AGunslinger();
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override; //Only called on Server
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void Elim();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;


protected:
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	virtual void Jump() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void PollInit();


private:
	float CalculateSpeed();
	void UpdateHUDHealth();
	void SimProxiesTurn();
	void PlayElimMontage();
	void HideCameraIfCharacterClose();
	void PlayHitReactMontage();
	void InitializeInput();
	void RotateInPlace(float DeltaTime);

	UPROPERTY()
	class AGunslingerPlayerController* GunslingerPlayerController;

	UPROPERTY()
	class AGunslingerPlayerState* GunslingerPlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

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

	UPROPERTY(Replicated, EditDefaultsOnly)
	float MinRunningSpeed = 250;

	UPROPERTY(Replicated, EditDefaultsOnly)
	float MaxRunningSpeed = 700;

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

	UPROPERTY(EditDefaultsOnly)
	float CameraThreshold = 100.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	//Health
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleInstanceOnly, Category = "Player Stats")
	float Health;

	UFUNCTION()
	void OnRep_Health();

	//Elimination
	bool bElimmed = false;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	FTimerHandle ElimTimer;
	void ElimTimerFinished();

	//Dissolve Effect
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

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UMaterialInstance* DissloveMaterialInstance;

	//Elim Bot
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	class USoundBase* ElimBotSound;


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
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
};
