#pragma once

#define TRACE_LENGTH 60000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	Pistol UMETA(DisplayName = "Pistol"),
	SMG UMETA(DispayName = "SMG"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),

	MAX UMETA(DisplayName = "DefaultMAX")
};