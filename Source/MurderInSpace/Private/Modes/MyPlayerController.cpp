// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerController.h"

#include <algorithm>

#include "EnhancedInputSubsystems.h"
#include "Actors/MyCharacter.h"
#include "Actors/GyrationComponent.h"
#include "HUD/MyHUD.h"
#include "Input/MyEnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/FunctionLib.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MySessionManager.h"

AMyPlayerController::AMyPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputLocalPlayerSubsystem* Input =
        GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    Input->AddMappingContext(IMC_InGame.LoadSynchronous(), 0);

    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
    
    // interface actions
    InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::Zoom);

    UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(Player);

    const FInputTag InputTag = MyState->GetInputTags();
    BindPureUIAction<EPureUIAction::ToggleIngameMenu>(InputTag.ToggleIngameMenu);
    BindPureUIAction<EPureUIAction::ShowMyTrajectory>(InputTag.ShowMyTrajectory);
    BindPureUIAction<EPureUIAction::HideMyTrajectory>(InputTag.HideMyTrajectory);
    BindPureUIAction<EPureUIAction::ShowAllTrajectories>(InputTag.ShowAllTrajectories);
    BindPureUIAction<EPureUIAction::HideAllTrajectories>(InputTag.HideAllTrajectories);
    BindPureUIAction<EPureUIAction::ToggleMyTrajectory>(InputTag.ToggleMyTrajectory);
    
    // gameplay actions
    BindEAction("Accelerate", IE_Pressed , EAction::ACCELERATE_BEGIN);
    BindEAction("Accelerate", IE_Released, EAction::ACCELERATE_END  );
}

void AMyPlayerController::Zoom(float Delta)
{
    if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
    {
        if(abs(Delta) > 0)
        {
            CameraPosition = std::clamp<int8>(CameraPosition - Delta, 0, MaxCameraPosition);
            GetPawn<AMyCharacter>()->UpdateSpringArm(CameraPosition);
            GetPawn<AMyCharacter>()->SetVisibility(CameraPosition != 0);

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
    MyCharacter->OnRep_BodyRotation(); // replicatedUsing
}

void AMyPlayerController::SetShowAllTrajectories(bool bInShow) const
{
    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
    MyState->WithInstanceUI(this, [this, bInShow] (FInstanceUI InstanceUI)
    {
        InstanceUI.bShowAllTrajectories = bInShow;
        for(MyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
        {
            (*IOrbit)->UpdateVisibility(InstanceUI);
        }
    });
}

void AMyPlayerController::BindEAction(const FName ActionName, EInputEvent KeyEvent, EAction Action)
{
    BindInputLambda(ActionName, KeyEvent, [this, Action] ()
    {
        if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
        {
            // make sure that replication-relevant stuff is always executed on the server (regardless where we are)
            ServerRPC_HandleAction(Action);
            // ... whereas replication-irrelevant stuff is executed locally
            HandleActionUI(Action);
        }
    });
}

void AMyPlayerController::BindInputLambda(const FName ActionName, EInputEvent KeyEvent, std::function<void()> Handler)
{
    FInputActionBinding Binding(ActionName, KeyEvent);
    Binding.ActionDelegate.GetDelegateForManualSet().BindLambda(Handler);
    InputComponent->AddActionBinding(Binding);
}

void AMyPlayerController::ServerRPC_HandleAction_Implementation(EAction Action)
{
    HandleAction(Action);
}

void AMyPlayerController::HandleAction(EAction Action)
{
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    AOrbit* Orbit = Cast<AOrbit>(MyCharacter->Children[0]);
    switch (Action)
    {
    case EAction::ACCELERATE_BEGIN:
        Orbit->SetIsChanging(true);
        MyCharacter->RP_bIsAccelerating = true;
        break;
    case EAction::ACCELERATE_END:
        MyCharacter->RP_bIsAccelerating = false;
        Orbit->SetIsChanging(false);
        break;
    }
}

void AMyPlayerController::HandleActionUI(EAction Action)
{
    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
    AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
    AOrbit* Orbit = Cast<AOrbit>(MyCharacter->Children[0]);
    const FInstanceUI InstanceUI = MyState->GetInstanceUIAny(this);
    
    switch (Action)
    {
    case EAction::ACCELERATE_BEGIN:
        Orbit->SpawnSplineMesh
            ( MyCharacter->GetTempSplineMeshColor()
            , ESplineMeshParentSelector::Temporary
            , InstanceUI
            );
        Orbit->bIsVisibleAccelerating = true;
        Orbit->UpdateVisibility(InstanceUI);
        break;
    case EAction::ACCELERATE_END:
        Orbit->DestroyTempSplineMeshes();
        Orbit->bIsVisibleAccelerating = false;
        Orbit->UpdateVisibility(InstanceUI);
        break;
    }
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
    // only ticking for player controller that controls local pawn
    Super::Tick(DeltaSeconds);

    UMyLocalPlayer* MyPlayer = Cast<UMyLocalPlayer>(Player);
    if(!MyPlayer->GetIsInMainMenu())
    {
        // reacting to mouse move
        AMyCharacter* MyCharacter = GetPawn<AMyCharacter>();
        FVector Position, Direction;
        DeprojectMousePositionToWorld(Position, Direction);
        if(abs(Direction.Z) > 1e-8)
        {
            // TODO: only works for Position.Z == 0
            const float X = Position.X - Direction.X * Position.Z / Direction.Z;
            const float Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
            const float Z = MyCharacter->GetActorLocation().Z;
            // TODO: physical rotation/animation instead

            //MyCharacter->LookAt(FVector(X, Y, Z));
            const FVector VecP(X, Y, Z);
            
            const FVector VecMe = MyCharacter->GetActorLocation();
            const FVector VecDirection = VecP - VecMe;
            const FQuat Quat = FQuat::FindBetween(FVector(1, 0, 0), VecDirection);
            const float AngleDelta = Quat.GetTwistAngle
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
                    MyCharacter->OnRep_BodyRotation();
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

    // FVector Loc = InPawn->GetActorLocation();
    // UE_LOG(LogMyGame, Warning, TEXT("%s: onpossess: actor location: %f, %f, %f"), *GetFullName(), Loc.X, Loc.Y, Loc.Z)
    // FActorSpawnParameters Params;
    // Params.Owner = InPawn;
    // AOrbit* NewOrbit = InPawn->GetWorld()->SpawnActor<AOrbit>
    //     ( Cast<IHasOrbit>(InPawn)->GetOrbitClass()
    //     , Params
    //     );
    // NewOrbit->SetEnableVisibility(true);

    // freeze orbit state for all existing orbit components for replication (condition: initial only)
    auto FilterOrbits = [this, InPawn] (const AOrbit* Orbit) -> bool
    {
        return GetWorld() == Orbit->GetWorld()
            // exclude the orbit of `InPawn`
            && (Orbit->GetOwner() != InPawn);
    };
    // debugging
    //for(MyObjectIterator<AOrbit> IOrbit(FilterOrbits); IOrbit; ++IOrbit)
    for(MyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
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
    for(MyObjectIterator<UGyrationComponent> IGyration(FilterGyrations); IGyration; ++IGyration)
    {
        (*IGyration)->FreezeState();
    }
}

void AMyPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);
    
    SetActorTickEnabled(true);
    
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
    Cast<UMyLocalPlayer>(GetLocalPlayer())->CurrentLevel = CurrentLevel;
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if(IMC_InGame.IsNull())
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: IMC_InGame null"))
    }
    
    const FInputModeGameAndUI InputModeGameAndUI;
    SetInputMode(InputModeGameAndUI);
}
