// Fill out your copyright notice in the Description page of Project Settings.

#include "MobileTargetActor.h"


// Sets default values
AMobileTargetActor::AMobileTargetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void AMobileTargetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMobileTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

