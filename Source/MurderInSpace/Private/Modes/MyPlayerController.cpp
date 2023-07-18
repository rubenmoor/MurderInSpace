#include "Modes/MyPlayerController.h"

#include <algorithm>

#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Actors/MyCharacter.h"
#include "Actors/MyPlayerStart.h"
#include "MyComponents/GyrationComponent.h"
#include "HUD/MyHUD.h"
#include "GameplayAbilitySystem/MyGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/FunctionLib.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MySessionManager.h"
#include "Modes/MyGameState.h"
#include "Input/MyTaggedInputAction.h"

AMyPlayerController::AMyPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if(!IsValid(MyInputActionsData))
    {
        UE_LOG(LogMyGame, Error, TEXT("MyInputActionsData: invalid. Need to set in Blueprint defaults"))
        return;
    }

    Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    Input->AddMappingContext(IMC_InGame.LoadSynchronous(), 0);

    // interface actions

    using enum EInputAction;
    BindAction<IngameMenuToggle       >();
    BindAction<MyTrajectoryShowHide   >();
    BindAction<AllTrajectoriesShowHide>();
    BindAction<MyTrajectoryToggle     >();
    BindAction<Zoom                   >();
    BindAction<Select                 >();
    
    // gameplay actions
    BindAction<TowardsCircleBeginEnd>();
    BindAction<AccelerateBeginEnd   >();
    BindAction<EmbraceBeginEnd      >();
    BindAction<KickPositionExecute  >();
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
    MyCharacter->SetRotationAim(Quat);
}

void AMyPlayerController::ServerRPC_HandleAction_Implementation(EInputAction Action)
{
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    AOrbit* Orbit = MyCharacter->GetOrbit();
    
    const auto Value = GetInputActionValue(Action);

    // so far all game play action are keys that rely on the triggers pressed and released
    // `bPressedReleased` is true for pressed, false for released
    // for any action that doesn't have a value of type bool, `bPressedReleased` doesn't make sense
    const bool bPressedReleased = Value.Get<bool>();
    
    switch (Action)
    {
    using enum EInputAction;
    case AccelerateBeginEnd:
        Orbit->bIsChanging              = bPressedReleased;
        MyCharacter->RP_bIsAccelerating = bPressedReleased;
        break;
    case TowardsCircleBeginEnd:
        Orbit->bIsChanging             = bPressedReleased;
        MyCharacter->RP_bTowardsCircle = bPressedReleased;
        break;
    case EmbraceBeginEnd:
        if(bPressedReleased)
        {
            MyCharacter->RP_ActionState.State = EActionState::Embracing;
        }
        else
        {
            MyCharacter->RP_ActionState.State = EActionState::Idle;
        }
        break;
    case KickPositionExecute:
        if(bPressedReleased)
        {
            MyCharacter->RP_ActionState.State = EActionState::KickPositioning;
        }
        else
        {
            // TODO: if the player doesn't hold the key for "minimum kick positioning time", the kick gets canceled
            MyCharacter->RP_ActionState = { EActionState::Idle, true };
            // TODO: execute kick
        }
        // TODO: cancel kick action
        break;
    default:
        UE_LOG
            ( LogMyGame
            , Error
            , TEXT("%s: the action %s isn't implemented here")
            , *GetFullName()
            , *UEnum::GetValueAsString(Action)
            )
        break;
    }
}

void AMyPlayerController::LocallyHandleAction(EInputAction Action)
{
    auto* GI = GetGameInstance<UMyGameInstance>();
    const AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    AOrbit* Orbit = MyCharacter->GetOrbit();
    UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(GetLocalPlayer());
    
    // polymorphic value, templated access via `Get<T>() -> T`
    auto Value = GetInputActionValue(Action);
    
    // well-defined but not meaningful for non-bool input action values
    auto bPressedReleased = Value.Get<bool>();
    // `FInputActionValue::Axis1D` is a type alias for `float`
    auto Axis1DValue = Value.Get<FInputActionValue::Axis1D>();
    
    switch (Action)
    {
    using enum EInputAction;
    case AccelerateBeginEnd:
    case TowardsCircleBeginEnd:
        Orbit->bIsVisibleAccelerating = bPressedReleased;
        if(bPressedReleased)
        {
            Orbit->SpawnSplineMesh
                ( MyCharacter->GetTempSplineMeshColor()
                , ESplineMeshParentSelector::Temporary
                , GI->InstanceUI
                );
            Orbit->UpdateVisibility(GI->InstanceUI);
        }
        else
        {
            Orbit->DestroyTempSplineMeshes();
            Orbit->UpdateVisibility(GI->InstanceUI);
        }
        break;
    case EmbraceBeginEnd:
        // maybe nothing? motion prediction?
        break;
    case KickPositionExecute:
        // maybe nothing? motion prediction?
        break;

    // UI
    case IngameMenuToggle:
        switch(LocalPlayer->InGame)
        {
        case EInGame::IngameMenu:   
            GetHUD<AMyHUD>()->InGameMenuHide();
            CurrentMouseCursor = EMouseCursor::Crosshairs;
            LocalPlayer->InGame = EInGame::IngamePlaying;
            break;
        case EInGame::IngamePlaying:
            GetHUD<AMyHUD>()->InGameMenuShow();
            CurrentMouseCursor = EMouseCursor::Default;
            LocalPlayer->InGame = EInGame::IngameMenu;
        case EInGame::IngameJoining:
        case EInGame::IngameUndefined:
            break;
        }
        break;
        
    case MyTrajectoryShowHide:
        Orbit->bIsVisibleShowMyTrajectory = bPressedReleased;
        Orbit->UpdateVisibility(GI->InstanceUI);
        break;
        
    case AllTrajectoriesShowHide:
        GI->InstanceUI.bShowAllTrajectories = bPressedReleased;
        for(TMyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
        {
            (*IOrbit)->UpdateVisibility(GI->InstanceUI);
        }
        break;
        
    case MyTrajectoryToggle:
        Orbit->bIsVisibleToggleMyTrajectory ^= true;
        Orbit->UpdateVisibility(GI->InstanceUI);
        break;
    case Zoom:
        if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
        {
            if(abs(Axis1DValue) > 0)
            {
                CameraPosition = std::clamp<int8>(CameraPosition - Axis1DValue, 0, MaxCameraPosition);
                GetPawn<AMyCharacter>()->UpdateSpringArm(CameraPosition);
                // TODO: at `CameraPosition = 0` lookAt mouse doesn't work anymore
            }		
        }
        break;
    case Select:
    {
        FHitResult HitResult;
        GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

        if(HitResult.IsValidBlockingHit())
        {
            AOrbit* SelectedOrbit = GI->InstanceUI.Selected.Orbit;
            if(IsValid(SelectedOrbit))
            {
                GI->InstanceUI.Selected.Orbit = nullptr;
                SelectedOrbit->UpdateVisibility(GI->InstanceUI);
            }
            if(HitResult.GetActor()->Implements<UHasOrbit>())
            {
                auto* ClickedOrbit = Cast<IHasOrbit>(HitResult.GetActor())->GetOrbit();
                if(SelectedOrbit != ClickedOrbit)
                {
                    double Size = Cast<IHasMesh>(HitResult.GetActor())->GetBounds().SphereRadius;
                    GI->InstanceUI.Selected = {ClickedOrbit, Size };
                }
                else
                {
                    // deselect selected orbit
                    GI->InstanceUI.Selected.Orbit = nullptr;
                }
                ClickedOrbit->UpdateVisibility(GI->InstanceUI);
            }
        }
        else
        {
            auto* SelectedOrbit = GI->InstanceUI.Selected.Orbit;
            if(IsValid(SelectedOrbit))
            {
                // deselect any selected orbit
                GI->InstanceUI.Selected.Orbit = nullptr;
                SelectedOrbit->UpdateVisibility(GI->InstanceUI);
            }
        }
        break;
    }
case KickCancel:
    // TODO
    break;
default: ;
    }
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

FInputActionValue AMyPlayerController::GetInputActionValue(EInputAction InputAction)
{
    return Input->GetPlayerInput()->GetActionValue(GetInputAction(InputAction));
}

UInputAction* AMyPlayerController::GetInputAction(EInputAction InputAction)
{
    const FGameplayTag Tag = FMyGameplayTags::Get().GetInputActionTag(InputAction);
    return MyInputActionsData->Data[Tag];
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
                MyCharacter->SetRotationAim(Quat);
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
