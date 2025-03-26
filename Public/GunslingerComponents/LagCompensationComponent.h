// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AGunslinger* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AGunslinger*, uint32> HeadShots;

	UPROPERTY()
	TMap<AGunslinger*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GUNCRAFT_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AGunslinger;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);
	// for Hitscan weapon
	FServerSideRewindResult ServerSideRewind(class AGunslinger* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	// for projectile weapon
	FServerSideRewindResult ProjectileServerSideRewind(AGunslinger* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	// shotgun
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<AGunslinger*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	void EnableCharacterMeshCollision(AGunslinger* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AGunslinger* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreReequest(AGunslinger* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<AGunslinger*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);


protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTIme);
	void CacheBoxPositions(AGunslinger* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AGunslinger* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AGunslinger* HitCharacter, const FFramePackage& Package);
	FFramePackage GetFrameToCheck(AGunslinger* HitCharacter, float HitTime);
	// Hitscan
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AGunslinger* HitCharacter, const FVector_NetQuantize& TraceStart ,const FVector_NetQuantize& HitLocation);
	// projectile
	FServerSideRewindResult ProjectileConfrimHit(const FFramePackage& Package ,AGunslinger* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	//	Shotgun
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);


private:
	void SaveFramePackage();

	UPROPERTY()
	AGunslinger* Character;

	UPROPERTY()
	class AGunslingerPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistroy;

	UPROPERTY(EditDefaultsOnly)
	float MaxRecordTime = 4.f;
};
