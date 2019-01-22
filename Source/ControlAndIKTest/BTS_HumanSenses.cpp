// Fill out your copyright notice in the Description page of Project Settings.

#include "BTS_HumanSenses.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "Human.h"
#include "AssassinationGameState.h"




void UBTS_HumanSenses::TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{

	AHumanAIController* AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AAssassinationGameState* AssassinationState = Cast<AAssassinationGameState>(GetWorld()->GetGameState());

	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human && AssassinationState)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		if (Human->IsDead_Implementation())
		{
			return;
		}

		// Check vision
		Human->UpdateVision();
		BlackboardComp->SetValueAsBool(AICon->CrawlerInSightKey, Human->bCrawlerInSight);

		// Try to feel crawler
		Human->CheckCrawlerOnBody();

		Human->UpdateActiveStrikeBox();
		if (Human->ActiveStrikeBox)
		{
			BlackboardComp->SetValueAsEnum(AICon->StrikePositionKey, (uint8)Human->ActiveStrikeBox->StrikePosition);
			BlackboardComp->SetValueAsVector(AICon->CrawlerLastKnownLocationKey, AssassinationState->GetCrawlerLastKnownLocation());

		}
		else
		{
			BlackboardComp->SetValueAsEnum(AICon->StrikePositionKey, (uint8)EStrikePosition::None);
		}


		//Human->PrintStimuli();
		Human->ProcessStimuli();

		// Update CrawlerLastKnownLocation & prediction Keys
		BlackboardComp->SetValueAsBool(AICon->CrawlerLastKnownLocationValidKey, AssassinationState->IsCrawlerLastKnownLocationValid());
		BlackboardComp->SetValueAsVector(AICon->CrawlerLastKnownLocationKey, AssassinationState->GetCrawlerLastKnownLocation());
		BlackboardComp->SetValueAsVector(AICon->CrawlerPredictedLocationKey, AssassinationState->GetCrawlerPredictedLocation(3.0f));
		
	}
}
