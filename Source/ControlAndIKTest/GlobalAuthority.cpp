// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalAuthority.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "WallCrawler.h"
#include "Human.h"

#include "Runtime/Engine/Classes/Engine/World.h"
#include "TensionMeterComponent.h"

// Sets default values
AGlobalAuthority::AGlobalAuthority()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TensionMeter = CreateDefaultSubobject<UTensionMeterComponent>("TensionMeter");
}

// Called when the game starts or when spawned
void AGlobalAuthority::BeginPlay()
{
	Super::BeginPlay();
	
	//TheGlobalAuthority = this;

	// Gather humans and let them know who is the global authority (it's this)
	TArray<AActor*> HumanActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHuman::StaticClass(), HumanActors);
	for (auto & HumanActor : HumanActors)
	{
		AHuman* Human = Cast<AHuman>(HumanActor);
		if (Human)
		{
			Humans.Add(Human);
			//Human->SetGlobalAuthority(this);
		}
	}

	// Find a crawler (hopefully just one)
	TArray<AActor*> CrawlerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWallCrawler::StaticClass(), CrawlerActors);
	if (CrawlerActors.Num() < 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority found no crawlers!")); 
		return;
	}
	else if (CrawlerActors.Num() > 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority found more than 1 crawler! Choosing one arbitrarily!"));
		Crawler = Cast<AWallCrawler>(CrawlerActors[0]);
	}
	else
	{
		Crawler = Cast<AWallCrawler>(CrawlerActors[0]);
	}

	// Debug stuff
	//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority:"));
	//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("Humans: ") + FString::FromInt(Humans.Num()));
	//for (auto & Human : Humans)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("		") + AActor::GetDebugName(Human));
	//}
	//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("Crawler: ") + (Crawler ? FString("yes") : FString("no")));

}

// Called every frame
void AGlobalAuthority::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AGlobalAuthority::GetCrawlerRealLocation()
{
	if (Crawler)
		return Crawler->GetActorLocation();
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority has no crawler!"));
		return FVector(0, 0, 0);
	}
}

FVector AGlobalAuthority::GetCrawlerLastKnownLocation()
{
	return CrawlerLastKnownLocation;
}

void AGlobalAuthority::SetCrawlerLastKnownLocation(FVector NewLastKnownLocation)
{
	CrawlerLastKnownLocation = NewLastKnownLocation;
	bCrawlerLastKnownLocationIsValid = true;
}

void AGlobalAuthority::ResetCrawlerLastKnownLocation()
{
	bCrawlerLastKnownLocationIsValid = false;
}

bool AGlobalAuthority::IsGlobalAlert()
{
	return TensionMeter->IsTense();
}

float AGlobalAuthority::GetAlertTimeRemaining()
{
	return TensionMeter->GetTensionValue();
}

FString AGlobalAuthority::GetAlertTimeRemainingAsString(int NumDecimalPlaces)
{
	// Multiply by 10^NumDecimalPlaces
	// Floor
	// Divide by NumDecimalPlaces
	// Convert to String

	//float Multiplier = powf(10, NumDecimalPlaces);
	//float Value = TensionMeter->GetTensionValue();
	//Value *= Multiplier;
	//Value = floorf(Value);
	//Value /= Multiplier;
	//
	//return FString::SanitizeFloat(Value, 2);

	FString FullString = FString::SanitizeFloat(TensionMeter->GetTensionValue(), 2);
	int DecimalPointIndex;
	if (FullString.FindChar('.', DecimalPointIndex))
	{
		return FullString.Left(DecimalPointIndex + 3);
	}
	
	return FullString;
}

void AGlobalAuthority::SetGlobalAlert()
{ 
	TensionMeter->SetTense();
}

AHuman* AGlobalAuthority::GetNearestLivingHuman(FVector ThisLocation, AHuman* IgnoredHuman)
{
	float CurrentMin = INFINITY;
	AHuman* ClosestHuman = nullptr;
	for (auto & Human : Humans)
	{
		if (Human == IgnoredHuman)
			continue;

		if (Human->IsDead_Implementation())
			continue;

		float SquareDistance = FVector::DistSquared(ThisLocation, Human->GetActorLocation());
		if (SquareDistance < CurrentMin)
		{
			CurrentMin = SquareDistance;
			ClosestHuman = Human;
		}
	}

	return ClosestHuman;
}


AGlobalAuthority* AGlobalAuthority::TheGlobalAuthority;

AGlobalAuthority* AGlobalAuthority::GetGlobalAuthority(UObject* AnyObjectInWorld)
{
	if(!TheGlobalAuthority)
	{
		TheGlobalAuthority = AnyObjectInWorld->GetWorld()->SpawnActor<AGlobalAuthority>(AGlobalAuthority::StaticClass());
	}
	return TheGlobalAuthority;
}