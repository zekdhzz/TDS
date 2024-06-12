#include "TDSGameInstance.h"

bool UTDSGameInstance::GetWeaponInfoByName(const FName NameWeapon, FWeaponInfo& OutInfo) const
{
	bool bIsFind = false;
	if (WeaponInfoTable)
	{
		const FWeaponInfo* WeaponInfoRow = WeaponInfoTable->FindRow<FWeaponInfo>(NameWeapon, "", false);
		if (WeaponInfoRow)
		{
			bIsFind = true;
			OutInfo = *WeaponInfoRow;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponInfoTable not setted in GameMode"));
	}
	return bIsFind;
}
