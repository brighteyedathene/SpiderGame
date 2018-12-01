// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanSenseComponent.h"
#include "WallCrawler.h"


// Sets default values for this component's properties
UHumanSenseComponent::UHumanSenseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SenseRadius = 500.f;
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
		}
	}
	SetSphereRadius(SenseRadius);
}


// Called every frame
void UHumanSenseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


void UHumanSenseComponent::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AWallCrawler* Crawler = Cast<AWallCrawler>(OtherActor);
	if (Crawler)
	{
		CrawlerTracker = Crawler;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Entered the big csphere!"));

	}
}


void UHumanSenseComponent::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AWallCrawler* Crawler = Cast<AWallCrawler>(OtherActor);
	if (Crawler)
	{
		CrawlerTracker = nullptr;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Left the big sphere!"));

	}

}






bool UHumanSenseComponent::CheckVision(AActor* Target)
{
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(GetOwner());
	ECollisionChannel TraceChannel = ECC_Visibility;

	FVector Start = GetComponentLocation();
	FVector End = Target->GetActorLocation();
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, CollisionParameters))
	{
		if (Hit.Actor.Get() == Target || Hit.Actor.Get()->GetParentActor() == Target)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("GATCHA! I see you"));
			return true;
		}
	}
	return false;
}


void UHumanSenseComponent::Disable()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
