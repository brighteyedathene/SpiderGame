// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointOfInterestSet.generated.h"

class APointOfInterest;

UCLASS()
class CONTROLANDIKTEST_API APointOfInterestSet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APointOfInterestSet();

	TArray<APointOfInterest*> PointsOfInterest;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	APointOfInterest* GetPointOfInterest();
	
	void InitializePointsOfInterest();

	UPROPERTY(Transient)
	bool Initialized;
};
