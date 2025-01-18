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

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	PickupWidget->SetupAttachment(GetRootComponent());
}

void AGun::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}
}

void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGun, WeaponState);
	DOREPLIFETIME(AGun, Ammo);
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
		SetHUDAmmo();
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

void AGun::Dropped()
{
	SetWeaponState(EWeaponState::Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	GunMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AGun::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
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

void AGun::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::Equipped:
		ShowPickupWidget(false);
		GunMesh->SetSimulatePhysics(false);
		GunMesh->SetEnableGravity(false);
		GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::SMG)
		{
			GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GunMesh->SetEnableGravity(true);
			GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;

	case EWeaponState::Dropped:
		GunMesh->SetSimulatePhysics(true);
		GunMesh->SetEnableGravity(true);
		GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		GunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		GunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}
}

void AGun::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::Equipped:
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
		break;

	case EWeaponState::Dropped:
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
		break;
	}
}

bool AGun::IsEmpty()
{
	return Ammo <= 0;
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
}

void AGun::OnRep_Ammo()
{
	SetHUDAmmo();
}