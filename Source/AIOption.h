// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIShared.h"
#include "AIOption.generated.h"

class UBehaviorTree;
class UAIConsideration;

/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class UTILITYAI_API UAIOption : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OptionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* BehaviorTree;

	/** If this option has any weight>0, no lower rank options will be chosen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rank = 0;

	/** Start with this value when applying consideration multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseAddend = 1.f;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
	TArray<UAIConsideration*> Considerations;

};
