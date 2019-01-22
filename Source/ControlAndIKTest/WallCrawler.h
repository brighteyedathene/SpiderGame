// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthInterface.h"
#include "VisibleInterface.h"

#include "GameFramework/Pawn.h"
#include "CrawlerMovement.h"
#include "CrawlerGaitControl.h"

#include "WallCrawler.generated.h"

class USphereComponent;
class AMobileTargetActor;
class AHuman;

UENUM()
enum class ECameraMode : uint8
{
	Follow,
	Orbit,
	Fixed
};

UCLASS()
class CONTROLANDIKTEST_API AWallCrawler : public APawn, public IHealthInterface, public IVisibleInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCrawlerMovement* CrawlerMovement;

	UPROPERTY(EditAnywhere)
	UCrawlerGaitControl* CrawlerGaitControl;
	


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* MySphereComponent;


	// Sets default values for this pawn's properties
	AWallCrawler();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


#pragma region Camera

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	ECameraMode CameraMode;

	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = Camera)
	TArray<AActor*> FixedCameras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	int FixedCameraIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MaxOrbitDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MinOrbitDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float ZoomSpeed;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	USceneComponent* FollowCamRailStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	USceneComponent* FollowCamRailEnd;

	UPROPERTY(Transient, BlueprintReadWrite, Category = Camera)
	float FollowCameraDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MaxFollowCameraDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float FollowCameraLagMaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float OrbirCameraLagMaxDistance;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** Assists with yaw turning to make it less sluggish */
	UPROPERTY(EditAnywhere, Category = Camera)
	float YawFactor;


	//void Zoom(float Value); // Handled in blueprint!! Can't get TargetArmLength here

	void UpdateCameraFollow();
	void UpdateCameraOrbit();
	void UpdateCameraFixed();

	void CycleCameraModes();

	void MoveStrafe(const FQuat & CameraQuat);
	void MoveRotate(const FQuat & CameraQuat);

#pragma endregion Camera


#pragma region Visibility

	/* Visibility Interface functions **/
	virtual FVector GetVisionTargetLocation_Implementation() override;

#pragma endregion Visibility


#pragma region Health

public:
	/** Health interface functions */
	virtual float GetHealth_Implementation() override;
	virtual void UpdateHealth_Implementation(float Delta) override;	
	virtual bool IsDead_Implementation() override;
	virtual void Die_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = Health)
	void ApplyKnockBack(FVector KnockbackVelocity, float KnockbackDuration);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	TArray<UStaticMeshComponent*> RagdollMeshes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	FName RagdollTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	float MaxHealth;
	float CurrentHealth;

	bool bDead;

#pragma endregion Health

#pragma region Attack

protected:

	UPROPERTY(EditAnywhere, Category = Attack)
	USceneComponent* BiteRayStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack)
	float BiteRayLength;

	/* This actor is to be attached to the bitten human */ 
	AMobileTargetActor* BiteTargetActor;
	AHuman* BiteVictim;

	/* How far before the bite target is forcibly released */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack)
	float BiteForceReleaseDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack)
	float BiteBaseDamage;
	float CurrentBiteDPS;

	void Bite();
	void BiteRelease();
	void ContinueBite();
	void EndBite();

	
	/* Is the crawler locked in a bite */
	UPROPERTY(Transient, BlueprintReadOnly, Category = Attack)
	bool BiteDown;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Attack)
	bool HasPotentialBiteTarget();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Attack)
	FHitResult TryGetBiteTarget();

#pragma endregion Attack


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


#pragma region Input

	void CollectForwardInput(float Value);
	void CollectRightInput(float Value);
	void CollectYawInput(float Value);
	void CollectPitchInput(float Value);
	
	void JumpPressed();
	void JumpReleased();
	void RollPressed();
	void RollReleased();
	
	void FlushInput();

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

#pragma endregion Input



#pragma region Sound

	/* Let the blueprint know there is a bite victim */
	UFUNCTION(BlueprintImplementableEvent, Category = Sound)
	void BiteVictimCaught_BPEvent(float DPS);

	/* Bite victim lost! */
	UFUNCTION(BlueprintImplementableEvent, Category = Sound)
	void BiteVictimLost_BPEvent();

	/* Bit something unbiteable */
	UFUNCTION(BlueprintImplementableEvent, Category = Sound)
	void BiteMissed_BPEvent();

	/* Struck */
	UFUNCTION(BlueprintImplementableEvent, Category = Sound)
	void KnockedBack_BPEvent();


#pragma endregion Sound


	void MarkSpot(FVector Point, FColor Colour, float Duration);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);

};
