#include "Modes/MyPlayerController.h"

#include <algorithm>

#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyGameplayTags.h"
#include "Actors/MyCharacter.h"
#include "Actors/MyPlayerStart.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "GameplayAbilitySystem/MyDeveloperSettings.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"
#include "MyComponents/GyrationComponent.h"
#include "HUD/MyHUD.h"
#include "Input/MyInputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/FunctionLib.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MySessionManager.h"
#include "Modes/MyGameState.h"

AMyPlayerController::AMyPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    Input->AddMappingContext(IMC_InGame.LoadSynchronous(), 0);

    for(auto Ptr : GetDefault<UMyDeveloperSettings>()->MyInputActions)
    {
        auto InputAction = Ptr.LoadSynchronous();
        for(auto Trigger : InputAction->Triggers)
        {
            if(Trigger.IsA(UInputTriggerPressed::StaticClass()))
            {
                GetInputComponent()->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::HandlePressed);
            }
            else if(Trigger.IsA(UInputTriggerReleased::StaticClass()))
            {
                GetInputComponent()->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::HandleReleased);
            }
            else if(Trigger.IsA(UInputTriggerTap::StaticClass()))
            {
                GetInputComponent()->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::HandleTapped);
            }
        }
    };
}

void AMyPlayerController::Zoom(float Delta)
{
    if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
    {
        if(abs(Delta) > 0)
        {
            CameraPosition = std::clamp<int8>(CameraPosition - Delta, 0, MaxCameraPosition);
            GetPawn<AMyCharacter>()->UpdateSpringArm(CameraPosition);
            // TODO: at `CameraPosition = 0` lookAt mouse doesn't work anymore
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

// void AMyPlayerController::LocallyHandleAction(EInputAction Action, const FInputActionInstance& IAInstance)
// {
//     auto* GI = GetGameInstance<UMyGameInstance>();
//     const AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
//     AOrbit* Orbit = MyCharacter->GetOrbit();
//     UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(GetLocalPlayer());
//     
//     // well-defined but not meaningful for non-bool input action values
//     //auto bPressedReleased = Value.Get<bool>();
//     auto bPressedReleased = IAInstance.GetValue().Get<bool>();
//     // `FInputActionValue::Axis1D` is a type alias for `float`
//     auto Axis1DValue = IAInstance.GetValue().Get<FInputActionValue::Axis1D>();
//     
//     switch (Action)
//     {
//     using enum EInputAction;
//     case AccelerateBeginEnd:
//     case TowardsCircleBeginEnd:
//         // TODO: move to local gameplay cue
//         Orbit->bIsVisibleAccelerating = bPressedReleased;
//         if(bPressedReleased)
//         {
//             Orbit->SpawnSplineMesh
//                 ( MyCharacter->GetTempSplineMeshColor()
//                 , ESplineMeshParentSelector::Temporary
//                 , GI->InstanceUI
//                 );
//             Orbit->UpdateVisibility(GI->InstanceUI);
//         }
//         else
//         {
//             Orbit->DestroyTempSplineMeshes();
//             Orbit->UpdateVisibility(GI->InstanceUI);
//         }
//         break;
//     case EmbraceBeginEnd:
//         // maybe nothing? motion prediction?
//         break;
//     case KickPositionExecute:
//         // maybe nothing? motion prediction?
//         break;
// 
//     // UI
//     case IngameMenuToggle:
//         switch(LocalPlayer->InGame)
//         {
//         case EInGame::IngameMenu:   
//             GetHUD<AMyHUD>()->InGameMenuHide();
//             CurrentMouseCursor = EMouseCursor::Crosshairs;
//             LocalPlayer->InGame = EInGame::IngamePlaying;
//             break;
//         case EInGame::IngamePlaying:
//             GetHUD<AMyHUD>()->InGameMenuShow();
//             CurrentMouseCursor = EMouseCursor::Default;
//             LocalPlayer->InGame = EInGame::IngameMenu;
//         case EInGame::IngameJoining:
//         case EInGame::IngameUndefined:
//             break;
//         }
//         break;
//         
//     case MyTrajectoryShowHide:
//         Orbit->bIsVisibleShowMyTrajectory = bPressedReleased;
//         Orbit->UpdateVisibility(GI->InstanceUI);
//         break;
//         
//     case AllTrajectoriesShowHide:
//         GI->InstanceUI.bShowAllTrajectories = bPressedReleased;
//         for(TMyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
//         {
//             (*IOrbit)->UpdateVisibility(GI->InstanceUI);
//         }
//         break;
//         
//     case MyTrajectoryToggle:
//         Orbit->bIsVisibleToggleMyTrajectory ^= true;
//         Orbit->UpdateVisibility(GI->InstanceUI);
//         break;
//     case Zoom:
//         if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
//         {
//             if(abs(Axis1DValue) > 0)
//             {
//                 CameraPosition = std::clamp<int8>(CameraPosition - Axis1DValue, 0, MaxCameraPosition);
//                 GetPawn<AMyCharacter>()->UpdateSpringArm(CameraPosition);
//                 // TODO: at `CameraPosition = 0` lookAt mouse doesn't work anymore
//             }		
//         }
//         break;
//     case Select:
//     {
//         FHitResult HitResult;
//         GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
// 
//         if(HitResult.IsValidBlockingHit())
//         {
//             AOrbit* SelectedOrbit = GI->InstanceUI.Selected.Orbit;
//             if(IsValid(SelectedOrbit))
//             {
//                 GI->InstanceUI.Selected.Orbit = nullptr;
//                 SelectedOrbit->UpdateVisibility(GI->InstanceUI);
//             }
//             if(HitResult.GetActor()->Implements<UHasOrbit>())
//             {
//                 auto* ClickedOrbit = Cast<IHasOrbit>(HitResult.GetActor())->GetOrbit();
//                 if(SelectedOrbit != ClickedOrbit)
//                 {
//                     double Size = Cast<IHasMesh>(HitResult.GetActor())->GetBounds().SphereRadius;
//                     GI->InstanceUI.Selected = {ClickedOrbit, Size };
//                 }
//                 else
//                 {
//                     // deselect selected orbit
//                     GI->InstanceUI.Selected.Orbit = nullptr;
//                 }
//                 ClickedOrbit->UpdateVisibility(GI->InstanceUI);
//             }
//         }
//         else
//         {
//             auto* SelectedOrbit = GI->InstanceUI.Selected.Orbit;
//             if(IsValid(SelectedOrbit))
//             {
//                 // deselect any selected orbit
//                 GI->InstanceUI.Selected.Orbit = nullptr;
//                 SelectedOrbit->UpdateVisibility(GI->InstanceUI);
//             }
//         }
//         break;
//     }
// case KickCancel:
//     // TODO
//     break;
// default: ;
//     }
// }

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

void AMyPlayerController::Tick(float DeltaSeconds)
{
    // only ticking for player controller that controls local pawn
    Super::Tick(DeltaSeconds);

    UMyLocalPlayer* MyPlayer = Cast<UMyLocalPlayer>(Player);
    if(!IsValid(MyPlayer))
    {
        return;
    }
    
    if(MyPlayer->GetIsInMainMenu())
    {
        return;
    }
    // react to mouse movement
    
    auto* GI = GetGameInstance<UMyGameInstance>();
    
    // hovering
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    auto* Body = HitResult.GetActor();
    if (   HitResult.IsValidBlockingHit()
        && Body->Implements<UHasOrbit>()
        && Body->Implements<UHasMesh>()
        )
    {
        // begin mouse over
        auto* Orbit = Cast<IHasOrbit>(Body)->GetOrbit();
        const double Size = Cast<IHasMesh>(Body)->GetBounds().SphereRadius;
        GI->InstanceUI.Hovered = {Orbit, Size };
        Orbit->bIsVisibleMouseover = true;
        Orbit->UpdateVisibility(GI->InstanceUI);
    }
    else
    {
        // end mouse over
        auto* HoveredOrbit = GI->InstanceUI.Hovered.Orbit;
        if(IsValid(HoveredOrbit))
        {
            GI->InstanceUI.Hovered.Orbit = nullptr;
            HoveredOrbit->bIsVisibleMouseover = false;
            HoveredOrbit->UpdateVisibility(GI->InstanceUI);
        }
    }
    
    // reacting to mouse movement
    const FVector VecMouseDirection = GetMouseDirection();
    const FQuat Quat = FQuat::FindBetween(FVector::UnitX(), VecMouseDirection);
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    if
        (  MyCharacter->RP_ActionState.State == EActionState::Idle
        || MyCharacter->RP_ActionState.State == EActionState::RotatingCW
        || MyCharacter->RP_ActionState.State == EActionState::RotatingCCW
        )
    {
        const double AngleDelta =
              Quat.GetTwistAngle(FVector(0, 0, 1))
            - MyCharacter->GetActorQuat().GetTwistAngle(FVector(0, 0, 1));
        if(abs(AngleDelta) > 15. / 180. * PI)
        {
            MyCharacter->RP_ActionState.State = AngleDelta > 0
                ? EActionState::RotatingCCW
                : EActionState::RotatingCW;

            // server-only
            ServerRPC_RotateTowards(Quat);
            
            if(GetLocalRole() == ROLE_AutonomousProxy)
            {
                // "movement prediction"
                //MyCharacter->SetRotationAim(Quat);
            }
        }
        else
        {
            MyCharacter->RP_ActionState.State = EActionState::Idle;
        }
    }
    else if(MyCharacter->RP_ActionState.State == EActionState::KickPositioning)
    {
        // TODO:
    }
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

    auto* MyState = UMyState::Get();
    auto* GI = GetGameInstance<UMyGameInstance>();
    auto* GS = GetWorld()->GetGameState<AMyGameState>();
    const FInstanceUI InstanceUI = MyState->GetInstanceUI(GI);
    const FPhysics Physics = MyState->GetPhysics(GS);
    PawnWithOrbit->SetInitialOrbitParams(MyPlayerStart->GetInitialOrbitParams());
    Orbit->UpdateByInitialParams(Physics, InstanceUI);

    Orbit->SetEnableVisibility(true);
    //PawnWithOrbit->SetInitialOrbitParams({FVector::Zero(), FVector(0., 0., 1.)});

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
}

void AMyPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    auto* GI = GetGameInstance<UMyGameInstance>();
    auto* MyState = UMyState::Get();
    auto InstanceUI = MyState->GetInstanceUI(GI);
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
        UE_LOG
            (LogNet, Warning, TEXT("%s: Couldn't determine current level: %s, setting to 'any test map'")
            , *GetFullName()
            , *LevelName
            )
        CurrentLevel = ECurrentLevel::AnyTestMap;
    }
    auto* LocalPlayer = Cast<UMyLocalPlayer>(GetLocalPlayer());
    LocalPlayer->CurrentLevel = CurrentLevel;
    LocalPlayer->InGame = EInGame::IngamePlaying;
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if(IMC_InGame.IsNull())
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: IMC_InGame null"), *GetFullName())
    }
    
    const FInputModeGameAndUI InputModeGameAndUI;
    SetInputMode(InputModeGameAndUI);
}

void AMyPlayerController::HandlePressed(const FInputActionInstance& InputActionInstance)
{
    if(InputActionInstance.GetValue().Get<bool>())
    {
        const auto InputActionTags = Cast<UMyInputAction>(InputActionInstance.GetSourceAction())->Tags;
        AbilitySystemComponent->TryActivateAbilitiesByTag(InputActionTags);

        const auto Tag = FMyGameplayTags::Get();
        auto Cues = InputActionTags.Filter(Tag.GameplayCue.GetSingleTagContainer());
        for(const auto Cue : Cues)
        {
            FGameplayCueParameters Parameters;
            AbilitySystemComponent->AddGameplayCueLocal(Cue, Parameters);
        }
    }
}

void AMyPlayerController::HandleReleased(const FInputActionInstance& InputActionInstance)
{
    if(!InputActionInstance.GetValue().Get<bool>())
    {
        const auto InputActionTags = Cast<UMyInputAction>(InputActionInstance.GetSourceAction())->Tags;
        for(auto Spec : AbilitySystemComponent->GetActiveAbilities(&InputActionTags))
        {
            Cast<UMyGameplayAbility>(Spec.Ability)->OnReleaseDelegate.Execute();
        }

        const auto Tag = FMyGameplayTags::Get();
        auto Cues = InputActionTags.Filter(Tag.GameplayCue.GetSingleTagContainer());
        for(auto Cue : Cues)
        {
            FGameplayCueParameters Parameters;
            AbilitySystemComponent->RemoveGameplayCueLocal(Cue, Parameters);
        }
    }
}

void AMyPlayerController::HandleTapped(const FInputActionInstance& InputActionInstance)
{
    const auto InputActionTags = Cast<UMyInputAction>(InputActionInstance.GetSourceAction())->Tags;
    if(!AbilitySystemComponent->TryActivateAbilitiesByTag(InputActionTags))
    {
        AbilitySystemComponent->CancelAbilities(&InputActionTags);
    }

    const auto Tag = FMyGameplayTags::Get();
    auto Cues = InputActionTags.Filter(Tag.GameplayCue.GetSingleTagContainer());
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
}
