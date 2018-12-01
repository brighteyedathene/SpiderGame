// Fill out your copyright notice in the Description page of Project Settings.

#include "PointOfInterestSet.h"
#include "PointOfInterest.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"


// Sets default values
APointOfInterestSet::APointOfInterestSet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APointOfInterestSet::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APointOfInterestSet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	for (auto & POI : PointsOfInterest)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, AActor::GetDebugName(POI) + FString::Printf(TEXT(" count = %d"), POI->GetOccupantCount()));
	}

}

APointOfInterest* APointOfInterestSet::GetPointOfInterest()
{
	for (int i = 0; i < PointsOfInterest.Num(); i++)
	{
		if (!PointsOfInterest[i]->IsOccupied())
		{
			return PointsOfInterest[i];
		}
	}

	if (PointsOfInterest.Num() < 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Red, TEXT("CRITICAL WARNING: No points of interest!"));
		return nullptr;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Orange, TEXT("WARNING: No unoccupied points of interest!"));
	return PointsOfInterest[0];

}


void APointOfInterestSet::InitializePointsOfInterest()
{
	TArray<AActor*> POIActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APointOfInterest::StaticClass(), POIActors);

	for (auto & POIActor : POIActors)
	{
		APointOfInterest* POI = Cast<APointOfInterest>(POIActor);
		if (POI)
		{
			PointsOfInterest.Add(POI);
			//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::White, AActor::GetDebugName(POI) + FString::Printf(TEXT(" occupants = %d"), POI->GetOccupantCount()));
		}
	}

	//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Green, FString::Printf(TEXT("PointsOfInterest initialized with point count = %d"), PointsOfInterest.Num()));

	Initialized = true;
}
