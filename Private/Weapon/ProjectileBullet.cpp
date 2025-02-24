// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Gunslinger.h"
#include "PlayerController/GunslingerPlayerController.h"
#include "GunslingerComponents/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectle Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialProjectileSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialProjectileSpeed;
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*FPredictProjectilePathParams PathParms;
	PathParms.bTraceWithChannel = true;
	PathParms.bTraceWithCollision = true;
	PathParms.DrawDebugTime = 5.f;
	PathParms.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParms.LaunchVelocity = GetActorForwardVector() * InitialProjectileSpeed;
	PathParms.MaxSimTime = 4.f;
	PathParms.ProjectileRadius = 5.f;
	PathParms.SimFrequency = 5.f;
	PathParms.StartLocation = GetActorLocation();
	PathParms.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParms.ActorsToIgnore.Add(this);
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParms, PathResult);*/
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialProjectileSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialProjectileSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialProjectileSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpluse, const FHitResult& Hit)
{
	AGunslinger* OwnerCharacter = Cast<AGunslinger>(GetOwner());
	if (OwnerCharacter)
	{
		AGunslingerPlayerController* OwnerController = Cast<AGunslingerPlayerController>(OwnerCharacter->GetController());
		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpluse, Hit);
				return;
			}
			AGunslinger* HitCharacter = Cast<AGunslinger>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreReequest(HitCharacter, TraceStart, InitialVelocity, OwnerController->GetServerTime() - OwnerController->SingleTripTime);
			}
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpluse, Hit);
}
