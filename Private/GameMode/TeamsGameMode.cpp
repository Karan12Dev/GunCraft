// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"
#include "GameState/GunslingerGameState.h"
#include "PlayerState/GunslingerPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/GunslingerPlayerController.h"


ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	GunslingerGameState = GunslingerGameState == nullptr ? Cast<AGunslingerGameState>(UGameplayStatics::GetGameState(this)) : GunslingerGameState;
	AGunslingerPlayerState* GunslingerPlayerState = NewPlayer->GetPlayerState<AGunslingerPlayerState>();
	if (GunslingerGameState && GunslingerPlayerState)
	{
		if (GunslingerPlayerState->GetTeam() == ETeam::NoTeam)
		{
			if (GunslingerGameState->BlueTeam.Num() >= GunslingerGameState->RedTeam.Num())
			{
				GunslingerGameState->RedTeam.AddUnique(GunslingerPlayerState);
				GunslingerPlayerState->SetTeam(ETeam::RedTeam);
			}
			else
			{
				GunslingerGameState->BlueTeam.AddUnique(GunslingerPlayerState);
				GunslingerPlayerState->SetTeam(ETeam::BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	GunslingerGameState = GunslingerGameState == nullptr ? Cast<AGunslingerGameState>(UGameplayStatics::GetGameState(this)) : GunslingerGameState;
	AGunslingerPlayerState* GunslingerPlayerState = Exiting->GetPlayerState<AGunslingerPlayerState>();
	if (GunslingerGameState && GunslingerPlayerState)
	{
		if (GunslingerGameState->RedTeam.Contains(GunslingerPlayerState))
		{
			GunslingerGameState->RedTeam.Remove(GunslingerPlayerState);
		}
		if (GunslingerGameState->BlueTeam.Contains(GunslingerPlayerState))
		{
			GunslingerGameState->BlueTeam.Remove(GunslingerPlayerState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	GunslingerGameState = GunslingerGameState == nullptr ? Cast<AGunslingerGameState>(UGameplayStatics::GetGameState(this)) : GunslingerGameState;
	if (GunslingerGameState)
	{
		for (APlayerState* PlayerState : GunslingerGameState->PlayerArray)
		{
			AGunslingerPlayerState* GunslingerPlayerState = Cast<AGunslingerPlayerState>(PlayerState);
			if (GunslingerPlayerState && GunslingerPlayerState->GetTeam() == ETeam::NoTeam)
			{
				if (GunslingerGameState->BlueTeam.Num() >= GunslingerGameState->RedTeam.Num())
				{
					GunslingerGameState->RedTeam.AddUnique(GunslingerPlayerState);
					GunslingerPlayerState->SetTeam(ETeam::RedTeam);
				}
				else
				{
					GunslingerGameState->BlueTeam.AddUnique(GunslingerPlayerState);
					GunslingerPlayerState->SetTeam(ETeam::BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AGunslingerPlayerState* AttackerPlayerState = Attacker->GetPlayerState<AGunslingerPlayerState>();
	AGunslingerPlayerState* VictimPlayerState = Victim->GetPlayerState<AGunslingerPlayerState>();

	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr) return BaseDamage;
	if (VictimPlayerState == AttackerPlayerState) return BaseDamage;

	if (AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AGunslinger* ElimmedCharacter, AGunslingerPlayerController* VictimController, AGunslingerPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	GunslingerGameState = GunslingerGameState == nullptr ? Cast<AGunslingerGameState>(UGameplayStatics::GetGameState(this)) : GunslingerGameState;
	AGunslingerPlayerState* AttackerPlayerState = AttackerController ? Cast<AGunslingerPlayerState>(AttackerController->PlayerState) : nullptr;
	if (GunslingerGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::BlueTeam)
		{
			GunslingerGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::RedTeam)
		{
			GunslingerGameState->RedTeamScores();
		}
	}
}
