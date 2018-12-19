// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HumanAIController.generated.h"

class UBehaviorTreeComponent;
class ATargetPoint;
class APointOfInterestSet;

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API AHumanAIController : public AAIController
{
	GENERATED_BODY()

public:
	UBehaviorTreeComponent* BehaviorComp;
	UBlackboardComponent* BlackboardComp;

	virtual void Possess(APawn* Pawn) override;

	AHumanAIController();

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName LocationToGoKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName PointOfInterestKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName PointOfInterestDirectionKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName CrawlerKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName StrikePositionKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName StrikeTargetKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName StrikeProgressKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName ReadyForActionKey;

	APointOfInterestSet* PointOfInterestSet;
};
