// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanAIController.h"
#include "Human.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"



AHumanAIController::AHumanAIController()
{
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	LocationToGoKey = "LocationToGo";
}

void AHumanAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	AHuman* Human = Cast<AHuman>(Pawn);

	if (Human)
	{
		if (Human->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(Human->BehaviorTree->BlackboardAsset));
		}

		MyTestTargetPoint = Human->CentrePoint;

		BehaviorComp->StartTree(*Human->BehaviorTree);
	}
}

