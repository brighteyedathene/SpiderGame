// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HumanAIController.generated.h"

class UBehaviorTreeComponent;
class ATargetPoint;

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API AHumanAIController : public AAIController
{
	GENERATED_BODY()

	UBehaviorTreeComponent* BehaviorComp;
	UBlackboardComponent* BlackboardComp;
	


	ATargetPoint* MyTestTargetPoint;

	virtual void Possess(APawn* Pawn) override;


public:
	AHumanAIController();

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	FORCEINLINE ATargetPoint* GetTargetPoint() { return MyTestTargetPoint; }
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName LocationToGoKey;
};
