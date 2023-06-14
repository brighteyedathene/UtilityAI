// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIShared.h"
#include "AIConsideration.generated.h"


/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class UTILITYAI_API UAIConsideration : public UObject
{
	GENERATED_BODY()
public:

	/* Should be something specific about this particular consideration. It will appear in the visual logger beside the score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default, meta = (DisplayPriority = "1"))
	FString Description;

	UAIConsideration();

	UFUNCTION(BlueprintNativeEvent)
	FAIConsiderationScore CalculateScore(const FDecisionMakerContext& Context);

	// Get a string to describe this consideration
	virtual FString GetConsiderationDescription();
};
