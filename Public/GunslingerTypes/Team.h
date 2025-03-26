#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	RedTeam UMETA(DisplayName = "Red Team"),
	BlueTeam UMETA(DisplayName = "Blue Team"),
	NoTeam UMETA(DisplayName = "NoTeam"),

	MAX UMETA(DisplayName = "DefaultMAX")
};