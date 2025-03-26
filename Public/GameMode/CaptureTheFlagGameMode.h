// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(class AGunslinger* ElimmedCharacter, class AGunslingerPlayerController* VictimController, AGunslingerPlayerController* AttackerController);
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};
