// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/GunslingerGameMode.h"
#include "Character/Gunslinger.h"
#include "PlayerController/GunslingerPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/GunslingerPlayerState.h"
#include "GameState/GunslingerGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AGunslingerGameMode::AGunslingerGameMode()
{
	bDelayedStart = true;
}

void AGunslingerGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AGunslingerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetMatchState() == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime < 0.f)
		{
			RestartGame();
		}
	}
}

void AGunslingerGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AGunslingerPlayerController* GunslingerPlayer = Cast<AGunslingerPlayerController>(*It);
		if (GunslingerPlayer)
		{
			GunslingerPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AGunslingerGameMode::PlayerEliminated(AGunslinger* ElimmedCharacter, AGunslingerPlayerController* VictimController, AGunslingerPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AGunslingerPlayerState* AttackerPlayerState = AttackerController ? Cast<AGunslingerPlayerState>(AttackerController->PlayerState) : nullptr;
	AGunslingerPlayerState* VictimPlayerState = VictimController ? Cast<AGunslingerPlayerState>(VictimController->PlayerState) : nullptr;

	AGunslingerGameState* GunslingerGameState = GetGameState<AGunslingerGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && GunslingerGameState)
	{
		TArray<AGunslingerPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : GunslingerGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		GunslingerGameState->UpdateTopScore(AttackerPlayerState);
		if (GunslingerGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			if (AGunslinger* Leader = Cast<AGunslinger>(AttackerPlayerState->GetPawn()))
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!GunslingerGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				if (AGunslinger* Loser = Cast<AGunslinger>(PlayersCurrentlyInTheLead[i]->GetPawn()))
				{
					Loser->MulticastLostTheLead();
				}

			}
		}
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
		if (VictimPlayerState)
		{
			VictimPlayerState->AddToDefeats(1);
		}
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AGunslingerPlayerController* GunslingerController = Cast<AGunslingerPlayerController>(*It);
		if (GunslingerController && AttackerPlayerState && VictimPlayerState)
		{
			GunslingerController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AGunslingerGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		if (PlayerStarts.Num() > 0)
		{
			int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
			RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
		}
	}
}

void AGunslingerGameMode::PlayerLeftGame(AGunslingerPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;

	AGunslingerGameState* GunslingerGameState = GetGameState<AGunslingerGameState>();
	if (GunslingerGameState && GunslingerGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		GunslingerGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AGunslinger* GunslingerLeaving = Cast<AGunslinger>(PlayerLeaving->GetPawn());
	if (GunslingerLeaving)
	{
		GunslingerLeaving->Elim(true);
	}
}
