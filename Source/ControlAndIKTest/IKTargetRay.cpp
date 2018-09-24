// Fill out your copyright notice in the Description page of Project Settings.

#include "IKTargetRay.h"


// Sets default values for this component's properties
UIKTargetRay::UIKTargetRay()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	Target = CreateDefaultSubobject<USceneComponent>(TEXT("Target"));
}


// Called when the game starts
void UIKTargetRay::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UIKTargetRay::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

