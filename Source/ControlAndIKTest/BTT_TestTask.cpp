// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_TestTask.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "HumanAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTT_TestTask::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());

	if (AICon)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();
		ATargetPoint* CurrentPoint = Cast<ATargetPoint>(BlackboardComp->GetValueAsObject(AICon->LocationToGoKey));

		BlackboardComp->SetValueAsObject(AICon->LocationToGoKey, AICon->GetTargetPoint());
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

