#include "TDSInventoryComponent.h"
#include "TDS/Game/TDSGameInstance.h"

UTDSInventoryComponent::UTDSInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTDSInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int8 i = 0; i < WeaponSlots.Num(); i++)
	{
		const UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
		if (GI)
		{
			if (!WeaponSlots[i].NameItem.IsNone())
			{
				FWeaponInfo Info;
				if (GI->GetWeaponInfoByName(WeaponSlots[i].NameItem, Info))
					WeaponSlots[i].AdditionalInfo.Round = Info.MaxRound;
				else
				{
					//WeaponSlots.RemoveAt(i);
					//i--;
				}
			}
		}
	}
	MaxSlotsWeapon = WeaponSlots.Num();

	if (WeaponSlots.IsValidIndex(0))
	{
		if (!WeaponSlots[0].NameItem.IsNone())
			OnSwitchWeapon.Broadcast(WeaponSlots[0].NameItem, WeaponSlots[0].AdditionalInfo, 0);
	}
}

void UTDSInventoryComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UTDSInventoryComponent::SwitchWeaponToIndex(int32 ChangeToIndex, int32 OldIndex, FAdditionalWeaponInfo OldInfo,
                                                 bool bIsForward)
{
	bool bIsSuccess = false;
	int8 CorrectIndex = ChangeToIndex;
	if (ChangeToIndex > WeaponSlots.Num() - 1)
	{
		CorrectIndex = 0;
	}
	else if (ChangeToIndex < 0)
	{
		CorrectIndex = WeaponSlots.Num() - 1;
	}

	FName NewIdWeapon;
	FAdditionalWeaponInfo NewAdditionalInfo;
	int32 NewCurrentIndex = 0;

	if (WeaponSlots.IsValidIndex(CorrectIndex))
	{
		if (!WeaponSlots[CorrectIndex].NameItem.IsNone())
		{
			if (WeaponSlots[CorrectIndex].AdditionalInfo.Round > 0)
			{
				bIsSuccess = true;
			}
			else
			{
				UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
				if (GI)
				{
					FWeaponInfo WeaponInfo;
					GI->GetWeaponInfoByName(WeaponSlots[CorrectIndex].NameItem, WeaponInfo);

					bool bIsFind = false;
					int8 j = 0;
					while (j < AmmoSlots.Num() && !bIsFind)
					{
						if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType && AmmoSlots[j].Cout > 0)
						{
							bIsSuccess = true;
							bIsFind = true;
						}
						j++;
					}
				}
			}
			if (bIsSuccess)
			{
				NewCurrentIndex = CorrectIndex;
				NewIdWeapon = WeaponSlots[CorrectIndex].NameItem;
				NewAdditionalInfo = WeaponSlots[CorrectIndex].AdditionalInfo;
			}
		}
	}
	if (!bIsSuccess)
	{
		if (bIsForward)
		{
			int8 Iteration = 0;
			int8 SecondIteration = 0;
			while (Iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				Iteration++;
				int8 TmpIndex = ChangeToIndex + Iteration;
				if (WeaponSlots.IsValidIndex(TmpIndex))
				{
					if (!WeaponSlots[TmpIndex].NameItem.IsNone())
					{
						if (WeaponSlots[TmpIndex].AdditionalInfo.Round > 0)
						{
							bIsSuccess = true;
							NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
							NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
							NewCurrentIndex = TmpIndex;
						}
						else
						{
							FWeaponInfo WeaponInfo;
							UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());

							GI->GetWeaponInfoByName(WeaponSlots[TmpIndex].NameItem, WeaponInfo);

							bool bIsFind = false;
							int8 j = 0;
							while (j < AmmoSlots.Num() && !bIsFind)
							{
								if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType && AmmoSlots[j].Cout > 0)
								{
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
									NewCurrentIndex = TmpIndex;
									bIsFind = true;
								}
								j++;
							}
						}
					}
				}
				else
				{
					if (OldIndex != SecondIteration)
					{
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalInfo.Round > 0)
								{
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[SecondIteration].NameItem;
									NewAdditionalInfo = WeaponSlots[SecondIteration].AdditionalInfo;
									NewCurrentIndex = SecondIteration;
								}
								else
								{
									FWeaponInfo WeaponInfo;
									UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());

									GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].NameItem, WeaponInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType && AmmoSlots[j].Cout > 0)
										{
											//WeaponGood
											bIsSuccess = true;
											NewIdWeapon = WeaponSlots[SecondIteration].NameItem;
											NewAdditionalInfo = WeaponSlots[SecondIteration].AdditionalInfo;
											NewCurrentIndex = SecondIteration;
											bIsFind = true;
										}
										j++;
									}
								}
							}
						}
					}
					else
					{
						//go to same weapon when start
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalInfo.Round > 0)
								{
									//WeaponGood, it same weapon do nothing
								}
								else
								{
									FWeaponInfo WeaponInfo;
									UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());

									GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].NameItem, WeaponInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType)
										{
											if (AmmoSlots[j].Cout > 0)
											{
												//WeaponGood, it same weapon do nothing
											}
											else
											{
												//Not find weapon with amm need init Pistol with infinity ammo
												UE_LOG(LogTemp, Error,
												       TEXT(
													       "UTDSInventoryComponent::SwitchWeaponToIndex - Init PISTOL - NEED"
												       ));
											}
										}
										j++;
									}
								}
							}
						}
					}
					SecondIteration++;
				}
			}
		}
		else
		{
			int8 Iteration = 0;
			int8 SecondIteration = WeaponSlots.Num() - 1;
			while (Iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				Iteration++;
				int8 TmpIndex = ChangeToIndex - Iteration;
				if (WeaponSlots.IsValidIndex(TmpIndex))
				{
					if (!WeaponSlots[TmpIndex].NameItem.IsNone())
					{
						if (WeaponSlots[TmpIndex].AdditionalInfo.Round > 0)
						{
							//WeaponGood
							bIsSuccess = true;
							NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
							NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
							NewCurrentIndex = TmpIndex;
						}
						else
						{
							FWeaponInfo WeaponInfo;
							UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());

							GI->GetWeaponInfoByName(WeaponSlots[TmpIndex].NameItem, WeaponInfo);

							bool bIsFind = false;
							int8 j = 0;
							while (j < AmmoSlots.Num() && !bIsFind)
							{
								if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType && AmmoSlots[j].Cout > 0)
								{
									//WeaponGood
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
									NewCurrentIndex = TmpIndex;
									bIsFind = true;
								}
								j++;
							}
						}
					}
				}
				else
				{
					//go to end of LEFT of array weapon slots
					if (OldIndex != SecondIteration)
					{
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalInfo.Round > 0)
								{
									//WeaponGood
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[SecondIteration].NameItem;
									NewAdditionalInfo = WeaponSlots[SecondIteration].AdditionalInfo;
									NewCurrentIndex = SecondIteration;
								}
								else
								{
									FWeaponInfo WeaponInfo;
									UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());

									GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].NameItem, WeaponInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType && AmmoSlots[j].Cout > 0)
										{
											//WeaponGood
											bIsSuccess = true;
											NewIdWeapon = WeaponSlots[SecondIteration].NameItem;
											NewAdditionalInfo = WeaponSlots[SecondIteration].AdditionalInfo;
											NewCurrentIndex = SecondIteration;
											bIsFind = true;
										}
										j++;
									}
								}
							}
						}
					}
					else
					{
						//go to same weapon when start
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalInfo.Round > 0)
								{
									//WeaponGood, it same weapon do nothing
								}
								else
								{
									FWeaponInfo WeaponInfo;
									UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());

									GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].NameItem, WeaponInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType)
										{
											if (AmmoSlots[j].Cout > 0)
											{
												//WeaponGood, it same weapon do nothing
											}
											else
											{
												//Not find weapon with amm need init Pistol with infinity ammo
												UE_LOG(LogTemp, Error,
												       TEXT(
													       "UTDSInventoryComponent::SwitchWeaponToIndex - Init PISTOL - NEED"
												       ));
											}
										}
										j++;
									}
								}
							}
						}
					}
					SecondIteration--;
				}
			}
		}
	}

	if (bIsSuccess)
	{
		SetAdditionalInfoWeapon(OldIndex, OldInfo);
		OnSwitchWeapon.Broadcast(NewIdWeapon, NewAdditionalInfo, NewCurrentIndex);
		//OnWeaponAmmoAvailable.Broadcast()
	}


	return bIsSuccess;
}

FAdditionalWeaponInfo UTDSInventoryComponent::GetAdditionalInfoWeapon(int32 IndexWeapon)
{
	FAdditionalWeaponInfo Result;
	if (WeaponSlots.IsValidIndex(IndexWeapon))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (/*WeaponSlots[i].IndexSlot*/i == IndexWeapon)
			{
				Result = WeaponSlots[i].AdditionalInfo;
				bIsFind = true;
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning,
		       TEXT("UTDSInventoryComponent::SetAdditionalInfoWeapon - No Found Weapon with index - %d"), IndexWeapon);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UTDSInventoryComponent::SetAdditionalInfoWeapon - Not Correct index Weapon - %d"),
	       IndexWeapon);

	return Result;
}

int32 UTDSInventoryComponent::GetWeaponIndexSlotByName(const FName IdWeaponName)
{
	int32 Result = -1;
	int8 i = 0;
	bool bIsFind = false;
	while (i < WeaponSlots.Num() && !bIsFind)
	{
		if (WeaponSlots[i].NameItem == IdWeaponName)
		{
			bIsFind = true;
			Result = i/*WeaponSlots[i].IndexSlot*/;
		}
		i++;
	}
	return Result;
}

FName UTDSInventoryComponent::GetWeaponNameBySlotIndex(const int32 IndexSlot)
{
	FName Result;

	if (WeaponSlots.IsValidIndex(IndexSlot))
	{
		Result = WeaponSlots[IndexSlot].NameItem;
	}
	return Result;
}


void UTDSInventoryComponent::SetAdditionalInfoWeapon(const int32 IndexWeapon, const FAdditionalWeaponInfo NewInfo)
{
	if (WeaponSlots.IsValidIndex(IndexWeapon))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (/*WeaponSlots[i].IndexSlot*/i == IndexWeapon)
			{
				WeaponSlots[i].AdditionalInfo = NewInfo;
				bIsFind = true;

				OnWeaponAdditionalInfoChange.Broadcast(IndexWeapon, NewInfo);
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning,
		       TEXT("UTDSInventoryComponent::SetAdditionalInfoWeapon - No Found Weapon with index - %d"), IndexWeapon);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UTDSInventoryComponent::SetAdditionalInfoWeapon - Not Correct index Weapon - %d"),
	       IndexWeapon);
}

void UTDSInventoryComponent::AmmoSlotChangeValue(const EWeaponType TypeWeapon, const int32 CoutChangeAmmo)
{
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFind)
	{
		if (AmmoSlots[i].WeaponType == TypeWeapon)
		{
			AmmoSlots[i].Cout += CoutChangeAmmo;
			if (AmmoSlots[i].Cout > AmmoSlots[i].MaxCout)
				AmmoSlots[i].Cout = AmmoSlots[i].MaxCout;

			OnAmmoChange.Broadcast(AmmoSlots[i].WeaponType, AmmoSlots[i].Cout);

			bIsFind = true;
		}
		i++;
	}
}

bool UTDSInventoryComponent::CheckAmmoForWeapon(const EWeaponType TypeWeapon, int8& AvailableAmmoForWeapon)
{
	AvailableAmmoForWeapon = 0;
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFind)
	{
		if (AmmoSlots[i].WeaponType == TypeWeapon)
		{
			bIsFind = true;
			AvailableAmmoForWeapon = AmmoSlots[i].Cout;
			if (AmmoSlots[i].Cout > 0)
			{
				//OnWeaponAmmoAvailable.Broadcast(TypeWeapon);//remove not here, only when pickUp ammo this type, or switch weapon
				return true;
			}
		}

		i++;
	}

	OnWeaponAmmoEmpty.Broadcast(TypeWeapon); //visual empty ammo slot

	return false;
}

bool UTDSInventoryComponent::CheckCanTakeAmmo(EWeaponType AmmoType)
{
	bool bResult = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bResult)
	{
		if (AmmoSlots[i].WeaponType == AmmoType && AmmoSlots[i].Cout < AmmoSlots[i].MaxCout)
			bResult = true;
		i++;
	}
	return bResult;
}

bool UTDSInventoryComponent::CheckCanTakeWeapon(int32& FreeSlot)
{
	bool bIsFreeSlot = false;
	int8 i = 0;
	while (i < WeaponSlots.Num() && !bIsFreeSlot)
	{
		if (WeaponSlots[i].NameItem.IsNone())
		{
			bIsFreeSlot = true;
			FreeSlot = i;
		}
		i++;
	}
	return bIsFreeSlot;
}

bool UTDSInventoryComponent::SwitchWeaponToInventory(const FWeaponSlot NewWeapon, const int32 IndexSlot,
                                                     const int32 CurrentIndexWeaponChar, FDropItem& DropItemInfo)
{
	bool bResult = false;

	if (WeaponSlots.IsValidIndex(IndexSlot) && GetDropItemInfoFromInventory(IndexSlot, DropItemInfo))
	{
		WeaponSlots[IndexSlot] = NewWeapon;

		SwitchWeaponToIndex(CurrentIndexWeaponChar, -1, NewWeapon.AdditionalInfo, true);

		OnUpdateWeaponSlots.Broadcast(IndexSlot, NewWeapon);
		bResult = true;
	}
	return bResult;
}

bool UTDSInventoryComponent::TryGetWeaponToInventory(const FWeaponSlot NewWeapon)
{
	int32 indexSlot = -1;
	if (CheckCanTakeWeapon(indexSlot))
	{
		if (WeaponSlots.IsValidIndex(indexSlot))
		{
			WeaponSlots[indexSlot] = NewWeapon;

			OnUpdateWeaponSlots.Broadcast(indexSlot, NewWeapon);
			return true;
		}
	}
	return false;
}

bool UTDSInventoryComponent::GetDropItemInfoFromInventory(const int32 IndexSlot, FDropItem& DropItemInfo)
{
	bool result = false;

	FName DropItemName = GetWeaponNameBySlotIndex(IndexSlot);

	UTDSGameInstance* myGI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
	if (myGI)
	{
		result = myGI->GetDropItemInfoByName(DropItemName, DropItemInfo);
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			DropItemInfo.WeaponInfo.AdditionalInfo = WeaponSlots[IndexSlot].AdditionalInfo;
		}
	}

	return result;
}