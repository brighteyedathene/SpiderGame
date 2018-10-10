// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CrawlerMovement.h"
#include "CrawlerGaitControl.h"

#include "WallCrawler.generated.h"

class USphereComponent;


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
class CONTROLANDIKTEST_API AWallCrawler : public APawn
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere)
	USphereComponent* MySphereComponent;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere)
	UCrawlerMovement* CrawlerMovement;

	UPROPERTY(EditAnywhere)
	UCrawlerGaitControl* CrawlerGaitControl;

public:
	// Sets default values for this pawn's properties
	AWallCrawler();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;
	
	/** Radius of collider */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float ColliderSize;

	/** Lenmgth of surface feelers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceGroundRayLength;

	/** How far will the crawler try to stay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceIdealDistance;

	/** How close is good enough to stop clinging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceIdealDistanceThreshold;

	UPROPERTY(EditAnywhere, Category = WallCrawling)
	float MovementSpeed;

	/** How fast will the crawler correct its orientation */
	UPROPERTY(EditAnywhere, Category = WallCrawling)
	float RotationCorrectionAlpha;


	/** Raycasting shit - somethign complains about making this a UPROPERTY */
	//UPROPERTY(EditAnywhere, Category = WallCrawling)
	FCollisionQueryParams CollisionParameters;
	//UPROPERTY(EditAnywhere, Category = WallCrawling)
	ECollisionChannel TraceChannel;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float MaxJumpTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float MinJumpTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jumping)
	float JumpForce;

	float JumpTimer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CollectForwardInput(float Value);
	void CollectRightInput(float Value);
	void CollectYawInput(float Value);
	void CollectPitchInput(float Value);
	
	void JumpPressed();
	void JumpReleased();
	void DropPressed();
	void DropReleased();
	
	void FlushInput();

	void RotateTowardsNormal(FVector Normal, float t);

	/** This point anchors the crawler to a surface.
	* As long as there is a LatchPoint, the crawler will not begin to fall
	*/
	FVector LatchPoint, LatchNormal;
	void SetLatchPoint(FVector Location, FVector Normal);



	ECrawlerState CrawlerState;

	
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

	void ClingToPoint(FVector EnLocation, FVector EndNormal);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	float InputForward;
	float InputRight;
	float InputYaw;
	float InputPitch;

	float LocalPitch;
	float LocalYaw;


	FQuat FindLookAtQuat(const FVector& EyePosition, const FVector& LookAtPosition, const FVector& UpVector);

	FVector ProjectToPlane(const FVector& U, const FVector& N);

	void MarkSpot(FVector Point, FColor Colour, float Duration);

};
