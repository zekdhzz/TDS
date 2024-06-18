#include "TDSInventoryComponent.h"

#include "Engine/World.h"
#include "TDS/Game/TDSGameInstance.h"

UTDSInventoryComponent::UTDSInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTDSInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTDSInventoryComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UTDSInventoryComponent::SwitchWeaponToIndexByNextPreviousIndex(const int32 ChangeToIndex, const int32 OldIndex,
                                                                    const FAdditionalWeaponInfo OldInfo,
                                                                    const bool bIsForward)
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
		int8 Iteration = 0;
		int8 SecondIteration = 0;
		int8 TmpIndex;
		while (Iteration < WeaponSlots.Num() && !bIsSuccess)
		{
			Iteration++;
			if (bIsForward)
			{
				TmpIndex = ChangeToIndex + Iteration;
			}
			else
			{
				SecondIteration = WeaponSlots.Num() - 1;
				TmpIndex = ChangeToIndex - Iteration;
			}

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
					if (WeaponSlots.IsValidIndex(SecondIteration))
					{
						if (!WeaponSlots[SecondIteration].NameItem.IsNone())
						{
							if (WeaponSlots[SecondIteration].AdditionalInfo.Round > 0)
							{
								//do nothing
							}
							else
							{
								FWeaponInfo WeaponInfo;
								UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
								GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].NameItem, WeaponInfo);
								//constexpr bool bIsFind = false;
								int8 j = 0;
								while (j < AmmoSlots.Num())
								{
									if (AmmoSlots[j].WeaponType == WeaponInfo.WeaponType)
									{
										if (AmmoSlots[j].Cout > 0)
										{
											//do nothing
										}
										else
										{
											UE_LOG(LogTemp, Error,
											       TEXT(
												       "UTPSInventoryComponent::SwitchWeaponToIndex - Init PISTOL - NEED"
											       ));
										}
									}
									j++;
								}
							}
						}
					}
				}
				if (bIsForward)
				{
					SecondIteration++;
				}
				else
				{
					SecondIteration--;
				}
			}
		}
	}
	if (bIsSuccess)
	{
		SetAdditionalInfoWeapon(OldIndex, OldInfo);
		SwitchWeaponEvent(NewIdWeapon, NewAdditionalInfo, NewCurrentIndex);
	}
	return bIsSuccess;
}

bool UTDSInventoryComponent::SwitchWeaponByIndex(const int32 IndexWeaponToChange, const int32 PreviousIndex,
                                                 const FAdditionalWeaponInfo PreviousWeaponInfo)
{
	bool bIsSuccess = false;
	const FName ToSwitchIdWeapon = GetWeaponNameBySlotIndex(IndexWeaponToChange);
	const FAdditionalWeaponInfo ToSwitchAdditionalInfo = GetAdditionalInfoWeapon(IndexWeaponToChange);
	if (!ToSwitchIdWeapon.IsNone())
	{
		SetAdditionalInfoWeapon(PreviousIndex, PreviousWeaponInfo);
		SwitchWeaponEvent(ToSwitchIdWeapon, ToSwitchAdditionalInfo, IndexWeaponToChange);
		EWeaponType ToSwitchWeaponType;
		if (GetWeaponTypeByNameWeapon(ToSwitchIdWeapon, ToSwitchWeaponType))
		{
			int8 AvailableAmmoForWeapon = -1;
			if (CheckAmmoForWeapon(ToSwitchWeaponType, AvailableAmmoForWeapon))
			{
			}
		}
		bIsSuccess = true;
	}
	return bIsSuccess;
}

FAdditionalWeaponInfo UTDSInventoryComponent::GetAdditionalInfoWeapon(const int32 IndexWeapon)
{
	FAdditionalWeaponInfo Result;
	if (WeaponSlots.IsValidIndex(IndexWeapon))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (i == IndexWeapon)
			{
				Result = WeaponSlots[i].AdditionalInfo;
				bIsFind = true;
			}
			i++;
		}
		if (!bIsFind)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("UTDSInventoryComponent::SetAdditionalInfoWeapon - No Found Weapon with index - %d"),
			       IndexWeapon);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTDSInventoryComponent %i"), IndexWeapon);
	}
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
			Result = i;
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

bool UTDSInventoryComponent::GetWeaponTypeByIndexSlot(const int32 IndexSlot, EWeaponType& WeaponType)
{
	bool bIsFind = false;
	WeaponType = EWeaponType::RifleType;
	UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			FWeaponInfo OutInfo;
			GI->GetWeaponInfoByName(WeaponSlots[IndexSlot].NameItem, OutInfo);
			WeaponType = OutInfo.WeaponType;
			bIsFind = true;
		}
	}
	return bIsFind;
}

bool UTDSInventoryComponent::GetWeaponTypeByNameWeapon(const FName IdWeaponName, EWeaponType& WeaponType)
{
	bool bIsFind = false;
	WeaponType = EWeaponType::RifleType;
	UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		FWeaponInfo OutInfo;
		GI->GetWeaponInfoByName(IdWeaponName, OutInfo);
		WeaponType = OutInfo.WeaponType;
		bIsFind = true;
	}
	return bIsFind;
}

void UTDSInventoryComponent::SetAdditionalInfoWeapon(const int32 IndexWeapon, const FAdditionalWeaponInfo NewInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon"));
	if (WeaponSlots.IsValidIndex(IndexWeapon))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (i == IndexWeapon)
			{
				WeaponSlots[i].AdditionalInfo = NewInfo;
				bIsFind = true;
				WeaponAdditionalInfoChangeEvent(IndexWeapon, NewInfo);
			}
			i++;
		}
		if (!bIsFind)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon - Not Found Weapon with index - %d"),
			       IndexWeapon);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon - Not Correct index Weapon - %d"), IndexWeapon);
	}
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
			{
				AmmoSlots[i].Cout = AmmoSlots[i].MaxCout;
			}
			AmmoChangeEvent(AmmoSlots[i].WeaponType, AmmoSlots[i].Cout);
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
		SwitchWeaponToIndexByNextPreviousIndex(CurrentIndexWeaponChar, -1, NewWeapon.AdditionalInfo, true);
		UpdateWeaponSlotsEvent(IndexSlot, NewWeapon);
		bResult = true;
	}
	return bResult;
}

bool UTDSInventoryComponent::TryGetWeaponToInventory(const FWeaponSlot NewWeapon)
{
	int32 IndexSlot = -1;
	if (CheckCanTakeWeapon(IndexSlot))
	{
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			WeaponSlots[IndexSlot] = NewWeapon;

			OnUpdateWeaponSlots.Broadcast(IndexSlot, NewWeapon);
			return true;
		}
	}
	return false;
}

void UTDSInventoryComponent::DropWeaponByIndex(const int32 ByIndex)
{
	FDropItem DropItemInfo;
	bool bIsCanDrop = false;
	int8 i = 0;
	int8 AvailableWeaponNum = 0;
	while (i < WeaponSlots.Num() && !bIsCanDrop)
	{
		if (!WeaponSlots[i].NameItem.IsNone())
		{
			AvailableWeaponNum++;
			if (AvailableWeaponNum > 1)
				bIsCanDrop = true;
		}
		i++;
	}

	if (bIsCanDrop && WeaponSlots.IsValidIndex(ByIndex) && GetDropItemInfoFromInventory(ByIndex, DropItemInfo))
	{
		const FWeaponSlot EmptyWeaponSlot;
		//switch weapon to valid slot weapon from start weapon slots array
		bool bIsFindWeapon = false;
		int8 j = 0;
		while (j < WeaponSlots.Num() && !bIsFindWeapon)
		{
			if (!WeaponSlots[j].NameItem.IsNone())
			{
				//OnSwitchWeapon.Broadcast(WeaponSlots[j].NameItem, WeaponSlots[j].AdditionalInfo, j);
				SwitchWeaponEvent(WeaponSlots[j].NameItem, WeaponSlots[j].AdditionalInfo, j);
			}
			j++;
		}
		WeaponSlots[ByIndex] = EmptyWeaponSlot;
		// if (GetOwner()->GetClass()->ImplementsInterface(UTPS_IGameActor::StaticClass()))
		// {
		// 	ITPS_IGameActor::Execute_DropWeaponToWorld(GetOwner(),DropItemInfo);
		// }

		//OnUpdateWeaponSlots.Broadcast(ByIndex, EmptyWeaponSlot);
		UpdateWeaponSlotsEvent(ByIndex, EmptyWeaponSlot);
	}
}

bool UTDSInventoryComponent::GetDropItemInfoFromInventory(const int32 IndexSlot, FDropItem& DropItemInfo)
{
	bool bResult = false;
	const FName DropItemName = GetWeaponNameBySlotIndex(IndexSlot);
	UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		bResult = GI->GetDropItemInfoByName(DropItemName, DropItemInfo);
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			DropItemInfo.WeaponInfo.AdditionalInfo = WeaponSlots[IndexSlot].AdditionalInfo;
		}
	}
	return bResult;
}

TArray<FWeaponSlot> UTDSInventoryComponent::GetWeaponSlots()
{
	return WeaponSlots;
}

TArray<FAmmoSlot> UTDSInventoryComponent::GetAmmoSlots()
{
	return AmmoSlots;
}

void UTDSInventoryComponent::InitInventory(const TArray<FWeaponSlot>& NewWeaponSlotsInfo,
                                           const TArray<FAmmoSlot>& NewAmmoSlotsInfo)
{
	WeaponSlots = NewWeaponSlotsInfo;
	AmmoSlots = NewAmmoSlotsInfo;
	//Find init weaponsSlots and First Init Weapon
	MaxSlotsWeapon = WeaponSlots.Num();
	if (WeaponSlots.IsValidIndex(0))
	{
		if (!WeaponSlots[0].NameItem.IsNone())
		{
			SwitchWeaponEvent(WeaponSlots[0].NameItem, WeaponSlots[0].AdditionalInfo, 0);
		}
	}
}

void UTDSInventoryComponent::AmmoChangeEvent(const EWeaponType TypeWeapon, const int32 Cout) const
{
	OnAmmoChange.Broadcast(TypeWeapon, Cout);
}

void UTDSInventoryComponent::SwitchWeaponEvent(const FName WeaponName, const FAdditionalWeaponInfo AdditionalInfo,
                                               const int32 IndexSlot) const
{
	OnSwitchWeapon.Broadcast(WeaponName, AdditionalInfo, IndexSlot);
}

void UTDSInventoryComponent::WeaponAdditionalInfoChangeEvent(const int32 IndexSlot,
                                                             const FAdditionalWeaponInfo AdditionalInfo) const
{
	OnWeaponAdditionalInfoChange.Broadcast(IndexSlot, AdditionalInfo);
}

void UTDSInventoryComponent::WeaponAmmoEmptyEvent(const EWeaponType TypeWeapon) const
{
	OnWeaponAmmoEmpty.Broadcast(TypeWeapon);
}

void UTDSInventoryComponent::WeaponAmmoAvailableEvent(const EWeaponType TypeWeapon) const
{
	OnWeaponAmmoAvailable.Broadcast(TypeWeapon);
}

void UTDSInventoryComponent::UpdateWeaponSlotsEvent(const int32 IndexSlotChange, const FWeaponSlot NewInfo) const
{
	OnUpdateWeaponSlots.Broadcast(IndexSlotChange, NewInfo);
}

void UTDSInventoryComponent::WeaponNotHaveRoundEvent(const int32 IndexSlotWeapon) const
{
	OnWeaponNotHaveRound.Broadcast(IndexSlotWeapon);
}

void UTDSInventoryComponent::WeaponHaveRoundEvent(const int32 IndexSlotWeapon) const
{
	OnWeaponHaveRound.Broadcast(IndexSlotWeapon);
}
