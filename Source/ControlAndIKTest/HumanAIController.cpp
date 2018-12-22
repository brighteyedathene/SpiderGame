// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanAIController.h"
#include "Human.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "PointOfInterest.h"
#include "PointOfInterestSet.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"



AHumanAIController::AHumanAIController()
{
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	LocationToGoKey = "LocationToGo";
	PointOfInterestKey = "PointOfInterest";
	PointOfInterestDirectionKey = "PointOfInterestDirection";
	CrawlerLastKnownLocationKey = "CrawlerLastKnownLocation";
	StrikePositionKey = "StrikePosition";
	StrikeTargetKey = "StrikeTarget";
	StrikeProgressKey = "StrikeProgress";
	ReadyForActionKey = "ReadyForAction";
}

void AHumanAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	AHuman* Human = Cast<AHuman>(Pawn);

	if (Human)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::White, TEXT("Possessing Human!"));
		if (Human->BehaviorTree->BlackboardAsset)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::White, TEXT("Initializing blackboard!"));
			BlackboardComp->InitializeBlackboard(*(Human->BehaviorTree->BlackboardAsset));
		}

		TArray<AActor*> POISetActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APointOfInterestSet::StaticClass(), POISetActors);
		if (POISetActors.Num() == 1)
		{
			PointOfInterestSet = Cast<APointOfInterestSet>(POISetActors.Pop());
			if (!PointOfInterestSet)
			{
				GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Red, TEXT("PointOfInterestSet cast failed!"));
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::White, TEXT("PointOfInterestSet cast succeeded!"));
				if (!PointOfInterestSet->Initialized)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::White, TEXT("Initializing PointOfInterestSet"));
					PointOfInterestSet->InitializePointsOfInterest();
				}
			}

		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Red, TEXT("More or less than 1 PointOfInterestSet! There should be exactly one!"));
		}


		//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::White, TEXT("Starting behavior tree!"));
		BehaviorComp->StartTree(*Human->BehaviorTree);
	}
}

