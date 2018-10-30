// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MobileTargetActor.h"

#include "IKArm.generated.h"

UENUM()
enum class EIKLimbType : uint8
{
	/** A leg for walking */
	Leg,
	/** An arm for grabbing and attacking */
	Arm
};

USTRUCT()
struct FIKProbe
{
	GENERATED_BODY()

	USceneComponent* Start;
	USceneComponent* End;
	float Length;
	FIKProbe() {};
	FIKProbe(USceneComponent* RayStart, USceneComponent* RayEnd)
	{
		Start = RayStart;
		End = RayEnd;
		Length = FVector::Distance(RayStart->GetComponentLocation(), RayEnd->GetComponentLocation());
	};
	FVector GetStart()
	{
		return Start->GetComponentLocation();
	};
	/* Gets a normalised ray-end position
	*  (Start -> (End + Diretcion)) normalized
	*/
	FVector GetModifiedRayEnd(FVector Modifier)
	{
		//FVector Direction = (End->GetComponentLocation() + Modifier - Start->GetComponentLocation()).GetSafeNormal();
		//return Start->GetComponentLocation() + Direction * Length;
		return End->GetComponentLocation() + Modifier;
	};
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
	USceneComponent* ProbeBase;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* HighTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* MidTarget;
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* LowTarget;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* GroundTarget;	
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* OffAxisTargetA;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* OffAxisTargetB;

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
	TArray<FIKProbe> IKProbes;

	UPROPERTY(EditAnywhere, Category = IK)
	FVector IKTargetFinal;
	FVector IKTargetIntermediate;
	bool IKTargetInTransit;

	UPROPERTY(EditAnywhere, Category = IK)
	float IKTargetTransitionSpeed;
	float m_IKTargetTransitionTimer;

	/** Pointer to track moving IKTargets
	* This gets attached to other actors so that movement is inherited 
	*/
	AMobileTargetActor* m_pTargetParent;


	// The current IK solution
	FQuat m_IKFrameRotation;
	float m_IKUpperArmAngle;
	float m_IKLowerArmAngle;

public:	
	// Sets default values for this actor's properties
	AIKArm();

	UPROPERTY(EditAnywhere, Category = IK)
	bool SHOW_DEBUG_INFO;

	UPROPERTY(EditAnywhere, Category = IK)
	int GaitStepIndex;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector m_bMovementDelta;
	bool m_bNeedNewTarget;

	/* Calculated using the IKRoot as a base, looking at IKTarget.
	* The Up vector is interpolated between the 2 IKPins, based on how far out/in the target is.
	*/
	FMatrix GetIKFrameRotationMatrix(FVector IKTarget);

	bool AttemptSolveIK();
	
	bool IsLimbColliding();
	bool DoesThisSolutionCollide(FQuat FrameRotation, float UpperArmAngle, float LowerArmAngle);

	/** Find the angle opposite side 'a' in the triangle given by the lengths a, b and c
	* This doesn't check for a valid triangle!!
	* It just clamps the acosf input to [0,1]
	*/
	float FindAngleA(float a, float b, float c);

	void SmoothUpdateIKTarget();

	void DebugDrawArm();
	void DebugDrawProbes();
	void MarkSpot(FVector Point, FColor Colour, float Duration=1.f);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ReceiveGaitInput(FVector MovementDelta);

	void SetIKTarget(FVector NewTarget);
	void ProbeForIKTarget(FVector DirectionModifier = FVector(0,0,0));

	FVector GetIKTargetFinal() { return IKTargetFinal; }

	EIKLimbType GetLimbType() { return LimbType; }

	void Die();
	bool bDead;
};
