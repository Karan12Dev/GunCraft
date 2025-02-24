#pragma once


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Unoccuiped UMETA(DisplayName = "Unoccupied"),
	Reloading UMETA(DisplayName = "Reloading"),
	ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),
	SwappingWeapon UMETA(DisplayName = "Swapping Weapon"),

	MAX UMETA(DisplayName = "DefalutMAX")
};