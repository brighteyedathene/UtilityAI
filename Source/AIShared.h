// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AIShared.generated.h"


class AAIController;
class UDecisionMakerComponent;
class UAIOption;

DECLARE_LOG_CATEGORY_EXTERN(LogDM, Display, All);

#define TOFLAG(Enum) (1 << static_cast<uint8>(Enum))

UENUM(BlueprintType)
enum class EDecisionHistoryQueryTime : uint8
{
	Started,
	Ended

};

UENUM(BlueprintType, meta = (Bitflags))
enum class EDecisionHistoryQueryResult : uint8
{
	InProgress,
	Succeeded,
	Failed,
	Aborted
};


USTRUCT(BlueprintType)
struct FDecisionMakerContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintreadOnly)
	UDecisionMakerComponent* DecisionMaker = nullptr;

	UPROPERTY(BlueprintreadWrite)
	AAIController* AIController = nullptr;

	UPROPERTY(BlueprintreadWrite)
	APawn* Pawn = nullptr;
};


USTRUCT(BlueprintType)
struct FAIConsiderationScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Addend = 0;

	UPROPERTY(BlueprintReadWrite)
	float Multiplier = 1.f;
};


USTRUCT(BlueprintType)
struct FAIOptionScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Rank = 0;

	UPROPERTY(BlueprintReadWrite)
	float Weight = 0;

	UPROPERTY(BlueprintReadWrite)
	UAIOption* Option;
};