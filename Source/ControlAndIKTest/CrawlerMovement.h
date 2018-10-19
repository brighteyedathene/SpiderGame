// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CrawlerMovement.generated.h"

/**
 * 
 */
enum class ECrawlerState : uint8
{
	/** Lock movement to surfaces. */
	Crawling,
	/** Free-falling. */
	Falling,
	/** Jumping (still rising) */
	Jumping
};



UCLASS()
class CONTROLANDIKTEST_API UCrawlerMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

	UCrawlerMovement(const FObjectInitializer& ObjectInitializer);

	// UActorComponent interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//virtual void BeginPlay() override;

	// UMovementComponent interface
	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotation) override;


public:
	/** Maximum velocity magnitude allowed for the controlled Pawn on a surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float MaxSpeedOnSurface;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float Deceleration;
	
	/**
	* Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	* Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	* Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement, meta = (ClampMin = "0", UIMin = "0"))
	float TurningBoost;
	
	/** How fast will the crawler correct its orientation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float SurfaceRotationAlpha;

	/** Lenmgth of surface feelers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float SurfaceRayLength;

	/** How far will the crawler try to stay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float IdealDistanceToSurface;

	/** How far away do we pull at greater strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float IdealDistanceTolerance;

	/** How close is good enough to stop clinging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float IdealDistanceThreshold;
	
	/** How far until the grip is broken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float GripLossDistance;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float ClingMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurfaceMovement)
	float ClimbMultiplier;




	/** Maximum horizontal velocity magnitude for the Pawn to propel itself in the air. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AerialMovement)
	float MaxSpeedInAir;

	/** Absolute maximum velocity magnitude allowed for the Pawn while falling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AerialMovement)
	float TerminalVelocity;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AerialMovement)
	float AerialAcceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AerialMovement)
	float AerialDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AerialMovement)
	float AerialRotationAlpha;

	/** Max height of a jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float MaxJumpHeight;

	/** Minimum height of a jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float MinJumpHeight;

	/** Time taken to reach max jump height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float MaxJumpTime;
	
	/** Time taken to fall from max jump height on level ground */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float MaxFallTime;

	ECrawlerState CrawlerState;
	void SetFalling() { CrawlerState = ECrawlerState::Falling; };
	bool IsFalling() { return CrawlerState == ECrawlerState::Falling; };

	void SetJumping() { CrawlerState = ECrawlerState::Jumping; };
	bool IsJumping() { return CrawlerState == ECrawlerState::Jumping; };

	void SetCrawling() { CrawlerState = ECrawlerState::Crawling; AirTimer = 0.f; };
	bool IsCrawling() { return CrawlerState == ECrawlerState::Crawling; };

	FVector GetVelocity() { return Velocity; };

	
	/** Placeholder until camera movement is rebuilt */
	FVector CameraForward;
	void SetCameraForward(FVector Direction) { CameraForward = Direction; };
	

protected:

	void UpdateCrawlerMovementState(float DeltaTime);

	/** Update Velocity based on input without gravity. */
	virtual void ApplyControlInputToVelocity(float DeltaTime);

	void MoveActor(float DeltaTime);

	float GetAcceleration();
	float GetDeceleration();

	float GetMaxSpeed();
	bool IsThisExceedingMaxSpeed(float MaxSpeed, FVector Velo) const;




	/** This point anchors the crawler to a surface.
	* As long as there is a LatchPoint, the crawler will not begin to fall
	*/
	FVector LatchPoint, LatchNormal;
	void SetLatchPoint(FVector Location, FVector Normal);
	
	bool IsLookingToCling();
	FVector GetClingVector(const FVector& EnLocation, const FVector& EndNormal);

	/** Helps the crawler through concave transitions and get over low obstacles */
	FVector GetClimbVector(const FVector& Normal, float ClimbFactor);

	void RotateTowardsNormal(FVector Normal, float t);

	/** Checks all directions using RaysPerAxis
	* This function takes a number of pointers to write return values in
	* @param pAvgLocation	The mean position of each ray hit - misses do not contribute
	* @param pAvgNormal	The mean normal of each ray hit - misses do not contribute
	* @param pHitCount	The number of rays to Hit
	* @param pSuggestedClimbFactor	Increases towards 1 if a ray hit opposes MovementDirection
	* @param MovementDirection	The movement direction given by the current frame's movment input
	* @param RaysPerAxis	How many Rays to fire along each axis (5 is good, but 5*5*5 rays!!)
	* Returns false only if no rays hit
	*/
	bool ExploreEnvironmentWithRays(
		FVector* pAvgLocation,
		FVector* pAvgNormal,
		int* pHitCount,
		float* pSuggestedClimbFactor,
		FVector MovementDirection,
		int RaysPerAxis);

	/** Set to true when a position correction is applied. Used to avoid recalculating velocity when this occurs. */
	UPROPERTY(Transient)
	uint32 bPositionCorrected : 1;

	/** Raycasting shit - somethign complains about making this a UPROPERTY */
	//UPROPERTY(EditAnywhere, Category = WallCrawling)
	FCollisionQueryParams CollisionParameters;
	//UPROPERTY(EditAnywhere, Category = WallCrawling)
	ECollisionChannel TraceChannel;



#pragma region JumpStuff
public:
	void MaybeStartJump();
	void MaybeEndJump();

protected:
	bool bJumpInProgress;
	bool bStillWantToJump;

	void SetJumpDirection(FVector ControlInput);
	FVector GetCurrentJumpVector();

	void AddJumpVelocity();

	float GetMinJumpTime();
	float GetJumpingGravity();
	float GetFallingGravity();

	/** How long since the crawler touched land */
	float AirTimer;

	/** Initial direction of the current jump */
	FVector JumpDirection;

#pragma endregion JumpStuff

	// Delete these later
	void MarkSpot(FVector Point, FColor Colour);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);
};
