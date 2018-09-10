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

	UpperArm = CreateDefaultSubobject<USceneComponent>(TEXT("UpperArm"));
	UpperArm->SetupAttachment(RootComponent);
	LowerArm = CreateDefaultSubobject<USceneComponent>(TEXT("LowerArm"));
	LowerArm->SetupAttachment(UpperArm);
	

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

	//LowerArm->SetRelativeLocation(FVector(UpperArmLength, 0, 0));


	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.bTraceComplex = true;
	FHitResult Hit;
	if (!TargetReachable &&
		GetWorld()->LineTraceSingleByChannel(
			Hit,
			IKRoot->GetComponentLocation(),
			IKRoot->GetComponentLocation() + IKRoot->GetForwardVector() * 300 - IKRoot->GetUpVector() * 200,
			ECC_Visibility,
			CollisionParameters)
		)
	{
		IKTarget = Hit.ImpactPoint;
	}


	UpdateIK();

	if(DEBUG_SHOW_ANGLE)
		DebugDrawArm();
}

void AIKArm::UpdateIK()
{
	TargetReachable = true;

	FQuat FrameRotation = GetRotationMatrix().ToQuat(); // Rotates (World -> IKFrame_
	FQuat InverseFrameRotation = FrameRotation.Inverse(); // Rotates (IKFrame -> World)
	FQuat LocalRotation = IKRoot->GetComponentQuat().Inverse() * FrameRotation; // Rotates (IKRoot ->IKFrame)
	
	float FrameAngle = FMath::RadiansToDegrees(FQuat::FindBetween(FrameRotation.GetForwardVector(), IKRoot->GetComponentQuat().GetForwardVector()).GetAngle());
	if (FrameAngle > MaximumAngle)
		TargetReachable = false;

	// The translated points for the equation
	FVector Root, Pin, Target;
	
	// Translate all 3 points to origin and Rotate all 3 points to line up with x-axis
	FVector ToOrigin = -IKRoot->GetComponentLocation();
	Root = InverseFrameRotation * (IKRoot->GetComponentLocation() + ToOrigin);
	Pin = InverseFrameRotation * (IKPin->GetComponentLocation() + ToOrigin);
	Target = InverseFrameRotation * (IKTarget + ToOrigin);

	MarkSpot(Root, FColor::Red);
	MarkSpot(Pin, FColor::Yellow);
	MarkSpot(Target, FColor::Green);

	//FVector DRoot = RotationQuat * (Root - ToOrigin);
	//FVector DPin = RotationQuat * (Pin - ToOrigin);
	//FVector DTarget = RotationQuat * (Target - ToOrigin);
	//
	//MarkSpot(DRoot, FColor::Magenta);
	//MarkSpot(DPin, FColor::Blue);
	//MarkSpot(DTarget, FColor::Cyan);

	// Solve IK in 2D from here
	float UpperArmAngle;
	float LowerArmAngle;
	float Distance = FVector::Distance(Root, Target);
	if (Distance >= UpperArmLength + LowerArmLength)
	{
		UpperArmAngle = 0;
		LowerArmAngle = 0;
		TargetReachable = false;
	}
	else
	{
		UpperArmAngle = FindAngleA(LowerArmLength, Distance, UpperArmLength);
		LowerArmAngle = 180 + FindAngleA(Distance, LowerArmLength, UpperArmLength);
	}

	// Align the limb to the IK Frame, and set the pitch for each bone
	UpperArm->SetRelativeRotation(LocalRotation * FRotator(UpperArmAngle, 0, 0).Quaternion());
	LowerArm->SetRelativeRotation(FRotator(LowerArmAngle, 0, 0));

}


void AIKArm::SetIKTarget(FVector NewTarget)
{
	IKTarget = NewTarget;
}

FMatrix AIKArm::GetRotationMatrix()
{
	FVector Forward = IKTarget - IKRoot->GetComponentLocation();
	FVector Up = IKPin->GetComponentLocation() - IKRoot->GetComponentLocation();

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

void AIKArm::DebugDrawArm()
{
	if (TargetReachable)
	{
		// Upper Arm
		DrawDebugLine(
			GetWorld(),
			IKRoot->GetComponentLocation(),
			IKRoot->GetComponentLocation() + UpperArm->GetForwardVector() * UpperArmLength,
			FColor::Black, false, -1, 0, 6.f
		);

		// Lower Arm
		DrawDebugLine(
			GetWorld(),
			LowerArm->GetComponentLocation(),
			LowerArm->GetComponentLocation() + LowerArm->GetForwardVector() * LowerArmLength,
			FColor::Black, false, -1, 0, 6.f
		);

		// Direct Line from root to target
		//DrawDebugLine(
		//	GetWorld(),
		//	IKRoot->GetComponentLocation(),
		//	IKTarget,
		//	FColor::White, false, -1, 0, 1.f
		//);
	}
	else
	{
		//// Upper Arm
		//DrawDebugLine(
		//	GetWorld(),
		//	IKRoot->GetComponentLocation(),
		//	IKRoot->GetComponentLocation() + UpperArm->GetForwardVector() * UpperArmLength,
		//	FColor::Red, false, -1, 0, 1.f
		//);
		//
		//// Lower Arm
		//DrawDebugLine(
		//	GetWorld(),
		//	LowerArm->GetComponentLocation(),
		//	LowerArm->GetComponentLocation() + LowerArm->GetForwardVector() * LowerArmLength,
		//	FColor::Red, false, -1, 0, 1.f
		//);

		// Direct Line from root to target
		DrawDebugLine(
			GetWorld(),
			IKRoot->GetComponentLocation(),
			IKTarget,
			FColor::Red, false, -1, 0, 1.f
		);
	}
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
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, false, -1, 0, 1.f);
			}
		}
	}
}

