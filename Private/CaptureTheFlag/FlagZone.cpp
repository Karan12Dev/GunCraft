// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlag/FlagZone.h"
#include "Components/SphereComponent.h"
#include "Weapon/Flag.h"
#include "GameMode/CaptureTheFlagGameMode.h"


AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Zone"));
	SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}
