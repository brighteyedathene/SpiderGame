// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_ExecuteStrike.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "Human.h"



EBTNodeResult::Type UBTT_ExecuteStrike::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("finding new tarfet.."));

	bNotifyTick = true;

	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		if (Human->IsStriking())
		{
			return EBTNodeResult::InProgress;
		}
		else
		{
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}


void UBTT_ExecuteStrike::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		if (!Human->IsStriking())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}



