// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalAuthority.generated.h"

class AHuman;
class AWallCrawler;
class UTensionMeterComponent;

//struct TensionMeter
//{
//	static float Tension;
//	static float PreviousTimestamp;
//
//	float DrainRate;
//
//	float GetTension()
//	{
//		float NewTimestamp = GetWorld()->GetTimeSeconds();
//		float TimeDifference = NewTimestamp - PreviousTimestamp;
//		
//		Tension -= TimeDifference * DrainRate;
//	}
//};

UCLASS()
class CONTROLANDIKTEST_API AGlobalAuthority : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGlobalAuthority();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	TArray<AHuman*> Humans;
	AWallCrawler* Crawler;
	FVector CrawlerLastKnownLocation;
	FVector CrawlerLastKnownVelocity;
	bool bCrawlerLastKnownLocationIsValid;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	FVector GetCrawlerRealLocation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	FVector GetCrawlerLastKnownLocation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	FVector GetCrawlerPredictedLocation(float SecondsFromLastSighting);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	bool IsCrawlerLastKnownLocationValid() { return bCrawlerLastKnownLocationIsValid; };

	UFUNCTION(BlueprintCallable, Category = GlobalReferencing)
	void UpdateCrawlerLastKnownLocationAndVelocity();


	UFUNCTION(BlueprintCallable, Category = GlobalReferencing)
	void ResetCrawlerLastKnownLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlobalReferencing)
	UTensionMeterComponent* TensionMeter;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	bool IsGlobalAlert();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	float GetAlertTimeRemaining();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	FString GetAlertTimeRemainingAsString(int NumDecimalPlaces = 2);

	UFUNCTION(BlueprintCallable, Category = GlobalReferencing)
	void SetGlobalAlert();


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	AHuman* GetNearestLivingHuman(FVector ThisLocation, AHuman* IgnoredHuman = nullptr);


	//static AGlobalAuthority* TheGlobalAuthority;
	//
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = Globalreferencing)
	//static AGlobalAuthority* GetGlobalAuthority(UObject* AnyObjectInWorld);

};
