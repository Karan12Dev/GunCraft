// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GunslingerGameMode.generated.h"

namespace MatchState
{
	extern GUNCRAFT_API const FName Cooldown;	//Match duration has been reached. Display winner and begin cooldown timer.
}

UCLASS()
class GUNCRAFT_API AGunslingerGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGunslingerGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AGunslinger* ElimmedCharacter, class AGunslingerPlayerController* VictimController, AGunslingerPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;


private:
	float CountdownTime = 0.f;


public:
	FORCEINLINE float GetCountDownTime() const { return CountdownTime; }
};
