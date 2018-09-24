// Fill out your copyright notice in the Description page of Project Settings.

#include "CrawlerGaitControl.h"


// Sets default values for this component's properties
UCrawlerGaitControl::UCrawlerGaitControl()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UCrawlerGaitControl::BeginPlay()
{
	Super::BeginPlay();

	// Get all the legs attached to this actor
	NumberOfSteps = 1; // this will be the max of (GaitStepIndex + 1) of all legs found (allowing for 0 as an index)
	AActor* Owner = GetOwner();
	TArray<AActor*> Children;
	Owner->GetAllChildActors(Children, true);
	for (auto Child : Children)
	{
		AIKArm* PossibleLeg = Cast<AIKArm>(Child);
		if (PossibleLeg)
		{
			if (PossibleLeg->GetLimbType() == EIKLimbType::Leg)
			{
				Legs.Add(PossibleLeg);
				NumberOfSteps = FMath::Max(NumberOfSteps, PossibleLeg->GaitStepIndex + 1);
			}
		}
	}

}


// Called every frame
void UCrawlerGaitControl::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	static bool printed = false;
	if (!printed)
	{
		for (auto Leg : Legs)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, TEXT("GOT one"));
		}
		printed = true;
	}

}

void UCrawlerGaitControl::UpdateGait(FVector MovementDelta)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White,  MovementDelta.ToCompactString());

	static int CurrentStep = 0;

	CurrentGaitPosition += MovementDelta.Size();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("CurrentGaitPosition = %f"), CurrentGaitPosition));

	if (CurrentGaitPosition > GaitStepLength)
	{
		//Step legs
		for (auto Leg : Legs)
		{
			if (Leg->GaitStepIndex == CurrentStep)
			{
				Leg->PickNewIkTarget(MovementDelta);
			}
		}

		// Increment step
		CurrentStep = (CurrentStep + 1) % NumberOfSteps;

		// Reset gait position
		CurrentGaitPosition = 0;
	}
}
