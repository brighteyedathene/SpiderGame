// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IKArm.generated.h"

UCLASS()
class CONTROLANDIKTEST_API AIKArm : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* IKRoot;
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* IKPin;

	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* UpperArm;
	
	UPROPERTY(EditAnywhere, Category = IK)
	USceneComponent* LowerArm;

	UPROPERTY(EditAnywhere, Category = IK)
	float UpperArmLength;

	UPROPERTY(EditAnywhere, Category = IK)
	float LowerArmLength;

	UPROPERTY(EditAnywhere, Category = IK)
	float MaximumAngle;

	UPROPERTY(EditAnywhere, Category = IK)
	FVector IKTarget;

public:	
	// Sets default values for this actor's properties
	AIKArm();

	UPROPERTY(EditAnywhere, Category = IK)
	bool DEBUG_SHOW_ANGLE;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool TargetReachable;

	FMatrix GetRotationMatrix();

	void UpdateIK();

	/* Find the angle opposite side 'a' in the triangle given by the lengths a, b and c
	* Doesn't check for a valid triangle!!
	*/
	float FindAngleA(float a, float b, float c);

	void DebugDrawArm();
	void MarkSpot(FVector Point, FColor Colour);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetIKTarget(FVector NewTarget);
	
};
