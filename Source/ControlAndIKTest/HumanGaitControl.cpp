// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanGaitControl.h"

#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UHumanGaitControl::UHumanGaitControl()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHumanGaitControl::BeginPlay()
{
	Super::BeginPlay();

	LeftFoot = FTransform(GetComponentTransform());
	RightFoot = FTransform(GetComponentTransform());
	// ...

}


// Called every frame
void UHumanGaitControl::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	// ...
}


void UHumanGaitControl::UpdateGait(FVector Velocity, float Intensity)
{
	// Update foot positions
	FVector MovementDelta = Velocity * GetWorld()->GetDeltaSeconds();
	CurrentGaitPosition += MovementDelta.Size();
	if (CurrentGaitPosition > StepLength * Intensity)
	{
		CurrentGaitPosition = 0;
		FVector NewPosition = GetComponentLocation() + MovementDelta.GetSafeNormal() * StepLength * Intensity;
		FQuat NewRotation = MovementDelta.IsNearlyZero() ? GetComponentQuat() : MovementDelta.ToOrientationQuat();

		if (bLeftFootPlanted)
		{
			FVector StanceOffset = -NewRotation.GetRightVector() * StanceWidth;
			LeftFoot = FTransform(NewRotation, NewPosition + StanceOffset);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("LEFT"));
			bLeftFootPlanted = false;
		}
		else
		{
			FVector StanceOffset = NewRotation.GetRightVector() * StanceWidth;
			RightFoot = FTransform(NewRotation, NewPosition + StanceOffset);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, TEXT("RIGHT"));
			bLeftFootPlanted = true;
		}
	}

	float CurrentStepProgress;
	// Update GaitFraction
	if (FMath::IsNearlyZero(Intensity))
	{
		CurrentStepProgress = 1;
	}
	else
	{
		CurrentStepProgress = (CurrentGaitPosition / (StepLength * Intensity));

	}

	if (bLeftFootPlanted)
	{
		GaitFraction = CurrentStepProgress * 0.5f + 0.5f;
	}
	else
	{
		GaitFraction = CurrentStepProgress * 0.5f;
	}


	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("GaitFraction = %f"), GaitFraction));

	// Update IK blend values
	LeftFootIKBlend = (bLeftFootPlanted ? 1 - CurrentStepProgress : CurrentStepProgress);
	RightFootIKBlend = (bLeftFootPlanted ? CurrentStepProgress : 1 - CurrentStepProgress);
	LeftFootIKBlend *= 0;// LeftFootIKBlend;
	RightFootIKBlend *= 0;// RightFootIKBlend;

	//DebugDrawFeet();
}



void UHumanGaitControl::DebugDrawFeet()
{
	DrawFootAtTransform(LeftFoot, FColor::Yellow, 0);
	DrawFootAtTransform(RightFoot, FColor::Magenta, 0);
}

void UHumanGaitControl::DrawFootAtTransform(const FTransform& T, FColor Colour, float Duration)
{
	float footLength = 10.f;
	FVector F = T.GetRotation().GetForwardVector();
	FVector R = T.GetRotation().GetRightVector();
	FVector Back = T.GetLocation() - F * (footLength / 2) + FVector::UpVector;
	FVector Front = T.GetLocation() + F * (footLength / 2) + FVector::UpVector;
	FVector Left = T.GetLocation() - R * (footLength / 2) + FVector::UpVector;
	FVector Right = T.GetLocation() + R * (footLength / 2) + FVector::UpVector;

	MarkLine(Front, Back, Colour, Duration);
	MarkLine(Front, Left, Colour, Duration);
	MarkLine(Front, Right, Colour, Duration);
}

void UHumanGaitControl::MarkSpot(FVector Point, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	float length = 1.f;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, IsPersistant, Duration, 0, 2.f);
			}
		}
	}
}

void UHumanGaitControl::MarkLine(FVector Start, FVector End, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	DrawDebugLine(GetWorld(), Start, End, Colour, IsPersistant, Duration, 0, 2.f);
}