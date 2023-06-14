// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "DMBehaviorTreeComponent.generated.h"

/**
 * 
 */
UCLASS()
class UTILITYAI_API UDMBehaviorTreeComponent : public UBehaviorTreeComponent
{
	GENERATED_BODY()
public:

	void SetLooping(bool bEnabled);
};
