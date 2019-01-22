// Fill out your copyright notice in the Description page of Project Settings.

#include "TensionMeterComponent.h"


// Sets default values for this component's properties
UTensionMeterComponent::UTensionMeterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	
	DrainRate = 1.f;
	TensionCooldown = 25.f;
}


void UTensionMeterComponent::BeginPlay()
{
	//SetTickGroup(TG_PostUpdateWork);

}

// Called every frame
void UTensionMeterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Tension = fmaxf(0, Tension - DrainRate * DeltaTime);
}


void UTensionMeterComponent::SetTense()
{
	Tension = TensionCooldown;
	DrainRate = 1.f;
}

bool UTensionMeterComponent::IsTense()
{
	return Tension > 0;
}

void UTensionMeterComponent::SetTemporaryDrainRate(float TempDrainRate)
{
	DrainRate = TempDrainRate;
}