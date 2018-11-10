// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthInterface.h"

#include "Human.generated.h"

class UHumanMovement;
class UHumanGaitControl;
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);
	void MoveToCentre();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	ATargetPoint* CentrePoint;


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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	float MaxHealth;
	float CurrentHealth;

	bool bDead;

#pragma endregion Health



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
