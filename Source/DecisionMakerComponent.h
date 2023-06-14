// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIShared.h"
#include "AIOption.h"
#include "BehaviorTree/BehaviorTree.h"
#include "DecisionMakerComponent.generated.h"


class UAIOptionSetDataAsset;
class UDMBehaviorTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAIOptionSelectedEvent, UAIOption*, OldOption, UAIOption*, NewOption);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAIOptionBehaviorStartedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAIOptionBehaviorEndedEvent, EBTNodeResult::Type, Result);


USTRUCT(BlueprintType)
struct FDecisionRecord
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName OptionName = FName();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float StartedTimestamp = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float EndedTimestamp = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EDecisionHistoryQueryResult Result = EDecisionHistoryQueryResult::InProgress;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTILITYAI_API UDecisionMakerComponent : public UActorComponent
{
	GENERATED_BODY()
public:

	// This tree shold have a node to run the options from the decision maker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TenAIController")
	UBehaviorTree* BT_OptionTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DecisionMaker")
	UAIOption* CurrentOption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DecisionMaker")
	TArray<UAIOptionSetDataAsset*> BaseOptionSets;

	/** Options whose weights are close enough to the best weight can be randomised. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DecisionMaker")
	float MinimumWeightFractionForRandomSelection = 0.95f;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DecisionMaker|History")
	FDecisionRecord CurrentDecisionRecord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DecisionMaker|History")
	TArray<FDecisionRecord> DecisionHistory;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "DecisionMaker")
	FAIOptionSelectedEvent AIOptionSelectedEvent;

	UPROPERTY(BlueprintAssignable, Category = "DecisionMaker")
	FAIOptionBehaviorStartedEvent AIOptionBehaviorStartedEvent;

	UPROPERTY(BlueprintAssignable, Category = "DecisionMaker")
	FAIOptionBehaviorEndedEvent AIOptionBehaviorEndedEvent;



public:
	UDecisionMakerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Start();
	void Stop();

	// Should pause BTree and decision making without cancelling anything
	void SetPaused(bool bPaused);

	virtual void GetOptionSets(TArray<UAIOptionSetDataAsset*>& OutOptionSets);

	void RunDecisionMaker();

	FAIOptionScore CalculateOptionScore(UAIOption* Option, const FDecisionMakerContext& DMContext);

	void SetCurrentOption(UAIOption* NewOption);

	UFUNCTION(BlueprintCallable, Category = "DecisionMaker")
	UAIOption* GetCurrentOption() const;

	UFUNCTION(BlueprintCallable, Category = "DecisionMaker")
	UBehaviorTree* GetCurrentOptionTree() const;

	void SpawnDMBehaviorTreeComp();

	UDMBehaviorTreeComponent* GetDMBehaviorTreeComp() const;

	UFUNCTION(BlueprintCallable, Category = "DecisionMaker")
	float GetTimeSinceStarted(const FName& OptionName, int32 QueryResultBitmask) const;

	UFUNCTION(BlueprintCallable, Category = "DecisionMaker")
	float GetTimeSinceEnded(const FName& OptionName, int32 QueryResultBitmask) const;

	// --- Callbacks ---

	UFUNCTION()
	void OnAIOptionSelected(UAIOption* OldOption, UAIOption* NewOption);

	UFUNCTION()
	void OnAIOptionBehaviorStarted();

	UFUNCTION()
	void OnAIOptionBehaviorEnded(EBTNodeResult::Type Result);
};
