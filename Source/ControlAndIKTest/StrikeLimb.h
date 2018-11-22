// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "StrikeLimb.generated.h"




UENUM(BlueprintType)
enum class ELimb : uint8
{
	None UMETA(DisplayName = "None"),
	LeftArm UMETA(DisplayName = "LeftArm"),
	RightArm UMETA(DisplayName = "RightArm"),
	LeftLeg UMETA(DisplayName = "LeftLeg"),
	RightLeg UMETA(DisplayName = "RightLeg"),
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CONTROLANDIKTEST_API UStrikeLimb : public UBoxComponent
{
	GENERATED_BODY()

public:

	UStrikeLimb();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	ELimb LimbType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float StrikeDuration;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void BeginStrike();
	void EndStrike();


	static TMap<ELimb, FName> SocketMap;

protected:

	virtual void BeginPlay() override;


	/* Damage on hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float Damage;
};
