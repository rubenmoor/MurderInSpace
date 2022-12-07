// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "MyInputTags.h"
#include "MyEnhancedInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template<class UserClass, typename FuncType>
	void BindActionByTag
		( const UMyInputActionsData* MyInputActionsData
		, const FGameplayTag& GameplayTag
		, ETriggerEvent TriggerEvent
		, UserClass* Object
		, FuncType Func
		)
	{
		if (const UInputAction* IA = MyInputActionsData->FindInputActionForTag(GameplayTag))
		{
			BindAction(IA, TriggerEvent, Object, Func);
		}
	}
};
