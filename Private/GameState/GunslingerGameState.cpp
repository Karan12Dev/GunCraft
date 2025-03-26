// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/GunslingerGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/GunslingerPlayerState.h"
#include "PlayerController/GunslingerPlayerController.h"


void AGunslingerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TopScoringPlayers);
	DOREPLIFETIME(ThisClass, RedTeamScore);
	DOREPLIFETIME(ThisClass, BlueTeamScore);
}

void AGunslingerGameState::UpdateTopScore(AGunslingerPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AGunslingerGameState::RedTeamScores()
{
	++RedTeamScore;
	AGunslingerPlayerController* GunslingerController = Cast<AGunslingerPlayerController>(GetWorld()->GetFirstPlayerController());
	if (GunslingerController)
	{
		GunslingerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AGunslingerGameState::BlueTeamScores()
{
	++BlueTeamScore;
	AGunslingerPlayerController* GunslingerController = Cast<AGunslingerPlayerController>(GetWorld()->GetFirstPlayerController());
	if (GunslingerController)
	{
		GunslingerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AGunslingerGameState::OnRep_RedTeamScore()
{
	AGunslingerPlayerController* GunslingerController = Cast<AGunslingerPlayerController>(GetWorld()->GetFirstPlayerController());
	if (GunslingerController)
	{
		GunslingerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AGunslingerGameState::OnRep_BlueTeamScore()
{
	AGunslingerPlayerController* GunslingerController = Cast<AGunslingerPlayerController>(GetWorld()->GetFirstPlayerController());
	if (GunslingerController)
	{
		GunslingerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
