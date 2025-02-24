// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Gun.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/Gunslinger.h"
#include "PlayerController/GunslingerPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Weapon/Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GunslingerComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"


AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	RootComponent = GunMesh;
	GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	GunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	GunMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	PickupWidget->SetupAttachment(GetRootComponent());
}

void AGun::EnableCustomDepth(bool bEnable)
{
	if (GunMesh)
	{
		GunMesh->SetRenderCustomDepth(bEnable);
	}
}

void AGun::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
}

void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGun, WeaponState);
	DOREPLIFETIME_CONDITION(AGun, bUseServerSideRewind, COND_OwnerOnly);
}

void AGun::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<AGunslinger>(GetOwner()) : OwnerCharacter;
		if (OwnerCharacter && OwnerCharacter->GetEquippedGun() && OwnerCharacter->GetEquippedGun() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AGun::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget) 
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AGun::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		GunMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = GunMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GunMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

FVector AGun::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetGunMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetGunMesh());
	const FVector Start = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - Start;

	return FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AGun::Dropped()
{
	SetWeaponState(EWeaponState::Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	GunMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AGun::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGunslinger* Gunslinger = Cast<AGunslinger>(OtherActor);
	if (Gunslinger)
	{
		Gunslinger->SetOverlappingGun(this);
	}
}

void AGun::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AGunslinger* Gunslinger = Cast<AGunslinger>(OtherActor);
	if (Gunslinger)
	{
		Gunslinger->SetOverlappingGun(nullptr);
	}
}

void AGun::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::Equipped:
		OnEquipped();
		break;

	case EWeaponState::EquippedSecondary:
		OnEquippedSecondary();
		break;

	case EWeaponState::Dropped:
		OnDropped();
		break;
	}
}

void AGun::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetSimulatePhysics(false);
	GunMesh->SetEnableGravity(false);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::SMG)
	{
		GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GunMesh->SetEnableGravity(true);
		GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AGunslinger>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && bUseServerSideRewind)
	{
		OwnerController = OwnerController == nullptr ? Cast<AGunslingerPlayerController>(OwnerCharacter->GetController()) : OwnerController;
		if (OwnerController && HasAuthority() && !OwnerController->HighPingDelegate.IsBound())
		{
			OwnerController->HighPingDelegate.AddDynamic(this, &AGun::OnPingTooHigh);
		}
	}
}

void AGun::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetSimulatePhysics(false);
	GunMesh->SetEnableGravity(false);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::SMG)
	{
		GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GunMesh->SetEnableGravity(true);
		GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AGunslinger>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && bUseServerSideRewind)
	{
		OwnerController = OwnerController == nullptr ? Cast<AGunslingerPlayerController>(OwnerCharacter->GetController()) : OwnerController;
		if (OwnerController && HasAuthority() && OwnerController->HighPingDelegate.IsBound())
		{
			OwnerController->HighPingDelegate.RemoveDynamic(this, &AGun::OnPingTooHigh);
		}
	}
}

void AGun::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	GunMesh->SetSimulatePhysics(true);
	GunMesh->SetEnableGravity(true);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	GunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GunMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	GunMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AGunslinger>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && bUseServerSideRewind)
	{
		OwnerController = OwnerController == nullptr ? Cast<AGunslingerPlayerController>(OwnerCharacter->GetController()) : OwnerController;
		if (OwnerController && HasAuthority() && OwnerController->HighPingDelegate.IsBound())
		{
			OwnerController->HighPingDelegate.RemoveDynamic(this, &AGun::OnPingTooHigh);
		}
	}
}

void AGun::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AGun::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AGun::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

bool AGun::IsEmpty()
{
	return Ammo <= 0;
}

bool AGun::IsFull()
{
	return Ammo == MagCapacity;
}

void AGun::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AGunslinger>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ? Cast<AGunslingerPlayerController>(OwnerCharacter->GetController()) : OwnerController;
		if (OwnerController)
		{
			OwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AGun::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		++Sequence;
	}
}

void AGun::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AGun::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AGun::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AGunslinger>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && OwnerCharacter->GetCombat() && IsFull())
	{
		OwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}