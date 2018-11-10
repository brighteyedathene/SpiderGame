// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "HumanMovement.generated.h"

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API UHumanMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

	UHumanMovement(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void ApplyControlInputToVelocity(float DeltaTime);
	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotationQuat) override;
	


public:
	/** Maximum velocity magnitude allowed for the controlled Pawn on a surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float MaxSpeed;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float Deceleration;

	/** Turn speed in degrees/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float MaxTurnSpeed;
  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float TurnAcceleration;

	float CurrentTurnSpeed;

	/**
	* Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	* Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	* Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement, meta = (ClampMin = "0", UIMin = "0"))
	float TurningBoost;
	
	/** Set to true when a position correction is applied. Used to avoid recalculating velocity when this occurs. */
	UPROPERTY(Transient)
	uint32 bPositionCorrected : 1;


	/** Vertical offset from UpdatedComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float GroundRayStartOffset;

	/** How far from the ground should UpdatedComponent rest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float GroundRootOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float GroundRayLength;


	FVector GetVelocity();

protected:

	/** Get max speed in a given direction
	* Limits speed when moving backwards
	*/
	float GetMaxSpeed(FVector Direction);
	void TurnToFaceDirection(float DeltaTime, FVector Direction);

	void StickToGround();
};
