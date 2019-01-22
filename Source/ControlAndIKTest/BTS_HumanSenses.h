// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_HumanSenses.generated.h"

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API UBTS_HumanSenses : public UBTService
{
	GENERATED_BODY()

	virtual void TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds) override;

};
