// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GunslingerTypes/Team.h"
#include "GunslingerPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GUNCRAFT_API AGunslingerPlayerState : public APlayerState
{
	GENERATED_BODY()
	

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);


private:
	UPROPERTY()
	class AGunslinger* Character;

	UPROPERTY()
	class AGunslingerPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats; 

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::NoTeam;

	UFUNCTION()
	void OnRep_Team();


public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
};
