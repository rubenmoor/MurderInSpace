#include "GameplayAbilitySystem/Attributes.h"

#include "GameplayAbilitySystem/MyDeveloperSettings.h"
#include "Modes/MyGameState.h"
#include "Modes/MyState.h"
#include "Net/UnrealNetwork.h"


void UAttrSetTorque::OnRep_Torque(const FGameplayAttributeData& OldTorque)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetTorque, Torque, OldTorque)
}

void UAttrSetTorque::OnRep_OmegaMax(const FGameplayAttributeData& OldOmegaMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetTorque, OmegaMax, OldOmegaMax)
}

void UAttrSetTorque::PostInitProperties()
{
    Super::PostInitProperties();

    UE_LOG(LogMyGame, Error, TEXT("%s: PostInitProperties"), *GetFullName())
    
    auto* Settings = GetDefault<UMyDeveloperSettings>();
    if(Settings->InitialAttributeValues.IsNull())
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: Data Table initial attribute values: not set; can't load defaults"), *GetFullName())
    }
    else
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: initializing attributes from meta data"), *GetFullName())
        auto* Table = Settings->InitialAttributeValues.LoadSynchronous();
        check(Table->RowStruct == FMyAttributeMetaData::StaticStruct())
        Table->AddRow(FName("AttrSetTorque.Torque"), FMyAttributeMetaData(60., 10., 600., "", true));
        Table->AddRow(FName("AttrSetTorque.OmegaMax"), FMyAttributeMetaData(60., 10., 600., "", true));
        InitFromMetaDataTable(Table);
    }
}

void UAttrSetTorque::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetTorque, Torque, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetTorque, OmegaMax, COND_None, REPNOTIFY_Always);
}
