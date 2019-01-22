// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_MoveToLocationAndAvoid.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "HumanAIController.h"
#include "Human.h"


EBTNodeResult::Type UBTT_MoveToLocationAndAvoid::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 *  NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("finding new tarfet.."));

	bNotifyTick = true;

	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	if (AICon)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();



		// Set up the move

		//if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString("Target is an object"));
		//
		//
		//	// The following is for moving to objects
		//	UObject* KeyValue = BlackboardComp->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
		//	AActor* TargetActor = Cast<AActor>(KeyValue);
		//	if (TargetActor)
		//	{
		//
		//		AICon->SetFinalDestination(TargetActor->GetActorLocation());
		//
		//		//if (bTrackMovingGoal)
		//		//{
		//		//	MoveReq.SetGoalActor(TargetActor);
		//		//}
		//		//else
		//		//{
		//		//	MoveReq.SetGoalLocation(TargetActor->GetActorLocation());
		//		//}
		//	}
		//	else
		//	{
		//
		//		return EBTNodeResult::Failed;
		//	}
		//}
		//else if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString("Target is a vector"));
		//
		//	const FVector TargetLocation = BlackboardComp->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
		//	
		//	AICon->SetFinalDestination(TargetLocation);
		//}
		//else
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString("Target is neither object nor vector"));
		//	
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, BlackboardKey.SelectedKeyName.ToString());
		//
		//	return EBTNodeResult::Failed;
		//}




		UObject* KeyValue = BlackboardComp->GetValueAsObject(BlackboardKey.SelectedKeyName);
		AActor* TargetActor = Cast<AActor>(KeyValue);
		if (TargetActor)
		{
			AICon->SetFinalDestination(TargetActor->GetActorLocation());
			return EBTNodeResult::InProgress;
		}

		FVector TargetLocation = BlackboardComp->GetValueAsVector(BlackboardKey.SelectedKeyName);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Setting destination to ") + TargetLocation.ToCompactString());

		AICon->SetFinalDestination(TargetLocation);
		return EBTNodeResult::InProgress;

		
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
		//	FString("Blackboard key is neither object nor vector: ") + BlackboardKey.SelectedKeyName.ToString());
		//return EBTNodeResult::Failed;



		
	}
	return EBTNodeResult::Failed;
}


void UBTT_MoveToLocationAndAvoid::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	AHuman* Human = Cast<AHuman>(AICon->GetPawn());
	if (AICon && Human)
	{
		if (AICon->CurrentMoveStatus == EHumanAIMoveStatus::Arrived)
		{
			AICon->StopMove();
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

EBTNodeResult::Type UBTT_MoveToLocationAndAvoid::AbortTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AHumanAIController * AICon = Cast<AHumanAIController>(OwnerComp.GetAIOwner());
	if (AICon)
	{
		AICon->StopMove();
		return EBTNodeResult::Aborted;
	}
	return EBTNodeResult::Failed;
}

