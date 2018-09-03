// Fill out your copyright notice in the Description page of Project Settings.

#include "WallCrawler.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"






// Sets default values
AWallCrawler::AWallCrawler()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MySphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
	RootComponent = MySphereComponent;

	MySphereComponent->InitSphereRadius(ColliderSize);
	MySphereComponent->SetCollisionProfileName(TEXT("Pawn"));
	

	//PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("PawnMovement");
	CrawlerMovement = CreateDefaultSubobject<UCrawlerMovement>("CrawlerMovement");

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

	LatchHistoryLength = 10;
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

	//SphereComponent->AddForce(FVector(0,0,1));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::SanitizeFloat(SphereComponent->GetScaledSphereRadius()));

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

	//FRotator LocalRotator = (FRotator(0, 0, 0).Vector().RotateAngleAxis(LocalPitch, FVector(0, 1, 0))).Rotation(); //TODO refactor this mess
	//LocalRotator = (LocalRotator.Vector().RotateAngleAxis(LocalYaw, FVector(0, 0, 1))).Rotation();
	//FRotator LocalRotator = (FRotator(0, 0, 0).Vector().RotateAngleAxis(InputPitch, FVector(0, 1, 0))).Rotation(); //TODO refactor this mess
	//LocalRotator = (LocalRotator.Vector().RotateAngleAxis(InputYaw, FVector(0, 0, 1))).Rotation();
	//CameraBoom->SetRelativeRotation(LocalRotator);

	FRotator CameraRotation = FRotator(-LocalPitch, InputYaw*1.5, 0);

	CameraBoom->SetRelativeRotation(CameraRotation);
	//InputPitch = 0;
	//InputYaw = 0;

	// Movement control...
	// - This involves calculating camera forward/right 
	// - and projecting it onto the current surface (given by the RootComponent orientation or world up)
	const FRotator Rotation = CameraBoom->GetComponentRotation();
	const FVector CameraForward = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

	FVector FDirection = ProjectToPlane(CameraForward, RootComponent->GetUpVector()).GetSafeNormal();
	FVector RDirection = ProjectToPlane(CameraRight, RootComponent->GetUpVector()).GetSafeNormal();
	//FVector FDirection = CameraForward;
	//FVector RDirection = CameraRight;

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
			//MarkSpot(LatchPoint, FColor::Green, 0.1);
		}
		else
		{
			//MarkSpot(LatchPoint, FColor::Red, 0.1);
		}


		ClingToPoint(LatchPoint, LatchNormal);

		// Check for obstacles opposing movement, and add an upwward component to movement if found
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(
			Hit, 
			RootComponent->GetComponentLocation() - RootComponent->GetUpVector() * ColliderSize * 0.5f, 
			RootComponent->GetComponentLocation() + MovementDirection * SurfaceGroundRayLength, 
			TraceChannel, 
			CollisionParameters))
		{
			//AddMovementInput(RootComponent->GetUpVector(), MovementIntensity * MovementSpeed * GetWorld()->GetDeltaSeconds());
			//DrawDebugLine(GetWorld(), RootComponent->GetComponentLocation() - RootComponent->GetUpVector() * ColliderSize * 0.5f, Hit.ImpactPoint, FColor::Red, true, 1.f, 0, 0.2f);
			//DrawDebugLine(GetWorld(), RootComponent->GetComponentLocation(), RootComponent->GetComponentLocation() + RootComponent->GetUpVector() * 100, FColor::Yellow, true, 1.f, 0, 0.2f);

		}
		else
		{
			//DrawDebugLine(GetWorld(), RootComponent->GetComponentLocation() - RootComponent->GetUpVector() * ColliderSize * 0.5f, 
			//	RootComponent->GetComponentLocation() + MovementDirection * SurfaceGroundRayLength, FColor::White, true, 1.f, 0, 0.2f);
		}
		// A

		AddMovementInput(RootComponent->GetUpVector(), SuggestedClimbFactor * MovementSpeed * GetWorld()->GetDeltaSeconds());
		AddMovementInput(FDirection, InputForward * MovementSpeed * GetWorld()->GetDeltaSeconds());
		AddMovementInput(RDirection, InputRight * MovementSpeed * GetWorld()->GetDeltaSeconds());

		// B
		//AddMovementInput(CameraForward, InputForward * MovementSpeed * GetWorld()->GetDeltaSeconds());
		//AddMovementInput(CameraRight, InputRight * MovementSpeed * GetWorld()->GetDeltaSeconds());
		
	}
	
}

float ScaleTo01(float x, float lo, float hi)
{
	return (x - fabsf(lo)) * (fabsf(hi) / (fabsf(hi) - fabsf(lo)));
}

bool AWallCrawler::ExploreEnvironmentWithRays(
	FVector* AvgLocation, 
	FVector* AvgNormal, 
	int* HitCount, 
	float* SuggestedClimbFactor,
	FVector MovementDirection, 
	int RaysPerAxis)
{
	float delta = 2.f / (RaysPerAxis - 1);

	const FVector O = RootComponent->GetComponentLocation();
	const FVector F = FVector::ForwardVector;
	const FVector R = FVector::RightVector;
	const FVector U = FVector::UpVector;  
	
	int HitCountSum = 0;
	FVector NormalSum = FVector(0, 0, 0);
	FVector LocationSum = FVector(0, 0, 0);

	// Probe for hits, accumulate normal and location sums, calculate climb value
	float MaxClimb = 0;
	FHitResult Hit;
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

					//DrawDebugLine(GetWorld(), O, Hit.ImpactPoint, FColor::Black, true, 0.1, 0, 0.2f);
					float MoveDotRay = FVector::DotProduct(MovementDirection, -Ray);
					float UpDotRay = FVector::DotProduct(RootComponent->GetUpVector(), -Ray);
					float RightDotRay = FVector::DotProduct(FVector::CrossProduct(MovementDirection, RootComponent->GetUpVector()), -Ray);
					const float VertThreshold = 0.4;
					const float HoriThreshold = 0.4;
					if (MoveDotRay < 0 && fabsf(UpDotRay) < VertThreshold && fabsf(RightDotRay) < HoriThreshold)
					{
						//if (x == 0.0)
						DrawDebugLine(GetWorld(), O, Hit.ImpactPoint, FColor::Black, true, 2.1, 0, 0.2f);
						float NewClimbComponent = fabsf(MoveDotRay) - fabsf(UpDotRay);
						if (NewClimbComponent > MaxClimb)
						{
							MaxClimb = NewClimbComponent;
							//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("ClimbComponent = %f"), NewClimbComponent));
						}
							
					}

				}
			}
		}
	}
	if(MaxClimb > 0)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("ClimbComponent = %f"), MaxClimb));

	if (HitCountSum > 0)
	{
		*AvgLocation = LocationSum / HitCountSum;
		*AvgNormal = NormalSum / HitCountSum;
		*HitCount = HitCountSum;
		*SuggestedClimbFactor = MaxClimb;

		//MarkSpot(*AvgLocation, FColor::Red, 0.2);
		//DrawDebugLine(GetWorld(), O, Hit.ImpactPoint, FColor::Black, true, 0.1, 0, 0.2f);
		
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("NormalSum.Size() = %f"), NormalSum.Size()));
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("HitCount         = %d"), HitCountSum));

		return true;
	}
	else
	{
		//DrawDebugLine(GetWorld(), O, O + FVector(0,0,1000), FColor::White, true, 0.1, 0, 0.2f);
		return false;
	}
}



void AWallCrawler::ClingToPoint(FVector EndLocation, FVector EndNormal)
{
	FVector TargetLocation = EndLocation + EndNormal * SurfaceIdealDistance;
	const float Distance = FVector::Distance(TargetLocation, RootComponent->GetComponentLocation());
	float Force = fminf(MovementSpeed, Distance);
	Force = fmaxf(0, Force - SurfaceTransitionAllowance);
	if (Force > 0)
	{
		AddMovementInput(TargetLocation - RootComponent->GetComponentLocation(), Force * GetWorld()->GetDeltaSeconds());
		//RotateTowardsNormal(EndNormal, RotationCorrectionAlpha);
		//MarkSpot(TargetLocation, FColor::Red, 0.1);
	}
	RotateTowardsNormal(EndNormal, RotationCorrectionAlpha);

}



void AWallCrawler::RotateTowardsNormal(FVector Normal, float t)
{
	// //////////////////////////////////// SOMETHING IS WRONG HERE !!! /////////////////////////////////////

	// Get Pitch
	FVector CamForward = CameraBoom->GetComponentRotation().Vector();
	FVector ModelUp = RootComponent->GetUpVector();
	FVector ModelForward = ProjectToPlane(CamForward, ModelUp).GetSafeNormal();

	FVector PlaneForward = ProjectToPlane(ModelForward, Normal).GetSafeNormal();

	// Figure out the axis and angle 
	FVector PitchAxis = FVector::CrossProduct(ModelForward, ModelUp).GetSafeNormal();
	float ForwardDotNormal = FVector::DotProduct(PlaneForward, Normal);

	float UpDotNormal = FVector::DotProduct(ProjectToPlane(ModelForward, PitchAxis).GetSafeNormal(), ProjectToPlane(Normal, PitchAxis).GetSafeNormal());
	float PitchAngle = FMath::RadiansToDegrees(acosf(UpDotNormal));

	// rotate up and down on the pitch axis - then figure out which is closer
	FVector PitchUp = ModelForward.RotateAngleAxis(PitchAngle, PitchAxis);
	FVector PitchDown = ModelForward.RotateAngleAxis(PitchAngle, -PitchAxis);

	float PitchUpDotNormal = FVector::DotProduct(PitchUp, Normal);
	float PitchDownDotNormal = FVector::DotProduct(PitchDown, Normal);

	float diffForwardUp = fabsf(UpDotNormal - PitchUpDotNormal);
	float diffForwardDown = fabsf(UpDotNormal - PitchDownDotNormal);

	if (diffForwardUp < diffForwardDown)
	{
		ModelForward = PitchUp;
	}
	else
	{
		ModelForward = PitchDown;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::SanitizeFloat(PitchUpDotNormal));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::SanitizeFloat(PitchDownDotNormal));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::SanitizeFloat(PitchAngle));

	static bool wasFlipped = false;
	if (ForwardDotNormal > 0)
	{
		if (!wasFlipped)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("FLIPPED"));
			wasFlipped = true;
		}
	}
	else
	{
		if (wasFlipped)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("UNFLIPPED"));
			wasFlipped = false;
		}
	}
	FVector O = RootComponent->GetComponentLocation();

	PlaneForward = ProjectToPlane(PlaneForward, PitchAxis);

	FQuat LookAtQuat = FindLookAtQuat(O, O + PlaneForward * 1000, Normal); // Option A

	FQuat RootQuat = RootComponent->GetComponentQuat();
	FQuat FinalQuat = FQuat::Slerp(RootQuat, LookAtQuat, t);
	RootComponent->SetRelativeRotation(FinalQuat);

	//DrawDebugLine(GetWorld(), O, O + RootComponent->GetForwardVector() * 190, FColor::Magenta, 0.5, 0, 2.9f);

}

FQuat AWallCrawler::GetQuatFrom(FVector StartNormal, FVector TartgetNormal)
{
	FVector RotationAxis = FVector::CrossProduct(StartNormal, TartgetNormal);
	RotationAxis.Normalize();

	float CurrentDotTarget = FVector::DotProduct(StartNormal, TartgetNormal);
	float RotationAngle = acosf(CurrentDotTarget);

	return FQuat(RotationAxis, RotationAngle);
}


void AWallCrawler::SetLatchPoint(FVector Location, FVector Normal)
{
	LatchPoint = Location;
	LatchNormal = Normal;
	//LatchNormal = GetLatchNormalFromHistory();
}



void AWallCrawler::CollectYawInput(float Value)
{
	InputYaw = Value;
}

void AWallCrawler::CollectPitchInput(float Value)
{
	InputPitch = Value;
}

void AWallCrawler::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWallCrawler::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AWallCrawler::CollectForwardInput(float Value)
{
	InputForward = Value;
}

void AWallCrawler::CollectRightInput(float Value)
{
	InputRight = Value;
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