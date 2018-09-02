// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CrawlerMovement.h"

#include "WallCrawler.generated.h"

class USphereComponent;


enum class ECrawlerState : uint8
{
	/** Lock movement to surfaces. */
	Crawling,
	/** Free-falling. */
	Falling
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
	//class UFloatingPawnMovement* PawnMovement;

public:
	// Sets default values for this pawn's properties
	AWallCrawler();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;
	
	/** What is the Player's current surface colliding count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	int CurrentSurfaceCount;
	
	/** Radius of collider */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float ColliderSize;

	/** Lenmgth of surface feelers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceGroundRayLength;

	/** How far will the crawler try to stay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceIdealDistance;

	/** How far down (relative to starting orientation) will the convex transition target be placed 
	* Should be no longer than the thinnest wall is thick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceConvexTransitionDistance;

	/** How strong will the crawler adhere to surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceAdherenceStrength;

	/** How close is good enough to consider a transition complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceTransitionAllowance;

	/** Length of transition in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallCrawling)
	float SurfaceTransitionTime;

	UPROPERTY(EditAnywhere, Category = WallCrawling)
	float MovementSpeed;

	/** How fast will the crawler correct its orientation */
	UPROPERTY(EditAnywhere, Category = WallCrawling)
	float RotationCorrectionAlpha;

	/** How fast will the crawler correct its orientation */
	UPROPERTY(EditAnywhere, Category = WallCrawling)
	int LatchHistoryLength;

	/** Raycasting shit */
	//UPROPERTY(EditAnywhere, Category = WallCrawling)
	FCollisionQueryParams CollisionParameters;
	//UPROPERTY(EditAnywhere, Category = WallCrawling)
	ECollisionChannel TraceChannel;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CollectForwardInput(float Value);
	void CollectRightInput(float Value);
	void CollectYawInput(float Value);
	void CollectPitchInput(float Value);
	void CollectJumpInput(float Value);
	void CollectReleaseInput(float Value);


	void RotateTowardsNormal(FVector Normal, float t);
	FQuat GetQuatFrom(FVector StartNormal, FVector TartgetNormal);

	/** This point anchors the crawler to a surface.
	* As long as there is a LatchPoint, the crawler will not begin to fall
	*/
	FVector LatchPoint, LatchNormal;
	void SetLatchPoint(FVector Location, FVector Normal);



	ECrawlerState CrawlerState;

	
	/** Checks all directions using RaysPerAxis
	*/
	bool ExploreEnvironmentWithRays(
		FVector* AvgLocation,
		FVector* AvgNormal,
		int* HitCount,
		FVector* CorrectedDirection,
		FVector MovementDirection,
		int RaysPerAxis);

	void ClingToPoint(FVector EnLocation, FVector EndNormal);

	/**
	* NEED TO UPDATE THESE
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
