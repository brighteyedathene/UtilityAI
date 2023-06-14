// Fill out your copyright notice in the Description page of Project Settings.


#include "AIConsideration.h"

UAIConsideration::UAIConsideration()
{
}

FAIConsiderationScore UAIConsideration::CalculateScore_Implementation(const FDecisionMakerContext& Context)
{
	return FAIConsiderationScore();
}


FString UAIConsideration::GetConsiderationDescription()
{
	FString Output;
	Output.Append(GetClass()->GetName());
	if (Description.IsEmpty() == false)
	{
		Output.Append(FString(": "));
		Output.Append(Description);
	}
	return Output;
}
