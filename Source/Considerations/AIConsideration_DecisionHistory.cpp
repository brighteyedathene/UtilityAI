// Fill out your copyright notice in the Description page of Project Settings.


#include "AIConsideration_DecisionHistory.h"
#include "DecisionMakerComponent.h"


FAIConsiderationScore UAIConsideration_DecisionHistory::CalculateScore_Implementation(const FDecisionMakerContext& Context)
{

	float TimeElapsed = -1.f;

	if (Context.DecisionMaker)
	{
		if (QueryTime == EDecisionHistoryQueryTime::Started)
		{
			TimeElapsed = Context.DecisionMaker->GetTimeSinceStarted(OptionNameToQuery, QueryResultBitmask);
		}
		else
		{
			TimeElapsed = Context.DecisionMaker->GetTimeSinceEnded(OptionNameToQuery, QueryResultBitmask);
		}
	}

	if (TimeElapsed < 0)
	{
		TimeElapsed = INFINITY;
	}

	FAIConsiderationScore Score;

	Score.Multiplier = FMath::GetMappedRangeValueClamped(TimeRange, MultiplierRange, TimeElapsed);

	return Score;
}
