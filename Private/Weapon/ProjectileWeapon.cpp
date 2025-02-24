// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetGunMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World && InstigatorPawn)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetGunMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority()) // on Server
			{
				if (InstigatorPawn->IsLocallyControlled())	// server, locally controlling // use replicated projectile
				{
					if (ProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						if (SpawnedProjectile)
						{
							SpawnedProjectile->bUseServerSideRewind = false;
							SpawnedProjectile->Damage = Damage;
						}
					}
				}
				else // on server, not locally controlling ->Client // spawn non-replicated projectle, no SSR
				{
					if (ServerSideRewindProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						if (SpawnedProjectile)
						{
							SpawnedProjectile->bUseServerSideRewind = true;
						}
					}
				}
			}
			else // Client, using SSR
			{
				if (InstigatorPawn->IsLocallyControlled())	// client, locally controlled - spawn non-replicated projectile, use SSR
				{
					if (ServerSideRewindProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						if (SpawnedProjectile)
						{
							SpawnedProjectile->bUseServerSideRewind = true;
							SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
							SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialProjectileSpeed;
							SpawnedProjectile->Damage = Damage;
						}
					}
				}
				else // Client, not locally controller - spawn non-replicated projectile, no SSR
				{
					if (ServerSideRewindProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						if (SpawnedProjectile)
						{
							SpawnedProjectile->bUseServerSideRewind = false;
						}
					}
				}
			}
		}
		else // weapon not using SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				if (ProjectileClass)
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					if (SpawnedProjectile)
					{
						SpawnedProjectile->bUseServerSideRewind = false;
						SpawnedProjectile->Damage = Damage;
					}
				}
			}
		}
	}
}
