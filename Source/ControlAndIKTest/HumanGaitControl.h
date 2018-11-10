// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HumanGaitControl.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTROLANDIKTEST_API UHumanGaitControl : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHumanGaitControl();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		float FootRayStartOffset;

	UPROPERTY(EditAnywhere)
		float FootRayLength;

	UPROPERTY(EditAnywhere)
		float StepLength;

	UPROPERTY(EditAnywhere)
		float StanceWidth;

	float CurrentGaitPosition;


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateGait(FVector Velocity, float Intensity=1.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GaitFraction;

	FTransform LeftFoot;
	FTransform RightFoot;
	float LeftFootIKBlend;
	float RightFootIKBlend;
	bool bLeftFootPlanted;

	bool CheckConstraints();

	void DebugDrawFeet();
	void DrawFootAtTransform(const FTransform& T, FColor Colour, float Duration);
	void MarkSpot(FVector Point, FColor Colour, float Duration);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);
};