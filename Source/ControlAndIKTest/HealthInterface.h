// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HealthInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHealthInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CONTROLANDIKTEST_API IHealthInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
	float GetHealth();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
	void UpdateHealth(float Delta);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
	void Die();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
	bool IsDead();

	/** Lets the blueprint know we just died, so it can do some stuff with the level */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Health")
	void DeathNotice_BPEvent();

};
