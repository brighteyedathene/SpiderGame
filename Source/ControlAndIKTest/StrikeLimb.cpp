// Fill out your copyright notice in the Description page of Project Settings.

#include "StrikeLimb.h"
#include "WallCrawler.h"
#include "Human.h"

#include "DrawDebugHelpers.h"

UStrikeLimb::UStrikeLimb()
{
	PrimaryComponentTick.bCanEverTick = true;

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
	IgnoreActorWhenMoving(GetOwner(), true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UStrikeLimb::BeginStrike()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("GAHAAAAA"));
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	bHitboxActive = true;
}

void UStrikeLimb::EndStrike()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bHitboxActive = false;
}

void UStrikeLimb::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
		return;

	AWallCrawler* Crawler = Cast<AWallCrawler>(OtherActor);
	if (Crawler)
	{
		Crawler->UpdateHealth_Implementation(-Damage);
		
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("GATCHA"));
	}
	//AHuman* Human = Cast<AHuman>(OtherActor);
	//if (Human)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, FString("oopsie to ") + AActor::GetDebugName(OtherActor));
	//	Human->SetStunnedFor(2.2);
	//}
	//else
	//{
	//	Human = Cast<AHuman>(GetOwner());
	//	if (Human)
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString("Stunned by ") + AActor::GetDebugName(OtherActor));
	//		Human->SetStunnedFor(2.2);
	//	}
	//}
}


void UStrikeLimb::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//if (bHitboxActive)
	//	DrawDebugBox(GetWorld(), GetComponentLocation(), GetScaledBoxExtent(), GetComponentQuat(), FColor::Red, false, -1.f, 0, 0.1f);

}


// Definition for the static TMap SocketMap - this is necessary to prevent unresolved external blablabla error
TMap<ELimb, FName> UStrikeLimb::SocketMap;