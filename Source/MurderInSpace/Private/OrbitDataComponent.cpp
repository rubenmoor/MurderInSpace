// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitDataComponent.h"

#include "MyGameInstance.h"

// Sets default values for this component's properties
UOrbitDataComponent::UOrbitDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

float UOrbitDataComponent::GetCircleVelocity(float Alpha, FVector VecF1) const
{
	return sqrt(Alpha / (GetVecR() - VecF1).Length());
}

void UOrbitDataComponent::SetVelocity(FVector _VecVelocity, float Alpha, FVector VecF1)
{
	VecVelocity = _VecVelocity;
	Velocity = VecVelocity.Length();
	VelocityVCircle = Velocity / GetCircleVelocity(Alpha, VecF1);
	bInitialized = true;
}

void UOrbitDataComponent::AddVelocity(FVector _VecVelocity, float Alpha, FVector VecF1)
{
	SetVelocity(VecVelocity + _VecVelocity, Alpha, VecF1);
}

void UOrbitDataComponent::SpawnOrbit(UClass* OrbitClass, UMaterialInstance* Material)
{
 	if(Orbit)
 	{
 		UE_LOG(LogTemp, Warning, TEXT("AActorInSpace::SpawnOrbit: Orbit already set; skipping"))
 	}
    else
    {
 		Orbit = GetWorld()->SpawnActor<AOrbit>(OrbitClass);
		Orbit->SetOrbitData(this);
    	const auto GI = GetOwner()->GetGameInstance<UMyGameInstance>();
    	const auto Alpha = GI->Alpha;
    	const auto WorldRadius = GI->WorldRadius;
    	const auto VecF1 = GI->VecF1;
    	
		const auto VecRKepler = GetVecR() - VecF1;
    	SetVelocity(FVector(0, 0, 1).Cross(VecRKepler).GetSafeNormal() * GetCircleVelocity(Alpha, VecF1), Alpha, VecF1);
    	SplineMeshMaterial = Material;
		Orbit->Update(Alpha, WorldRadius, VecF1);
    }
}

FVector UOrbitDataComponent::GetVecVelocity() const
{
	if(!bInitialized)
	{
		RequestEngineExit(TEXT("UOrbitDataComponent::GetVecVelocity: not initialized"));
	}
	return VecVelocity;
}

float UOrbitDataComponent::GetVelocity() const
{
	if(!bInitialized)
	{
		RequestEngineExit(TEXT("UOrbitDataComponent::GetVelocity: not initialized"));
	}
	return Velocity;
}

// Called every frame
void UOrbitDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!Orbit)
	{
		UE_LOG(LogActor, Error, TEXT("%s: Orbit null"), *GetOwner()->GetFName().ToString());
		RequestEngineExit(TEXT("UOrbitDataComponent::TickComponent: Orbit null"));
		return;
	}
	
	const auto GI = GetOwner()->GetGameInstance<UMyGameInstance>();
	const auto VecF1 = GI->VecF1;
	const auto Alpha = GI->Alpha;
	
	const auto VecR = GetVecR();
	const auto VecRKepler = VecR - VecF1;

	Velocity = Orbit->NextVelocity(VecRKepler.Length(), Alpha, Velocity, DeltaTime, VecVelocity.Dot(VecRKepler));
	VelocityVCircle = Velocity / GetCircleVelocity(Alpha, VecF1);
	const auto DeltaR = Velocity * DeltaTime;

	const auto [NewVecVelocity, NewLocation] = Orbit->AdvanceOnSpline(DeltaR, Velocity, VecR, DeltaTime);
	// TODO: replace with Body->SetWorldLocation
	GetOwner()->SetActorLocation(NewLocation, true, nullptr, ETeleportType::None);
	//GetOwner()->GetRootComponent()->SetWorldLocation(NewLocation, true, nullptr, ETeleportType::TeleportPhysics);
	
	VecVelocity = NewVecVelocity;

	// TODO: account for acceleration
	// const auto RealDeltaR = (GetVecR() - VecR).Length();
	// const auto RelativeError = DeltaR / RealDeltaR - 1.;
	// if(abs(RelativeError) > 0.02)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s: Expected: %f; really: %f; relative error: %.1f%"), *GetFName().ToString(), DeltaR, RealDeltaR, RelativeError * 100.);
	// }
}

#if WITH_EDITOR
void UOrbitDataComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	const auto WorldRadius = UMyGameInstance::EditorDefaultWorldRadiusUU;
	const auto Alpha = UMyGameInstance::EditorDefaultAlpha;
	const auto VecF1 = UMyGameInstance::EditorDefaultVecF1;
	
	const auto Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	
	static const FName FNameOrbit = GET_MEMBER_NAME_CHECKED(UOrbitDataComponent, Orbit);
	static const FName FNameVelocity = GET_MEMBER_NAME_CHECKED(UOrbitDataComponent, Velocity);
	static const FName FNameVelocityVCircle = GET_MEMBER_NAME_CHECKED(UOrbitDataComponent, VelocityVCircle);
	static const FName FNameVecVelocity = GET_MEMBER_NAME_CHECKED(UOrbitDataComponent, VecVelocity);
	
	const auto VelocityNormal = VecVelocity.GetSafeNormal(1e-8, FVector(0., 1., 0.));
	const auto VecRKepler = GetVecR() - VecF1;

	if(Name == FNameVelocity)
	{
		VecVelocity = Velocity * VelocityNormal;
		VelocityVCircle = Velocity / sqrt(Alpha / VecRKepler.Length());
		Orbit->Update(Alpha, WorldRadius, VecF1);
	}
	else if(Name == FNameVelocityVCircle)
	{
		Velocity = sqrt(Alpha / VecRKepler.Length()) * VelocityVCircle;
		VecVelocity = Velocity * VelocityNormal;
		Orbit->Update(Alpha, WorldRadius, VecF1);
	}
	else if(Name == FNameVecVelocity)
	{
		Velocity = VecVelocity.Length();
		VelocityVCircle = Velocity / GetCircleVelocity(Alpha, VecF1);
		Orbit->Update(Alpha, WorldRadius, VecF1);
	}
	else if(Name == FNameOrbit)
	{
		if(Orbit)
		{
			SetVelocity(FVector(0, 0, 1).Cross(VecRKepler).GetSafeNormal() * GetCircleVelocity(Alpha, VecF1), Alpha, VecF1);
			Orbit->SetOrbitData(this);
			Orbit->Update(Alpha, WorldRadius, VecF1);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("In UOrbitDataComponent::PostEditChangeProperty: Orbit set to null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("In UKeplerOrbitComponent::PostEditChangeProperty: %s, not doing anything"), *Name.ToString());
	}
}
#endif
