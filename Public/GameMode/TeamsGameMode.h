// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/GunslingerGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API ATeamsGameMode : public AGunslingerGameMode
{
	GENERATED_BODY()
	
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class AGunslinger* ElimmedCharacter, class AGunslingerPlayerController* VictimController, AGunslingerPlayerController* AttackerController) override;


protected:
	virtual void HandleMatchHasStarted() override;


private:
	UPROPERTY()
	class AGunslingerGameState* GunslingerGameState;
};
