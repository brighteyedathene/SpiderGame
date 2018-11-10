// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthInterface.h"

#include "GameFramework/Pawn.h"
#include "CrawlerMovement.h"
#include "CrawlerGaitControl.h"

#include "WallCrawler.generated.h"

class USphereComponent;


UENUM()
enum class ECameraMode : uint8
{
	Follow,
	Orbit,
	Fixed
};

UCLASS()
class CONTROLANDIKTEST_API AWallCrawler : public APawn, public IHealthInterface
{
	GENERATED_BODY()




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

	void UpdateCameraFollow();
	void UpdateCameraOrbit();
	void UpdateCameraFixed();

	void CycleCameraModes();

	void MoveStrafe(const FQuat & CameraQuat);
	void MoveRotate(const FQuat & CameraQuat);

#pragma endregion Camera


#pragma region Health

public:
	/** Health interface functions */
	virtual float GetHealth_Implementation() override;
	virtual void UpdateHealth_Implementation(float Delta) override;	
	virtual bool IsDead_Implementation() override;
	virtual void Die_Implementation() override;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack)
	float BiteDamage;

	void Bite();

#pragma endregion Attack


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


#pragma region Input

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** Assists with yaw turning to make it less sluggish */
	UPROPERTY(EditAnywhere, Category = Camera)
	float YawFactor;

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

	void MarkSpot(FVector Point, FColor Colour, float Duration);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);

};
