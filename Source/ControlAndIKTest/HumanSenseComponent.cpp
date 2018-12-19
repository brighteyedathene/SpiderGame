// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanSenseComponent.h"
#include "WallCrawler.h"
#include "Human.h"



#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UHumanSenseComponent::UHumanSenseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SenseRadius = 500.f;
	FieldOfView = 45.f;
	this->OnComponentBeginOverlap.AddDynamic(this, &UHumanSenseComponent::OnOverlapBegin);
	this->OnComponentEndOverlap.AddDynamic(this, &UHumanSenseComponent::OnOverlapEnd);
	SetCollisionProfileName("SensorCollision");
}


// Called when the game starts
void UHumanSenseComponent::BeginPlay()
{
	Super::BeginPlay();
	IgnoreActorWhenMoving(GetOwner(), true);


	AActor* Owner = GetOwner();
	if (Owner)
	{
		USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Owner->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

		// Attach this sense component to the head socket on the skeletal mesh!
		if (SkeletalMesh)
		{
			AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, FName("HeadSocket"));
			IgnoreComponentWhenMoving(SkeletalMesh, true);
		}
	}
	SetSphereRadius(SenseRadius);

	CollisionParameters.AddIgnoredActor(GetOwner());
}


// Called every frame
void UHumanSenseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


void UHumanSenseComponent::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{



}


void UHumanSenseComponent::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{



}



void UHumanSenseComponent::UpdateVision()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString("Update vision should not be called on HumanSenseComponent"));

	//bool CrawlerSeen = false;
	//bool BodySeen = false;
	//
	//
	//
	//for (auto & Thing : NearbyThings)
	//{
	//	if (IsThisActorVisible(Thing))
	//	{
	//		AWallCrawler* Crawler = Cast<AWallCrawler>(Thing);
	//		if (Crawler)
	//		{
	//			CrawlerSeen = true;
	//			CrawlerLastKnownLocation = Thing->GetActorLocation();
	//		}
	//		AHuman* Human = Cast<AHuman>(Thing);
	//		if (Human)
	//		{
	//			if (Human->IsDead_Implementation())
	//			{
	//				BodySeen = true;
	//			}
	//		}
	//	}
	//}
	//CrawlerInSight = CrawlerSeen;
	//BodyInSight = BodySeen;
}

bool UHumanSenseComponent::IsThisActorVisible(AActor* Target)
{
	if (!Target)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Tried to look at null"));
		return false;
	}

	FVector TargetLocation;

	IVisibleInterface* VisibleInterface = Cast<IVisibleInterface>(Target);
	if (VisibleInterface)
	{
		TargetLocation = VisibleInterface->GetVisionTargetLocation_Implementation();
	}
	else
	{
		TargetLocation = Target->GetActorLocation();
	}

	// Check angle
	FVector TargetDirection = (TargetLocation - GetComponentLocation()).GetSafeNormal();
	float Angle = FMath::RadiansToDegrees( acosf(FVector::DotProduct(TargetDirection, GetUpVector())));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::SanitizeFloat(Angle));


	//DrawDebugLine(GetWorld(), GetComponentLocation(), GetComponentLocation() + GetUpVector() * 200, FColor::Yellow, false, -1, 0, 0.1f);



	if (Angle > FieldOfView)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Not In field of view"));
		return false;
	}

	
	//ECollisionChannel TraceChannel = ECollisionChannel::ECC_GameTraceChannel18; // Human vision channel
	ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;

	FVector Start = GetComponentLocation();
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, TargetLocation, TraceChannel, CollisionParameters))
	{
		if (Hit.Actor.Get() == Target || Hit.Actor.Get()->GetParentActor() == Target)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("GATCHA! I see you"));
			DrawDebugLine(GetWorld(), Hit.ImpactPoint, GetComponentLocation(), FColor::Red, false, -1, 0, 0.1f);

			return true;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, AActor::GetDebugName(GetOwner()) +  FString(" was BLOCKED by ") + AActor::GetDebugName(Hit.Actor.Get()) + FString("'s ") + Hit.Component.Get()->GetFName().ToString());
			DrawDebugLine(GetWorld(), Hit.ImpactPoint, GetComponentLocation(), FColor::Yellow, false, -1, 0, 0.1f);
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Blocked by somethin..."));

	return false;
}


void UHumanSenseComponent::Disable()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


