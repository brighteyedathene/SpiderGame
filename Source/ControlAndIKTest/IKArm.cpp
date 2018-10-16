// Fill out your copyright notice in the Description page of Project Settings.

#include "IKArm.h"
#include "DrawDebugHelpers.h"



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



void AIKArm::BeginPlay()
{
	Super::BeginPlay();
	LowerArm->SetRelativeLocation(FVector(UpperArmLength, 0, 0));
	
	IKProbes.Add(FIKProbe(IKRoot, HighTarget));
	IKProbes.Add(FIKProbe(IKRoot, GroundTarget));
	IKProbes.Add(FIKProbe(HighTarget, LowTarget));
	IKProbes.Add(FIKProbe(GroundTarget, UnderTarget));
	IKProbes.Add(FIKProbe(LowTarget, UnderTarget));
}



void AIKArm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SmoothUpdateIKTarget();

	if (!AttemptSolveIK())
	{
		m_bNeedNewTarget = true;
	}

	if (m_bNeedNewTarget)
	{
		ProbeForIKTarget(m_bMovementDelta.GetSafeNormal() * DirectionModifierStrength);
	}
	if (m_bNeedNewTarget)
	{
		ProbeForIKTarget();
	}


	UpperArm->SetRelativeRotation(m_IKFrameRotation * FRotator(m_IKUpperArmAngle, 0, 0).Quaternion());
	LowerArm->SetRelativeRotation(FRotator(m_IKLowerArmAngle, 0, 0));

	m_bMovementDelta = FVector(0,0,0);
	
	
	if (SHOW_DEBUG_INFO)
	{
		//MarkSpot(IKTargetIntermediate, FColor::White);
		//MarkSpot(IKTargetFinal, FColor::Red);
	}
}



void AIKArm::SmoothUpdateIKTarget()
{
	// Simple and bad, should improve so that the target moves up a little between points
	float t = 0.4;
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



void AIKArm::ProbeForIKTarget(FVector DirectionModifier)
{
	/*
	* Shoot rays to find a new IKTarget
	* If all rays fail, set the IKTarget to the RestTarget position
	*/
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActor(this->GetParentActor());
	CollisionParameters.bTraceComplex = true;
	FHitResult Hit;

	for (auto IKProbe : IKProbes)
	{
		if (GetWorld()->LineTraceSingleByChannel(
			Hit,
			IKProbe.GetStart(),
			IKProbe.GetModifiedRayEnd(DirectionModifier),
			ECC_Visibility,
			CollisionParameters))
		{
			SetIKTarget(Hit.ImpactPoint);
			if (AttemptSolveIK())
			{
				m_bNeedNewTarget = false;
				return;
			}

		}
	}

	// As a fallback, put the IKTarget in the rest position
	SetIKTarget(RestTarget->GetComponentLocation() - FVector(0, 0, RestTargetSlack)); //TODO make this better somehow (rest target + gravity*slack)
	if (AttemptSolveIK())
	{
		m_bNeedNewTarget = true;
		return;
	}

}



bool AIKArm::AttemptSolveIK()
{
	bool SolutionInvalid = false;

	FQuat FinalFrameRotation = GetIKFrameRotationMatrix(IKTargetFinal).ToQuat(); // Rotates (World -> IKFrame)
	FQuat FrameRotation = GetIKFrameRotationMatrix(IKTargetIntermediate).ToQuat(); // Rotates (World -> IKFrame)
	FQuat LocalRotation = IKRoot->GetComponentQuat().Inverse() * FrameRotation; // Rotates (IKRoot ->IKFrame)
	

	// First, check the final IK target
	{
		// Test 1: Is this IKFrame at too steep an angle?
		float ForwardFrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FinalFrameRotation.GetForwardVector(), IKRoot->GetComponentQuat().GetForwardVector()).GetAngle());
		float DownFrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FinalFrameRotation.GetForwardVector(), -IKRoot->GetComponentQuat().GetUpVector()).GetAngle());
		if (ForwardFrameAngle > MaximumAngle && DownFrameAngle > MaximumAngleUnderneath)
		{
			if (SHOW_DEBUG_INFO)
			{
				MarkSpot(IKTargetFinal, FColor::Blue);
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("ANGLE FAILURE!"));
			}

			SolutionInvalid = true;
		}
		// Test 2: Is the final target close enough?
		float FinalDistance = FVector::Distance(IKRoot->GetComponentLocation(), IKTargetFinal);
		if (FinalDistance >= UpperArmLength + LowerArmLength)
		{
			SolutionInvalid = true;
		}
		
		float FinalUpperArmAngle = FindAngleA(LowerArmLength, FinalDistance, UpperArmLength);
		float FinalLowerArmAngle = 180 + FindAngleA(FinalDistance, LowerArmLength, UpperArmLength);
		
		if (DoesThisSolutionCollide(IKRoot->GetComponentQuat().Inverse() * FinalFrameRotation, FinalUpperArmAngle, FinalLowerArmAngle))
		{

			SolutionInvalid = true;
		}
	}

	// Solve IK for the intermediate target
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
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("DISTANCE FAILURE!"));
		}

		SolutionInvalid = true;
	}
	else
	{
		UpperArmAngle = FindAngleA(LowerArmLength, Distance, UpperArmLength);
		LowerArmAngle = 180 + FindAngleA(Distance, LowerArmLength, UpperArmLength);
	}


	m_IKFrameRotation = LocalRotation;
	m_IKUpperArmAngle = UpperArmAngle;
	m_IKLowerArmAngle = LowerArmAngle;

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
	m_IKTargetTransitionTimer = 0;
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
		LowerArm->GetComponentLocation() + LowerArm->GetForwardVector() * LowerArmLength * 0.8f, // ALMOST
		ECC_Visibility,
		CollisionParameters))
	{
		if (SHOW_DEBUG_INFO)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, AActor::GetDebugName(Hit.Actor.Get()));

		return true;
	}

	return false;
}



bool AIKArm::DoesThisSolutionCollide(FQuat FrameRotation, float UpperArmAngle, float LowerArmAngle)
{
	// Get the points!
	// Base: IKRoot->GetComponentLocation()
	// Mid: RootRotation * LocalRotation * FRotator(UpperAngle, 0, 0) 
	// End: IKTargetFinal

	FQuat UpperArmRotation = IKRoot->GetComponentQuat() * (FrameRotation * FRotator(UpperArmAngle, 0, 0).Quaternion());
	FQuat LowerArmRotation = UpperArmRotation * FRotator(LowerArmAngle, 0, 0).Quaternion();

	FVector JointPosition = IKRoot->GetComponentLocation() + UpperArmRotation.GetForwardVector() * UpperArmLength;
	FVector EndPosition = JointPosition + LowerArmRotation.GetForwardVector() * LowerArmLength * 0.8; // Don't quite hit the target.


	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActor(this->GetParentActor());
	CollisionParameters.bTraceComplex = true;
	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		IKRoot->GetComponentLocation(),
		JointPosition,
		ECC_Visibility,
		CollisionParameters))
	{
		if (SHOW_DEBUG_INFO)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, AActor::GetDebugName(Hit.Actor.Get()));

		return true;
	}
	// From LowerArm to IKTarget ALMOST (We don't want to actually hit the target!) see EndPosition
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		JointPosition,
		EndPosition,
		ECC_Visibility,
		CollisionParameters))
	{
		if (SHOW_DEBUG_INFO)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, AActor::GetDebugName(Hit.Actor.Get()));

		return true;
	}

	return false;
}



void AIKArm::ReceiveGaitInput(FVector MovementDelta)
{
	m_bNeedNewTarget = true;
	m_bMovementDelta = MovementDelta;
}


void AIKArm::DebugDrawArm()
{
	FColor LegColor = FColor::Red;

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
