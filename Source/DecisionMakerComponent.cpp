// Fill out your copyright notice in the Description page of Project Settings.


#include "DecisionMakerComponent.h"
#include "AIOptionSetDataAsset.h"
#include "AIOption.h"
#include "AIConsideration.h"
#include "AIController.h"
#include "DMBehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "VisualLogger/VisualLogger.h"

UDecisionMakerComponent::UDecisionMakerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetComponentTickEnabled(false);

}

void UDecisionMakerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDecisionMakerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RunDecisionMaker();
}

void UDecisionMakerComponent::Start()
{
	AIOptionSelectedEvent.AddUniqueDynamic(this, &UDecisionMakerComponent::OnAIOptionSelected);
	AIOptionBehaviorStartedEvent.AddUniqueDynamic(this, &UDecisionMakerComponent::OnAIOptionBehaviorStarted);
	AIOptionBehaviorEndedEvent.AddUniqueDynamic(this, &UDecisionMakerComponent::OnAIOptionBehaviorEnded);

	// Make sure the AI controller has the right behavior tree component class
	SpawnDMBehaviorTreeComp();

	// We want the behavior tree to use SingleRun so it doesn't keep repeating the selected option
	if (UDMBehaviorTreeComponent* BehaviorTreeComp = GetDMBehaviorTreeComp())
	{
		if (BT_OptionTree)
			BehaviorTreeComp->StartTree(*BT_OptionTree, EBTExecutionMode::SingleRun);
		
	}

	SetComponentTickEnabled(true);

}

void UDecisionMakerComponent::Stop()
{
	AIOptionSelectedEvent.RemoveDynamic(this, &UDecisionMakerComponent::OnAIOptionSelected);
	AIOptionBehaviorStartedEvent.RemoveDynamic(this, &UDecisionMakerComponent::OnAIOptionBehaviorStarted);
	AIOptionBehaviorEndedEvent.RemoveDynamic(this, &UDecisionMakerComponent::OnAIOptionBehaviorEnded);

	if (UDMBehaviorTreeComponent* BehaviorTreeComp = GetDMBehaviorTreeComp())
	{
		BehaviorTreeComp->StopTree();
	}

	SetComponentTickEnabled(false);

}

void UDecisionMakerComponent::SetPaused(bool bPaused)
{
	if (bPaused)
	{
		// Pause the tree
		if (UDMBehaviorTreeComponent* BehaviorTreeComp = GetDMBehaviorTreeComp())
		{
			BehaviorTreeComp->PauseLogic(FString("DecisionMaker asked to pause"));
		}

		// Stop moving
		AAIController* AIController = Cast<AAIController>(GetOwner());
		if (AIController)
		{
			AIController->StopMovement();
		}

		// Stop ticking (and making decisions)
		SetComponentTickEnabled(false);
	}
	else
	{
		if (UDMBehaviorTreeComponent* BehaviorTreeComp = GetDMBehaviorTreeComp())
		{
			BehaviorTreeComp->ResumeLogic(FString("DecisionMaker asked to resume"));
		}

		// Resume ticking
		SetComponentTickEnabled(true);
	}
}

void UDecisionMakerComponent::GetOptionSets(TArray<UAIOptionSetDataAsset*>& OutOptionSets)
{
	OutOptionSets.Append(BaseOptionSets);
}

void UDecisionMakerComponent::RunDecisionMaker()
{

#if ENABLE_VISUAL_LOG
	if (FVisualLogger::Get().IsRecording())
	{
		UE_VLOG_UELOG(GetOwner(), LogDM, Log, TEXT("------------\nRunning DM  %s..."), *GetOwner()->GetName());
	}
#endif //ENABLE_VISUAL_LOG

	FDecisionMakerContext DMContext;
	DMContext.DecisionMaker = this;
	DMContext.AIController = Cast<AAIController>(GetOwner());
	if(DMContext.AIController)
		DMContext.Pawn = DMContext.AIController->GetPawn();

	// Calculate scores for each option, and keep the best
	TArray<FAIOptionScore> OptionScores;

	// Track the max rank, so we can prune low rank options
	float MaxRank = -INFINITY;

	TArray<UAIOptionSetDataAsset*> OptionSets;

	GetOptionSets(OptionSets);

	// Check all options and considerations
	for (UAIOptionSetDataAsset* OptionSet : OptionSets)
	{
		if (!OptionSet)
			continue;

		for (UAIOption* Option : OptionSet->Options)
		{
			if (!Option)
				continue;

			// Get the option score
			FAIOptionScore OptionScore = CalculateOptionScore(Option, DMContext);

#if ENABLE_VISUAL_LOG
			if (FVisualLogger::Get().IsRecording())
			{
				UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("Rank: %f Weight: %f   (%s)"),
					OptionScore.Rank, OptionScore.Weight, *Option->OptionName.ToString());
			}
#endif //ENABLE_VISUAL_LOG

			// Only add to the list if it has weight
			if (OptionScore.Weight > 0)
			{
				OptionScores.Add(OptionScore);
				MaxRank = fmaxf(MaxRank, OptionScore.Rank);
			}
		}
	}

	// Prune low rank options
	OptionScores.RemoveAllSwap([&](FAIOptionScore OptionScore) -> bool
	{
		return OptionScore.Rank < MaxRank;
	});

	// Do something to prune low weight options by percentage
	// I'm just sorting by weight to get the highest. It's not that efficient.
	OptionScores.Sort([](const FAIOptionScore& A, const FAIOptionScore& B)
	{
		return A.Weight > B.Weight;
	});

	// At this point we already know the best option, but we might want to randomise a bit
	if (OptionScores.IsValidIndex(0))
	{
		float BestWeight = OptionScores[0].Weight;
		float MinimumWeight = BestWeight * MinimumWeightFractionForRandomSelection;

		OptionScores.RemoveAllSwap([&](FAIOptionScore OptionScore) -> bool
		{
			return OptionScore.Weight < MinimumWeight;
		});


#if ENABLE_VISUAL_LOG
		if (FVisualLogger::Get().IsRecording())
		{
			UE_VLOG_UELOG(GetOwner(), LogDM, VeryVerbose, TEXT("Pruning Options whose weights are less than %f * %f = %f"),
				BestWeight, MinimumWeightFractionForRandomSelection, MinimumWeight);
		}
#endif //ENABLE_VISUAL_LOG
	}

	if(OptionScores.Num() > 0)
	{
		int RandomIndex = FMath::Rand() % OptionScores.Num();
		if (OptionScores.IsValidIndex(RandomIndex))
		{
			UAIOption* SelectedOption = OptionScores[RandomIndex].Option;

			// Handle switching trees here
			SetCurrentOption(SelectedOption);

#if ENABLE_VISUAL_LOG
			if (FVisualLogger::Get().IsRecording())
			{

				UE_VLOG_UELOG(GetOwner(), LogDM, Log, TEXT("\n---\nSelected (%s) - Rank: %f   Weight: %f\n---"),
					*SelectedOption->OptionName.ToString(), OptionScores[RandomIndex].Rank, OptionScores[RandomIndex].Weight);

				UE_VLOG_LOCATION(GetOwner(), LogDM, Log, DMContext.Pawn->GetActorLocation(), 50.f, FColor::White, TEXT("%s"),
					SelectedOption ? *SelectedOption->OptionName.ToString() : *FString("nullptr"));

			}
#endif //ENABLE_VISUAL_LOG

		}
	}
	else
	{
#if ENABLE_VISUAL_LOG
		if (FVisualLogger::Get().IsRecording())
		{
			UE_VLOG_UELOG(GetOwner(), LogDM, Warning, TEXT("%s Could not find an option!"), *GetOwner()->GetName());
			UE_VLOG_LOCATION(GetOwner(), LogDM, Warning, DMContext.Pawn->GetActorLocation(), 50.f, FColor::Yellow, TEXT("No option found!"));
		}
#endif //ENABLE_VISUAL_LOG
	}
}

FAIOptionScore UDecisionMakerComponent::CalculateOptionScore(UAIOption* Option, const FDecisionMakerContext& DMContext)
{
	if (!Option)
		return FAIOptionScore();

	FAIOptionScore OptionScore;
	OptionScore.Option = Option;
	OptionScore.Rank = Option->Rank;
	OptionScore.Weight = 0.f; // default to 0 weight

	float AddendSum = Option->BaseAddend;
	float MultiplierProduct = 1.f;

#if ENABLE_VISUAL_LOG
	if (FVisualLogger::Get().IsRecording())
	{
		UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("(%s)..."), *Option->OptionName.ToString());
	}
#endif //ENABLE_VISUAL_LOG

	// Run through each consideration to gather consideration scores
	for (UAIConsideration* Consideration : Option->Considerations)
	{
		if (!Consideration)
			continue;

		FAIConsiderationScore ConsiderationScore = Consideration->CalculateScore(DMContext);

		AddendSum += ConsiderationScore.Addend;
		MultiplierProduct *= ConsiderationScore.Multiplier;
		
#if ENABLE_VISUAL_LOG
		if (FVisualLogger::Get().IsRecording())
		{
			UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("- Addend: %f Multiplier: %f    [%s]"),
				ConsiderationScore.Addend, ConsiderationScore.Multiplier, *Consideration->GetConsiderationDescription());
		}
#endif //ENABLE_VISUAL_LOG

		// Exit early if we have a multiplier of 0. It's unrecoverable.
		if (MultiplierProduct == 0)
			return OptionScore;
		
	}
	
	OptionScore.Weight = AddendSum * MultiplierProduct;

	return OptionScore;
}


void UDecisionMakerComponent::SetCurrentOption(UAIOption* NewOption)
{
	// If we picked the option that's currently running (and it hasn't finished yet) then leave it alone
	if (NewOption == CurrentOption && CurrentDecisionRecord.StartedTimestamp > 0)
	{
		return;
	}


	UAIOption* OldOption = CurrentOption;
	CurrentOption = NewOption;

	if (CurrentOption)
	{
		UDMBehaviorTreeComponent* BehaviorTreeComp = GetDMBehaviorTreeComp();
		if (BehaviorTreeComp)
		{
			BehaviorTreeComp->RestartTree();
		}
	}


	// AI controller probably wants to listen for this
	AIOptionSelectedEvent.Broadcast(OldOption, CurrentOption);


}


UAIOption* UDecisionMakerComponent::GetCurrentOption() const
{
	return CurrentOption;
}

UBehaviorTree* UDecisionMakerComponent::GetCurrentOptionTree() const
{
	if (CurrentOption) 
	{
		return CurrentOption->BehaviorTree;
	}
	return nullptr;
}

void UDecisionMakerComponent::SpawnDMBehaviorTreeComp()
{
	// Don't spawn anything if there already is a DM behavior tree comp
	if (GetDMBehaviorTreeComp())
		return;

	// We need this to spawn the correct blackboard
	if (!BT_OptionTree)
		return;

	/* This process is copied from AAIController::RunBehaviorTree. 
		Its spawns a behavior tree component and sets up the blackboard. 
		I've changed it to spawn a special DMBehaviorTree instead.
	*/
	if (AAIController* AICon = Cast<AAIController>(GetOwner()))
	{
		bool bSuccess = true;

		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComponent();
		if (BT_OptionTree->BlackboardAsset && (BlackboardComp == nullptr || BlackboardComp->IsCompatibleWith(BT_OptionTree->BlackboardAsset) == false))
		{
			bSuccess = AICon->UseBlackboard(BT_OptionTree->BlackboardAsset, BlackboardComp);
		}

		if (bSuccess)
		{
			//UE_VLOG(this, LogBehaviorTree, Log, TEXT("RunBehaviorTree: spawning DMBehaviorTreeComponent.."));
			UDMBehaviorTreeComponent* DMBTComp;
			DMBTComp = NewObject<UDMBehaviorTreeComponent>(AICon, TEXT("DMBTComponent"));
			DMBTComp->RegisterComponent();

			// make sure BrainComponent points at the newly created BT component
			AICon->BrainComponent = DMBTComp;
		}
	}
}


UDMBehaviorTreeComponent* UDecisionMakerComponent::GetDMBehaviorTreeComp() const
{
	if (AAIController* AICon = Cast<AAIController>(GetOwner()))
	{
		return Cast<UDMBehaviorTreeComponent>(AICon->GetBrainComponent());
	}

	return nullptr;
}

float UDecisionMakerComponent::GetTimeSinceStarted(const FName& OptionName, int32 QueryResultBitmask) const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// check the current decision first
	if (OptionName == CurrentDecisionRecord.OptionName)
	{
		if (TOFLAG(CurrentDecisionRecord.Result) & QueryResultBitmask)
			return CurrentTime - CurrentDecisionRecord.StartedTimestamp;
	}

	// Loop through history looking for the right option
	for (const FDecisionRecord& Decision : DecisionHistory)
	{
		if (OptionName == Decision.OptionName)
		{
			if (TOFLAG(Decision.Result) & QueryResultBitmask)
				return CurrentTime - Decision.StartedTimestamp;
		}
	}

	return -1.f;
}


float UDecisionMakerComponent::GetTimeSinceEnded(const FName& OptionName, int32 QueryResultBitmask) const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Loop through history looking for the right option, and check the result
	for (const FDecisionRecord& Decision : DecisionHistory)
	{
		if (OptionName == Decision.OptionName)
		{
			if (TOFLAG(Decision.Result) & QueryResultBitmask)
			{
				return CurrentTime - Decision.EndedTimestamp;
			}
		}
	}

	return -1.f;
}


void UDecisionMakerComponent::OnAIOptionSelected(UAIOption* OldOption, UAIOption* NewOption)
{
	
}

void UDecisionMakerComponent::OnAIOptionBehaviorStarted()
{
	// Create a new DecisionRecord.  This should be called when an option has started successfully (eg it has a BehaviorTree running)
	CurrentDecisionRecord = FDecisionRecord();
	CurrentDecisionRecord.OptionName = CurrentOption != nullptr ? CurrentOption->OptionName : FName();
	CurrentDecisionRecord.StartedTimestamp = GetWorld()->GetTimeSeconds();
	CurrentDecisionRecord.Result = EDecisionHistoryQueryResult::InProgress;

#if ENABLE_VISUAL_LOG
	if (FVisualLogger::Get().IsRecording())
	{
		UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("(%s) started %f"),
			*CurrentDecisionRecord.OptionName.ToString(), CurrentDecisionRecord.StartedTimestamp);
	}
#endif //ENABLE_VISUAL_LOG
}


void UDecisionMakerComponent::OnAIOptionBehaviorEnded(EBTNodeResult::Type Result)
{
	// Current option is most likely not the option we just finished, so we should leave it alone.
	// CurrentDecisionRecord should still be relevant though. So we need to push it to history

	// Store results in decision history
	CurrentDecisionRecord.EndedTimestamp = GetWorld()->GetTimeSeconds();
	CurrentDecisionRecord.Result =
		Result == EBTNodeResult::Succeeded ? EDecisionHistoryQueryResult::Succeeded :
		Result == EBTNodeResult::Failed ? EDecisionHistoryQueryResult::Failed :
		Result == EBTNodeResult::Aborted ? EDecisionHistoryQueryResult::Aborted :
		EDecisionHistoryQueryResult::InProgress; // Shouldn't ever pick this one since the tree has finished.

	// insert all records at 0, so that it's easier to iterate on them. Could be slow,,,
	DecisionHistory.Insert(CurrentDecisionRecord, 0);

#if ENABLE_VISUAL_LOG
	if (FVisualLogger::Get().IsRecording())
	{
		UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("(%s) started %f"),
			*CurrentDecisionRecord.OptionName.ToString(), CurrentDecisionRecord.StartedTimestamp);

		if (Result == EBTNodeResult::Succeeded)
		{
			UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("(%s) ended %f succesfully"),
				*CurrentDecisionRecord.OptionName.ToString(), CurrentDecisionRecord.EndedTimestamp);
		}
		else if (Result == EBTNodeResult::Failed)
		{
			UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("(%s) ended %f failed"),
				*CurrentDecisionRecord.OptionName.ToString(), CurrentDecisionRecord.EndedTimestamp);
		}
		else
		{
			UE_VLOG_UELOG(GetOwner(), LogDM, Verbose, TEXT("(%s) ended %f aborted"),
				*CurrentDecisionRecord.OptionName.ToString(), CurrentDecisionRecord.EndedTimestamp);
		}
	}
#endif //ENABLE_VISUAL_LOG

	// Reset this - it's not needed any more.
	CurrentDecisionRecord = FDecisionRecord();

	if (IsComponentTickEnabled() == false)
	{
		SetComponentTickEnabled(true);
	}

}

