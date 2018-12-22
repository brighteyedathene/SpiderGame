// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_FindNewDestination.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HumanAIController.h"
#include "PointOfInterest.h"
#include "PointOfInterestSet.h"

EBTNodeResult::Type UBTT_FindNewDestination::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("finding new tarfet.."));

	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	if (AICon)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();
		APointOfInterest* CurrentPoint = Cast<APointOfInterest>(BlackboardComp->GetValueAsObject(AICon->PointOfInterestKey));
		APointOfInterest* NextPoint = AICon->PointOfInterestSet->GetPointOfInterest();

		// Stick with current point if no new point is found
		if (CurrentPoint && !NextPoint)
		{
			NextPoint = CurrentPoint;
		}

		if (NextPoint)
		{
			NextPoint->Occupy(); 
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, AActor::GetDebugName(NextPoint));
			BlackboardComp->SetValueAsObject(AICon->PointOfInterestKey, NextPoint);
			BlackboardComp->SetValueAsVector(AICon->LocationToGoKey, NextPoint->GetActorLocation());
			BlackboardComp->SetValueAsVector(AICon->PointOfInterestDirectionKey, NextPoint->GetActorLocation() + NextPoint->GetActorForwardVector() * 100);
		}

		if (CurrentPoint)
		{
			CurrentPoint->Abandon();
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, AActor::GetDebugName(CurrentPoint));
		}



		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("PointsOfInterest count = %d"), PointsOfInterest.Num()));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White,  BlackboardComp->GetValueAsVector(AICon->LocationToGoKey).ToCompactString());


		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}




