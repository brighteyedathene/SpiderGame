// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IKArm.generated.h"

UENUM()
enum class EIKLimbType : uint8
{
	/** A leg for walking */
	Leg,
	/** An arm for grabbing and attacking */
	Arm
};

UCLASS()
class CONTROLANDIKTEST_API AIKArm : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* IKRoot;
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* IKPin;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* UpperArm;
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* LowerArm;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* HighTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* LowTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* UnderTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* UnderTargetOrigin;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* RestTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	float UpperArmLength;

	UPROPERTY(EditAnywhere, Category = IK)
	float LowerArmLength;

	UPROPERTY(EditAnywhere, Category = IK)
	float MaximumAngle;

	UPROPERTY(EditAnywhere, Category = IK)
	float DirectionModifierStrength;

	UPROPERTY(EditAnywhere, Category = IK)
	EIKLimbType LimbType;

	UPROPERTY(EditAnywhere, Category = IK)
	TArray<USceneComponent*> ArmTargets;

	UPROPERTY(EditAnywhere, Category = IK)
	FVector IKTarget;

public:	
	// Sets default values for this actor's properties
	AIKArm();

	UPROPERTY(EditAnywhere, Category = IK)
	bool DEBUG_SHOW_ANGLE;

	UPROPERTY(EditAnywhere, Category = IK)
	int GaitStepIndex;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool TargetReachable;

	/* Calculated using the IKRoot as a base, looking at IKTarget with IKPin pointing up.
	*/
	FMatrix GetIKFrameRotationMatrix();

	void SolveIKAndSetArmRotation();

	bool IsLimbColliding();

	/** Find the angle opposite side 'a' in the triangle given by the lengths a, b and c
	* This doesn't check for a valid triangle!!
	* It just clamps the acosf input to [0,1]
	*/
	float FindAngleA(float a, float b, float c);

	void DebugDrawArm();
	void MarkSpot(FVector Point, FColor Colour);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetIKTarget(FVector NewTarget);
	void PickNewIkTarget(FVector DirectionModifier = FVector(0,0,0));
	EIKLimbType GetLimbType() { return LimbType; }
};
