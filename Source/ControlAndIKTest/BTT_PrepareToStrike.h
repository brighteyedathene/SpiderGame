// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_PrepareToStrike.generated.h"

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API UBTT_PrepareToStrike : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds) override;
	

	UPROPERTY(EditAnywhere, Category = Strike)
	float StrikeChargeTime;
};
