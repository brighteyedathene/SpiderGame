// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_PrepareToStrike.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "Human.h"
#include "WallCrawler.h"

EBTNodeResult::Type UBTT_PrepareToStrike::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("finding new tarfet.."));

	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		if (Human->ActiveStrikeBox && !Human->IsStunned())
		{

			//Human->HumanSense->CrawlerTracker.GetActorLocation();
			//BlackboardComp->SetValueAsVector(AICon->StrikeTargetKey, Human->HumanSense->CrawlerTracker->GetActorLocation());
			//BlackboardComp->SetValueAsFloat(AICon->StrikeProgressKey, 0);

			Human->BeginStrike();

			return EBTNodeResult::Succeeded;
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}
	return EBTNodeResult::Failed;
}





