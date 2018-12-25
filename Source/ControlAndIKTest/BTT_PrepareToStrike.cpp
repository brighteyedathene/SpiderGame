// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_PrepareToStrike.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "Human.h"
#include "WallCrawler.h"

EBTNodeResult::Type UBTT_PrepareToStrike::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("finding new tarfet.."));
	
	bNotifyTick = true;

	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		if (Human->ActiveStrikeBox && !Human->IsStunned())
		{

			//Human->HumanSense->CrawlerTracker.GetActorLocation();
			//BlackboardComp->SetValueAsVector(AICon->StrikeTargetKey, Human->HumanSense->CrawlerTracker->GetActorLocation());
			BlackboardComp->SetValueAsFloat(AICon->StrikeChargeKey, 0);

			return EBTNodeResult::InProgress;
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}
	return EBTNodeResult::Failed;
}





void UBTT_PrepareToStrike::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		if (Human->ActiveStrikeBox && !Human->IsStunned())
		{
			float CurrentCharge = BlackboardComp->GetValueAsFloat(AICon->StrikeChargeKey);
			float NewCharge = CurrentCharge + DeltaSeconds;

			if (NewCharge > StrikeChargeTime)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
			else
			{
				BlackboardComp->SetValueAsFloat(AICon->StrikeChargeKey, NewCharge);
			}
		}
		else
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
	else
	{
		// Failed to cast to AICon or Human
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}