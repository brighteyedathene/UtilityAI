// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AIOption.h"
#include "AIOptionSetDataAsset.generated.h"



/**
 *
 */
UCLASS()
class UTILITYAI_API UAIOptionSetDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AIOptionSet")
	TArray<UAIOption*> Options;

};
