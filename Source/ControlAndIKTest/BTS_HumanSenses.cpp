// Fill out your copyright notice in the Description page of Project Settings.

#include "BTS_HumanSenses.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "Human.h"
#include "HumanSenseComponent.h"



void UBTS_HumanSenses::OnSearchStart(FBehaviorTreeSearchData & SearchData)
{

	
	//AHumanAIController* AICon = Cast<AHumanAIController>(SearchData.OwnerComp.GetAIOwner());
	//AHuman* Human = Cast<AHuman>(SearchData.OwnerComp.GetOwner());
	//if (AICon && Human)
	//{
	//	UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();
	//
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Using my senses.."));
	//
	//	// Try to see crawler
	//
	//
	//	// Try to feel crawler
	//	Human->StrikePosition = Human->HumanSense->CheckStrikeBoxes();
	//	BlackboardComp->SetValueAsEnum("StrikePosition", (uint8)Human->StrikePosition);
	//}
}


void UBTS_HumanSenses::TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("Ticking ") + AActor::GetDebugName(OwnerComp.GetAIOwner()));

	

	AHumanAIController* AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();

		// Check vision
		Human->UpdateVision();

		// Try to feel crawler
		Human->UpdateActiveStrikeBox();
		if (Human->ActiveStrikeBox)
		{
			BlackboardComp->SetValueAsEnum(AICon->StrikePositionKey, (uint8)Human->ActiveStrikeBox->StrikePosition);
		}
		else
		{
			BlackboardComp->SetValueAsEnum(AICon->StrikePositionKey, (uint8)EStrikePosition::None);
		}
		
	}
}
