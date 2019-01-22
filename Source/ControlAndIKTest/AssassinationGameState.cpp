// Fill out your copyright notice in the Description page of Project Settings.

#include "AssassinationGameState.h"


#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "WallCrawler.h"
#include "Human.h"
#include "HumanAIController.h"

#include "Runtime/Engine/Classes/Engine/World.h"

#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"

#include "TensionMeterComponent.h"

#include "DrawDebugHelpers.h"


AAssassinationGameState::AAssassinationGameState()
{
	PrimaryActorTick.bCanEverTick = true;

	AlertCooldown = 5.f;
	CautionCooldown = 10.f;
	DrainRate = 1.f;

	MaxSearchRadius = 310.f;
	CombatRadiusStepSize = 60.f;
	InvestigationRadiusStepSize = 100.f;
	ProbeCountInitialSize = 3.f;
	ProbeCountStepSize = 3.f;
	ResetEpicentreDistance = 50.f;

	bUseNavigationForSuggesteeSelection = true;
}


void AAssassinationGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();


}

// Called when the game starts or when spawned
void AAssassinationGameState::BeginPlay()
{
	Super::BeginPlay();

	// Find humans
	TArray<AActor*> ActorsArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHuman::StaticClass(), ActorsArray);
	for (auto & HumanActor : ActorsArray)
	{
		AHuman* Human = Cast<AHuman>(HumanActor);
		if (Human)
		{
			Humans.Add(Human);
			//Human->AssassinationState = this;
		}
	}

	// Find a crawler (hopefully just one)
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWallCrawler::StaticClass(), ActorsArray);
	if (ActorsArray.Num() < 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("No crawlers!"));
	}
	else if (ActorsArray.Num() > 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("Found more than 1 crawler! Choosing one arbitrarily!"));
		Crawler = Cast<AWallCrawler>(ActorsArray[0]);
	}
	else
	{
		Crawler = Cast<AWallCrawler>(ActorsArray[0]);
	}

	// Initialize navigation system
	NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
}


void AAssassinationGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update Alert cooldowns
	if (IsGlobalAlert())
	{
		AlertTimeRemaining = fmaxf(0, AlertTimeRemaining - DrainRate * DeltaTime);
		if (AlertTimeRemaining == 0)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("Beginning CAUTION after ALERT phase!"));
			SetGlobalCaution();
		}
	}
	else if (IsGlobalCaution())
	{
		CautionTimeRemaining = fmaxf(0, CautionTimeRemaining - DrainRate * DeltaTime);

		if (CautionTimeRemaining == 0)
		{
			FlushInvestigationPoints();
			bCrawlerLastKnownLocationIsValid = false;
			DrainRate = 1.f;
		}
	}


	// Should we spawn a completely new set of points and suggest them?
	if (bShouldGenerateNewPoints)
	{
		FlushInvestigationPoints();
		GenerateCombatPoints();
		SuggestInvestigationPoints();
		bShouldGenerateNewPoints = false;
	}


	//if (InvestigationPoints.IsValidIndex(NextInvestigationPointIndex))
	//{
	//	 DrawDebugSphere(GetWorld(), InvestigationPoints[NextInvestigationPointIndex], 7.f, 3, FColor::Green, false, 0, 8, 2.f);
	//}

	//GEngine->AddOnScreenDebugMessage(999, 5.0f, FColor::Yellow, FString("Total IPs: ") + FString::FromInt(InvestigationPoints.Num()) + FString("   Next IP index: ") + FString::FromInt(NextInvestigationPointIndex));
	
	
	//DrawDebugSphere(GetWorld(), Epicentre, 4.f, 3, FColor::Green, false, 0, 8, 0.5f);
	
	for (auto & Point : InvestigationPoints)
	{
		//DrawDebugSphere(GetWorld(), Point, 3.f, 3, FColor::White, false, 0, 8, 0.5f);
		//DrawDebugLine(GetWorld(), Epicentre, Point, FColor::White, false, 0, 8, 0.1f);
	}

}



FVector AAssassinationGameState::GetCrawlerRealLocation()
{
	if (Crawler)
		return Crawler->GetActorLocation();
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("No crawler!"));
		return FVector(0, 0, 0);
	}
}

FVector AAssassinationGameState::GetCrawlerLastKnownLocation()
{
	return CrawlerLastKnownLocation;
}

void AAssassinationGameState::UpdateCrawlerLastKnownLocationAndVelocity()
{
	if (Crawler)
	{
		CrawlerLastKnownLocation = Crawler->GetActorLocation();
		CrawlerLastKnownVelocity = Crawler->GetVelocity();
		bCrawlerLastKnownLocationIsValid = true;
	}
}

FVector AAssassinationGameState::GetCrawlerPredictedLocation(float SecondsFromLastSighting)
{
	if (Crawler)
	{
		FVector PredictedLocation = CrawlerLastKnownLocation + CrawlerLastKnownVelocity * SecondsFromLastSighting;
		//DrawDebugSphere(GetWorld(), PredictedLocation, 5.f, 24, FColor::Orange);

		ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;
		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(Crawler);
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, CrawlerLastKnownLocation, PredictedLocation, TraceChannel, CollisionParameters))
		{
			PredictedLocation = Hit.ImpactPoint;
			//DrawDebugSphere(GetWorld(), PredictedLocation, 5.f, 24, FColor::Purple);
		}

		return PredictedLocation;
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("No crawler!"));
		return FVector(0, 0, 0);
	}
}

void AAssassinationGameState::ResetCrawlerLastKnownLocation()
{
	bCrawlerLastKnownLocationIsValid = false;
}


FString AAssassinationGameState::GetTimeRemainingAsString(float TimeRemaining, int NumDecimalPlaces)
{
	FString FullString = FString::SanitizeFloat(TimeRemaining, 2);
	int DecimalPointIndex;
	if (FullString.FindChar('.', DecimalPointIndex))
	{
		return FullString.Left(DecimalPointIndex + 3);
	}

	return FullString;
}

void AAssassinationGameState::SetGlobalAlert()
{
	AlertTimeRemaining = AlertCooldown;
	DrainRate = 1.f;
}
void AAssassinationGameState::SetGlobalCaution()
{
	CautionTimeRemaining = CautionCooldown;
}

void AAssassinationGameState::SetTemporaryDrainRate(float TempDrainRate)
{
	DrainRate = TempDrainRate;
}









AHuman* AAssassinationGameState::GetNearestLivingHuman(FVector ThisLocation, AHuman* IgnoredHuman)
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


float AAssassinationGameState::GetCrawlerDistanceToNearestLivingHuman()
{
	float CurrentMin = INFINITY;


	for (auto & Human : Humans)
	{
		if (Human->IsDead_Implementation())
			continue;

		float SquareDistance = FVector::DistSquared(GetCrawlerRealLocation(), Human->GetActorLocation());
		if (SquareDistance < CurrentMin)
		{
			CurrentMin = SquareDistance;
		}

	}
	return sqrtf(CurrentMin);
}


float AAssassinationGameState::GetHighestHumanTension()
{
	float CurrentMax = 0;


	for (auto & Human : Humans)
	{
		// LIVING OR DEAD!
		//if (Human->IsDead_Implementation())
		//	continue;

		float TensionFraction = Human->Tension / Human->TensionThreshold;

		if (TensionFraction > CurrentMax)
		{
			CurrentMax = TensionFraction;
		}

	}
	return CurrentMax;
}




#pragma region Investigation

void AAssassinationGameState::SetNewEpicentre(FVector NewEpicentre)
{ 
	// Should we generate more investigation points?
	//if (IsGlobalAlert())
	//{
		if (FVector::Distance(Epicentre, NewEpicentre) > ResetEpicentreDistance)
		{
			bShouldGenerateNewPoints = true;
			Epicentre = NewEpicentre;
		}
	//}

};

void AAssassinationGameState::FlushInvestigationPoints()
{
	InvestigationPoints.Empty();
	NextInvestigationPointIndex = 0;
}

void AAssassinationGameState::GenerateCombatPoints()
{
	TryToAddInvestigationPoint(Epicentre);

	float Radius = CombatRadiusStepSize;
	float ProbeCount = ProbeCountInitialSize;

	while(Radius < MaxSearchRadius)
		//&& InvestigationPoints.Num() < Humans.Num())
	{
		// Add another ring of investigation points
		float offset = FMath::FRandRange(0.f, 360.f);
		for (float angle = 0 + offset; angle < 360.f + offset; angle += 360.f / ProbeCount)
		{
			FVector ProbeVector = FVector(cosf(angle), sinf(angle), 0).GetSafeNormal() * Radius;
			FVector NewPoint = Epicentre + ProbeVector;
			TryToAddInvestigationPoint(NewPoint, !IsGlobalAlert());
		}

		Radius += CombatRadiusStepSize;
		ProbeCount += ProbeCountStepSize;
	}
}

void AAssassinationGameState::AddInvestigationPoints(FVector Location, int NewPointsLimit)
{
	int PointsAdded = 0;

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString("Adding ") + FString::FromInt(NewPointsLimit) + FString(" new investigation points at ") + Location.ToCompactString());

	//if (TryToAddInvestigationPoint(Location))
	//{
	//	PointsAdded++;
	//	if (PointsAdded >= NewPointsLimit)
	//	{
	//		return;
	//	}
	//}


	//float Radius = InvestigationRadiusStepSize;
	float Radius = InvestigationRadiusStepSize;
	const float MaxRadius = MaxSearchRadius;
	float ProbeCount = ProbeCountInitialSize;

	while (Radius < MaxRadius && PointsAdded < NewPointsLimit)
	{
		// Add another ring of investigation points
		float offset = FMath::FRandRange(0.f, 360.f);
		for (float angle = 0 + offset; angle < 360.f + offset; angle += 360.f / ProbeCount)
		{
			FVector ProbeVector = FVector(cosf(angle), sinf(angle), 0).GetSafeNormal() * Radius;
			FVector NewPoint = Location + ProbeVector;

			//DrawDebugLine(GetWorld(), Location, NewPoint, FColor::Cyan, true, 1.f, -1, 0.1f);

			if (TryToAddInvestigationPoint(NewPoint, true))
			{
				//DrawDebugLine(GetWorld(), Location, InvestigationPoints.Last(), FColor::Cyan, true, 1.f, -1, 1.f);
				
				PointsAdded++;
				if (PointsAdded >= NewPointsLimit)
				{
					return;
				}
			}
		}

		Radius += InvestigationRadiusStepSize;
		ProbeCount += ProbeCountStepSize;
	}


}


void AAssassinationGameState::SuggestInvestigationPoints(bool LimitSuggestions, int MaxSuggestions)
{
	
	int Limit = (LimitSuggestions) ? FMath::Min(MaxSuggestions, InvestigationPoints.Num()) : InvestigationPoints.Num();
	for (int i = 0; i < Limit; i++)
	{

		// Find best unbusy human for the job
		float ClosestDistance = INFINITY;
		AHuman* ClosestHuman = nullptr;

		for (auto & Human : Humans)
		{
			if (!Human->IsDead_Implementation() && !Human->GetHumanAI()->bSuggestionAvailable)
			{
				if (bUseNavigationForSuggesteeSelection)
				{
					UNavigationPath* NavPath = NavigationSystem->FindPathToLocationSynchronously(GetWorld(), Human->GetActorLocation(), InvestigationPoints[i]);
					float Distance = NavPath->GetPathLength();
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						ClosestHuman = Human;
					}
				}
				else
				{
					//Simple distance
					float Distance = FVector::Distance(Human->GetActorLocation(), InvestigationPoints[i]);
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						ClosestHuman = Human;
					}
				}
			}
		}
		if (ClosestHuman)
		{
			AHumanAIController* HumanAI = ClosestHuman->GetHumanAI();
			if (HumanAI)
			{
				HumanAI->SuggestLocation(InvestigationPoints[i], true);
			}
		}
	}
}

bool AAssassinationGameState::TryToAddInvestigationPoint(FVector Location, bool IgnoreOcclusion)
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	//CollisionParams.AddIgnoredActor(Crawler);
	if (!IgnoreOcclusion && GetWorld()->LineTraceSingleByChannel(Hit, Epicentre, Location, ECollisionChannel::ECC_Camera, CollisionParams))
	{
		//DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 2.f, 3, FColor::Red, true, 1.f, -1, 0.1f);
		//DrawDebugLine(GetWorld(), Epicentre, Hit.ImpactPoint, FColor::Red, true, 1.f, -1, 0.1f);

		return false;
	}

	if (NavigationSystem)
	{
		FNavLocation NavLocation;
		if (NavigationSystem->ProjectPointToNavigation(Location, NavLocation))
		{
			InvestigationPoints.Add(NavLocation.Location);
			//DrawDebugSphere(GetWorld(), NavLocation.Location, 3.f, 3, FColor::Yellow, true, 0.2f, -1, 3.1f);
			return true;
		}
		else
		{
			//DrawDebugSphere(GetWorld(), Location, 8.f, 3, FColor::Red, true, 3.f, -1, 0.1f);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("NavigationSystem not found!"));
	}
	return false;
}


bool AAssassinationGameState::IsALivingHumanCloserThan(float DistanceSquared, FVector Location)
{
	for (auto & Human : Humans)
	{
		if (Human->IsDead_Implementation())
			continue;

		if (FVector::DistSquared(Location, Human->GetActorLocation()) < DistanceSquared)
		{
			return true;
		}
	}
	return false;
}


FVector AAssassinationGameState::GetBestInvestigationPoint(FVector MyLocation)
{
	while (InvestigationPoints.IsValidIndex(NextInvestigationPointIndex))
	{
		// Check for nearby humans
		if (IsALivingHumanCloserThan(360.f, InvestigationPoints[NextInvestigationPointIndex]))
		{
			DrawDebugSphere(GetWorld(), InvestigationPoints[NextInvestigationPointIndex], 8.f, 3, FColor::Red, true, 3.f, -1, 0.1f);
			NextInvestigationPointIndex++;
		}
		else
		{
			return InvestigationPoints[NextInvestigationPointIndex++];
		}
	}

	// No suitable points found
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString("No investigation points found!"));
	FlushInvestigationPoints();
	if (IsGlobalAlert())
	{
		bShouldGenerateNewPoints = true;
	} 
	else if(IsGlobalCaution())
	{
		bShouldGenerateNewPoints = true;
		//AddInvestigationPoints(MyLocation, 1);
	}
	return MyLocation;
	
}


#pragma region Investigation
