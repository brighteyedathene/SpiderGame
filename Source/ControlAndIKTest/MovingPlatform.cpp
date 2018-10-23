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
}

// Called every frame
void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActive && TargetPoints.Num() > 0)
	{
		MoveTowardsNextPoint();

		if (FVector::DistSquared(RootComponent->GetComponentLocation(), TargetPoints[Index]->GetActorLocation()) < ArrivalThresholdSquared)
		{
			// Increment index
			PreviousIndex = Index;
			if (bReversing)
				Index--;
			else
				Index++;

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

void AMovingPlatform::MoveTowardsNextPoint()
{
	FVector Difference = TargetPoints[Index]->GetActorLocation() - RootComponent->GetComponentLocation();
	FVector Delta = Difference.GetSafeNormal() * Speed * GetWorld()->GetDeltaSeconds();
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