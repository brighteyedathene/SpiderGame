// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TensionMeterComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTROLANDIKTEST_API UTensionMeterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTensionMeterComponent();
		
protected:

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override; 
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tension)
	float TensionCooldown;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Tension)
	float Tension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tension)
	float DrainRate;

	UFUNCTION(BlueprintCallable, Category = Tension)
	float GetTensionValue() { return Tension; };

	UFUNCTION(BlueprintCallable, Category = Tension)
	void SetTense();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Tension)
	bool IsTense();

	UFUNCTION(BlueprintCallable, Category = Tension)
	void SetTemporaryDrainRate(float TempDrainRate);
};
