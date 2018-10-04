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
	//if(DEBUG_SHOW_ANGLE)
	//	DebugDrawArm();
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

	FQuat FrameRotation = GetIKFrameRotationMatrix().ToQuat(); // Rotates (World -> IKFrame)
	FQuat InverseFrameRotation = FrameRotation.Inverse(); // Rotates (IKFrame -> World)
	FQuat LocalRotation = IKRoot->GetComponentQuat().Inverse() * FrameRotation; // Rotates (IKRoot ->IKFrame)
	
	float ForwardFrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FrameRotation.GetForwardVector(), IKRoot->GetComponentQuat().GetForwardVector()).GetAngle());
	float DownFrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FrameRotation.GetForwardVector(), -IKRoot->GetComponentQuat().GetUpVector()).GetAngle());
	if (ForwardFrameAngle > MaximumAngle && DownFrameAngle > MaximumAngleUnderneath)
	{
		//if (DEBUG_SHOW_ANGLE)
		//{
		//	MarkSpot(IKTargetIntermediate, FColor::Blue);
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("ForwardFrameAngle = %f"), ForwardFrameAngle));
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("DownFrameAngle = %f"), DownFrameAngle));
		//}

		SolutionInvalid = true;
	}


	// The translated points for the equation
	FVector Root, Pin, Target;
	
	// Translate all 3 points to origin and rotate all 3 points to line up with x-axis
	FVector ToOrigin = -IKRoot->GetComponentLocation();
	Root = InverseFrameRotation * (IKRoot->GetComponentLocation() + ToOrigin);
	Pin = InverseFrameRotation * (IKPin->GetComponentLocation() + ToOrigin);
	Target = InverseFrameRotation * (IKTargetIntermediate + ToOrigin);

	// Solve IK in 2D from here
	float UpperArmAngle;
	float LowerArmAngle;
	float Distance = FVector::Distance(Root, Target);
	if (Distance >= UpperArmLength + LowerArmLength)
	{
		UpperArmAngle = 0;
		LowerArmAngle = 0;
		if (DEBUG_SHOW_ANGLE)
			MarkSpot(IKTargetIntermediate, FColor::Cyan);

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
		if (DEBUG_SHOW_ANGLE)
			MarkSpot(IKTargetIntermediate, FColor::Magenta);

		//SolutionInvalid = true;
	}
	
	
	return !SolutionInvalid;
}

FMatrix AIKArm::GetIKFrameRotationMatrix()
{
	const float IKPinBlendRange = 50.f;
	
	FVector Forward = IKTargetIntermediate - IKRoot->GetComponentLocation();
	
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
		UsingUnderTargetIKPin = false;
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = false;
			return;
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
		UsingUnderTargetIKPin = false;
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = false;
			return;
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
		UsingUnderTargetIKPin = false;
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = false;
			return;
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
		UsingUnderTargetIKPin = true;
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = false;
			return;
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
		UsingUnderTargetIKPin = true;
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = false;
			return;
		}
	}
	
	{
		SetIKTarget(RestTarget->GetComponentLocation() - FVector(0, 0, -RestTargetSlack)); //TODO make this better somehow (rest target + gravity*slack)
		UsingRestTarget = true;
		if (AttemptSolveIKAndSetArmRotation())
		{
			NeedNewTarget = true;
			return;
		}
	}

}

bool AIKArm::IsIKTargetUnderNeath()
{
	//FQuat InvertRotation = IKRoot->GetComponentQuat().Inverse();
	//FVector RelativeIKTarget = InvertRotation * (IKTargetIntermediate - IKRoot->GetComponentLocation()) ;
	//return RelativeIKTarget.X < -1 && RelativeIKTarget.Z < -1;
	
	return UsingUnderTargetIKPin;
}

bool AIKArm::IsLimbColliding()
{
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
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
		return true;
	}
	// From LowerArm to IKTarget ALMOST (We don't want to actually hit the target!)
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		LowerArm->GetComponentLocation(),
		LowerArm->GetComponentLocation() + LowerArm->GetForwardVector() * LowerArmLength * 0.9f,
		ECC_Visibility,
		CollisionParameters))
	{
		return true;
	}

	return false;
}

void AIKArm::DebugDrawArm()
{
	FColor LegColor;
	if (IsIKTargetUnderNeath())
	{
		LegColor = FColor::Orange;
	}
	else if (!TargetReachable)
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

