// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerController.h"

#include <algorithm>

#include "EnhancedInputSubsystems.h"
#include "Actors/MyCharacter.h"
#include "Actors/MyPlayerStart.h"
#include "MyComponents/GyrationComponent.h"
#include "HUD/MyHUD.h"
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

    if(!IsValid(MyInputActionsData))
    {
        UE_LOG(LogMyGame, Error, TEXT("MyInputActionsData: invalid. Need to set in Blueprint defaults"))
        return;
    }

    Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    Input->AddMappingContext(IMC_InGame.LoadSynchronous(), 0);

    // interface actions

    BindAction<EInputAction::IngameMenuToggle       >();
    BindAction<EInputAction::MyTrajectoryShowHide   >();
    BindAction<EInputAction::AllTrajectoriesShowHide>();
    BindAction<EInputAction::MyTrajectoryToggle     >();
    BindAction<EInputAction::Zoom                   >();
    BindAction<EInputAction::Select                 >();
    
    // gameplay actions
    BindAction<EInputAction::TowardsCircleBeginEnd>();
    BindAction<EInputAction::AccelerateBeginEnd   >();
    BindAction<EInputAction::EmbraceBeginEnd      >();
    BindAction<EInputAction::KickPositionExecute  >();
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

void AMyPlayerController::ServerRPC_LookAt_Implementation(FQuat Quat)
{
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    MyCharacter->RP_Rotation = Quat;
    MyCharacter->OnRep_Rotation();
}

void AMyPlayerController::ServerRPC_HandleAction_Implementation(EInputAction Action)
{
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    AOrbit* Orbit = MyCharacter->GetOrbit();
    switch (Action)
    {
    using enum EInputAction;
    case AccelerateBeginEnd:
        Orbit->ToggleIsChanging();
        MyCharacter->RP_bIsAccelerating ^= true;
        break;
    case TowardsCircleBeginEnd:
        Orbit->ToggleIsChanging();
        MyCharacter->RP_bTowardsCircle ^= true;
        break;
    case EmbraceBeginEnd:
        MyCharacter->RP_bActionIdle    =  MyCharacter->RP_bActionEmbrace;
        MyCharacter->RP_bActionEmbrace ^= true;
        break;
    case KickPositionExecute:
        {
        auto* MyState = UMyState::Get();
        auto* IA = MyState->GetInputAction(MyInputActionsData, KickPositionExecute);
        auto Value = Input->GetPlayerInput()->GetActionValue(IA);
        bool bPressed = Value.Get<bool>();
        UE_LOG(LogMyGame, Warning, TEXT("%s: bPressed: %s"), *GetFullName(), bPressed ? TEXT("true") : TEXT("false"))
        }
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
    
    switch (Action)
    {
    using enum EInputAction;
    case AccelerateBeginEnd:
    case TowardsCircleBeginEnd:
        if(Orbit->bIsVisibleAccelerating)
        {
            Orbit->bIsVisibleAccelerating = false;
            Orbit->DestroyTempSplineMeshes();
            Orbit->UpdateVisibility(GI->InstanceUI);
        }
        else
        {
            Orbit->SpawnSplineMesh
                ( MyCharacter->GetTempSplineMeshColor()
                , ESplineMeshParentSelector::Temporary
                , GI->InstanceUI
                );
            Orbit->bIsVisibleAccelerating = true;
            Orbit->UpdateVisibility(GI->InstanceUI);
        }
        break;
    case EmbraceBeginEnd:
        break;
    case KickPositionExecute:
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
        
    case EInputAction::MyTrajectoryShowHide:
        Orbit->bIsVisibleShowMyTrajectory = !Orbit->bIsVisibleShowMyTrajectory;
        Orbit->UpdateVisibility(GI->InstanceUI);
        break;
        
    case EInputAction::AllTrajectoriesShowHide:
        // TODO: set InstanceUI in GameInstance
        GI->InstanceUI.bShowAllTrajectories = !GI->InstanceUI.bShowAllTrajectories;
        for(TMyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
        {
            (*IOrbit)->UpdateVisibility(GI->InstanceUI);
        }
        break;
        
    case EInputAction::MyTrajectoryToggle:
        Orbit->bIsVisibleToggleMyTrajectory = !Orbit->bIsVisibleToggleMyTrajectory;
        Orbit->UpdateVisibility(GI->InstanceUI);
        break;
    case EInputAction::Zoom:
        {
            auto* MyState = UMyState::Get();
            auto* IA = MyState->GetInputAction(MyInputActionsData, EInputAction::Zoom);
            auto Value = Input->GetPlayerInput()->GetActionValue(IA);
            float Delta = Value.Get<float>();
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
        break;
    case EInputAction::Select:
        
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
    
    if(!MyPlayer->GetIsInMainMenu())
    {
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
        
        // reacting to mouse move
        AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
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
            const FVector VecDirection = VecP - VecMe;
            const FQuat Quat = FQuat::FindBetween(FVector(1, 0, 0), VecDirection);
            const double AngleDelta = Quat.GetTwistAngle
                ( FVector(0, 0, 1)) -
                    MyCharacter->GetActorQuat().GetTwistAngle(FVector(0, 0, 1)
                );
            if(abs(AngleDelta) > 15. / 180. * PI)
            {
                // server-only
                ServerRPC_LookAt(Quat);
                
                if(GetLocalRole() == ROLE_AutonomousProxy)
                {
                    // "movement prediction"
                    MyCharacter->RP_Rotation = Quat;
                    MyCharacter->OnRep_Rotation();
                }
            }
            // debugging direction
            DrawDebugDirectionalArrow(GetWorld(), VecMe, VecP, 20, FColor::Red);
        }
    }
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
        UE_LOG(LogNet, Error, TEXT("%s: Couldn't determine current level: %s"), *GetFullName(), *LevelName)
        CurrentLevel = ECurrentLevel::MainMenu;
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
