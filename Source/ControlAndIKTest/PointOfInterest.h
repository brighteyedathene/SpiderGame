// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "PointOfInterest.generated.h"

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API APointOfInterest : public ATargetPoint
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Transient, Category = PointOfInterest)
	int OccupantCount;

public:
	bool IsOccupied() { return OccupantCount > 0; }
	void Occupy() { OccupantCount++; }
	void Abandon() { OccupantCount = FMath::Max(0, OccupantCount - 1); }

	int GetOccupantCount() { return OccupantCount; }
};
