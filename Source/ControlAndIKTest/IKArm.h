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
	USceneComponent* UnderTargetIKPin;
	bool UsingUnderTargetIKPin;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* UpperArm;
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* LowerArm;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* HighTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* GroundTarget;	
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* LowTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* UnderTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* RestTarget;
	bool UsingRestTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	float RestTargetSlack;

	UPROPERTY(EditAnywhere, Category = IK)
	float UpperArmLength;

	UPROPERTY(EditAnywhere, Category = IK)
	float LowerArmLength;

	UPROPERTY(EditAnywhere, Category = IK)
	float MaximumAngle;	
	UPROPERTY(EditAnywhere, Category = IK)
	float MaximumAngleUnderneath;

	UPROPERTY(EditAnywhere, Category = IK)
	float DirectionModifierStrength;

	UPROPERTY(EditAnywhere, Category = IK)
	EIKLimbType LimbType;

	UPROPERTY(EditAnywhere, Category = IK)
	TArray<USceneComponent*> ArmTargets;

	UPROPERTY(EditAnywhere, Category = IK)
	FVector IKTargetFinal;
	FVector IKTargetIntermediate;
	bool IKTargetInTransit;

	UPROPERTY(EditAnywhere, Category = IK)
	float IKTargetTransitionDuration;
	float IKTargetTransitionTimer;

public:	
	// Sets default values for this actor's properties
	AIKArm();

	UPROPERTY(EditAnywhere, Category = IK)
	bool SHOW_DEBUG_INFO;

	UPROPERTY(EditAnywhere, Category = IK)
	int GaitStepIndex;

	FVector MovementDelta;
	bool NeedNewTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool TargetReachable;

	/* Calculated using the IKRoot as a base, looking at IKTarget.
	* The Up vector is interpolated between the 2 IKPins, based on how far out/in the target is.
	*/
	FMatrix GetIKFrameRotationMatrix(FVector IKTarget);

	bool AttemptSolveIKAndSetArmRotation();
	
	bool IsLimbColliding();

	/** Find the angle opposite side 'a' in the triangle given by the lengths a, b and c
	* This doesn't check for a valid triangle!!
	* It just clamps the acosf input to [0,1]
	*/
	float FindAngleA(float a, float b, float c);

	void SmoothUpdateIKTarget();

	void DebugDrawArm();
	void MarkSpot(FVector Point, FColor Colour);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetIKTarget(FVector NewTarget);
	void ProbeForIKTarget(FVector DirectionModifier = FVector(0,0,0));

	FVector GetIKTargetFinal() { return IKTargetFinal; }

	EIKLimbType GetLimbType() { return LimbType; }
};
