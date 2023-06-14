// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_RunOptionBehaviorTree.h"
#include "AIController.h"
#include "DecisionMakerComponent.h"


TAutoConsoleVariable<int32> CVarShowAITaskTreeResults(
	TEXT("ShowAITaskTreeResults"),
	0,
	TEXT("Show the results of AI Task trees.\n")
	TEXT("  0: off\n")
	TEXT("  1: on\n")
);



EBTNodeResult::Type UBTT_RunOptionBehaviorTree::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Set the behavior tree using CAIController's current AI Task tree
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (UDecisionMakerComponent* DecisionMaker = AIController->FindComponentByClass<UDecisionMakerComponent>())
		{
			if (UBehaviorTree* OptionTree = DecisionMaker->GetCurrentOptionTree())
			{
				SetBehaviorAsset(OptionTree);
				DecisionMaker->AIOptionBehaviorStartedEvent.Broadcast();
			}
		}

	}

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UBTT_RunOptionBehaviorTree::OnSubtreeDeactivated(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type NodeResult)
{
	Super::OnSubtreeDeactivated(OwnerComp, NodeResult);

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (UDecisionMakerComponent* DecisionMaker = AIController->FindComponentByClass<UDecisionMakerComponent>())
		{
			DecisionMaker->AIOptionBehaviorEndedEvent.Broadcast(NodeResult);
		}
	}
}
