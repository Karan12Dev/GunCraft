// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SheildPickup.h"
#include "Character/Gunslinger.h"
#include "GunslingerComponents/BuffComponent.h"


void ASheildPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AGunslinger* Gunslinger = Cast<AGunslinger>(OtherActor);
	if (Gunslinger)
	{
		UBuffComponent* Buff = Gunslinger->GetBuffComponent();
		if (Buff)
		{
			Buff->ReplenishSheild(SheildReplenishAmount, SheildReplenishTime);
		}
	}
	Destroy();
}
