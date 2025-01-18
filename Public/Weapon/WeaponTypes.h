#pragma once

#define TRACE_LENGTH 60000.f


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	Pistol UMETA(DisplayName = "Pistol"),
	SMG UMETA(DispayName = "SMG"),
	Shotgun UMETA(DisplayName = "Shotgun"),

	MAX UMETA(DisplayName = "DefaultMAX")
};