// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Stimulus.generated.h"


UENUM()
enum class EStimulusType : uint8
{
	CrawlerSighting UMETA(DisplayName = CrawlerSighting),
	TroubleSighting UMETA(DisplayName = TroubleSighting),
	BodySighting UMETA(DisplayName = BodySighting),
	CrawlerFelt UMETA(DisplayName = CrawlerFelt)
};


/**
 * 
 */
UCLASS()
class CONTROLANDIKTEST_API UStimulus : public UObject
{
	GENERATED_BODY()
	
public:
	UStimulus();
	UStimulus(EStimulusType StimulusType, FVector StimulusLocation, AActor* StimulusActor);

	virtual void BeginDestroy() override;

	UPROPERTY()
	EStimulusType Type;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	AActor* Actor;
	
	UFUNCTION(BlueprintCallable, Category = Stimulus)
	FVector GetLocation();
};
