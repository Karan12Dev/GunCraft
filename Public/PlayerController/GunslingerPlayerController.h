// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunslingerPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class GUNCRAFT_API AGunslingerPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime();
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDSheild(float Sheild, float MaxSheild);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDWeaponName(FName WeaponName);
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDAnnouncementCountDown(float CountDownTime);
	void SetHUDGrenades(int32 Grenades);
	void HideTeamScore();
	void InitTeamScores();
	void SetHUDBlueTeamScore(int32 BlueScore);
	void SetHUDRedTeamScore(int32 RedScore);
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);


protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void SetHUDTime();
	void PollInt();

	//	Sync time between Client and Server	//
	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeofClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeofClientRequest, float TimeServerReceivedClient);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();


private:
	void CheckTimeSync(float DeltaTime);

	UPROPERTY()
	class AGunslingerHUD* GunslingerHUD;

	UPROPERTY()
	class AGunslingerGameMode* GunslingerGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountDownInt = 0;

	float ClientServerDelta = 0.f;
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDSheild;
	bool bInitializeSheild = false;
	float HUDMaxSheild;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	//	Ping
	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);
	float HighPingRunningTime = 0.f;
	float PingAnimationRunningTime = 0.f;
	float HighPingDuration = 5.f;
	float CheckPingFrequency = 5.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	float HighPingThreshold = 140.f;
	//

	// Return to MainMenu
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* QuitAction;
	void ShowReturnToMainMenu();

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

	FString GetInfoText(const TArray<class AGunslingerPlayerState*>& Players);
	FString GetTeamsInfoText(class AGunslingerGameState* GunslingerGameState);
};
