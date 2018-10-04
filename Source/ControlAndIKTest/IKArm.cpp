// Fill out your copyright notice in the Description page of Project Settings.

#include "IKArm.h"
#include "DrawDebugHelpers.h"

// Sets default values
AIKArm::AIKArm()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	IKRoot = CreateDefaultSubobject<USceneComponent>(TEXT("IKRoot"));
	RootComponent = IKRoot;

	IKPin = CreateDefaultSubobject<USceneComponent>(TEXT("IKPin"));
	IKPin->SetupAttachment(RootComponent);
	UnderTargetIKPin = CreateDefaultSubobject<USceneComponent>(TEXT("UnderTargetIKPin"));
	UnderTargetIKPin->SetupAttachment(RootComponent);

	UpperArm = CreateDefaultSubobject<USceneComponent>(TEXT("UpperArm"));
	UpperArm->SetupAttachment(RootComponent);
	LowerArm = CreateDefaultSubobject<USceneComponent>(TEXT("LowerArm"));
	LowerArm->SetupAttachment(UpperArm);
	

	HighTarget = CreateDefaultSubobject<USceneComponent>(TEXT("HighTarget"));
	HighTarget->SetupAttachment(RootComponent);
	GroundTarget = CreateDefaultSubobject<USceneComponent>(TEXT("GroundTarget"));
	GroundTarget->SetupAttachment(RootComponent);
	LowTarget = CreateDefaultSubobject<USceneComponent>(TEXT("LowTarget"));
	LowTarget->SetupAttachment(RootComponent);
	UnderTarget = CreateDefaultSubobject<USceneComponent>(TEXT("UnderTarget"));
	UnderTarget->SetupAttachment(RootComponent);

	RestTarget = CreateDefaultSubobject<USceneComponent>(TEXT("RestTarget"));
	RestTarget->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AIKArm::BeginPlay()
{
	Super::BeginPlay();
	LowerArm->SetRelativeLocation(FVector(UpperArmLength, 0, 0));
}

// Called every frame
void AIKArm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SmoothUpdateIKTarget();

	if (!AttemptSolveIKAndSetArmRotation())
	{
		NeedNewTarget = true;
	}

	if (NeedNewTarget)
	{
		ProbeForIKTarget(MovementDelta);
	}
	if (NeedNewTarget)
	{
		ProbeForIKTarget();
	}

	MovementDelta = FVector(0,0,0);
	
	
	if (SHOW_DEBUG_INFO)
	{
		MarkSpot(IKTargetIntermediate, FColor::White);
		MarkSpot(IKTargetFinal, FColor::Red);
	}
}

void AIKArm::SmoothUpdateIKTarget()
{
	// Simple and bad, should improve so that the target moves up a little between points
	float t = 0.35;
	IKTargetIntermediate = IKTargetIntermediate * (1 - t) + IKTargetFinal * t;

	//IKTargetTransitionTimer += GetWorld()->GetDeltaSeconds();
	//if (IKTargetTransitionTimer < IKTargetTransitionDuration)
	//{
	//	float t = IKTargetTransitionTimer / IKTargetTransitionDuration;
	//	float alpha = FMath::SmoothStep(0.f, 1.f, t);
	//	IKTargetIntermediate = IKTargetIntermediate * (1 - alpha) + IKTargetFinal * alpha;
	//}
	//else
	//{
	//	IKTargetIntermediate = IKTargetFinal;
	//}
}


bool AIKArm::AttemptSolveIKAndSetArmRotation()
{
	bool SolutionInvalid = false;

	FQuat FinalFrameRotation = GetIKFrameRotationMatrix(IKTargetFinal).ToQuat(); // Rotates (World -> IKFrame)
	FQuat FrameRotation = GetIKFrameRotationMatrix(IKTargetIntermediate).ToQuat(); // Rotates (World -> IKFrame)
	FQuat LocalRotation = IKRoot->GetComponentQuat().Inverse() * FrameRotation; // Rotates (IKRoot ->IKFrame)
	

	// Test 1: Is this IKFrame at too steep an angle?
	float ForwardFrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FinalFrameRotation.GetForwardVector(), IKRoot->GetComponentQuat().GetForwardVector()).GetAngle());
	float DownFrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FinalFrameRotation.GetForwardVector(), -IKRoot->GetComponentQuat().GetUpVector()).GetAngle());
	if (ForwardFrameAngle > MaximumAngle && DownFrameAngle > MaximumAngleUnderneath)
	{
		if (SHOW_DEBUG_INFO)
		{
			MarkSpot(IKTargetFinal, FColor::Blue);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("ANGLE FAILURE!"));
		}

		SolutionInvalid = true;
	}

	// Solve IK in 2D from here
	float UpperArmAngle;
	float LowerArmAngle;
	float Distance = FVector::Distance(IKRoot->GetComponentLocation(), IKTargetIntermediate);
	if (Distance >= UpperArmLength + LowerArmLength)
	{
		UpperArmAngle = 0;
		LowerArmAngle = 0;
		if (SHOW_DEBUG_INFO)
		{
			MarkSpot(IKTargetIntermediate, FColor::Cyan);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("DISTANCE FAILURE!"));
		}

		SolutionInvalid = true;
	}
	else
	{
		UpperArmAngle = FindAngleA(LowerArmLength, Distance, UpperArmLength);
		LowerArmAngle = 180 + FindAngleA(Distance, LowerArmLength, UpperArmLength);
	}

	// Align the limb to the IK Frame, and set the pitch for each bone
	UpperArm->SetRelativeRotation(LocalRotation * FRotator(UpperArmAngle, 0, 0).Quaternion());
	LowerArm->SetRelativeRotation(FRotator(LowerArmAngle, 0, 0));

	if (IsLimbColliding())
	{
		if (SHOW_DEBUG_INFO)
		{
			MarkSpot(IKTargetIntermediate, FColor::Magenta);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("COLLISION FAILURE!"));
		}

		//SolutionInvalid = true;
	}
	
	
	return !SolutionInvalid;
}

FMatrix AIKArm::GetIKFrameRotationMatrix(FVector IKTarget)
{
	/* We will use Forward and Up vectors to get the rotation matrix 
	*  The Up vector depends on whether the IKTarget is in front of, or behind the IKRoot.
	*  We can figure this (RelativeForward) out by:
	*  - getting the difference between IKRoot and IKTarget (as a vector),
	*  - and rotating it by the inverse of the IKRoot's rotation. 
	*  Then we can just look at this (RelativeForward) vector's X component to see whether it's >0.
	*  IKPinBlendRange allows us to interpolate smoothly between the two IKPin locations.
	*/
	FVector Forward = IKTarget - IKRoot->GetComponentLocation();
	
	// When IKTarget's relative forward is below this value, the IKPins will blend together.
	const float IKPinBlendRange = 50.f;
	FVector RelativeForward = IKRoot->GetComponentQuat().Inverse() * Forward;
	float IKPinBlend = fmaxf(0, fminf(1, RelativeForward.X / IKPinBlendRange));
	FVector Up = (IKPinBlend * IKPin->GetComponentLocation() + (1 - IKPinBlend) * UnderTargetIKPin->GetComponentLocation()) - IKRoot->GetComponentLocation();

	return FRotationMatrix::MakeFromXZ(Forward, Up);
}

float AIKArm::FindAngleA(float a, float b, float c)
{
	return FMath::RadiansToDegrees(
		acosf(
			fmaxf(-1,
				fminf(1,
					(b*b + c * c - a * a) / (2 * b*c)
				)
			)
		)
	);
}


void AIKArm::SetIKTarget(FVector NewTarget)
{
	IKTargetFinal = NewTarget;
	IKTargetInTransit = true;
	IKTargetTransitionTimer = 0;
}

void AIKArm::ProbeForIKTarget(FVector DirectionModifier)
{
	/* 
	* Shoot rays to find a new IKTarget
	* If all rays fail, set the IKTarget to the RestTarget position
	*/
	UsingRestTarget = false; // Unless we miss all rays, we won't use the rest target
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActor(this->GetParentActor());
	CollisionParameters.bTraceComplex = true;
	FHitResult Hit;

	// Root -> HighTarget
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		IKRoot->GetComponentLocation(),
		HighTarget->GetComponentLocation() + DirectionModifier * DirectionModifierStrength,
		ECC_Visibility,
		CollisionParameters))
	{
		SetIKTarget(Hit.ImpactPoint);
		if (AttemptSolveIKAndSetArmRotation())
		{
			if (SHOW_DEBUG_INFO)
				MarkLine(IKRoot->GetComponentLocation(), Hit.ImpactPoint, FColor::Orange, 1);
			NeedNewTarget = false;
			return;
		}
		else if (SHOW_DEBUG_INFO)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("1 -- Root->HighTarget"));
		}
	}

	// Root -> GroundTarget
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		IKRoot->GetComponentLocation(),
		GroundTarget->GetComponentLocation() + DirectionModifier * DirectionModifierStrength,
		ECC_Visibility,
		CollisionParameters))
	{
		SetIKTarget(Hit.ImpactPoint);
		if (AttemptSolveIKAndSetArmRotation())
		{
			if (SHOW_DEBUG_INFO)
				MarkLine(IKRoot->GetComponentLocation(), Hit.ImpactPoint, FColor::Orange, 1);
			NeedNewTarget = false;
			return;
		}
		else if (SHOW_DEBUG_INFO)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("2 -- Root->GroudnTarget"));
		}
	}

	// HighTarget -> LowTarget
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		HighTarget->GetComponentLocation(),
		LowTarget->GetComponentLocation() + DirectionModifier * DirectionModifierStrength,
		ECC_Visibility,
		CollisionParameters))
	{
		SetIKTarget(Hit.ImpactPoint);
		if (AttemptSolveIKAndSetArmRotation())
		{
			if (SHOW_DEBUG_INFO)
				MarkLine(HighTarget->GetComponentLocation(), Hit.ImpactPoint, FColor::Orange, 1);
			NeedNewTarget = false;
			return;
		}
		else if (SHOW_DEBUG_INFO)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("3 -- HighTarget->LowTarget"));
		}
	}

	// GroundTarget -> UnderTarget
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		GroundTarget->GetComponentLocation(),
		UnderTarget->GetComponentLocation() + DirectionModifier * DirectionModifierStrength *0.5,
		ECC_Visibility,
		CollisionParameters))
	{
		SetIKTarget(Hit.ImpactPoint);
		if (AttemptSolveIKAndSetArmRotation())
		{
			if (SHOW_DEBUG_INFO)
				MarkLine(GroundTarget->GetComponentLocation(), Hit.ImpactPoint, FColor::Orange, 1);
			NeedNewTarget = false;
			return;
		}
		else if (SHOW_DEBUG_INFO)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("4 -- GroundTarget->UnderTarget"));
		}
	}

	// LowTarget -> UnderTarget
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		LowTarget->GetComponentLocation(),
		UnderTarget->GetComponentLocation() + DirectionModifier * DirectionModifierStrength *0.5,
		ECC_Visibility,
		CollisionParameters))
	{
		SetIKTarget(Hit.ImpactPoint);
		if (AttemptSolveIKAndSetArmRotation())
		{
			if (SHOW_DEBUG_INFO)
				MarkLine(LowTarget->GetComponentLocation(), Hit.ImpactPoint, FColor::Orange, 1);
			NeedNewTarget = false;
			return;
		}
		else if (SHOW_DEBUG_INFO)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("5 -- LowTarget->UnderTarget"));
		}
	}
	
	{
		SetIKTarget(RestTarget->GetComponentLocation() - FVector(0, 0, RestTargetSlack)); //TODO make this better somehow (rest target + gravity*slack)
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = true;
			return;
		}
	}

}



bool AIKArm::IsLimbColliding()
{
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActor(this->GetParentActor());
	CollisionParameters.bTraceComplex = true;
	FHitResult Hit;
	// From UpperArm to LowerArm
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		IKRoot->GetComponentLocation(),
		LowerArm->GetComponentLocation(),
		ECC_Visibility,
		CollisionParameters))
	{	
		if(SHOW_DEBUG_INFO)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, AActor::GetDebugName(Hit.Actor.Get()));

		return true;
	}
	// From LowerArm to IKTarget ALMOST (We don't want to actually hit the target!)
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		LowerArm->GetComponentLocation(),
		LowerArm->GetComponentLocation() + LowerArm->GetForwardVector() * LowerArmLength * 0.6f, // ALMOST
		ECC_Visibility,
		CollisionParameters))
	{
		if (SHOW_DEBUG_INFO)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, AActor::GetDebugName(Hit.Actor.Get()));

		return true;
	}

	return false;
}

void AIKArm::DebugDrawArm()
{
	FColor LegColor;
	if (!TargetReachable)
	{
		LegColor = FColor::Red;
	}
	else
	{
		LegColor = FColor::Black;
	}

	
	// Upper Arm
	DrawDebugLine(
		GetWorld(),
		IKRoot->GetComponentLocation(),
		IKRoot->GetComponentLocation() + UpperArm->GetForwardVector() * UpperArmLength,
		LegColor, false, -1, 0, 6.f
	);

	// Lower Arm
	DrawDebugLine(
		GetWorld(),
		LowerArm->GetComponentLocation(),
		LowerArm->GetComponentLocation() + LowerArm->GetForwardVector() * LowerArmLength,
		LegColor, false, -1, 0, 6.f
	);

	// Direct Line from root to target
	DrawDebugLine(
		GetWorld(),
		IKRoot->GetComponentLocation(),
		IKTargetIntermediate,
		FColor::White, false, -1, 0, 1.f
	);

		
}

void AIKArm::MarkSpot(FVector Point, FColor Colour)
{
	float length = 10.f;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, true, -1, 0, 1.f);
			}
		}
	}
}

void AIKArm::MarkLine(FVector Start, FVector End, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	DrawDebugLine(GetWorld(), Start, End, Colour, IsPersistant, Duration, 0, 1.f);
}
