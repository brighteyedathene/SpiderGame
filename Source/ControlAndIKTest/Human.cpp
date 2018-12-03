// Fill out your copyright notice in the Description page of Project Settings.

#include "Human.h"
#include "HumanMovement.h"
#include "HumanGaitControl.h"
#include "HumanSenseComponent.h"
#include "MobileTargetActor.h"

#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"

#include "DrawDebugHelpers.h"

// Sets default values
AHuman::AHuman()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HumanCollider = CreateDefaultSubobject<UCapsuleComponent>("HumanCollider");

	HumanMovement = CreateDefaultSubobject<UHumanMovement>("HumanMovement");
	
	HumanGaitControl = CreateDefaultSubobject<UHumanGaitControl>("HumanGaitControl");

	HumanSense = CreateDefaultSubobject<UHumanSenseComponent>("HumanSense");

	EffectiveProgressMultiplier = 2.f;

	MaxHealth = 100.f;

	StunDecayDuration = 1.f;
}

// Called when the game starts or when spawned
void AHuman::BeginPlay()
{
	Super::BeginPlay();

	// For some reason it's easier to edit these components if they're not made with CreateDefaultSubobject!
	UActorComponent* PossibleSkeletalMesh = GetComponentByClass(USkeletalMeshComponent::StaticClass());
	SkeletalMesh = Cast<USkeletalMeshComponent>(PossibleSkeletalMesh);


	// Populate StrikeBoxes array and attach them to their bones on the skeletal mesh
	TArray<UActorComponent*> StrikeBoxActorComponents = GetComponentsByClass(UStrikeBox::StaticClass());
	for (auto & StrikeBoxActorComponent : StrikeBoxActorComponents)
	{
		UStrikeBox* StrikeBox = Cast<UStrikeBox>(StrikeBoxActorComponent);
		if (StrikeBox)
		{
			StrikeBoxes.Add(StrikeBox);

			if (!StrikeBox->SocketName.IsEqual(FName("None")))
			{
				if (!StrikeBox->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepWorldTransform, StrikeBox->SocketName))
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString("FAILED to attach StrikeBox to ") + StrikeBox->SocketName.ToString());
				}
			}
			else
			{
				if (!StrikeBox->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepWorldTransform, UStrikeBox::SocketMap[StrikeBox->StrikePosition]))
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString("FAILED to attach StrikeBox to ") + UStrikeBox::SocketMap[StrikeBox->StrikePosition].ToString());
				}
			}
			 
		}
	}
	// Sort strike boxes according to priority
	StrikeBoxes.Sort(UStrikeBox::StrikeBoxCompare);
	
	// DEBUG List the strike boxes in order 
	for (auto & SB : StrikeBoxes)
	{
		GEngine->AddOnScreenDebugMessage(-1, 45.0f, FColor::Yellow, SB->GetName());
	}

	// Populate the StrikeLimbMap and attach to skeleton sockets
	TArray<UActorComponent*> StrikeLimbActorComponents = GetComponentsByClass(UStrikeLimb::StaticClass());
	for (auto & StrikeLimbActorComponent : StrikeLimbActorComponents)
	{
		UStrikeLimb* StrikeLimb = Cast<UStrikeLimb>(StrikeLimbActorComponent);
		if (StrikeLimb)
		{
			StrikeLimbMap.Add(StrikeLimb->LimbType, StrikeLimb);
			
			if (!StrikeLimb->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepWorldTransform, UStrikeLimb::SocketMap[StrikeLimb->LimbType]))
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString("FAILED to attach StrikeLimb to ") + UStrikeLimb::SocketMap[StrikeLimb->LimbType].ToString());
			}
		}
	}


	CurrentHealth = MaxHealth;
}

// Called every frame
void AHuman::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StunRemaining = fmaxf(0, StunRemaining - DeltaTime);
	if (IsStunned())
	{
		HumanMovement->bMovementDisabled = true;
	}
	else
	{
		HumanMovement->bMovementDisabled = false;
	}

	if (!IsDead_Implementation() && !IsStunned() && !bStrikeLockedIn)
	{
		HumanGaitControl->UpdateGait(HumanMovement->GetVelocity(), HumanMovement->GetVelocity().Size() / HumanMovement->MaxSpeed);
	}

	if (IsStriking())
	{
		ContinueStrike(DeltaTime);
	}
}

// Called to bind functionality to input
void AHuman::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Controls for testing
	PlayerInputComponent->BindAxis("MoveForward", this, &AHuman::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHuman::MoveRight);
}


void AHuman::MoveForward(float value)
{
	AddMovementInput(FVector::ForwardVector, value);
}

void AHuman::MoveRight(float value)
{
	AddMovementInput(FVector::RightVector, value);
}



#pragma region Health

float AHuman::GetHealth_Implementation()
{
	return CurrentHealth;
}

void AHuman::UpdateHealth_Implementation(float Delta)
{
	if (Delta < 0)
	{
		StunRemaining = StunDecayDuration;
	}

	CurrentHealth = fminf(CurrentHealth + Delta, MaxHealth);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("ouch I recieved %f damage! Now my health is %f"), Delta, CurrentHealth));
	if (IsDead_Implementation())
	{
		Die_Implementation();
	}
}

bool AHuman::IsDead_Implementation()
{
	return CurrentHealth <= 0 || bDead;
}
void AHuman::Die_Implementation()
{
	if (!bDead)
	{
		bDead = true;

		if (HumanCollider)
		{
			HumanCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (HumanSense)
		{
			HumanSense->Disable();
		}
		if (SkeletalMesh)
		{
			SkeletalMesh->SetSimulatePhysics(true);
		}
		for (auto & StrikeBox : StrikeBoxes)
		{
			StrikeBox->Disable();
		}
		for (auto & StrikeLimbElement : StrikeLimbMap)
		{
			StrikeLimbElement.Value->EndStrike();
		}

		//GetCom
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("UWAAA!<dead>"));
	}
}

float AHuman::GetBiteDamageForBone(FName BoneName)
{
	if (!LocationDamageMap.Contains(BoneName))
	{
		return 0;
	}
	else
	{
		return LocationDamageMap[BoneName];
	}
}

#pragma endregion Health



#pragma region Attack

void AHuman::UpdateActiveStrikeBox()
{
	if (!IsStriking())
	{
		ActiveStrikeBox = CheckStrikeBoxes();
	}
}

UStrikeBox* AHuman::CheckStrikeBoxes()
{
	bool CrawlerOnBody = IsCrawlerOnBody();

	for (auto & StrikeBox : StrikeBoxes)
	{
		if (StrikeBox->RequiresCrawlerOnBody == CrawlerOnBody && StrikeBox->CrawlerInside)
		{
			return StrikeBox;
		}
	}
	return nullptr;
}

bool AHuman::IsCrawlerOnBody()
{
	TArray<AActor*> ChildActors;
	//GetAllChildActors(ChildActors, true);
	GetAttachedActors(ChildActors);
	for (auto & Child : ChildActors)
	{
		AMobileTargetActor* MTA = Cast<AMobileTargetActor>(Child);
		if (MTA)
		{
			if (MTA->MTAOwnerType == EMTAOwnerType::CrawlerBody)
			{
				return true;
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, AActor::GetDebugName(Child));

	}
	return false;
}

void AHuman::BeginStrike(FVector Target)
{
	StrikeTarget = Target;

	if (!StrikeLimbMap.Contains(ActiveStrikeBox->LimbType))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Couldn't find the right limb to strike with!"));
		return;
	}
	ActiveStrikeLimb = StrikeLimbMap[ActiveStrikeBox->LimbType];
	ActiveStrikeLimb->BeginStrike();

	StrikeTimer = 0;

	bStrikeLockedIn = true;
}

void AHuman::ContinueStrike(float DeltaTime)
{
	DrawDebugLine(GetWorld(), ActiveStrikeLimb->GetComponentLocation(), StrikeTarget, FColor::Red, false, -1, 0, 0.1f);

	StrikeTimer += DeltaTime;

	// Deactivate hitbox after strike period
	if (ActiveStrikeLimb->bHitboxActive && StrikeTimer > ActiveStrikeBox->StrikeDuration)
	{
		ActiveStrikeLimb->EndStrike();
	}
	// Fully end strike process after strike + cooldown period
	if (IsStunned() || StrikeTimer > ActiveStrikeBox->StrikeDuration + ActiveStrikeBox->CooldownDuration)
	{
		bStrikeLockedIn = false;
		if(ActiveStrikeLimb)
			ActiveStrikeLimb->EndStrike();
		ActiveStrikeLimb = nullptr;
	}
}

float AHuman::GetStrikeProgress()
{
	// Strike progress
	// During striking period: ease in from [0 -> 0.5]
	// During cooldown period: ease out from [0.5 -> 1]
	// Timing is not necessarily symmetrical!

	if (!ActiveStrikeBox)
	{
		return 0.f;
	}

	if (StrikeTimer < ActiveStrikeBox->StrikeDuration)
	{
		float t = StrikeTimer / ActiveStrikeBox->StrikeDuration;
		t = fminf(1, t * EffectiveProgressMultiplier);
		return FMath::InterpEaseIn(0.f, 0.5f, t, 2.f);
	}
	else
	{
		float t = (StrikeTimer - ActiveStrikeBox->StrikeDuration) / ActiveStrikeBox->CooldownDuration;
		t = fminf(1, t * EffectiveProgressMultiplier);
		return FMath::InterpEaseOut(0.5f, 1.f, t, 2.f);
	}
}

float AHuman::GetStrikeIKWeight()
{
	// Strike IK Weight
	// During Strike: Ease in towards 1
	// After Strike: Ease in towards 0
	if (ActiveStrikeBox)
	{
		if (StrikeTimer < ActiveStrikeBox->StrikeDuration)
		{
			float t = StrikeTimer / ActiveStrikeBox->StrikeDuration;
			t = fminf(1, t * EffectiveProgressMultiplier);
			float Weight = FMath::InterpEaseIn(0.f, 1.f, t, 4.f);

			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::SanitizeFloat(Weight));

			return Weight;
		}
		else
		{
			float t =  (StrikeTimer - ActiveStrikeBox->StrikeDuration) / ActiveStrikeBox->CooldownDuration;
			t = fminf(1, t * EffectiveProgressMultiplier);
			float Weight = FMath::InterpEaseOut(1.f, 0.f, t, 4.f);

			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, FString::SanitizeFloat(Weight));

			return Weight;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("StrikeDuration was 0"));
		return 0.f;
	}
}

ELimb AHuman::GetStrikeLimbType()
{
	if (ActiveStrikeLimb)
	{
		return ActiveStrikeLimb->LimbType;
	}
	else
	{
		return ELimb::None;
	}
}

bool AHuman::IsStriking()
{
	//if (bStrikeLockedIn)
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("LOCKED IN"));
	//else
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Free"));
	//}
	return bStrikeLockedIn;
}

#pragma endregion Attack


#pragma region Gait


float AHuman::GetGaitFraction()
{
	//if (HumanGaitControl->GaitFraction > 0)
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("GaitFraction = %f"), HumanGaitControl->GaitFraction));
	//else
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("GaitFraction = %f"), HumanGaitControl->GaitFraction));
	//
	//if (HumanGaitControl->bLeftFootPlanted)
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("LEFT")));
	//else
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("RIGHT")));
	//


	//HumanGaitControl->MarkLine(
	//	RootComponent->GetComponentLocation() + (FVector::UpVector * 100),
	//	RootComponent->GetComponentLocation() + (FVector::UpVector * 100) + (FVector::UpVector * 100 * HumanGaitControl->GaitFraction),
	//	FColor::Red, 0);
	//
	//HumanGaitControl->MarkLine(
	//	RootComponent->GetComponentLocation() + (FVector::UpVector * 100) + (RootComponent->GetRightVector() * 10),
	//	RootComponent->GetComponentLocation() + (FVector::UpVector * 100) + (RootComponent->GetRightVector() * 10) + (FVector::UpVector * 100 * HumanGaitControl->RightFootIKBlend),
	//	FColor::Orange, 0);
	//
	//HumanGaitControl->MarkLine(
	//	RootComponent->GetComponentLocation() + (FVector::UpVector * 100) - (RootComponent->GetRightVector() * 10),
	//	RootComponent->GetComponentLocation() + (FVector::UpVector * 100) - (RootComponent->GetRightVector() * 10) + (FVector::UpVector * 100 * HumanGaitControl->LeftFootIKBlend),
	//	FColor::Blue, 0);



	return HumanGaitControl->GaitFraction;
}

float AHuman::GetSpeedFraction()
{
	//if (HumanGaitControl->GaitFraction > 0)
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("Speed = %f"), HumanMovement->GetVelocity().Size()));
	return HumanMovement->GetVelocity().Size() / HumanMovement->MaxSpeed;
}


FTransform AHuman::GetLeftFootIKTransform()
{
	return HumanGaitControl->LeftFoot;
}

FTransform AHuman::GetRightFootIKTransform()
{
	return HumanGaitControl->RightFoot;
}

float AHuman::GetLeftFootIKBlend()
{
	return HumanGaitControl->LeftFootIKBlend;
}

float AHuman::GetRightFootIKBlend()
{
	return HumanGaitControl->RightFootIKBlend;
}

#pragma endregion Gait