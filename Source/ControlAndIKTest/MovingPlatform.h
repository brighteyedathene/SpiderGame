// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Runtime/Engine/Classes/Engine/TargetPoint.h"

#include "MovingPlatform.generated.h"

UENUM()
enum class ESequenceType : uint8
{
	Once,
	Loop,
	Pendulum
};

UCLASS()
class CONTROLANDIKTEST_API AMovingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMovingPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveTowardsNextPointSmooth();
	void MoveTowardsNextPointFlatSpeed();


	int Index;
	int PreviousIndex;
	bool bReversing;
	float ArrivalThresholdSquared;

	float Timer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	TArray<ATargetPoint*> TargetPoints;


	UPROPERTY(EditAnywhere)
	float TimeBetweenPoints;

	UPROPERTY(EditAnywhere)
	bool bUseSmoothStep;

	UPROPERTY(EditAnywhere)
	bool bUseFlatSpeed;
		
	UPROPERTY(EditAnywhere)
	float FlatSpeed;

	UPROPERTY(EditAnywhere)
	float ArrivalThreshold;

	UPROPERTY(EditAnywhere)
	ESequenceType SequenceType;

	UPROPERTY(EditAnywhere)
	bool bActive;
};
