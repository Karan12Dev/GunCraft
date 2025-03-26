// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CaptureTheFlagGameMode.h"
#include "Weapon/Flag.h"
#include "CaptureTheFlag/FlagZone.h"
#include "GameState/GunslingerGameState.h"


void ACaptureTheFlagGameMode::PlayerEliminated(AGunslinger* ElimmedCharacter, AGunslingerPlayerController* VictimController, AGunslingerPlayerController* AttackerController)
{
	AGunslingerGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AGunslingerGameState* GunslingerGS = Cast<AGunslingerGameState>(GameState);
	if (GunslingerGS)
	{
		if (Zone->Team == ETeam::BlueTeam)
		{
			GunslingerGS->BlueTeamScores();
		}
		if (Zone->Team == ETeam::RedTeam)
		{
			GunslingerGS->RedTeamScores();
		}
	}
}
