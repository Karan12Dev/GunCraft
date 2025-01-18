// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Gunslinger.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Gun.h"
#include "GunslingerComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/GunslingerAnimInstance.h"
#include "GunCraft/GunCraft.h"
#include "PlayerController/GunslingerPlayerController.h"
#include "GameMode/GunslingerGameMode.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerState/GunslingerPlayerState.h"
#include "Weapon/WeaponTypes.h"


AGunslinger::AGunslinger()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	OverHeadWidget->SetupAttachment(GetRootComponent());

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	TurningInPlace = ETurningInPlace::NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AGunslinger::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		InitializeInput();
	}

	Health = MaxHealth;
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void AGunslinger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	HideCameraIfCharacterClose();
	PollInit();
}

void AGunslinger::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeInput();
}

void AGunslinger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AGunslinger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGunslinger, OverlappingGun, COND_OwnerOnly);
	DOREPLIFETIME(AGunslinger, MinRunningSpeed);
	DOREPLIFETIME(AGunslinger, MaxRunningSpeed);
	DOREPLIFETIME(AGunslinger, Health);
	DOREPLIFETIME(ThisClass, bDisableGameplay);
}

void AGunslinger::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	AGunslingerGameMode* GunslingerGameMode = Cast<AGunslingerGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = GunslingerGameMode && GunslingerGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedGun && bMatchNotInProgress)
	{
		Combat->EquippedGun->Destroy();
	}
}

void AGunslinger::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ThisClass::Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ThisClass::StopRun);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ThisClass::Equip);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::StartCrouch);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::Disengage);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::Fire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::StopFire);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::Reload);
	}
}

void AGunslinger::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AGunslinger::Elim()
{
	if (Combat && Combat->EquippedGun)
	{
		Combat->EquippedGun->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
}

void AGunslinger::MulticastElim_Implementation()
{
	if (GunslingerPlayerController)
	{
		GunslingerPlayerController->SetHUDWeaponAmmo(0);
		GunslingerPlayerController->SetHUDWeaponName(FName(" "));
	}
	bElimmed = true;
	PlayElimMontage();

	if (DissloveMaterialInstance)
	{
		DynamicDissloveMaterialInstance = UMaterialInstanceDynamic::Create(DissloveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissloveMaterialInstance);
		DynamicDissloveMaterialInstance->SetScalarParameterValue(TEXT("DissolveValue"), -0.55f);
		DynamicDissloveMaterialInstance->SetScalarParameterValue(TEXT("GlowIntensity"), 100.f);
	}
	StartDissolve();

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn Elim Bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
		if (ElimBotSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ElimBotSound, ElimBotSpawnPoint);
		}
	}
}

void AGunslinger::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
		AnimInstance->Montage_JumpToSection(FName("Elim"), ElimMontage);
	}
}

void AGunslinger::ElimTimerFinished()
{
	AGunslingerGameMode* GunslingerGameMode = GetWorld()->GetAuthGameMode<AGunslingerGameMode>();
	if (GunslingerGameMode)
	{
		GunslingerGameMode->RequestRespawn(this, GetController());
	}
}

void AGunslinger::UpdateDissolveMaterial(float DissloveValue)
{
	if (DynamicDissloveMaterialInstance)
	{
		DynamicDissloveMaterialInstance->SetScalarParameterValue(TEXT("DissolveValue"), DissloveValue);
	}
}

void AGunslinger::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AGunslinger::UpdateHUDHealth()
{
	GunslingerPlayerController = GunslingerPlayerController == nullptr ? Cast<AGunslingerPlayerController>(GetController()) : GunslingerPlayerController;
	if (GunslingerPlayerController)
	{
		GunslingerPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AGunslinger::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedGun == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleIronsight") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName, FireWeaponMontage);
	}
}

void AGunslinger::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedGun == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedGun->GetWeaponType())
		{
		case EWeaponType::AssaultRifle:
			SectionName = FName("AssaultRifleReload");
			break;

		case EWeaponType::RocketLauncher:
			SectionName = FName("AssaultRifleReload");
			break;

		case EWeaponType::Pistol:
			SectionName = FName("AssaultRifleReload");
			break;

		case EWeaponType::SMG:
			SectionName = FName("AssaultRifleReload");
			break;

		case EWeaponType::Shotgun:
			SectionName = FName("AssaultRifleReload");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, ReloadMontage);
	}
}

void AGunslinger::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedGun == nullptr) return;
	if (bElimmed) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName;
		SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AGunslinger::InitializeInput()
{
	GunslingerPlayerController = GunslingerPlayerController == nullptr ? Cast<AGunslingerPlayerController>(GetController()) : GunslingerPlayerController;
	if (GunslingerPlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GunslingerPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(GunslingerContext, 0);
		}
	}
}

void AGunslinger::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AGunslinger::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedGun == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation ,StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::NotTurning;
	}

	CalculateAO_Pitch();
}

void AGunslinger::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AGunslinger::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedGun == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::NotTurning;
}

void AGunslinger::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::Right;
	}
	else if (AO_Yaw < -90)
	{
		TurningInPlace = ETurningInPlace::Left;
	}
	if (TurningInPlace != ETurningInPlace::NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 8.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AGunslinger::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	const FVector2D MovementAxis = Value.Get<FVector2D>();

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	const FVector YawDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(YawDirection, MovementAxis.X);

	const FVector PitchDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(PitchDirection, MovementAxis.Y);
}

void AGunslinger::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerPitchInput(LookAxis.Y);
		AddControllerYawInput(LookAxis.X);
	}
}

void AGunslinger::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		Super::UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AGunslinger::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		AGunslingerGameMode* GunslingerGameMode = GetWorld()->GetAuthGameMode<AGunslingerGameMode>();
		if (GunslingerGameMode)
		{
			GunslingerPlayerController = GunslingerPlayerController == nullptr ? Cast<AGunslingerPlayerController>(GetController()) : GunslingerPlayerController;
			AGunslingerPlayerController* AttackerController = Cast<AGunslingerPlayerController>(InstigatorController);
			GunslingerGameMode->PlayerEliminated(this, GunslingerPlayerController, AttackerController);
		}
	}
}

void AGunslinger::PollInit()
{
	if (GunslingerPlayerState == nullptr)
	{
		GunslingerPlayerState = GetPlayerState<AGunslingerPlayerState>();
		if (GunslingerPlayerState)
		{
			GunslingerPlayerState->AddToScore(0.f);
			GunslingerPlayerState->AddToDefeats(0);
		}
	}
}

void AGunslinger::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

float AGunslinger::CalculateSpeed()
{
	return GetVelocity().Size2D();
}

void AGunslinger::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((Camera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedGun && Combat->EquippedGun->GetGunMesh())
		{
			Combat->EquippedGun->GetGunMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedGun && Combat->EquippedGun->GetGunMesh())
		{
			Combat->EquippedGun->GetGunMesh()->bOwnerNoSee = false;
		}
	}
}

void AGunslinger::Run()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxRunningSpeed;
	ClientRun();
}

void AGunslinger::StopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = MinRunningSpeed;
	ClientStopRun();
}

void AGunslinger::ClientRun_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxRunningSpeed;
}

void AGunslinger::ClientStopRun_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = MinRunningSpeed;
}

void AGunslinger::Equip()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipGun(OverlappingGun);
		}
		else
		{
			ServerEquip();
		}
	}
}

void AGunslinger::ServerEquip_Implementation()
{
	if (Combat)
	{
		Combat->EquipGun(OverlappingGun);
	}
}

void AGunslinger::OnRep_OverlappingGun(AGun* LastGun)
{
	if (OverlappingGun)
	{
		OverlappingGun->ShowPickupWidget(true);
	}
	if (LastGun)
	{
		LastGun->ShowPickupWidget(false);
	}
}

void AGunslinger::StartCrouch()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AGunslinger::Aim()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AGunslinger::Disengage()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AGunslinger::Fire()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AGunslinger::StopFire()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AGunslinger::Reload()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void AGunslinger::SetOverlappingGun(AGun* Gun)
{
	if (IsLocallyControlled())
	{
		if (OverlappingGun)
		{
			OverlappingGun->ShowPickupWidget(false);
		}
	}

	OverlappingGun = Gun;

	if (IsLocallyControlled())
	{
		if (OverlappingGun)
		{
			OverlappingGun->ShowPickupWidget(true);
		}
	}
}

bool AGunslinger::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedGun);
}

bool AGunslinger::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AGun* AGunslinger::GetEquippedGun()
{
	if (Combat == nullptr) return nullptr;

	return Combat->EquippedGun;
}

FVector AGunslinger::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState AGunslinger::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::MAX;
	return Combat->CombatState;
}
