#include "TDSGameInstance.h"

bool UTDSGameInstance::GetWeaponInfoByName(const FName NameWeapon, FWeaponInfo& OutInfo)
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


bool UTDSGameInstance::GetDropItemInfoByName(const FName NameItem, FDropItem& OutInfo)
{
	bool bIsFind = false;
	
	if (DropItemInfoTable)
	{
		TArray<FName>RowNames = DropItemInfoTable->GetRowNames();
		
		int8 i = 0;
		while (i < RowNames.Num() && !bIsFind)
		{
			const FDropItem* DropItemInfoRow = DropItemInfoTable->FindRow<FDropItem>(RowNames[i], "");
			if (DropItemInfoRow->WeaponInfo.NameItem == NameItem)
			{
				OutInfo = (*DropItemInfoRow);	
				bIsFind = true;
			}
			i++;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTPSGameInstance::GetDropItemInfoByName - DropItemInfoTable -NULL"));
	}
	return bIsFind;
}
