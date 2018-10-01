// Fill out your copyright notice in the Description page of Project Settings.

#include "WallCrawler.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"



// Just keeping this here to c+p 
//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("Intensity = %f"), MovementIntensity));


// Sets default values
AWallCrawler::AWallCrawler()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MySphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
	RootComponent = MySphereComponent;

	MySphereComponent->InitSphereRadius(ColliderSize);
	MySphereComponent->SetCollisionProfileName(TEXT("Pawn"));
	
	CrawlerMovement = CreateDefaultSubobject<UCrawlerMovement>("CrawlerMovement");
	CrawlerGaitControl = CreateDefaultSubobject<UCrawlerGaitControl>("CrawlerGaitControl");

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 

	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.bTraceComplex = true;
	TraceChannel = ECC_Visibility;

}

// Called to bind functionality to input
void AWallCrawler::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWallCrawler::CollectForwardInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWallCrawler::CollectRightInput);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	// turnrate isn't implemented yet :p
	PlayerInputComponent->BindAxis("Turn", this, &AWallCrawler::CollectYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWallCrawler::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AWallCrawler::CollectPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWallCrawler::LookUpAtRate);

}

// Called when the game starts or when spawned
void AWallCrawler::BeginPlay()
{
	Super::BeginPlay();
	MySphereComponent->SetSphereRadius(ColliderSize);


	CrawlerState = ECrawlerState::Crawling;
}

// Called every frame
void AWallCrawler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rotation control...
	LocalPitch += InputPitch;
	LocalYaw += InputYaw;
	if (LocalPitch > 89.0f)
		LocalPitch = 89.0f;
	if (LocalPitch < -89.0f)
		LocalPitch = -89.0f;

	if (LocalYaw > 360.0f)
		LocalYaw -= 360.0f;
	if (LocalYaw < 0.0f)
		LocalYaw += 360.0f;

	FRotator CameraRotation = FRotator(-LocalPitch, InputYaw*2.5, 0);
	CameraBoom->SetRelativeRotation(CameraRotation);

	// Movement control...
	// - This involves calculating camera forward/right 
	// - and projecting it onto the current surface (given by the RootComponent orientation or world up)
	const FRotator Rotation = CameraBoom->GetComponentRotation();
	const FVector CameraForward = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

	FVector FDirection = ProjectToPlane(CameraForward, RootComponent->GetUpVector()).GetSafeNormal();
	FVector RDirection = ProjectToPlane(CameraRight, RootComponent->GetUpVector()).GetSafeNormal();

	float MovementIntensity = fmaxf(fabsf(InputForward), fabsf(InputRight));
	FVector MovementDirection = InputForward * FDirection + InputRight * RDirection;
	MovementDirection.Normalize();


	// Handle the crawling behaviour
	switch(CrawlerState)
	{

	case ECrawlerState::Falling:
		
		// try to cling

		// else do nothing (continue to fall)
		
		break;

	case ECrawlerState::Crawling:

		FVector AverageLocation, AverageNormal;
		float SuggestedClimbFactor;
		int HitCount;
		if (ExploreEnvironmentWithRays(&AverageLocation, &AverageNormal, &HitCount, &SuggestedClimbFactor, MovementDirection, 5))
		{
			SetLatchPoint(AverageLocation, AverageNormal);
		}
		else
		{
			// Keep the old latch point
		}

		ClingToPoint(LatchPoint, LatchNormal);

		AddMovementInput(RootComponent->GetUpVector(), SuggestedClimbFactor * MovementSpeed * GetWorld()->GetDeltaSeconds());
		AddMovementInput(MovementDirection, MovementIntensity * MovementSpeed * GetWorld()->GetDeltaSeconds());

		CrawlerGaitControl->UpdateGait(CrawlerMovement->GetVelocity());
	}

	// Clear all Inputs 
	FlushInput();
}


bool AWallCrawler::ExploreEnvironmentWithRays(
	FVector* pAvgLocation, 
	FVector* pAvgNormal, 
	int* pHitCount, 
	float* pSuggestedClimbFactor,
	FVector MovementDirection, 
	int RaysPerAxis)
{
	// Increment x, y, z by this delta during the loop
	// so that we shoot the right number of rays per axis
	float delta = 2.f / (RaysPerAxis - 1);

	const FVector O = RootComponent->GetComponentLocation();
	const FVector F = FVector::ForwardVector;
	const FVector R = FVector::RightVector;
	const FVector U = FVector::UpVector;  
	
	// We write to these values in the following loop
	int HitCountSum = 0;
	FVector NormalSum = FVector(0, 0, 0);
	FVector LocationSum = FVector(0, 0, 0);
	float MaxClimb = 0;
	FHitResult Hit;

	// Probe for hits, accumulate normal and location sums, calculate climb value
	for (float x = -1.f; x <= 1.f; x += delta)
	{
		for (float y = -1.f; y <= 1.f; y += delta)
		{
			for (float z = -1.f; z <= 1.f; z += delta)
			{
				FVector Ray = (F * x + R * y + U * z).GetSafeNormal();
				if (GetWorld()->LineTraceSingleByChannel(Hit, O, O + Ray * SurfaceGroundRayLength, TraceChannel, CollisionParameters))
				{
					LocationSum += Hit.ImpactPoint;
					NormalSum -= Ray; // subtract, because we want the opposing direction to the ray
					HitCountSum++;

					// Does this ray conflict with the movement direction enough to warrant a climb value?
					// This is determined by measuring the dot product of the ray and 3 other vectors:
					// - MovementDirection, 
					// - Up (relative to the model), 
					// - Right (relative to MovementDirection)
					// The climb value is maximised when the ray directly opposes movement
					const float MoveDotRay = FVector::DotProduct(MovementDirection, -Ray);
					const float UpDotRay = FVector::DotProduct(RootComponent->GetUpVector(), -Ray);
					const float RightDotRay = FVector::DotProduct(FVector::CrossProduct(MovementDirection, RootComponent->GetUpVector()), -Ray);
					const float VertThreshold = 0.4;
					const float HoriThreshold = 0.4;
					if (MoveDotRay < 0 && fabsf(UpDotRay) < VertThreshold && fabsf(RightDotRay) < HoriThreshold)
					{
						MaxClimb = fmaxf(MaxClimb, fabsf(MoveDotRay) - fabsf(UpDotRay));
					}
				}
			}
		}
	}

	// If there were any hits, we can write something to our pointers
	if (HitCountSum > 0)
	{
		*pAvgLocation = LocationSum / HitCountSum;
		*pAvgNormal = NormalSum / HitCountSum;
		*pHitCount = HitCountSum;
		*pSuggestedClimbFactor = MaxClimb;
		return true;
	}
	else
	{
		return false;
	}
}



void AWallCrawler::ClingToPoint(FVector EndLocation, FVector EndNormal)
{
	// Try to move towards a point above EndLocation while rotating in line with EndNormal
	// Uses AddMovementInput(..) so that collision and velocity limiting can be applied
	// WARNING: If the point can't be reached within SurfaceIdealDistanceThreshold, then you might get stuck!
	const FVector TargetLocation = EndLocation + EndNormal * SurfaceIdealDistance;
	const float Distance = FVector::Distance(TargetLocation, RootComponent->GetComponentLocation());
	float Force = fminf(MovementSpeed, Distance);
	Force = fmaxf(0, Force - SurfaceIdealDistanceThreshold);
	if (Force > 0)
	{
		AddMovementInput(TargetLocation - RootComponent->GetComponentLocation(), Force * GetWorld()->GetDeltaSeconds());
	}
	RotateTowardsNormal(EndNormal, RotationCorrectionAlpha);
}



void AWallCrawler::RotateTowardsNormal(FVector Normal, float t)
{
	// We will calculate a forward vector based on the model rotation, the normal and the camera
	const FVector CamForward = CameraBoom->GetComponentRotation().Vector();
	const FVector ModelUp = RootComponent->GetUpVector();
	const FVector ModelForward = ProjectToPlane(CamForward, ModelUp).GetSafeNormal();

	// We want to align our forward vector with the plane given by the normal
	const FVector PlaneForward = ProjectToPlane(ModelForward, Normal).GetSafeNormal();

	// We also want to flatten our forward vector against the plane defined by the model's pitch axis
	// This prevents the model from turning sideways (on its yaw axis)
	const FVector PitchAxis = FVector::CrossProduct(ModelForward, ModelUp).GetSafeNormal();
	FVector FinalForward = ProjectToPlane(PlaneForward, PitchAxis);

	// Calculate the rotation given by our forward vector and the normal (interpreted as up)
	const FVector O = RootComponent->GetComponentLocation();
	FQuat LookAtQuat = FindLookAtQuat(O, O + FinalForward * 1000, Normal);

	// Apply the rotation to RootComponent
	FQuat RootQuat = RootComponent->GetComponentQuat();
	FQuat FinalQuat = FQuat::Slerp(RootQuat, LookAtQuat, t);
	RootComponent->SetRelativeRotation(FinalQuat);
}


void AWallCrawler::SetLatchPoint(FVector Location, FVector Normal)
{
	LatchPoint = Location;
	LatchNormal = Normal;
}


void AWallCrawler::CollectYawInput(float Value)
{
	InputYaw += Value;
}

void AWallCrawler::CollectPitchInput(float Value)
{
	InputPitch += Value;
}
void AWallCrawler::CollectForwardInput(float Value)
{
	InputForward += Value;
}
void AWallCrawler::CollectRightInput(float Value)
{
	InputRight += Value;
}
void AWallCrawler::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	InputYaw += Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds();
}

void AWallCrawler::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	InputPitch += Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds();
}

void AWallCrawler::FlushInput()
{
	InputYaw = 0;
	InputPitch = 0;
	InputForward = 0;
	InputRight = 0;
}

void AWallCrawler::CollectJumpInput(float Value)
{

}
void AWallCrawler::CollectReleaseInput(float Value)
{

}

FVector ForwardRightUpYoRightUpBack(const FVector& V)
{
	return FVector(V.Y, V.Z, -V.X);
}
FVector RUBtoFRU(const FVector& V)
{
	return FVector(-V.Z, V.X, V.Y);
}

FQuat AWallCrawler::FindLookAtQuat(const FVector& EyePosition, const FVector& LookAtPosition, const FVector& UpVector)
{
	const FVector XAxis = (LookAtPosition - EyePosition).GetSafeNormal();
	const FVector YAxis = (UpVector ^ XAxis).GetSafeNormal();
	const FVector ZAxis = XAxis ^ YAxis;

	return FRotationMatrix::MakeFromXZ(XAxis, ZAxis).ToQuat();
}

FVector AWallCrawler::ProjectToPlane(const FVector& U, const FVector& N)
{
	return U - ((FVector::DotProduct(U, N)) / N.SizeSquared()) * N;
}

void AWallCrawler::MarkSpot(FVector Point, FColor Colour, float Duration)
{
	float length = 10.f;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, true, Duration, 0, 1.f);
			}
		}
	}
}