// Fill out your copyright notice in the Description page of Project Settings.

#include "MovingPlatform.h"


// Sets default values
AMovingPlatform::AMovingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReversing = false;
	bActive = true;
}

// Called when the game starts or when spawned
void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();
	
	Index = 0;
	PreviousIndex = 0;
	ArrivalThresholdSquared = ArrivalThreshold * ArrivalThreshold;

	Timer = 0;

	if (TimeBetweenPoints == 0)
		TimeBetweenPoints = 1;

}

// Called every frame
void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActive && TargetPoints.Num() > 0)
	{
		bool Arrived;
		if (bUseFlatSpeed)
		{
			MoveTowardsNextPointFlatSpeed();
			Arrived = FVector::DistSquared(RootComponent->GetComponentLocation(), TargetPoints[Index]->GetActorLocation()) < ArrivalThresholdSquared;
		}

		else
		{
			MoveTowardsNextPointSmooth();
			Arrived = Timer >= TimeBetweenPoints;
		}
		
		if (Arrived)
		{
			// Increment index
			PreviousIndex = Index;
			if (bReversing)
				Index--;
			else
				Index++;

			// Reset timer
			Timer = 0;

			// Apply edge-case rules
			if (Index >= TargetPoints.Num() || Index < 0)
			{
				switch (SequenceType)
				{
				case ESequenceType::Loop:
					Index = 0;
					break;

				case ESequenceType::Pendulum:
					bReversing = !bReversing;
					Index = PreviousIndex;
					break;

				case ESequenceType::Once:
					bActive = false;
					break;
				}
			}
		}
	}
}

void AMovingPlatform::MoveTowardsNextPointFlatSpeed()
{
	FVector Difference = TargetPoints[Index]->GetActorLocation() - RootComponent->GetComponentLocation();
	FVector Delta = Difference.GetSafeNormal() * FlatSpeed * GetWorld()->GetDeltaSeconds();
	float CurrentDistance = Difference.Size();
	if (Delta.Size() > CurrentDistance)
	{
		Delta = Difference;
	}

	float TotalDistance = FVector::Distance(TargetPoints[Index]->GetActorLocation(), TargetPoints[PreviousIndex]->GetActorLocation());
	float t = (TotalDistance > 0) ? fmaxf(0, 1 - (CurrentDistance / TotalDistance)) : 1.f;

	FQuat NewRotation = FQuat::Slerp(TargetPoints[PreviousIndex]->GetActorQuat(), TargetPoints[Index]->GetActorQuat(), t);

	RootComponent->MoveComponent(Delta, NewRotation, true);
}

void AMovingPlatform::MoveTowardsNextPointSmooth()
{
	float t = fminf(1, fmaxf(0, Timer / TimeBetweenPoints));

	if (bUseSmoothStep)
		t = FMath::SmoothStep(0, 1, t);

	FVector Location = TargetPoints[PreviousIndex]->GetActorLocation() * (1 - t) + TargetPoints[Index]->GetActorLocation() * (t);
	FQuat Rotation = FQuat::Slerp(TargetPoints[PreviousIndex]->GetActorQuat(), TargetPoints[Index]->GetActorQuat(), t);

	RootComponent->SetWorldLocationAndRotation(Location, Rotation);

	Timer += GetWorld()->GetDeltaSeconds();
}
