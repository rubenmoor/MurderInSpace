#include "Modes/MyPlayerController.h"

#include <algorithm>
#include <Input/MyInputActionSet.h>

#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyGameplayTags.h"
#include "Spacebodies/MyCharacter.h"
#include "Spacebodies/MyPlayerStart.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "MyComponents/GyrationComponent.h"
#include "HUD/MyHUD.h"
#include "Input/MyInputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/FunctionLib.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MySessionManager.h"
#include "Modes/MyGameState.h"
#include "EnhancedInputComponent.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"
#include "Input/MyInputActions.h"

AMyPlayerController::AMyPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

    TArray<FDuplicateKeyBinding> Duplicates;
    
    auto* IMC = NewObject<UInputMappingContext>(this, "InputMappingContext");
    for(auto [Name, InputActionSet] : MyInputActions->Map)
    {
        InputActionSet->BindActions(this, IMC);
        CheckForDuplicateKeyBindings(InputActionSet, Duplicates);
    }

    for(auto [Key, IAS1, IAS2, CommonTriggers] : Duplicates)
    {
        FString StrCommonTriggers = "";
        bool bFirst = true;
        for(const auto Trigger : CommonTriggers)
        {
            if(!bFirst)
            {
                StrCommonTriggers += ", ";
            }
            StrCommonTriggers += UEnum::GetDisplayValueAsText(Trigger).ToString();
            bFirst = false;
        }
        UE_LOGFMT(LogMyGame, Warning, "Duplicate binding for key {KEY}: {IAS1} - {IAS2}: common triggers: {TRIGGERS}"
            , Key.GetDisplayName().ToString()
            , IAS1->GetFName()
            , IAS2->GetFName()
            , StrCommonTriggers
            );
    }
    
    Input->AddMappingContext(IMC, 0);
}

void AMyPlayerController::CheckForDuplicateKeyBindings(UMyInputActionSet* IAS1, TArray<FDuplicateKeyBinding>& DuplicateKeyBindings)
{
    TMap<FKey, TTuple<UMyInputActionSet*, TSet<EInputTrigger>>> KeyMap;
    
    for(auto Key : IAS1->Keys)
    {
        if(auto* Tuple = KeyMap.Find(Key))
        {
            UMyInputActionSet* IAS2;
            TSet<EInputTrigger> OtherTriggers;
            Tie(IAS2, OtherTriggers) = *Tuple;
            if(auto CommonTriggers = OtherTriggers.Intersect(IAS1->InputTriggers); !CommonTriggers.IsEmpty())
            {
                DuplicateKeyBindings.Add({ Key, IAS1, IAS2, CommonTriggers});
            }
        }
        else
        {
            KeyMap.Add(Key, TTuple<UMyInputActionSet*, TSet<EInputTrigger>>(IAS1, IAS1->InputTriggers));
        }
    }
}

void AMyPlayerController::ClientRPC_LeaveSession_Implementation()
{
    GetGameInstance()->GetSubsystem<UMySessionManager>()->LeaveSession();
}

void AMyPlayerController::ServerRPC_RotateTowards_Implementation(FQuat Quat)
{
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    //MyCharacter->SetRotationAim(Quat);
}

FVector AMyPlayerController::GetMouseDirection()
{
    const AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    FVector Position, Direction;
    DeprojectMousePositionToWorld(Position, Direction);
    if(abs(Direction.Z) > 1e-8)
    {
        // TODO: only works for Position.Z == 0
        const double X = Position.X - Direction.X * Position.Z / Direction.Z;
        const double Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
        const double Z = MyCharacter->GetActorLocation().Z;
        // TODO: physical rotation/animation instead

        const FVector VecP(X, Y, Z);
        
        const FVector VecMe = MyCharacter->GetActorLocation();
        return VecP - VecMe;
    }
    else
    {
        return FVector(1., 0., 0.);
    }       
}

UEnhancedInputComponent* AMyPlayerController::GetInputComponent()
{
     return Cast<UEnhancedInputComponent>(InputComponent);
}

void AMyPlayerController::RunInputAction(const FGameplayTagContainer& InputActionTags, EInputTrigger InputTrigger,
    const FInputActionInstance& InputActionInstance)
{
    auto& Tag = FMyGameplayTags::Get();
    const auto InputAbilityTags = InputActionTags.Filter(Tag.InputBindingAbility.GetSingleTagContainer());
    auto Cues = InputActionTags.Filter(Tag.GameplayCue.GetSingleTagContainer());
    const auto InputCustomTags = InputActionTags.Filter(Tag.InputBindingCustom.GetSingleTagContainer());

    for(auto InputCustomTag : InputCustomTags)
    {
        RunCustomInputAction(InputCustomTag, InputTrigger, InputActionInstance);
    }
    
    if(!InputAbilityTags.IsEmpty())
    {
        TArray<FGameplayAbilitySpec*> Specs;
        switch(InputTrigger)
        {
        case EInputTrigger::Down:
        case EInputTrigger::Pressed:
        case EInputTrigger::Hold:
            AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InputAbilityTags, Specs, false);
            for(auto Spec : Specs)
            {
                check(!Spec->IsActive())
                AbilitySystemComponent->TryActivateAbility(Spec->Handle);
            }

            for(const auto Cue : Cues)
            {
                FGameplayCueParameters Parameters;
                AbilitySystemComponent->AddGameplayCueLocal(Cue, Parameters);
            }
            break;
        case EInputTrigger::Released:
        case EInputTrigger::HoldAndRelease:
            for(auto Spec : AbilitySystemComponent->GetActiveAbilities(&InputAbilityTags))
            {
                Cast<UMyGameplayAbility>(Spec.Ability)->SetReleased();
            }

            for(auto Cue : Cues)
            {
                FGameplayCueParameters Parameters;
                AbilitySystemComponent->RemoveGameplayCueLocal(Cue, Parameters);
            }
            break;
        case EInputTrigger::Tap:
            AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InputAbilityTags, Specs, false);
            for(auto Spec : Specs)
            {
                if(Spec->IsActive())
                {
                    Cast<UMyGameplayAbility>(Spec->Ability)->SetReleased();
                }
                else
                {
                    AbilitySystemComponent->TryActivateAbility(Spec->Handle);
                }
            }
            
            for(auto Cue : Cues)
            {
                FGameplayCueParameters Parameters;
                if(AbilitySystemComponent->IsGameplayCueActive(Cue))
                {
                    AbilitySystemComponent->RemoveGameplayCueLocal(Cue, Parameters);
                }
                else
                {
                    AbilitySystemComponent->AddGameplayCueLocal(Cue, Parameters);
                }
            }
            break;
        case EInputTrigger::Pulse:
        case EInputTrigger::ChordAction:
            UE_LOGFMT(LogMyGame, Error, "{THIS}: Input triggers Pulse and Chord not implemented.", GetFName());
            break;
        }
    }
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
    // only ticking for player controller that controls local pawn
    Super::Tick(DeltaSeconds);

    UMyLocalPlayer* MyPlayer = Cast<UMyLocalPlayer>(Player);
    if(!IsValid(MyPlayer))
    {
        return;
    }
    
    // react to mouse movement
    
    // hovering
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    auto* Body = HitResult.GetActor();
    if  (  HitResult.IsValidBlockingHit()
        && Body->Implements<UHasOrbit>()
        && Body->Implements<UHasMesh>()
        && Body != GetPawn()
        )
    {
        if(IsValid(Hovered.Orbit))
        {
            // un-hover previously hovered orbit
            Hovered.Orbit->UpdateVisibility(false);
        }
        // begin mouse over
        auto* Orbit = Cast<IHasOrbit>(Body)->GetOrbit();
        const double Size = Cast<IHasMesh>(Body)->GetBounds().SphereRadius;
        Hovered = {Orbit, Size};
        Orbit->UpdateVisibility(true);
    }
    else
    {
        // end mouse over
        if(IsValid(Hovered.Orbit))
        {
            Hovered.Orbit->UpdateVisibility(false);
            Hovered.Orbit = nullptr;
        }
    }
    
    // reacting to mouse movement
    const FVector VecMouseDirection = GetMouseDirection();
    const FQuat Quat = FQuat::FindBetween(FVector::UnitX(), VecMouseDirection);
    auto MyCharacter = GetPawn<AMyCharacter>();
    // if
    //     (  MyCharacter->RP_ActionState.State == EActionState::Idle
    //     || MyCharacter->RP_ActionState.State == EActionState::RotatingCW
    //     || MyCharacter->RP_ActionState.State == EActionState::RotatingCCW
    //     )
    // {
    //     const double AngleDelta =
    //           Quat.GetTwistAngle(FVector(0, 0, 1))
    //         - MyCharacter->GetActorQuat().GetTwistAngle(FVector(0, 0, 1));
    //     if(abs(AngleDelta) > 15. / 180. * PI)
    //     {
    //         MyCharacter->RP_ActionState.State = AngleDelta > 0
    //             ? EActionState::RotatingCCW
    //             : EActionState::RotatingCW;

    //         // server-only
    //         ServerRPC_RotateTowards(Quat);
    //         
    //         if(GetLocalRole() == ROLE_AutonomousProxy)
    //         {
    //             // "movement prediction"
    //             //MyCharacter->SetRotationAim(Quat);
    //         }
    //     }
    //     else
    //     {
    //         MyCharacter->RP_ActionState.State = EActionState::Idle;
    //     }
    // }
    // else if(MyCharacter->RP_ActionState.State == EActionState::KickPositioning)
    // {
    //     // TODO:
    // }
    // debugging direction
    const FVector VecMe = MyCharacter->GetActorLocation();
    DrawDebugDirectionalArrow(GetWorld(), VecMe, VecMe + VecMouseDirection, 20, FColor::Red);
}

// server-only
void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    auto* PawnWithOrbit = Cast<IHasOrbit>(InPawn);
    AOrbit* Orbit = PawnWithOrbit->GetOrbit();

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();
    const FPhysics Physics = GS->RP_Physics;
    
    PawnWithOrbit->SetInitialOrbitParams(MyPlayerStart->GetInitialOrbitParams());
    Orbit->UpdateByInitialParams(Physics);
    Orbit->SetEnableVisibility(true);

    // exclude the case of the server-player possessing their pawn
    if(!InPawn->IsLocallyControlled())
    {
        for(TMyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
        {
            (*IOrbit)->FreezeOrbitState();
        }
        
        // freeze gyration state for all existing gyration components for replication (condition: initial only)
        auto FilterGyrations = [this, InPawn] (const UGyrationComponent* Gyration) -> bool
        {
            const AMyPawn* Owner = Gyration->GetOwner<AMyPawn>();
            return GetWorld() == Gyration->GetWorld()
                // exclude the gyration of `InPawn`
                && (Owner != InPawn);
        };
        for(TMyObjectIterator<UGyrationComponent> IGyration(FilterGyrations); IGyration; ++IGyration)
        {
            (*IGyration)->FreezeState();
        }
    }

    // trigger orbit replication when tag AccelerateTranslational is removed
    const auto& Tag = FMyGameplayTags::Get();
    AbilitySystemComponent->RegisterGameplayTagEvent
        ( Tag.AccelerationTranslational
        , EGameplayTagEventType::NewOrRemoved
        ).AddLambda([this, Orbit] (const FGameplayTag Tag, int32 Count)
        {
            UE_LOGFMT(LogMyGame, Display, "{THIS} Tag changed: {TAG}, new count: {COUNT}", GetFName(), Tag.GetTagName(), Count);
            if(Count == 0)
            {
                Orbit->FreezeOrbitState();
            }
        });
}

void AMyPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    AMyCharacter* MyCharacter = Cast<AMyCharacter>(P);

    AbilitySystemComponent = UMyAbilitySystemComponent::Get(MyCharacter);
    
    MyCharacter->GetAbilitySystemComponent()->InitAbilityActorInfo(MyCharacter, MyCharacter);
    MyCharacter->UpdateSpringArm(CameraPosition);
    MyCharacter->ShowEffects();
    
    const FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
    ECurrentLevel CurrentLevel;
    if(LevelName == FString(TEXT("MainMenu")))
    {
        CurrentLevel = ECurrentLevel::MainMenu;
    }
    else if(LevelName == FString(TEXT("Spacefootball")))
    {
        CurrentLevel = ECurrentLevel::SpaceFootball;
    }
    else
    {
        UE_LOGFMT
            (LogNet, Warning, "{0}: Couldn't determine current level: {0}, setting to 'any test map'"
            , GetFName()
            , LevelName
            );
        CurrentLevel = ECurrentLevel::AnyTestMap;
    }
    auto* LocalPlayer = Cast<UMyLocalPlayer>(GetLocalPlayer());
    LocalPlayer->CurrentLevel = CurrentLevel;
    LocalPlayer->InGame = EInGame::IngamePlaying;
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    const FInputModeGameAndUI InputModeGameAndUI;
    SetInputMode(InputModeGameAndUI);
}

void AMyPlayerController::RunCustomInputAction(FGameplayTag CustomBindingTag, EInputTrigger InputTrigger, const FInputActionInstance& InputActionInstance)
{
    const auto& Tag = FMyGameplayTags::Get();

    // select
    if(CustomBindingTag == Tag.InputBindingCustomSelect)
    {
         FHitResult HitResult;
         GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

         if  (
                HitResult.IsValidBlockingHit()
             && HitResult.GetActor()->Implements<UHasOrbit>()
             && HitResult.GetActor() != GetPawn()
             )
         {
             auto* ClickedOrbit = Cast<IHasOrbit>(HitResult.GetActor())->GetOrbit();
             if(Selected.Orbit != ClickedOrbit)
             {
                 if(IsValid(Selected.Orbit))
                 { // deselect selected orbit
                     Selected.Orbit->UpdateVisibility(false);
                     Selected.Orbit = nullptr;
                 }
                 double Size = Cast<IHasMesh>(HitResult.GetActor())->GetBounds().SphereRadius;
                 Selected = {ClickedOrbit, Size };
                 Selected.Orbit->UpdateVisibility(true);
             }
         }
         else if(IsValid(Selected.Orbit))
         {
             Selected.Orbit->UpdateVisibility(false);
             Selected.Orbit = nullptr;
         }
    }
    
    // zoom
    else if(CustomBindingTag == Tag.InputBindingCustomZoom)
    {
        const auto Delta = static_cast<int8>(InputActionInstance.GetValue().Get<FInputActionValue::Axis1D>());
        if(abs(Delta) > 0)
        {
            CameraPosition = std::clamp<uint8>(CameraPosition - Delta, 0, MaxCameraPosition);
            GetPawn<AMyCharacter>()->UpdateSpringArm(CameraPosition);
        }		
    }

    // all orbits show/hide
    else if(CustomBindingTag == Tag.InputBindingCustomAllOrbitsShowHide)
    {
        using enum EInputTrigger;
        if(InputTrigger == Pressed || InputTrigger == Released)
        {
            ShowAllOrbitsDelegate.Broadcast(InputActionInstance.GetValue().Get<bool>());
        }
        else
        {
            UE_LOGFMT(LogMyGame, Warning, "{0} Unsupported trigger: {1}", GetFName(), UEnum::GetValueAsString(InputTrigger));
        }
    }

    // my orbit show/hide
    else if(CustomBindingTag == Tag.InputBindingCustomMyOrbitShowHide)
    {
        const auto Orbit = Cast<IHasOrbit>(GetPawn())->GetOrbit();
        switch(InputTrigger)
        {
        case EInputTrigger::Down:
        case EInputTrigger::Pressed:
        case EInputTrigger::Hold:
            Orbit->UpdateVisibility(true);
            break;
        case EInputTrigger::Released:
        case EInputTrigger::HoldAndRelease:
            Orbit->UpdateVisibility(false);
            break;
        case EInputTrigger::Tap:
            Orbit->UpdateVisibility(bMyOrbitShowHide ^= true);
            break;
        case EInputTrigger::Pulse:
        case EInputTrigger::ChordAction:
            UE_LOGFMT(LogMyGame, Error, "{THIS}: Input triggers Pulse and Chord not implemented.", GetFName());
            break;
        }
    }

    else if(CustomBindingTag == Tag.InputBindingCustomIngameMenuToggle)
    {
        auto* LocalPlayer = Cast<UMyLocalPlayer>(GetLocalPlayer());
        if(LocalPlayer->InGame == EInGame::IngameMenu)
        {
            GetHUD<AMyHUD>()->InGameMenuHide();
            CurrentMouseCursor = EMouseCursor::Crosshairs;
            LocalPlayer->InGame = EInGame::IngamePlaying;
        }
        else
        {
            GetHUD<AMyHUD>()->InGameMenuShow();
            CurrentMouseCursor = EMouseCursor::Default;
            LocalPlayer->InGame = EInGame::IngameMenu;
        }
    }
}
