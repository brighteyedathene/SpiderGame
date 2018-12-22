// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"

#include "HumanSenseComponent.generated.h"
/**
 * 
 */

class AWallCrawler;

UCLASS()
class CONTROLANDIKTEST_API UHumanSenseComponent : public USphereComponent
{
	GENERATED_BODY()
	

public:
	// Sets default values for this component's properties
	UHumanSenseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// declare overlap begin function
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// declare overlap end function
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = SensesSetup)
	float SenseRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SensesSetup)
	float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SensesSetup)
	float TensionThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SensesSetup)
	bool SHOW_DEBUG_INFO;


	UFUNCTION(BlueprintCallable, Category = Senses)
	void UpdateVision();

	UFUNCTION(BlueprintCallable, Category = Senses)
	bool IsThisActorVisible(AActor* Target);


	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Senses)
	float Tension;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Senses)
	bool BodyInSight;
	
	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Senses)
	bool CrawlerInSight;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Senses)
	FVector CrawlerLastKnownLocation;

	// Collision parameters for vision ray casts
	FCollisionQueryParams CollisionParameters;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Senses)
	FVector GetCrawlerLastKnownLocation() { return CrawlerLastKnownLocation; };

	void Disable();



	void MarkSpot(FVector Point, FColor Colour, float Duration);
	void MarkLine(FVector Start, FVector End, FColor Colour, float Duration);
};
