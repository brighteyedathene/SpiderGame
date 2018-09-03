// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IKArm.generated.h"

UCLASS()
class CONTROLANDIKTEST_API AIKArm : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIKArm();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
