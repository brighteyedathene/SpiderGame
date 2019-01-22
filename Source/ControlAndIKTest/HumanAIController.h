// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"

#include "HumanAIController.generated.h"

class UBehaviorTreeComponent;
class ATargetPoint;
class APointOfInterestSet;
class AHuman;
//class UNavigationSystemV1;
//class UNavigationPath;

/**
 * 
 */

enum class EHumanAIMoveStatus : uint8
{
	Idle UMETA(DisplayName = Idle),
	Moving UMETA(DisplayName = Moving),
	Arrived UMETA(DisplayName = Arrived)
};


UCLASS()
class CONTROLANDIKTEST_API AHumanAIController : public AAIController
{
	GENERATED_BODY()

public:
	UBehaviorTreeComponent* BehaviorComp;
	UBlackboardComponent* BlackboardComp;

	AHumanAIController();

	virtual void Possess(APawn* Pawn) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AHuman* Human;


	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	

#pragma region BlackboardKeys

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName LocationToGoKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName PointOfInterestKey;								   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName PointOfInterestDirectionKey;						   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName CrawlerLastKnownLocationKey;						   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName CrawlerPredictedLocationKey;						   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName CrawlerLastKnownLocationValidKey;					   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName StrikePositionKey;								   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName StrikeTargetKey;									   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName StrikeChargeKey;									   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName ReadyForActionKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardKeys")
	FName CrawlerInSightKey;

#pragma endregion BlackboardKeys



#pragma region AIMovement

	UFUNCTION(BlueprintCallable, Category = "SuggestedMovement")
	void SuggestLocation(FVector Location, bool ForceAbandonCurrentMove = false);

	UFUNCTION(BlueprintCallable, Category = "SuggestedMovement")
	FVector ConsumeSuggestion();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "SuggestedMovement")
	FVector SuggestedLocation;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "SuggestedMovement")
	bool bSuggestionAvailable;



	APointOfInterestSet* PointOfInterestSet;

	UNavigationSystemV1* NavigationSystem;

	UFUNCTION(BlueprintCallable, Category = "Path")
	UNavigationPath* GetPathToLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Path")
	void StopMove();

	UFUNCTION(BlueprintCallable, Category = "Path")
	void SetFinalDestination(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Path")
	bool IsAtLocation(FVector Location);

	void UpdateMove(float DeltaTime);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Path")
	FVector FinalDestination;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Path")
	UNavigationPath* CurrentPath;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Path")
	int CurrentPathIndex;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Path")
	bool bNeedNewPath;

	//UPROPERTY(Transient, BlueprintReadOnly, Category = "Path")
	EHumanAIMoveStatus CurrentMoveStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	float ArrivalThreshold;

	/** What fraction of the final movement vector can be from the avoid component? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
		float MaxAvoidBlend;

	/** Obstacles beyond this distance will not be considered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
		float AvoidMaxDistanceThreshold;

	//UFUNCTION(BlueprintCallable, Category = "AI")
	//void DetectObstacles

#pragma endregion AIMovement

};
