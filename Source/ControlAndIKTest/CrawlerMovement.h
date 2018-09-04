// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CrawlerMovement.generated.h"

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API UCrawlerMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

	UCrawlerMovement(const FObjectInitializer& ObjectInitializer);

	// UActorComponent interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	// UMovementComponent interface
	virtual float GetMaxSpeed() const override { return MaxSpeed; }
	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotation) override;

public:
	/** Maximum velocity magnitude allowed for the controlled Pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrawlerMovement)
	float MaxSpeed;

	/** Absolute maximum velocity magnitude allowed for the controlled Pawn while falling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrawlerMovement)
	float TerminalVelocity;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrawlerMovement)
	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrawlerMovement)
	float Deceleration;

	/** Acceleration applied by gravity (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrawlerMovement)
	float GravityAcceleration;

	/**
	* Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	* Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	* Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrawlerMovement, meta = (ClampMin = "0", UIMin = "0"))
	float TurningBoost;

	void SetFalling(bool ApplyIfTrue);
	bool IsFalling();

protected:

	/** Update Velocity based on input. Also applies gravity. */
	virtual void ApplyControlInputToVelocity(float DeltaTime);

	bool IsThisExceedingMaxSpeed(float MaxSpeed, FVector Velo) const;

	/** Set to true when a position correction is applied. Used to avoid recalculating velocity when this occurs. */
	UPROPERTY(Transient)
	uint32 bPositionCorrected : 1;

	UPROPERTY(Transient)
	bool bFalling;


};
