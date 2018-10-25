// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "IKArm.h"

#include "CrawlerGaitControl.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTROLANDIKTEST_API UCrawlerGaitControl : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCrawlerGaitControl();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Accumulates as the crawler moves to keep track of gait
	float CurrentGaitPosition;

	// This is the number of steps in a walk cycle
	int NumberOfSteps;

	int CurrentStep;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = Gait)
	float GaitStepLength;

	UPROPERTY(EditAnywhere, Category = Gait)
	TArray<AIKArm*> Legs;

	void UpdateGait(FVector MovementDelta);

	void Slide(FVector MovementDelta);


	// DELETE THIS LATER
	void MarkSpot(FVector Point, FColor Colour);
};
