// Fill out your copyright notice in the Description page of Project Settings.

#include "CrawlerGaitControl.h"
#include "DrawDebugHelpers.h"


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
	CurrentStep = 0;
}


// Called every frame
void UCrawlerGaitControl::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCrawlerGaitControl::UpdateGait(FVector MovementDelta)
{
	CurrentGaitPosition += MovementDelta.Size();

	if (CurrentGaitPosition > GaitStepLength)
	{
		//Step legs
		for (auto Leg : Legs)
		{
			if (Leg->GaitStepIndex == CurrentStep)
			{
				Leg->ReceiveGaitInput(MovementDelta);
				if(Leg->SHOW_DEBUG_INFO)
					MarkSpot(Leg->GetIKTargetFinal(), ((CurrentStep==0) ? FColor::Yellow : FColor::Green));
			}
		}

		// Increment step
		CurrentStep = (CurrentStep + 1) % NumberOfSteps;

		// Reset gait position
		CurrentGaitPosition = 0;
	}



	//AActor* Owner = GetOwner();
	//if (CurrentStep == 0)
	//{
	//	DrawDebugLine(GetWorld(), Owner->GetActorLocation(), Owner->GetActorLocation() + Owner->GetActorUpVector() * 50, FColor::Red, false, -1, 0, 3.f);
	//}
	//else
	//{
	//	DrawDebugLine(GetWorld(), Owner->GetActorLocation() + Owner->GetActorUpVector() * 25 - Owner->GetActorRightVector()*25, 
	//		Owner->GetActorLocation() + Owner->GetActorUpVector() * 25 + Owner->GetActorRightVector() * 25, FColor::Red, false, -1, 0, 3.f);
	//}
}

void UCrawlerGaitControl::MarkSpot(FVector Point, FColor Colour)
{
	float length = 10.f;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, true, -1, 0, 1.f);
			}
		}
	}
}