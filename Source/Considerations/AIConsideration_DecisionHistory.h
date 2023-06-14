// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIConsideration.h"
#include "AIConsideration_DecisionHistory.generated.h"



/**
 * 
 */
UCLASS(BlueprintType)
class UTILITYAI_API UAIConsideration_DecisionHistory : public UAIConsideration
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere)
	FName OptionNameToQuery = FName();

	UPROPERTY(EditAnywhere)
	EDecisionHistoryQueryTime QueryTime;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "EDecisionHistoryQueryResult"))
	int32 QueryResultBitmask;

	UPROPERTY(EditAnywhere)
	FVector2D TimeRange;

	UPROPERTY(EditAnywhere)
	FVector2D MultiplierRange;

	virtual FAIConsiderationScore CalculateScore_Implementation(const FDecisionMakerContext& Context) override;
};
