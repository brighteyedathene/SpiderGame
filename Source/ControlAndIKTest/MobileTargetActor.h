// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MobileTargetActor.generated.h"

UCLASS()
class CONTROLANDIKTEST_API AMobileTargetActor : public AActor
{
	GENERATED_BODY()
	
	USceneComponent* RootComponent;


public:	
	// Sets default values for this actor's properties
	AMobileTargetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
