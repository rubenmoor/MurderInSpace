#include "GameplayAbilitySystem/MyAttributes.h"

#include "Net/UnrealNetwork.h"


void UAttrSetTorque::OnRep_TorqueMax(const FGameplayAttributeData& OldTorqueMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetTorque, TorqueMax, OldTorqueMax)
}

void UAttrSetTorque::OnRep_OmegaMax(const FGameplayAttributeData& OldOmegaMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetTorque, OmegaMax, OldOmegaMax)
}

TArray<FMyAttributeRow> UAttrSetTorque::GetAttributeInitialValueRows()
{
    return
        { { "AttrSetTorque.TorqueMax", 60. * PI / 180. }
        , { "AttrSetTorque.OmegaMax" , 60. * PI / 180. } 
        };
}

void UAttrSetTorque::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetTorque, TorqueMax, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetTorque, OmegaMax , COND_None, REPNOTIFY_Always);
}
