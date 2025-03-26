// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/GunslingerPlayerState.h"
#include "Character/Gunslinger.h"
#include "PlayerController/GunslingerPlayerController.h"
#include "Net/UnrealNetwork.h"


void AGunslingerPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Defeats);
	DOREPLIFETIME(ThisClass, Team);
}

void AGunslingerPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AGunslinger>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
	
}

void AGunslingerPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AGunslinger>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AGunslingerPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AGunslinger>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AGunslingerPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AGunslinger>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AGunslingerPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AGunslingerPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	AGunslinger* Gunslinger = Cast<AGunslinger>(GetPawn());
	if (Gunslinger)
	{
		Gunslinger->SetTeamColor(Team);
	}
}
void AGunslingerPlayerState::OnRep_Team()
{
	AGunslinger* Gunslinger = Cast<AGunslinger>(GetPawn());
	if (Gunslinger)
	{
		Gunslinger->SetTeamColor(Team);
	}
}