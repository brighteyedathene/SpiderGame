// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalAuthority.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#include "AssassinationGameState.h"
#include "WallCrawler.h"
#include "Human.h"

#include "Runtime/Engine/Classes/Engine/World.h"
#include "TensionMeterComponent.h"

#include "DrawDebugHelpers.h"

// Sets default values
AGlobalAuthority::AGlobalAuthority()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TensionMeter = CreateDefaultSubobject<UTensionMeterComponent>("TensionMeter");

	//TheGlobalAuthority = nullptr;
}

// Called when the game starts or when spawned
void AGlobalAuthority::BeginPlay()
{
	Super::BeginPlay();
	
	// In case there is a global authority already in the level, let it be THE static global auth
	//if (TheGlobalAuthority)
	//{
	//	TheGlobalAuthority->Destroy();
		//TheGlobalAuthority = this;
	//}

	// Gather humans and let them know who is the global authority (it's this)
	//GetWorld()->

	//TArray<AActor*> HumanActors;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHuman::StaticClass(), HumanActors);
	//for (auto & HumanActor : HumanActors)
	//{
	//	AHuman* Human = Cast<AHuman>(HumanActor);
	//	if (Human)
	//	{
	//		Humans.Add(Human);
	//	}
	//}
	//
	//// Find a crawler (hopefully just one)
	//TArray<AActor*> CrawlerActors;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWallCrawler::StaticClass(), CrawlerActors);
	//if (CrawlerActors.Num() < 1)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority found no crawlers!")); 
	//	return;
	//}
	//else if (CrawlerActors.Num() > 1)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority found more than 1 crawler! Choosing one arbitrarily!"));
	//	Crawler = Cast<AWallCrawler>(CrawlerActors[0]);
	//}
	//else
	//{
	//	Crawler = Cast<AWallCrawler>(CrawlerActors[0]);
	//}

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

void AGlobalAuthority::UpdateCrawlerLastKnownLocationAndVelocity()
{
	if (Crawler)
	{
		CrawlerLastKnownLocation = Crawler->GetActorLocation();
		CrawlerLastKnownVelocity = Crawler->GetVelocity();
		bCrawlerLastKnownLocationIsValid = true;
	}
}

FVector AGlobalAuthority::GetCrawlerPredictedLocation(float SecondsFromLastSighting)
{
	if (Crawler)
	{
		FVector PredictedLocation = CrawlerLastKnownLocation + CrawlerLastKnownVelocity * SecondsFromLastSighting;
		DrawDebugSphere(GetWorld(), PredictedLocation, 5.f, 24, FColor::Orange);

		ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;
		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(Crawler);
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, CrawlerLastKnownLocation, PredictedLocation, TraceChannel, CollisionParameters))
		{
			PredictedLocation = Hit.ImpactPoint;
			DrawDebugSphere(GetWorld(), PredictedLocation, 5.f, 24, FColor::Purple);
		}

		return PredictedLocation;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("GlobalAuthority has no crawler!"));
		return FVector(0, 0, 0);
	}
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


//void AGlobalAuthority::EndPlay(const EEndPlayReason::Type EndPlayReason)
//{
//	TheGlobalAuthority = nullptr;
//	Destroy();
//}

//AGlobalAuthority* AGlobalAuthority::TheGlobalAuthority;
//
//AGlobalAuthority* AGlobalAuthority::GetGlobalAuthority(UObject* AnyObjectInWorld)
//{
//	
//	if(!TheGlobalAuthority)
//	{
//		TheGlobalAuthority = AnyObjectInWorld->GetWorld()->SpawnActor<AGlobalAuthority>(AGlobalAuthority::StaticClass());
//	}
//	return TheGlobalAuthority;
//}