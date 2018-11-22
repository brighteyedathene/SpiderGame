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


	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Senses)
	float SenseRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Senses)
	float TensionThreshold;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Senses)
	float Tension;


	UFUNCTION(BlueprintCallable, Category = Senses)
	bool CheckVision(AActor* Target);
	
	AWallCrawler* CrawlerTracker;
	
	void Disable();

};
