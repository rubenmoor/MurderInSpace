#include "Modes/MyPlayerController.h"

#include <algorithm>
#include <Input/MyInputActionSet.h>

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyGameplayTags.h"
#include "Spacebodies/MyCharacter.h"
#include "Spacebodies/MyPlayerStart.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "MyComponents/GyrationComponent.h"
#include "HUD/MyHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/FunctionLib.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MySessionManager.h"
#include "Modes/MyGameState.h"
#include "EnhancedInputComponent.h"
#include "GameplayAbilitySystem/GA_Accelerate.h"
#include "GameplayAbilitySystem/GA_LookAt.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"
#include "Input/MyInputActions.h"
#include "UE5Coro/LatentAwaiters.h"

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

        const FVector VecP(X, Y, Z);
        
        const FVector VecMe = MyCharacter->GetActorLocation();
        return (VecP - VecMe).GetSafeNormal();
    }
    else
    {
        return FVector::ZeroVector;
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
    const auto InputAbilityTags = InputActionTags.Filter(Tag.Ability.GetSingleTagContainer());
    const auto InputCustomTags = InputActionTags.Filter(Tag.CustomInputBinding.GetSingleTagContainer());

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
                if(!Spec->IsActive())
                    AbilitySystemComponent->TryActivateAbility(Spec->Handle);
            }
            break;
        case EInputTrigger::Released:
        case EInputTrigger::HoldAndRelease:
            for(auto Spec : AbilitySystemComponent->GetActiveAbilities(InputAbilityTags))
            {
                // InstancingPolicy: InstancedPerActor
                auto* AbilityInstance = Spec->GetPrimaryInstance();
                if(!AbilityInstance)
                    // InstancingPolicy: NonInstanced (Get the CDO)
                    AbilityInstance = Spec->Ability;
                // TODO: InstancedPerExecution doesn't make sense in this setup
                Cast<UMyGameplayAbility>(AbilityInstance)->ServerRPC_SetReleased();
            }
            break;
        case EInputTrigger::Tap:
            AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InputAbilityTags, Specs, false);
            for(auto Spec : Specs)
            {
                if(Spec->IsActive())
                {
                    // InstancingPolicy: InstancedPerActor
                    auto* AbilityInstance = Spec->GetPrimaryInstance();
                    if(!AbilityInstance)
                        // InstancingPolicy: NonInstanced (Get the CDO)
                        AbilityInstance = Spec->Ability;
                    // TODO: InstancedPerExecution doesn't make sense in this setup
                    Cast<UMyGameplayAbility>(AbilityInstance)->ServerRPC_SetReleased();
                }
                else
                {
                    AbilitySystemComponent->TryActivateAbility(Spec->Handle);
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
    if(!VecMouseDirection.IsZero())
    {
        auto MyCharacter = GetPawn<AMyCharacter>();
        const FVector VecMe = MyCharacter->GetActorLocation();
        
        const double MouseAngleNew = FQuat::FindBetween(FVector::UnitX(), VecMouseDirection).GetTwistAngle(FVector::UnitZ());

        if(FMath::Abs(MouseAngle - MouseAngleNew) > 0.1)
        {
            MouseAngle = MouseAngleNew;
            const float DeltaTheta = UFunctionLib::WrapRadians(MouseAngle - MyCharacter->GetActorQuat().GetTwistAngle(FVector::UnitZ()));
            if(abs(DeltaTheta) > 15. / 180. * PI)
            {
                auto& Tag = FMyGameplayTags::Get();
                auto Specs = AbilitySystemComponent->GetActiveAbilities(Tag.AbilityLookAt.GetSingleTagContainer());
                
                if(Specs.IsEmpty())
                {
                    ActivateLookAt(DeltaTheta);
                }
                else
                {
                    check(Specs.Num() == 1)
                    AbilitySystemComponent->CancelAbilitySpec(*Specs[0], nullptr);
                    Handle.Cancel();
                    Handle = ActivateLookAtAfterNextTick();
                }

                VecAngle = (FQuat(FVector::UnitZ(), DeltaTheta) * MyCharacter->GetActorQuat()).RotateVector(FVector::UnitX());
            }
        }

        // debugging angle
        DrawDebugDirectionalArrow(GetWorld(), VecMe, VecMe + 750. * VecAngle, 80, FColor::Turquoise);
        // debugging direction
        DrawDebugDirectionalArrow(GetWorld(), VecMe, VecMe + 500. * VecMouseDirection, 20, FColor::Red);
    }
}

TCoroutine<> AMyPlayerController::ActivateLookAtAfterNextTick()
{
    co_await Latent::Ticks(2);
    const auto MyCharacter = GetPawn<AMyCharacter>();
    const float DeltaTheta = UFunctionLib::WrapRadians(MouseAngle - MyCharacter->GetActorQuat().GetTwistAngle(FVector::UnitZ()));
    ActivateLookAt(DeltaTheta);
}

void AMyPlayerController::ActivateLookAt(float DeltaTheta)
{
    auto& Tag = FMyGameplayTags::Get();
    FGameplayEventData EventData;
    EventData.EventMagnitude = DeltaTheta;
    UE_LOGFMT(LogMyGame, Display, "SendGameplayEvent: {PAYLOAD}", EventData.EventMagnitude);
    AbilitySystemComponent->SendGameplayEvent(Tag.AbilityLookAt, EventData);
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
    TArray<FGameplayAbilitySpec*> Specs;
    AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Tag.AbilityAccelerate.GetSingleTagContainer(), Specs);
    Specs[0]->GetPrimaryInstance()->OnGameplayAbilityEnded.AddLambda([this, Orbit] (UGameplayAbility* Ability)
    {
        UE_LOGFMT(LogMyGame, Display, "{NAME} ended, freeze orbit states", Ability->GetFName());
        Orbit->FreezeOrbitState();
    });
}

void AMyPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    auto* MyCharacter = Cast<AMyCharacter>(P);

    AbilitySystemComponent = UMyAbilitySystemComponent::Get(MyCharacter);
    AbilitySystemComponent->InitAbilityActorInfo(MyCharacter, MyCharacter);

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
    if(CustomBindingTag == Tag.CustomInputBindingSelect)
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
    else if(CustomBindingTag == Tag.CustomInputBindingZoom)
    {
        const auto Delta = static_cast<int8>(InputActionInstance.GetValue().Get<FInputActionValue::Axis1D>());
        if(abs(Delta) > 0)
        {
            CameraPosition = std::clamp<uint8>(CameraPosition - Delta, 0, MaxCameraPosition);
            GetPawn<AMyCharacter>()->UpdateSpringArm(CameraPosition);
        }		
    }

    // all orbits show/hide
    else if(CustomBindingTag == Tag.CustomInputBindingAllOrbitsShowHide)
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
    else if(CustomBindingTag == Tag.CustomInputBindingMyOrbitShowHide)
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

    else if(CustomBindingTag == Tag.CustomInputBindingIngameMenuToggle)
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
