// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_ExecuteStrike.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "Human.h"
#include "HumanSenseComponent.h"


EBTNodeResult::Type UBTT_ExecuteStrike::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("finding new tarfet.."));

	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		if (Human->bStrikeLockedIn)
		{
			return EBTNodeResult::InProgress;
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}





