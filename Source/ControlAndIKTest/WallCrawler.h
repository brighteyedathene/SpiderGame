// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CrawlerMovement.h"
#include "CrawlerGaitControl.h"

#include "WallCrawler.generated.h"

class USphereComponent;


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




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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


	void MarkSpot(FVector Point, FColor Colour, float Duration);

};
