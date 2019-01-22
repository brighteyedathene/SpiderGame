// Fill out your copyright notice in the Description page of Project Settings.

#include "Stimulus.h"

#include "WallCrawler.h"
#include "Human.h"

UStimulus::UStimulus() {};

UStimulus::UStimulus(EStimulusType StimulusType, FVector StimulusLocation, AActor* StimulusActor) 
{
	Type = StimulusType;
	Location = StimulusLocation;
	Actor = StimulusActor;
}

void UStimulus::BeginDestroy()
{
	
	if (Type == EStimulusType::BodySighting)
	{
		AHuman * Human = Cast<AHuman>(Actor);
		if (Human)
		{
			Human->MarkAsSeenDead();
		}
	}

	Super::BeginDestroy();
}

FVector UStimulus::GetLocation()
{
	if (Type == EStimulusType::CrawlerSighting)
	{
		AWallCrawler * Crawler = Cast<AWallCrawler>(Actor);
		if (Crawler)
		{
			return Crawler->GetVisionTargetLocation_Implementation();
		}
	}
	//else if (Type == EStimulusType::TroubleSighting)
	//{
	//	AHuman * Human = Cast<AHuman>(Actor);
	//	if (Human)
	//	{
	//		return Human->EyeLocationMarker->GetComponentLocation();
	//	}
	//}
	//else if (Type == EStimulusType::BodySighting)
	//{
	//	AHuman * Human = Cast<AHuman>(Actor);
	//	if (Human)
	//	{
	//		return Human->GetVisionTargetLocation_Implementation();
	//	}
	//}

	return Location;
}