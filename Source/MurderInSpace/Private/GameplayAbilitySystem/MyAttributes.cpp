#include "GameplayAbilitySystem/MyAttributes.h"

#include "Net/UnrealNetwork.h"


void UAttrSetAcceleration::OnRep_AccelerationSIMax(const FGameplayAttributeData& OldAccelerationSIMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, AccelerationSIMax, OldAccelerationSIMax)
}

void UAttrSetAcceleration::OnRep_AccelerationSI(const FGameplayAttributeData& OldAccelerationSI)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, AccelerationSI, OldAccelerationSI)
}

void UAttrSetAcceleration::OnRep_TorqueMax(const FGameplayAttributeData& OldTorqueMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, TorqueMax, OldTorqueMax)
}

void UAttrSetAcceleration::OnRep_Torque(const FGameplayAttributeData& OldTorque)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, Torque, OldTorque)
}

void UAttrSetAcceleration::OnRep_OmegaMax(const FGameplayAttributeData& OldOmegaMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, OmegaMax, OldOmegaMax)
}

void UAttrSetAcceleration::OnRep_Omega(const FGameplayAttributeData& OldOmega)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, Omega, OldOmega)
}

void UAttrSetAcceleration::OnRep_ForwardSpeed(const FGameplayAttributeData& OldForwardSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, OmegaMax, OldForwardSpeed)
}

void UAttrSetAcceleration::OnRep_ForwardSpeedMax(const FGameplayAttributeData& OldForwardSpeedMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSetAcceleration, ForwardSpeedMax, OldForwardSpeedMax)
}

TArray<FMyAttributeRow> UAttrSetAcceleration::GetAttributeInitialValueRows()
{
    return
        { { "AttrSetAcceleration.TorqueMax"        , 120. * PI / 180. }
        , { "AttrSetAcceleration.Torque"           , 0.               }
        , { "AttrSetAcceleration.Omega"            , 0.               }
        , { "AttrSetAcceleration.OmegaMax"         , 200. * PI / 180. }
        , { "AttrSetAcceleration.AccelerationSIMax", 3.               }
        , { "AttrSetAcceleration.AccelerationSI"   , 0.               }
        , { "AttrSetAcceleration.ForwardSpeed"     , 0.               }
        , { "AttrSetAcceleration.ForwardSpeedMax"  , 180.             }
        };
}

void UAttrSetAcceleration::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetAcceleration, AccelerationSIMax, COND_None, REPNOTIFY_Always)
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetAcceleration, AccelerationSI   , COND_None, REPNOTIFY_Always)
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetAcceleration, TorqueMax        , COND_None, REPNOTIFY_Always)
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetAcceleration, Torque           , COND_None, REPNOTIFY_Always)
    DOREPLIFETIME_CONDITION_NOTIFY(UAttrSetAcceleration, OmegaMax         , COND_None, REPNOTIFY_Always)
}
