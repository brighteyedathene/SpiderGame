// Fill out your copyright notice in the Description page of Project Settings.

#include "StrikeLimb.h"
#include "WallCrawler.h"

UStrikeLimb::UStrikeLimb()
{
	this->OnComponentBeginOverlap.AddDynamic(this, &UStrikeLimb::OnOverlapBegin);
	//this->OnComponentEndOverlap.AddDynamic(this, &UStrikeLimb::OnOverlapEnd);
	SetCollisionProfileName("StrikeBoxCollision");
	//SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Damage = 200.f;

	if (SocketMap.Num() == 0)
	{
		SocketMap.Add(ELimb::LeftArm, "LeftHandSocket");
		SocketMap.Add(ELimb::LeftLeg, "LeftFootSocket");
		SocketMap.Add(ELimb::RightArm, "RightHandSocket");
		SocketMap.Add(ELimb::RightLeg, "RightFootSocket");
		SocketMap.Add(ELimb::None, "");
	}
}

void UStrikeLimb::BeginPlay()
{
	Super::BeginPlay();
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UStrikeLimb::BeginStrike()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UStrikeLimb::EndStrike()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UStrikeLimb::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AWallCrawler* Crawler = Cast<AWallCrawler>(OtherActor);
	if (Crawler)
	{
		Crawler->UpdateHealth_Implementation(-Damage);
		
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("GATCHA"));
	}
}


// Definition for the static TMap SocketMap - this is necessary to prevent unresolved external blablabla error
TMap<ELimb, FName> UStrikeLimb::SocketMap;