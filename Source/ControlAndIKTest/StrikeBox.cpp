// Fill out your copyright notice in the Description page of Project Settings.

#include "StrikeBox.h"
#include "WallCrawler.h"

// Sets default values for this component's properties
UStrikeBox::UStrikeBox()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	this->OnComponentBeginOverlap.AddDynamic(this, &UStrikeBox::OnOverlapBegin);
	this->OnComponentEndOverlap.AddDynamic(this, &UStrikeBox::OnOverlapEnd);
	SetCollisionProfileName("StrikeBoxCollision");
	SocketName = FName("None");
	StrikePosition = EStrikePosition::None;

	StrikeDuration = 0.3f;
	CooldownDuration = 2.f;

	if (SocketMap.Num() == 0)
	{
		SocketMap.Add(EStrikePosition::Chest, "ChestSocket");
		SocketMap.Add(EStrikePosition::LeftArm, "LeftForeArmSocket");
		//EStrikePosition::RightArm
		//EStrikePosition::Neck
		//EStrikePosition::LowSurface
		//EStrikePosition::HighSurface
	}
}


// Called when the game starts
void UStrikeBox::BeginPlay()
{
	Super::BeginPlay();
	

}


// Called every frame
void UStrikeBox::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStrikeBox::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AWallCrawler* Crawler = Cast<AWallCrawler>(OtherActor);
	if (Crawler)
	{
		CrawlerInside = true;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Entered!"));
	}
}


void UStrikeBox::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AWallCrawler* Crawler = Cast<AWallCrawler>(OtherActor);
	if (Crawler)
	{
		CrawlerInside = false;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Left!"));
	}

}


void UStrikeBox::Disable()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Definition for the static TMap SocketMap - this is necessary to prevent unresolved external blablabla error
TMap<EStrikePosition, FName> UStrikeBox::SocketMap;