// Fill out your copyright notice in the Description page of Project Settings.

#include "Human.h"
#include "HumanMovement.h"
#include "HumanGaitControl.h"
#include "MobileTargetActor.h"

#include "AssassinationGameState.h"
#include "HumanAIController.h"
#include "Stimulus.h"

#include "WallCrawler.h"


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

	// = CreateDefaultSubobject<UHumanSenseComponent>("HumanSense");
	
	EyeLocationMarker = CreateDefaultSubobject<USceneComponent>("EyeLocationMarker");

	VisionTargetMarker = CreateDefaultSubobject<USceneComponent>("VisionTargetMarker");

	EffectiveProgressMultiplier = 2.f;

	MaxHealth = 100.f;

	StunDecayDuration = 1.f;

	TensionThreshold = 1.f;
	TensionCooldownRate = 0.5f;
}

void AHuman::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	
}

// Called when the game starts or when spawned
void AHuman::BeginPlay()
{
	Super::BeginPlay();

	//AssassinationState = Cast<AAssassinationGameState>(GetWorld()->GetGameState());

	// ATTACHMENTS

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
	//for (auto & SB : StrikeBoxes)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 45.0f, FColor::Yellow, SB->GetName());
	//}

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

	if (SkeletalMesh)
	{
		VisionTargetMarker->SetRelativeLocation(FVector(0, 0, 0));
		EyeLocationMarker->SetRelativeLocation(FVector(0, 0, 0));
		VisionTargetMarker->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, FName("HipsSocket"));
		EyeLocationMarker->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, FName("HeadSocket"));
		EyeLocationMarker->SetRelativeRotation(FRotator(0, 0, EyeTilt));
	}

	//StrikeTargetTracker = CreateDefaultSubobject<AMobileTargetActor>("StrikeTargetTracker");
	StrikeTargetTracker = GetWorld()->SpawnActor<AMobileTargetActor>(AMobileTargetActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

	// END ATTACHMENTS


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

		MarkAsNoLongerInTrouble();
	}

	if (!IsDead_Implementation() && !IsStunned() && !bStrikeLockedIn)
	{
		HumanGaitControl->UpdateGait(HumanMovement->GetVelocity(), HumanMovement->GetVelocity().Size() / HumanMovement->MaxSpeed);
	}


	if (IsStriking())
	{
		ContinueStrike(DeltaTime);
	}

	UpdateTension(DeltaTime);

	//PrintStatusVariables();
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



#pragma region InnerFeeling

void AHuman::UpdateTension(float DeltaTime)
{

	//if (Tension >= TensionThreshold && !IsStunned() && !IsDead_Implementation())
	//{
	//	GetAssassinationState()->SetGlobalAlert(); // Set (or reset) global alert
	//}

	// Update Tension, maybe trigger alert
	if (bCrawlerInSight || bCrawlerOnSensitiveArea || bHumanInTroubleInSight || 
		(GetAssassinationState()->IsGlobalAlert() && ActiveStrikeBox))
	{
		Tension = fminf(TensionThreshold, Tension + DeltaTime);

		
	}
	else if (!IsStunned())
	{
		Tension = fmaxf(GetTensionMin(), Tension - TensionCooldownRate * DeltaTime);
	}
	
	
}

float AHuman::GetTensionMin()
{
	if (IsDead_Implementation())
	{
		return 0;
	}
	else if (GetAssassinationState()->IsGlobalAlert())
	{
		return TensionThreshold * 0.9;
	}
	else if (GetAssassinationState()->IsGlobalCaution())
	{
		return TensionThreshold * 0.5;
	}
	else
	{
		return 0;
	}
}



void AHuman::AddNewStimulus(EStimulusType Type, FVector Location, AActor* Actor)
{
	UStimulus* NewStimulus = NewObject<UStimulus>(UStimulus::StaticClass());
	NewStimulus->Type = Type;
	NewStimulus->Location = Location;
	NewStimulus->Actor = Actor;

	Stimuli.Add(NewStimulus);
}

void AHuman::PrintStimuli()
{
	for (int i = 0; i < Stimuli.Num(); i++)
	{
		FString msg = FString("");
		FColor clr = FColor::White;
		switch (Stimuli[i]->Type)
		{
		case EStimulusType::CrawlerSighting:
			msg = FString("  CrawlerSighting ");
			clr = FColor::Red;
			break;
		case EStimulusType::TroubleSighting:
			msg = FString("  TroubleSighting ");
			clr = FColor::Orange;
			break;
		case EStimulusType::BodySighting:
			msg = FString("  BodySighting ");
			clr = FColor::Magenta;
			break;
		}

		GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() + (i +1) * 23, 1.0f, clr, AActor::GetDebugName(this) + msg + Stimuli[i]->GetLocation().ToCompactString());
	}
}

void AHuman::PrintStatusVariables()
{
	int Offset = 1;
	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (Tension > 0 ? FColor::Red : FColor::White),
		AActor::GetDebugName(this) + FString("Tension: ") + FString::SanitizeFloat(Tension));

	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (bCrawlerInSight ? FColor::Red : FColor::White),
		FString(" bCrawlerInSight: ") + (bCrawlerInSight ? "1" : "0"));

	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (bCrawlerOnBody ? FColor::Red : FColor::White),
		FString(" bCrawlerOnBody: ") + (bCrawlerOnBody ? "1" : "0"));

	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (bCrawlerOnSensitiveArea ? FColor::Red : FColor::White),
		FString(" bCrawlerOnSensitiveArea: ") + (bCrawlerOnSensitiveArea ? "1" : "0"));

	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (bHumanInTroubleInSight ? FColor::Red : FColor::White),
		FString(" bHumanInTroubleInSight: ") + (bHumanInTroubleInSight ? "1" : "0"));

	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (bDeadBodyInSight ? FColor::Red : FColor::White),
		FString(" bDeadBodyInSight: ") + (bDeadBodyInSight ? "1" : "0"));

	GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() * Offset++, 5.0f, (IsStunned() ? FColor::Red : FColor::White),
		FString(" StunRemaining: ") + FString::SanitizeFloat(StunRemaining));
}

void AHuman::ProcessStimuli()
{
	for (auto & Stimulus : Stimuli)
	{
		if (Stimulus->Type == EStimulusType::CrawlerSighting)
		{
			TryUpdateLookTarget(Stimulus->GetLocation(), 10);

			// Set (or reset) global alert
			if (Tension >= TensionThreshold && !IsStunned())
			{
				GetAssassinationState()->SetGlobalAlert(); 
			}

			if (GetAssassinationState()->IsGlobalAlert())
			{
				GetAssassinationState()->UpdateCrawlerLastKnownLocationAndVelocity();
				GetAssassinationState()->SetNewEpicentre(GetAssassinationState()->GetCrawlerPredictedLocation(0.8f));
			}
		}
		else if (Stimulus->Type == EStimulusType::CrawlerFelt)
		{
			// Set (or reset) global alert
			if (Tension >= TensionThreshold && !IsStunned())
			{
				GetAssassinationState()->SetGlobalAlert(); 
			}

			if (GetAssassinationState()->IsGlobalAlert())
			{
				GetAssassinationState()->UpdateCrawlerLastKnownLocationAndVelocity();
				GetAssassinationState()->SetNewEpicentre(GetAssassinationState()->GetCrawlerPredictedLocation(0.8f));

			}
		}
		else if (Stimulus->Type == EStimulusType::TroubleSighting)
		{
			TryUpdateLookTarget(Stimulus->GetLocation(), 5);

			AHuman* Human = Cast<AHuman>(Stimulus->Actor);
			if (Human && !Human->bSeenInTrouble)
			{
				if (!GetAssassinationState()->IsGlobalAlert())
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString("Someone in trouble??"));

					GetAssassinationState()->SetGlobalCaution();
					//GetAssassinationState()->AddInvestigationPoints(Stimulus->GetLocation(), 3);
					GetAssassinationState()->SetNewEpicentre(Stimulus->GetLocation());

					//DrawDebugSphere(GetWorld(), Stimulus->GetLocation(), 4, 2, FColor::Orange, true, 5, 0, 4);

					Human->MarkAsSeenInTrouble();
				}
			}

		}
		else if (Stimulus->Type == EStimulusType::BodySighting)
		{
			TryUpdateLookTarget(Stimulus->GetLocation(), 3);

			AHuman* Human = Cast<AHuman>(Stimulus->Actor);
			if (Human && !Human->bSeenAsDeadBody)
			{
				if (!GetAssassinationState()->IsGlobalAlert())
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString("A body??"));


					GetAssassinationState()->SetGlobalCaution();
					//GetAssassinationState()->AddInvestigationPoints(Stimulus->GetLocation(), 3);
					GetAssassinationState()->SetNewEpicentre(Stimulus->GetLocation());

					
					//DrawDebugSphere(GetWorld(), Stimulus->GetLocation(), 4, 2, FColor::Magenta, true, 5, 0, 4);
					
				}
				Human->MarkAsSeenDead();
			}
			
		}
	}
	Stimuli.Reset();
}

void AHuman::ClearInnerFeeling()
{
	bCrawlerInSight = false;
	bCrawlerOnBody = false;
	bCrawlerOnSensitiveArea = false;
	bHumanInTroubleInSight = false;
	bDeadBodyInSight = false;
	
	StunRemaining = 0;
}


#pragma endregion InnerFeeling



#pragma region Vision

void AHuman::UpdateVision()
{
	//DrawDebugCone(GetWorld(),
	//	EyeLocationMarker->GetComponentLocation(),
	//	EyeLocationMarker->GetUpVector(),
	//	GetEffectiveIdentifyCrawlerRange(),
	//	FMath::DegreesToRadians(GetEffectiveFieldOfView()),
	//	FMath::DegreesToRadians(GetEffectiveFieldOfView()),
	//	20,
	//	FColor::White,
	//	false,
	//	0,
	//	0,
	//	0.1f);


	bool DeadBodySeen = false;
	bool CrawlerSeen = false;
	bool StunnedHumanSeen = false;

	// look for humans
	if (!GetAssassinationState())
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString("Cast to Assassination game state failed during UpdateVision!"));
		return;
	}

	for (auto & FellowHuman : GetAssassinationState()->Humans)
	{
		if (FellowHuman == this)
		{
			continue;
		}

		if (IsThisActorVisible(Cast<AActor>(FellowHuman)))
		{
			if (FellowHuman->IsDead_Implementation())
			{
				DeadBodySeen = true;
				AddNewStimulus(EStimulusType::BodySighting, FellowHuman->GetVisionTargetLocation_Implementation(), FellowHuman);
				//DrawDebugLine(GetWorld(), EyeLocationMarker->GetComponentLocation(), FellowHuman->GetVisionTargetLocation_Implementation(), FColor::Orange);

			}
			else if(FellowHuman->IsStunned())
			{
				StunnedHumanSeen = true;
				AddNewStimulus(EStimulusType::TroubleSighting, FellowHuman->EyeLocationMarker->GetComponentLocation(), FellowHuman);
				//DrawDebugLine(GetWorld(), EyeLocationMarker->GetComponentLocation(), FellowHuman->GetVisionTargetLocation_Implementation(), FColor::Yellow);
			}
		}
	}

	// look for crawler
	if (IsThisActorVisible(GetAssassinationState()->Crawler))
	{
		float Distance = FVector::Distance(EyeLocationMarker->GetComponentLocation(), GetAssassinationState()->GetCrawlerRealLocation());
		if (Distance < GetEffectiveIdentifyCrawlerRange())
		{
			CrawlerSeen = true;
			AddNewStimulus(EStimulusType::CrawlerSighting, GetAssassinationState()->Crawler->GetVisionTargetLocation_Implementation(), GetAssassinationState()->Crawler);

		}
	}

	bCrawlerInSight = CrawlerSeen;
	bDeadBodyInSight = DeadBodySeen;
	bHumanInTroubleInSight = StunnedHumanSeen;

	if (bCrawlerInSight || bDeadBodyInSight || bHumanInTroubleInSight)
	{
		bLookTargetValid = true;
	}
	else if (Tension == GetTensionMin())
	{
		bLookTargetValid = false;
		LookTargetPriority = 0;
	}
	//if (!bLookTargetValid && Tension == GetTensionMin())
	//{
	//	LookTargetPriority = 0;
	//}
}

void AHuman::TryUpdateLookTarget(FVector NewTarget, int Priority)
{
	if (Priority > LookTargetPriority)
	{
		LookTarget = NewTarget;
		LookTargetPriority = Priority;
	}
	else if (Priority == LookTargetPriority)
	{
		if (FVector::DistSquared(GetActorLocation(), NewTarget) < FVector::DistSquared(GetActorLocation(), LookTarget))
		{
			LookTarget = NewTarget;
			LookTargetPriority = Priority;
		}
	}
}



bool AHuman::IsThisActorVisible(AActor* Target)
{
	if (!Target)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Tried to look at null"));
		return false;
	}

	// Get exact viewing target
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
	FVector TargetDirection = (TargetLocation - EyeLocationMarker->GetComponentLocation()).GetSafeNormal();
	float Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(TargetDirection, EyeLocationMarker->GetUpVector())));
	if (Angle > GetEffectiveFieldOfView())
	{
		return false;
	}

	ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocationMarker->GetComponentLocation(), TargetLocation, TraceChannel, CollisionParameters))
	{
		if (Hit.Actor.Get() == Target || Hit.Actor.Get()->GetParentActor() == Target)
		{
			return true;
		}
	}

	return false;
}


float AHuman::GetEffectiveIdentifyCrawlerRange()
{
	if (GetAssassinationState()->IsGlobalAlert())
		return AlertIdentifyCrawlerRange;
	else
		return IdentifyCrawlerRange;
}


float AHuman::GetEffectiveFieldOfView()
{
	if (GetAssassinationState()->IsGlobalAlert())
		return AlertFieldOfView;
	else
		return FieldOfView;
}


FVector AHuman::GetLookTarget()
{
	if (IsLookTargetValid())
	{
		return LookTarget;
	}
	else
	{
		return EyeLocationMarker->GetComponentLocation() + GetActorForwardVector();
	}
}

#pragma endregion Vision


#pragma region Visibility

FVector AHuman::GetVisionTargetLocation_Implementation()
{
	return VisionTargetMarker->GetComponentLocation();
}

#pragma endregion Visibility


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
		Tension = TensionThreshold;
	}

	CurrentHealth = fminf(CurrentHealth + Delta, MaxHealth);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::Printf(TEXT("%f damage! Current health %f"), Delta, CurrentHealth));
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

		ClearInnerFeeling();

		Execute_DeathNotice_BPEvent(this);

		//GetCom
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("UWAAA!<dead>"));
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
	bool CrawlerFelt = false;

	if (!IsStriking())
	{
		ActiveStrikeBox = FindActiveStrikeBox();
	}

	if (ActiveStrikeBox)
	{
		// check sensitivity
		if (ActiveStrikeBox->Sensitivity > 0)
		{
			CrawlerFelt = true;
			AddNewStimulus(EStimulusType::CrawlerFelt, ActiveStrikeBox->GetComponentLocation());
		}
	}

	bCrawlerOnSensitiveArea = CrawlerFelt;
}

UStrikeBox* AHuman::FindActiveStrikeBox()
{
	FName CrawlerBoneName = GetCrawlerBoneName();

	// Do not activate strikebox when crawler is on the other side of a wall!
	if (CrawlerBoneName == FName("None") && !bCrawlerInSight)
		return nullptr;

	for (auto & StrikeBox : StrikeBoxes)
	{
		if(StrikeBox->CrawlerInside && StrikeBox->IsBoneValidForDetection(CrawlerBoneName))
		{
			return StrikeBox;
		}
	}
	return nullptr;
}

void AHuman::CheckCrawlerOnBody()
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
				bCrawlerOnBody = true;
				return;
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, AActor::GetDebugName(Child));

	}
	bCrawlerOnBody = false;
	return;
}

FName AHuman::GetCrawlerBoneName()
{
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);
	for (auto & Child : ChildActors)
	{
		AMobileTargetActor* MTA = Cast<AMobileTargetActor>(Child);
		if (MTA)
		{
			if (MTA->MTAOwnerType == EMTAOwnerType::CrawlerBody)
			{
				FName CrawlerBoneName = MTA->GetAttachParentSocketName();
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, CrawlerBoneName.ToString());

				return CrawlerBoneName;
			}
		}

	}
	return FName("None");
}


void AHuman::BeginStrike()
{
	if (!GetAssassinationState())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Game state not founnd"));
		return;
	}


	StrikeTargetTracker->SetActorLocation(GetAssassinationState()->GetCrawlerRealLocation());
	if (ActiveStrikeBox->bDetectionDependsOnBone && ActiveStrikeBox->DetectionBoneNames.Contains(FName("None")))
	{
		StrikeTargetTracker->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		StrikeTargetTracker->AttachToComponent(ActiveStrikeBox, FAttachmentTransformRules::KeepWorldTransform);
	}

	if (!StrikeLimbMap.Contains(ActiveStrikeBox->LimbType))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Couldn't find the right limb to strike with!"));
		return;
	}
	ActiveStrikeLimb = StrikeLimbMap[ActiveStrikeBox->LimbType];
	ActiveStrikeLimb->BeginStrike();

	StrikeTimer = 0;

	bStrikeLockedIn = true;

	StrikeBegan_BPEvent();
}

void AHuman::ContinueStrike(float DeltaTime)
{
	//DrawDebugLine(GetWorld(), ActiveStrikeLimb->GetComponentLocation(), StrikeTargetTracker->GetActorLocation(), FColor::Red, false, -1, 0, 0.1f);
	//FCollisionQueryParams CollisionParams;
	//CollisionParams.AddIgnoredActor(this);
	//TArray<FHitResult> Hits;
	//GetWorld()->LineTraceMultiByChannel(Hits, ActiveStrikeLimb->GetComponentLocation(), StrikeTargetTracker->GetActorLocation(), ECC_GameTraceChannel7, CollisionParams);
	//for (auto & Hit : Hits)
	//{
	//	//if (ActiveStrikeLimb->IgnoredBones.Contains(Hit.BoneName))
	//	//	continue;
	//
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, AActor::GetDebugName(Hit.Actor.Get()) + FString("'s ") + Hit.Component.Get()->GetFName().ToString() + FString(" on the ") + Hit.BoneName.ToString());
	//
	//
	//	if (Cast<AWallCrawler>(Hit.Actor.Get()))
	//	{
	//		HumanGaitControl->MarkSpot(Hit.ImpactPoint, FColor::Yellow, 0.5);
	//	}
	//	else
	//	{
	//		StrikeTargetTracker->SetActorLocation(Hit.ImpactPoint);
	//		HumanGaitControl->MarkSpot(Hit.ImpactPoint, FColor::Red, 0.5);
	//	}
	//}
	

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

FVector AHuman::GetCurrentStrikeTarget()
{ 
	if (StrikeTargetTracker)
	{
		return StrikeTargetTracker->GetActorLocation();
	}
	else if (ActiveStrikeBox)
	{
		return ActiveStrikeBox->GetComponentLocation();
	}
	else
	{
		return FVector(0, 0, 0);
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

float AHuman::GetForwardMovement()
{
	return HumanGaitControl->GetForwardMovement();
}

float AHuman::GetRightMovement()
{
	return HumanGaitControl->GetRightMovement();
}

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


#pragma region Movement

void AHuman::TurnToFaceDirection(FVector Direction)
{
	HumanMovement->SetFaceDirection(Direction);
}

void AHuman::TurnToFaceVelocity()
{
	HumanMovement->SetFaceVelocity();
	//HumanMovement->SetFaceDirection(GetActorLocation() - GetHumanAI()->GetImmediateMoveDestination());
}

#pragma endregion Movement

#pragma region GlobalReferencing

AAssassinationGameState* AHuman::GetAssassinationState()
{
	//if(!AssassinationState)
	//	AssassinationState = Cast<AAssassinationGameState>(GetWorld()->GetGameState());
	//return AssassinationState;
	return Cast<AAssassinationGameState>(GetWorld()->GetGameState());
}


AHumanAIController* AHuman::GetHumanAI()
{
	return Cast<AHumanAIController>(GetController());
}


#pragma endregion GlobalReferencing