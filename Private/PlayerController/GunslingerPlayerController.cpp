// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/GunslingerPlayerController.h"
#include "HUD/GunslingerHUD.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Announcement.h"
#include "HUD/ReturnToMainMenu.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/BackgroundBlur.h"
#include "Character/Gunslinger.h"
#include "GunslingerComponents/CombatComponent.h"
#include "GameMode/GunslingerGameMode.h"
#include "GameState/GunslingerGameState.h"
#include "PlayerState/GunslingerPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "GunslingerTypes/Announcement.h"


void AGunslingerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	GunslingerHUD = Cast<AGunslingerHUD>(GetHUD());
	ServerCheckMatchState();
}

void AGunslingerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Started, this, &ThisClass::ShowReturnToMainMenu);
	}
}

void AGunslingerPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuClass == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AGunslingerPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();	
	CheckTimeSync(DeltaTime);
	PollInt();
	CheckPing(DeltaTime);
}

void AGunslingerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AGunslinger* Gunslinger = Cast<AGunslinger>(InPawn);
	if (Gunslinger)
	{
		Gunslinger->UpdateHUDAmmo();
	}
}

void AGunslingerPlayerController::PollInt()
{
	if (CharacterOverlay == nullptr)
	{
		if (GunslingerHUD && GunslingerHUD->CharacterOverlay)
		{
			CharacterOverlay = GunslingerHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeSheild) SetHUDSheild(HUDSheild, HUDMaxSheild);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				AGunslinger* Gunslinger = Cast<AGunslinger>(GetPawn());
				if (Gunslinger && Gunslinger->GetCombat())
				{
					SetHUDGrenades(Gunslinger->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AGunslingerPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		
	DOREPLIFETIME(ThisClass, MatchState);
	DOREPLIFETIME(ThisClass, bShowTeamScores);
}

void AGunslingerPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->HealthBar && GunslingerHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		GunslingerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		GunslingerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AGunslingerPlayerController::SetHUDSheild(float Sheild, float MaxSheild)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->SheildBar && GunslingerHUD->CharacterOverlay->SheildText)
	{
		const float SheildPercent = Sheild / MaxSheild;
		GunslingerHUD->CharacterOverlay->SheildBar->SetPercent(SheildPercent);
		FString SheildText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Sheild));
		GunslingerHUD->CharacterOverlay->SheildText->SetText(FText::FromString(SheildText));
	}
	else
	{
		bInitializeSheild = true;
		HUDSheild = Sheild;
		HUDMaxSheild = MaxSheild;
	}
}

void AGunslingerPlayerController::SetHUDScore(float Score)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		GunslingerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AGunslingerPlayerController::SetHUDDefeats(int32 Defeats)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		GunslingerHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AGunslingerPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		GunslingerHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AGunslingerPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
		GunslingerHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = CarriedAmmo;
	}
}

void AGunslingerPlayerController::SetHUDWeaponName(FName WeaponName)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->WeaponName)
	{
		GunslingerHUD->CharacterOverlay->WeaponName->SetText(FText::FromName(WeaponName));
	}
}

void AGunslingerPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->MatchCountDownText)
	{
		if (CountDownTime < 0.f)
		{
			GunslingerHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		GunslingerHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void AGunslingerPlayerController::SetHUDAnnouncementCountDown(float CountDownTime)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->Announcement && GunslingerHUD->Announcement->WarmupTime)
	{
		if (CountDownTime < 0.f)
		{
			GunslingerHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		GunslingerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

void AGunslingerPlayerController::SetHUDGrenades(int32 Grenades)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->GrenadesText)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		GunslingerHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
}

void AGunslingerPlayerController::HideTeamScore()
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->TeamScoreBox)
	{
		GunslingerHUD->CharacterOverlay->TeamScoreBox->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AGunslingerPlayerController::InitTeamScores()
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->BlueTeamScore && GunslingerHUD->CharacterOverlay->RedTeamScore && GunslingerHUD->CharacterOverlay->TeamScoreBox)
	{
		GunslingerHUD->CharacterOverlay->TeamScoreBox->SetVisibility(ESlateVisibility::Visible);
		GunslingerHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString("0"));
		GunslingerHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString("0"));
	}
}

void AGunslingerPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->BlueTeamScore)
	{
		GunslingerHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(FString::FromInt(BlueScore)));
	}
}

void AGunslingerPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->RedTeamScore)
	{
		GunslingerHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(FString::FromInt(RedScore)));
	}
}

void AGunslingerPlayerController::ServerCheckMatchState_Implementation()
{
	AGunslingerGameMode* GameMode = Cast<AGunslingerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;	
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

		if (GunslingerHUD && MatchState == MatchState::WaitingToStart)
		{
			GunslingerHUD->AddAnnouncement();
		}
	}
}

void AGunslingerPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (GunslingerHUD && MatchState == MatchState::WaitingToStart)
	{
		GunslingerHUD->AddAnnouncement();
	}
}

void AGunslingerPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AGunslingerPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
		if (GunslingerHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				GunslingerHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				GunslingerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				GunslingerHUD->AddElimAnnouncement("You", "yoursef");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				GunslingerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			GunslingerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AGunslingerPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AGunslingerPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AGunslingerPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AGunslingerPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AGunslingerPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD)
	{
		if (GunslingerHUD->CharacterOverlay == nullptr) GunslingerHUD->AddCharacterOverlay();
		if (GunslingerHUD->Announcement)
		{
			GunslingerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScore();
		}
	}
}

void AGunslingerPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScore();
	}
}

void AGunslingerPlayerController::HandleCooldown()
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD)
	{
		GunslingerHUD->CharacterOverlay->RemoveFromParent();
		if (GunslingerHUD->Announcement && GunslingerHUD->Announcement->AnnouncementText && GunslingerHUD->Announcement->InfoText)
		{
			GunslingerHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			GunslingerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AGunslingerGameState* GunslingerGameState = Cast<AGunslingerGameState>(UGameplayStatics::GetGameState(this));
			AGunslingerPlayerState* GunslingerPlayerState = GetPlayerState<AGunslingerPlayerState>();
			if (GunslingerGameState && GunslingerPlayerState)
			{
				TArray<AGunslingerPlayerState*> TopPlayers = GunslingerGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(GunslingerGameState) : GetInfoText(TopPlayers);
				GunslingerHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString)); 
			}
		}
	}
	AGunslinger* Gunslinger = Cast<AGunslinger>(GetPawn());
	if (Gunslinger && Gunslinger->GetCombat())
	{
		Gunslinger->bDisableGameplay = true;
		Gunslinger->GetCombat()->FireButtonPressed(false);
	}
}

FString AGunslingerPlayerController::GetInfoText(const TArray<class AGunslingerPlayerState*>& Players)
{
	AGunslingerPlayerState* GunslingerPlayerState = GetPlayerState<AGunslingerPlayerState>();
	if (GunslingerPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == GunslingerPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString AGunslingerPlayerController::GetTeamsInfoText(AGunslingerGameState* GunslingerGameState)
{
	if (GunslingerGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = GunslingerGameState->RedTeamScore;
	const int32 BlueTeamScore = GunslingerGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(FString("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(FString("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeam;
		InfoTextString.Append(" Wins!");
		InfoTextString.Append(FString("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeam;
		InfoTextString.Append(" Wins!");
		InfoTextString.Append(FString("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

float AGunslingerPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AGunslingerPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		if (GunslingerGameMode == nullptr)
		{
			GunslingerGameMode = Cast<AGunslingerGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = GunslingerGameMode->LevelStartingTime;
		}
		GunslingerGameMode = GunslingerGameMode == nullptr ? Cast<AGunslingerGameMode>(UGameplayStatics::GetGameMode(this)) : GunslingerGameMode;
		if (GunslingerGameMode)
		{
			SecondsLeft = FMath::CeilToInt(GunslingerGameMode->GetCountDownTime() + LevelStartingTime);
		}
	}
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		}
	}
	CountDownInt = SecondsLeft;
}

void AGunslingerPlayerController::ServerRequestServerTime_Implementation(float TimeofClientRequest)
{
	float ServerTimeofReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeofClientRequest, ServerTimeofReceipt);
}

void AGunslingerPlayerController::ClientReportServerTime_Implementation(float TimeofClientRequest, float TimeServerReceivedClient)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeofClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClient + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AGunslingerPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? TObjectPtr<APlayerState>(GetPlayerState<APlayerState>()) : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0;
	}
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->HighPingImageAnim && GunslingerHUD->CharacterOverlay->IsAnimationPlaying(GunslingerHUD->CharacterOverlay->HighPingImageAnim))
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

// Is the ping too high?
void AGunslingerPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AGunslingerPlayerController::HighPingWarning()
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->HighPingImage && GunslingerHUD->CharacterOverlay->HighPingImageAnim)
	{
		GunslingerHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		GunslingerHUD->CharacterOverlay->PlayAnimation(GunslingerHUD->CharacterOverlay->HighPingImageAnim, 0.f, 5);
	}
}

void AGunslingerPlayerController::StopHighPingWarning()
{
	GunslingerHUD = GunslingerHUD == nullptr ? Cast<AGunslingerHUD>(GetHUD()) : GunslingerHUD;
	if (GunslingerHUD && GunslingerHUD->CharacterOverlay && GunslingerHUD->CharacterOverlay->HighPingImage && GunslingerHUD->CharacterOverlay->HighPingImageAnim)
	{
		GunslingerHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (GunslingerHUD->CharacterOverlay->IsAnimationPlaying(GunslingerHUD->CharacterOverlay->HighPingImageAnim))
		{
			GunslingerHUD->CharacterOverlay->StopAnimation(GunslingerHUD->CharacterOverlay->HighPingImageAnim);
		}
	}
}
