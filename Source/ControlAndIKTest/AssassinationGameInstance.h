// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GlobalAuthority.h"


#include "AssassinationGameInstance.generated.h"

class AGlobalAuthority;

/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API UAssassinationGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlobalReferencing)
	AGlobalAuthority* GlobalAuth;
	
};
