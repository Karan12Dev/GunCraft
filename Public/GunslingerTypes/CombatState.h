#pragma once


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Unoccuiped UMETA(DisplayName = "Unoccupied"),
	Reloading UMETA(DisplayName = "Reloading"),

	MAX UMETA(DisplayName = "DefalutMAX")
};