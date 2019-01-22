// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AssassinationGameState.generated.h"

class AHuman;
class AWallCrawler;
class UTensionMeterComponent;
class UNavigationSystemV1;
/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API AAssassinationGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	AAssassinationGameState();

protected:
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

public:

	// References to world and functions for AI
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
	TArray<AHuman*> Humans;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
	AWallCrawler* Crawler;
	
	FVector CrawlerLastKnownLocation;
	FVector CrawlerLastKnownVelocity;
	bool bCrawlerLastKnownLocationIsValid;
	UNavigationSystemV1* NavigationSystem;

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


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	AHuman* GetNearestLivingHuman(FVector ThisLocation, AHuman* IgnoredHuman = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	float GetCrawlerDistanceToNearestLivingHuman();


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GlobalReferencing)
	float GetHighestHumanTension();


	// Investivation and where to stand during a fight

	// InvestigationPoints - points for the humans to visit during a crisis
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Investigation")
	TArray<FVector> InvestigationPoints;

	/** The centre of the investigation, around which investigation points are generated */
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Investigation")
	FVector Epicentre;

	/** At what distance does the set of investigation points need to be reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	float ResetEpicentreDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	float MaxSearchRadius;

	/** How much will the search radius increase with each step for combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	float CombatRadiusStepSize;

	/** How much will the search radius increase with each step for investigation (of bodies, post alert, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	float InvestigationRadiusStepSize;

	/** How many probes at the first search ring*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	float ProbeCountInitialSize;

	/** Each time the search radius increases, how many extra probes will we add*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	float ProbeCountStepSize;


	UFUNCTION(BlueprintCallable, Category = "Investigation")
	void SetNewEpicentre(FVector NewEpicentre);

	UFUNCTION(BlueprintCallable, Category = "Investigation")
	void GenerateCombatPoints();
	
	UFUNCTION(BlueprintCallable, Category = "Investigation")
	void AddInvestigationPoints(FVector Location, int NewPointsLimit);

	UFUNCTION(BlueprintCallable, Category = "Investigation")
	void FlushInvestigationPoints();

	UFUNCTION(BlueprintCallable, Category = "Investigation")
	void SuggestInvestigationPoints(bool LimitSuggestions = false, int MaxSuggestions = 3);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigation")
	bool bUseNavigationForSuggesteeSelection;

	UFUNCTION(BlueprintCallable, Category = "Investigation")
	bool TryToAddInvestigationPoint(FVector Location, bool IgnoreOcclusion = false);


	UFUNCTION(BlueprintCallable, Category = "Investigation")
	bool IsALivingHumanCloserThan(float DistanceSquared, FVector Location);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Investigation")
	FVector GetBestInvestigationPoint(FVector MyLocation);

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Investigation")
	int NextInvestigationPointIndex;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Investigation")
	bool bShouldGenerateNewPoints;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Investigation")
	bool IsInvestigationPointAvailable() { return NextInvestigationPointIndex < InvestigationPoints.Num(); };


	// Objective counters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	int ThreatsRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	int TargetsRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	bool bAllTargetsDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	bool bAllThreatsDead;



	// Global states and cooldown

	// Set a temporary drain rate (reset during SetGlobalAlert)
	UFUNCTION(BlueprintCallable, Category = Alert)
	void SetTemporaryDrainRate(float TempDrainRate);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Alert)
	float DrainRate;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Alert)
	FString GetTimeRemainingAsString(float TimeRemaining, int NumDecimalPlaces = 2);

	// ALERT
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Alert)
	bool IsGlobalAlert() { return AlertTimeRemaining > 0; };

	UFUNCTION(BlueprintCallable, Category = Alert)
	void SetGlobalAlert();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Alert)
	float GetAlertTimeRemaining() { return AlertTimeRemaining; };




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Alert)
	float AlertCooldown;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Alert)
	float AlertTimeRemaining;


	// CAUTION
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Alert)
	bool IsGlobalCaution() { return CautionTimeRemaining > 0; };

	UFUNCTION(BlueprintCallable, Category = Alert)
	void SetGlobalCaution();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Alert)
	float GetCautionTimeRemaining() { return CautionTimeRemaining; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Alert)
	float CautionCooldown;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Alert)
	float CautionTimeRemaining;

};
