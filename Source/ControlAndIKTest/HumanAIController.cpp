// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanAIController.h"
#include "Human.h"
#include "AssassinationGameState.h"

#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "GameFramework/PawnMovementComponent.h"

#include "PointOfInterest.h"
#include "PointOfInterestSet.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"

#include "DrawDebugHelpers.h"


AHumanAIController::AHumanAIController()
{
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	LocationToGoKey = "LocationToGo";
	PointOfInterestKey = "PointOfInterest";
	PointOfInterestDirectionKey = "PointOfInterestDirection";
	CrawlerLastKnownLocationKey = "CrawlerLastKnownLocation";
	CrawlerPredictedLocationKey = "CrawlerPredictedLocation";
	CrawlerLastKnownLocationValidKey = "CrawlerLastKnownLocationValid";
	StrikePositionKey = "StrikePosition";
	StrikeTargetKey = "StrikeTarget";
	StrikeChargeKey = "StrikeCharge";
	ReadyForActionKey = "ReadyForAction";
	CrawlerInSightKey = "CrawlerInSight";


	ArrivalThreshold = 90.f;

	MaxAvoidBlend = 0.4f;
	AvoidMaxDistanceThreshold = 100;
}

void AHumanAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	Human = Cast<AHuman>(Pawn);
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

	NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());

}


void AHumanAIController::Tick(float DeltaTime)
{
	if (Human)
	{

		// turn to face the crawler during combat
		if (Human->GetAssassinationState()->IsGlobalAlert() &&
			Human->GetAssassinationState()->IsCrawlerLastKnownLocationValid() && 
			!Human->IsCrawlerOnBody() &&
			FVector::Distance(Human->GetActorLocation(), Human->GetAssassinationState()->GetCrawlerLastKnownLocation()) < 200)
		{
			Human->TurnToFaceDirection(Human->GetAssassinationState()->GetCrawlerLastKnownLocation() - Human->GetActorLocation());
		}
		else
		{
			//Human->TurnToFaceVelocity();
		}


		//if(CurrentMoveStatus == EHumanAIMoveStatus::Moving)
		//	UpdateMove(DeltaTime);
		

		//if (GetMoveStatus() == EPathFollowingStatus::Moving)
		//{
		//	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
		//		BlackboardComp->GetValueAsVector(LocationToGoKey), FColor::Green, false, -1, -1, 1.f);
		//}
		//if (bSuggestionAvailable)
		//{
		//	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
		//		SuggestedLocation, FColor::Magenta, false, -1, -1, 1.f);
		//}
	}

}

void AHumanAIController::UpdateMove(float DeltaTime)
{
	// We will blend between these two competing vectors
	FVector PathVector = FVector(0, 0, 0);
	FVector AvoidVector = FVector(0, 0, 0);


	// get new path if necessary
	if (bNeedNewPath)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("Getting new path"));
		CurrentPath = GetPathToLocation(FinalDestination);
		CurrentPathIndex = 0;
		bNeedNewPath = false;
	}

	// PATH vector
	if (CurrentMoveStatus == EHumanAIMoveStatus::Moving && CurrentPath)
	{
		if (!CurrentPath->IsValid())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("Invalid path"));
		}
		else if (!CurrentPath->PathPoints.IsValidIndex(CurrentPathIndex))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("Invalid path index"));
		}
		else
		{
			// check if arrived, increment destination / complete move
			if (IsAtLocation(CurrentPath->PathPoints[CurrentPathIndex]))
			{
				if (CurrentPathIndex + 1 < CurrentPath->PathPoints.Num())
				{
					CurrentPathIndex++;
				}
				else
				{
					// Set move complete
					CurrentMoveStatus = EHumanAIMoveStatus::Arrived;
					GEngine->AddOnScreenDebugMessage((int32)Human->GetUniqueID(), 5.0f, FColor::White, AActor::GetDebugName(Human) + FString(" arrived ") + FString::SanitizeFloat(GetWorld()->GetTimeSeconds()));
					return;
				}

			}
			// get input towards immediate destination
			FVector ImmediateDestination = CurrentPath->PathPoints[CurrentPathIndex];
			PathVector = ImmediateDestination - GetPawn()->GetActorLocation();
		}
	}

	// AVOID vector
	const float HighestPriorityDistance = 40;
	const float LowestPriorityDistance = 60;
	int NumberOfContributions = 0;
	for (auto & OtherHuman : Human->GetAssassinationState()->Humans)
	{
		if (OtherHuman == Human || OtherHuman->IsDead_Implementation())
			continue;

		FVector Difference = Human->GetActorLocation() - OtherHuman->GetActorLocation();
		float Distance = Difference.Size();

		if (Distance < AvoidMaxDistanceThreshold)
		{
			FVector AvoidContribution = Difference.GetSafeNormal() * (1.f * Distance);
			AvoidVector += AvoidContribution;
			NumberOfContributions++;

			DrawDebugLine(GetWorld(), Human->GetActorLocation(),
				Human->GetActorLocation() + AvoidContribution, FColor::Magenta, false, 0, -1, 2.f);
		}
	}
	if (NumberOfContributions > 0)
		AvoidVector = AvoidVector * (1.f / NumberOfContributions);


	GEngine->AddOnScreenDebugMessage((int32)Human->GetUniqueID(), 5.0f, FColor::White, AActor::GetDebugName(Human) + FString(" Avoid size ") + FString::SanitizeFloat(AvoidVector.Size()));

	// Blend input between path-following and collision-avoiding vectors
	float t = FMath::GetMappedRangeValueClamped(FVector2D(HighestPriorityDistance, LowestPriorityDistance), FVector2D(0, MaxAvoidBlend), AvoidVector.Size());

	DrawDebugLine(GetWorld(), Human->GetActorLocation(),
		Human->GetActorLocation() + AvoidVector, FColor::Orange, false, 0, -1, 2.f);

	//DrawDebugLine(GetWorld(), Human->GetActorLocation(),
	//	Human->GetActorLocation() + AvoidDirection.GetSafeNormal() * (1 - t) * 200, FColor::Magenta, false, 0, -1, 2.f);

	//DrawDebugLine(GetWorld(), Human->GetActorLocation(),
	//	Human->GetActorLocation() + PathDirection.GetSafeNormal() * t * 200, FColor::Cyan, false, 0, -1, 2.f);


	FVector CombinedInput = PathVector.GetSafeNormal() * (1-t) + AvoidVector.GetSafeNormal() * t;

	//GetPawn()->GetMovementComponent()->AddInputVector(CombinedInput);
	GetPawn()->GetMovementComponent()->AddInputVector(PathVector);

}


void AHumanAIController::SetFinalDestination(FVector Location)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("Setting destination to ") + Location.ToCompactString());
	FinalDestination = Location;
	CurrentMoveStatus = EHumanAIMoveStatus::Moving;
	bNeedNewPath = true;
}


UNavigationPath* AHumanAIController::GetPathToLocation(FVector Location)
{
	UNavigationPath* Path = nullptr;
	if (NavigationSystem)
	{
		Path = NavigationSystem->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), Location);

		int NumPoints = Path->PathPoints.Num();
		for (int i = 0; i < NumPoints-1; i++)
		{
			DrawDebugLine(GetWorld(), Path->PathPoints[i], Path->PathPoints[i+1], FColor::Green, false, 0, -1, 2.f);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString("Num Points") + FString::FromInt(NumPoints));

	}
	return Path;
	
}

bool AHumanAIController::IsAtLocation(FVector Location)
{
	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), Location, FColor::Red, false, 0, -1, 1.f);
	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), 
		GetPawn()->GetActorLocation() + (Location - GetPawn()->GetActorLocation()).GetSafeNormal() * ArrivalThreshold, FColor::Green, false, 0, -1, 1.f);


	return FVector::DistSquared(GetPawn()->GetActorLocation(), Location) < ArrivalThreshold * ArrivalThreshold;
}

void AHumanAIController::StopMove()
{
	CurrentMoveStatus = EHumanAIMoveStatus::Idle;
	bNeedNewPath = false;

}


void AHumanAIController::SuggestLocation(FVector Location, bool ForceAbandonCurrentMove)
{
	SuggestedLocation = Location;
	bSuggestionAvailable = true;
	if (ForceAbandonCurrentMove)
	{
		StopMovement();
	}
}

FVector AHumanAIController::ConsumeSuggestion()
{
	bSuggestionAvailable = false;
	return SuggestedLocation;
}
