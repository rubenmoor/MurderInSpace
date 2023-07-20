#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueManager.h"
#include "Actors/MyPawn.h"
#include "GameplayAbilitySystem/MyDeveloperSettings.h"
#include "..\..\Public\GameplayAbilitySystem\MyInputGameplayAbility.h"
#include "Modes/MyPlayerController.h"

UMyAbilitySystemComponent::UMyAbilitySystemComponent()
{
    ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

UMyAbilitySystemComponent* UMyAbilitySystemComponent::Get(AMyPawn* InPawn)
{
    check(InPawn->Implements<UAbilitySystemInterface>())
    return Cast<UMyAbilitySystemComponent>(Cast<IAbilitySystemInterface>(InPawn)->GetAbilitySystemComponent());
}

UMyAbilitySystemComponent* UMyAbilitySystemComponent::Get(const FGameplayAbilityActorInfo* ActorInfo)
{
    return Cast<UMyAbilitySystemComponent>(ActorInfo->AbilitySystemComponent);
}

void UMyAbilitySystemComponent::ApplyGE_MoveTowardsCircle()
{
    GE_MoveTowardsCircleHandle = ApplyGameplayEffectSpecToSelf( *MakeOutgoingSpec(GE_MoveTowardsCircle, 1.0, MakeEffectContext()).Data.Get());
}

void UMyAbilitySystemComponent::RemoveGE_MoveTowardsCircle()
{
    RemoveActiveGameplayEffect(GE_MoveTowardsCircleHandle);
}

void UMyAbilitySystemComponent::AddGameplayTag(const FGameplayTag InTag)
{
    const auto GameplayEffectClass = GetDefault<UMyDeveloperSettings>()->GameplayEffectInfiniteClass;
    const FGameplayEffectSpecHandle SpecHandle =
        MakeOutgoingSpec(GameplayEffectClass, 1., MakeEffectContext());
    FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
    Spec->DynamicGrantedTags.AddTag(InTag);
    ApplyGameplayEffectSpecToSelf(*Spec);
}

void UMyAbilitySystemComponent::RemoveGameplayTag(const FGameplayTag InTag)
{
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(InTag));
	RemoveActiveEffects(Query);
}

void UMyAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();
    check(IsValid(GetDefault<UMyDeveloperSettings>()->GameplayEffectInfiniteClass))
}

void UMyAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
    Super::OnGiveAbility(AbilitySpec);
    
    // bind the pressed/released actions to gameplay abilities of type `InputGameplayAbilities`
    const auto* MyAbility = Cast<UMyInputGameplayAbility>(AbilitySpec.Ability);
    if(IsValid(MyAbility))
    {
        GetMyInputComponent()->BindAction
            (MyAbility->InputAction
            , ETriggerEvent::Triggered
            , this
            , &UMyAbilitySystemComponent::HandleInputAction
            );
    }
}

void UMyAbilitySystemComponent::HandleInputAction(const FInputActionInstance& InputActionInstance)
{
    for(auto Spec : GetActivatableAbilities())
    {
        const auto InputGameplayAbility = Cast<UMyInputGameplayAbility>(Spec.Ability);
        if(IsValid(InputGameplayAbility) && InputGameplayAbility->InputAction == InputActionInstance.GetSourceAction())
        {
            if(InputActionInstance.GetValue().Get<bool>())
            {
                TryActivateAbility(Spec.Handle);
            }
            else
            {
                Cast<UMyInputGameplayAbility>(Spec.Ability)->OnReleaseDelegate.Execute();
            }
        }
    }
}

UMyEnhancedInputComponent* UMyAbilitySystemComponent::GetMyInputComponent() const
{
    return Cast<UMyEnhancedInputComponent>(Cast<AMyPlayerController>(Cast<AMyPawn>(GetOwnerActor())->GetController())->InputComponent);
}

void UMyAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UMyAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UMyAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}