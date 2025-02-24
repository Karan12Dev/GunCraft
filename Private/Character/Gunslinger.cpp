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
#include "GameState/GunslingerGameState.h"
#include "Weapon/WeaponTypes.h"
#include "GunslingerComponents/BuffComponent.h"
#include "Components/BoxComponent.h"
#include "GunslingerComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


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
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

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

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff Component"));
	BuffComponent->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("Lag Compenstation"));

	TurningInPlace = ETurningInPlace::NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//	Hit boxes for server side rewind
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AGunslinger::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		InitializeInput();
		UpdateHUDHealth();
		UpdateHUDSheild();
	}
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
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

void AGunslinger::SpawnDefaultWeapon()
{
	AGunslingerGameMode* GunslingerGameMode = Cast<AGunslingerGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (GunslingerGameMode && World && !bElimmed && DefaultWeaponClass && Combat)
	{
		AGun* StartingWeapon = World->SpawnActor<AGun>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		Combat->EquipGun(StartingWeapon);
	}
}

void AGunslinger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
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

			if (AGunslingerGameState* GunslingerGameState = Cast<AGunslingerGameState>(UGameplayStatics::GetGameState(this)))
			{
				if (GunslingerGameState->TopScoringPlayers.Contains(GunslingerPlayerState))
				{
					MulticastGainedTheLead();
				}
			}
		}
	}
}

void AGunslinger::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeInput();
	Health = MaxHealth;
	UpdateHUDHealth();
	UpdateHUDSheild();
	SpawnDefaultWeapon();
}

void AGunslinger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (GetController())
		{
			LagCompensation->Controller = Cast<AGunslingerPlayerController>(GetController());
		}
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
	DOREPLIFETIME(ThisClass, Sheild);
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
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &ThisClass::Throw);
	}
}

void AGunslinger::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AGunslinger::Elim(bool bPlayerLeftGame)
{
	if (Combat)
	{
		if (Combat->EquippedGun)
		{
			if (Combat->EquippedGun->bDestroyWeapon)
			{
				Combat->EquippedGun->Destroy();
			}
			else
			{
				Combat->EquippedGun->Dropped();
			}
		}
		if (Combat->SecondaryWeapon)
		{
			if (Combat->SecondaryWeapon->bDestroyWeapon)
			{
				Combat->SecondaryWeapon->Destroy();
			}
			else
			{
				Combat->SecondaryWeapon->Dropped();
			}
		}
		
	}
	MulticastElim(bPlayerLeftGame);
}

void AGunslinger::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
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
	if (IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedGun && Combat->EquippedGun->GetWeaponType() == EWeaponType::SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
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
	if (GunslingerGameMode && !bLeftGame)
	{
		GunslingerGameMode->RequestRespawn(this, GetController());
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void AGunslinger::ServerLeaveGame_Implementation()
{
	AGunslingerGameMode* GunslingerGameMode = GetWorld()->GetAuthGameMode<AGunslingerGameMode>();
	GunslingerPlayerState = GunslingerPlayerState == nullptr ? GetPlayerState<AGunslingerPlayerState>() : GunslingerPlayerState;
	if (GunslingerGameMode && GunslingerPlayerState)
	{
		GunslingerGameMode->PlayerLeftGame(GunslingerPlayerState);

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

void AGunslinger::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr && GetMesh())
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetMesh(), FName("CrownSocket"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, false);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void AGunslinger::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
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
			SectionName = FName("ReloadRifle");
			break;

		case EWeaponType::RocketLauncher:
			SectionName = FName("ReloadRocket");
			break;

		case EWeaponType::Pistol:
			SectionName = FName("ReloadPistol");
			break;

		case EWeaponType::SMG:
			SectionName = FName("ReloadRifle");
			break;

		case EWeaponType::Shotgun:
			SectionName = FName("ReloadShotgun");
			break;

		case EWeaponType::SniperRifle:
			SectionName = FName("ReloadSniper");
			break;

		case EWeaponType::GrenadeLauncher:
			SectionName = FName("ReloadGrenadeLauncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, ReloadMontage);
	}
}

void AGunslinger::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void AGunslinger::PlaySwapWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
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
	if (bElimmed) return;
	float DamageToHealth = Damage;
	if (Sheild > 0.f)
	{
		if (Sheild >= Damage)
		{
			Sheild = FMath::Clamp(Sheild - Damage, 0.f, MaxSheild);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Sheild, 0.f, Damage);
			Sheild = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDSheild();
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

void AGunslinger::UpdateHUDHealth()
{
	GunslingerPlayerController = GunslingerPlayerController == nullptr ? Cast<AGunslingerPlayerController>(GetController()) : GunslingerPlayerController;
	if (GunslingerPlayerController)
	{
		GunslingerPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AGunslinger::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AGunslinger::UpdateHUDSheild()
{
	GunslingerPlayerController = GunslingerPlayerController == nullptr ? Cast<AGunslingerPlayerController>(GetController()) : GunslingerPlayerController;
	if (GunslingerPlayerController)
	{
		GunslingerPlayerController->SetHUDSheild(Sheild, MaxSheild);
	}
}

void AGunslinger::UpdateHUDAmmo()
{
	GetWorldTimerManager().SetTimer(UpdateHUDTimer, this, &ThisClass::UpdateHUD, 1.f);
}

void AGunslinger::UpdateHUD()
{
	GunslingerPlayerController = GunslingerPlayerController == nullptr ? Cast<AGunslingerPlayerController>(GetController()) : GunslingerPlayerController;
	if (GunslingerPlayerController && Combat && Combat->EquippedGun)
	{
		GunslingerPlayerController->SetHUDWeaponAmmo(Combat->EquippedGun->GetAmmo());
		Combat->UpdateCarriedAmmoAndWeaponName();
	}
}

void AGunslinger::OnRep_Sheild(float LastSheild)
{
	UpdateHUDSheild();
	if (Sheild < LastSheild)
	{
		PlayHitReactMontage();
	}
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
		if (Combat->CombatState == ECombatState::Unoccuiped) ServerEquip();
		if (Combat->ShouldSwapWeapons() && !HasAuthority() && Combat->CombatState == ECombatState::Unoccuiped && OverlappingGun == nullptr)
		{
			PlaySwapWeaponMontage();
			Combat->CombatState = ECombatState::SwappingWeapon;
		}
	}
}

void AGunslinger::ServerEquip_Implementation()
{
	if (Combat)
	{
		if (OverlappingGun)
		{
			Combat->EquipGun(OverlappingGun);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
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

void AGunslinger::Throw()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
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

bool AGunslinger::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}
