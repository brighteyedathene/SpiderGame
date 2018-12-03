// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthInterface.h"
#include "StrikeBox.h"

#include "Human.generated.h"

class UHumanMovement;
class UHumanGaitControl;
class UHumanSenseComponent;
class ATargetPoint;
class UBehaviorTree;
class UCapsuleComponent;

UCLASS()
class CONTROLANDIKTEST_API AHuman : public APawn, public IHealthInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHuman();

	UPROPERTY(EditAnywhere)
	UHumanMovement* HumanMovement;

	UPROPERTY(EditAnywhere)
	UHumanGaitControl* HumanGaitControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHumanSenseComponent* HumanSense;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;


#pragma region Health

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCapsuleComponent* HumanCollider;

	/** Health interface functions */
	virtual float GetHealth_Implementation() override;
	virtual void UpdateHealth_Implementation(float Delta) override;
	virtual bool IsDead_Implementation() override;
	virtual void Die_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	float GetBiteDamageForBone(FName BoneName);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	TMap<FName, float> LocationDamageMap;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	bool IsStunned() { return StunRemaining > 0; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	float GetStunRemaing() { return StunRemaining/StunDecayDuration; };




protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	float MaxHealth;
	float CurrentHealth;

	bool bDead;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Health)
	float StunRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	float StunDecayDuration;

#pragma endregion Health


#pragma region Attack

public:

	TArray<UStrikeBox*> StrikeBoxes;
	TMap<ELimb, UStrikeLimb*> StrikeLimbMap;

	UFUNCTION(BlueprintCallable, Category = Senses)
	void UpdateActiveStrikeBox();

	UFUNCTION(BlueprintCallable, Category = Senses)
	UStrikeBox* CheckStrikeBoxes();

	/** Returns true if a crawler is latched to this human */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	bool IsCrawlerOnBody();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	EStrikePosition StrikePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	UStrikeBox* ActiveStrikeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	UStrikeLimb* ActiveStrikeLimb;

	/* This value augments strike progression for IK weight and animation blending - Should always be greater than 1!! 
	* Effectively speeds up the motion of the strike without affecting the overall time taken
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	float EffectiveProgressMultiplier;

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bStrikeLockedIn;

	void BeginStrike(FVector Target);

	void ContinueStrike(float DeltaTime);

	UPROPERTY(Transient, BlueprintReadWrite)
	float StrikeTimer;

	FVector StrikeTarget;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	FVector GetStrikeTarget() { return StrikeTarget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	bool IsStriking();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
		bool HasActiveStrikeBox() { return ActiveStrikeBox != nullptr; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	float GetStrikeProgress();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	float GetStrikeIKWeight();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	ELimb GetStrikeLimbType();

#pragma endregion Attack


#pragma region Gait

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetGaitFraction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetSpeedFraction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	FTransform GetLeftFootIKTransform();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	FTransform GetRightFootIKTransform();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetLeftFootIKBlend();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetRightFootIKBlend();

#pragma endregion Gait

};
