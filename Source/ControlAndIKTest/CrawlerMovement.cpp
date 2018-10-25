// Fill out your copyright notice in the Description page of Project Settings.

#include "CrawlerMovement.h"

#include "DrawDebugHelpers.h"



UCrawlerMovement::UCrawlerMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxSpeedOnSurface = 1200.f;
	Acceleration = 4000.f;
	Deceleration = 8000.f;
	TurningBoost = 8.0f;

	SurfaceRotationAlpha = 0.2;
	SurfaceRayLength = 180.f;
	IdealDistanceToSurface = 80.f;
	IdealDistanceTolerance = 80.f;
	IdealDistanceThreshold = 20.f;
	GripLossDistance = 180.f;
	PiggybackStrength = 0.99f;
	ClingMultiplier = 1400;
	ClimbMultiplier = 200;

	MaxSpeedInAir = 1500.f;
	TerminalVelocity = 9000.f;
	AerialAcceleration = 3000.f;
	AerialDeceleration = 6000.f;
	AerialRotationAlpha = 0.1;
	
	MaxJumpHeight = 400.f;
	MinJumpHeight = 40.f;
	MaxJumpTime = 0.5;
	MaxFallTime = 0.4;


	
	bPositionCorrected = false;
	CrawlerState = ECrawlerState::Falling;
	
	CollisionParameters.AddIgnoredActor(GetOwner());
	CollisionParameters.bTraceComplex = true;
	TraceChannel = ECC_Visibility;

	ResetMoveState();
}

//TODO Calculate gravity and min jump time here
void UCrawlerMovement::BeginPlay()
{
	Super::BeginPlay();

	MobileTargetActor = GetWorld()->SpawnActor<AMobileTargetActor>(AMobileTargetActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
}


void UCrawlerMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}

	const AController* Controller = PawnOwner->GetController();
	if (Controller && Controller->IsLocalController())
	{
		// apply input for local players
		if (Controller->IsLocalPlayerController() == true)
		{
			// Apply controller-based changes to velocity and state
			UpdateCrawlerMovementState(DeltaTime);
		}
		// Perform the movement and handle colliding/sliding
		MoveActor(DeltaTime);
	}
	
}

void UCrawlerMovement::MoveActor(float DeltaTime)
{
	bPositionCorrected = false;

	FVector Delta = Velocity * DeltaTime;
	if (!Delta.IsNearlyZero(1e-6f))
	{
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat Rotation = UpdatedComponent->GetComponentQuat();

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HandleImpact(Hit, DeltaTime, Delta);
			// Try to slide the remaining distance along the surface.
			SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
		}

		// Update velocity
		// We don't want position changes to vastly reverse our direction (which can happen due to penetration fixups etc)
		if (!bPositionCorrected)
		{
			const FVector NewLocation = UpdatedComponent->GetComponentLocation();
			Velocity = ((NewLocation - OldLocation) / DeltaTime);
		}
	}

	// Finalize
	UpdateComponentVelocity();
}

void UCrawlerMovement::UpdateCrawlerMovementState(float DeltaTime)
{
	FVector PendingInput = GetPendingInputVector();


	if (IsCrawling() && bJumpInProgress)
	{
		AddJumpVelocity();
		SetJumping();
		UpdatedComponent->GetOwner()->RemoveFromRoot();
		FVector PiggybackVelocity = (MobileTargetActor->GetActorLocation() - LatchPoint) / GetWorld()->GetDeltaSeconds();
		Velocity += PiggybackVelocity;
	}

	switch (CrawlerState)
	{
	case ECrawlerState::Crawling:

		// Update the position and rotation wrt the MobileTargetActor's movement since last frame
		{
			FVector LatchPointDiff = MobileTargetActor->GetActorLocation() - LatchPoint;
			FQuat LatchNormalDiff = FQuat::FindBetween(MobileTargetActor->GetActorRotation().Vector(), LatchNormal);

			UpdatedComponent->AddRelativeRotation(LatchNormalDiff, true);
			UpdatedComponent->AddRelativeLocation(LatchPointDiff * PiggybackStrength, true);

			LatchPoint = MobileTargetActor->GetActorLocation();
			LatchNormal = MobileTargetActor->GetActorRotation().Vector();
		}

		{
			// Scoped to avoid redefinition of the following variables
			FVector AverageLocation, AverageNormal;
			float SuggestedClimbFactor = 0;
			int HitCount;
			if (ExploreEnvironmentWithRays(&AverageLocation, &AverageNormal, &HitCount, &SuggestedClimbFactor, PendingInput.GetSafeNormal(), 5))
			{
				SetLatchPoint(AverageLocation, AverageNormal);
			}
			FVector ClingVector = GetClingVector(LatchPoint, LatchNormal);
			FVector ClimbVector = GetClimbVector(LatchNormal, SuggestedClimbFactor);

			AddInputVector(ClingVector * DeltaTime);
			AddInputVector(ClimbVector * DeltaTime);
		}
		RotateTowardsNormal(LatchNormal, SurfaceRotationAlpha);
		ApplyControlInputToVelocity(DeltaTime);

		if (FVector::Distance(LatchPoint, UpdatedComponent->GetComponentLocation()) > GripLossDistance)
		{
			SetFalling();
		}

		break;

	case ECrawlerState::Jumping:

		RotateTowardsNormal(FVector(0, 0, 1), AerialRotationAlpha);
		ApplyControlInputToVelocity(DeltaTime);


		if (AirTimer > MaxJumpTime || (!bStillWantToJump && AirTimer > GetMinJumpTime()))
		{
			SetFalling();
			bJumpInProgress = false;
		}

		AirTimer += DeltaTime;

		break;

	case ECrawlerState::Falling:

		{
			// Scoped to avoid redefinition of the following variables
			FVector AverageLocation, AverageNormal;
			float SuggestedClimbFactor = 0;
			int HitCount;
			if (ExploreEnvironmentWithRays(&AverageLocation, &AverageNormal, &HitCount, &SuggestedClimbFactor, PendingInput.GetSafeNormal(), 5))
			{
				SetLatchPoint(AverageLocation, AverageNormal);
				SetCrawling();
			}
		}
		RotateTowardsNormal(FVector(0, 0, 1), AerialRotationAlpha);
		ApplyControlInputToVelocity(DeltaTime);

		AirTimer += DeltaTime;

		break;

	}


}


void UCrawlerMovement::ApplyControlInputToVelocity(float DeltaTime)
{
	const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;

	//FVector WorkingVelocity = (!IsCrawling()) ? FVector(Velocity.X, Velocity.Y, 0.f) : Velocity;
	FVector WorkingVelocity = Velocity;

	const bool bExceedingMaxSpeed = IsThisExceedingMaxSpeed(MaxPawnSpeed, WorkingVelocity);

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (WorkingVelocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			WorkingVelocity = WorkingVelocity + (ControlAcceleration * WorkingVelocity.Size() - WorkingVelocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (WorkingVelocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = WorkingVelocity;
			const float VelSize = FMath::Max(WorkingVelocity.Size() - FMath::Abs(GetDeceleration()) * DeltaTime, 0.f);
			WorkingVelocity = WorkingVelocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && WorkingVelocity.SizeSquared() < FMath::Square(MaxPawnSpeed))
			{
				WorkingVelocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? WorkingVelocity.Size() : MaxPawnSpeed;
	WorkingVelocity += ControlAcceleration * FMath::Abs(GetAcceleration()) * DeltaTime;
	WorkingVelocity = WorkingVelocity.GetClampedToMaxSize(NewMaxSpeed);

	// Apply jump force
	if (IsJumping())
	{
		WorkingVelocity.Z = fmaxf(Velocity.Z - GetJumpingGravity() * DeltaTime, -TerminalVelocity);
	}

	// Apply gravity
	if (IsFalling())
	{
		WorkingVelocity.Z = fmaxf(Velocity.Z - GetFallingGravity() * DeltaTime, -TerminalVelocity);
	}

	Velocity = WorkingVelocity;
	ConsumeInputVector();
}


void UCrawlerMovement::MaybeStartJump()
{
	if (!IsJumping() && !IsFalling())
	{
		bJumpInProgress = true;
		bStillWantToJump = true;
	}
}

void UCrawlerMovement::MaybeEndJump()
{
	bStillWantToJump = false;
}

void UCrawlerMovement::AddJumpVelocity()
{
	const float InitialJumpSpeed = 2 * MaxJumpHeight / MaxJumpTime;
	Velocity += UpdatedComponent->GetUpVector() * InitialJumpSpeed;
}
float UCrawlerMovement::GetMinJumpTime()
{
	return FMath::Sqrt(2 * MinJumpHeight / GetJumpingGravity());
}
float UCrawlerMovement::GetJumpingGravity()
{
	return 2 * MaxJumpHeight / (MaxJumpTime * MaxJumpTime);
}
float UCrawlerMovement::GetFallingGravity()
{
	return 2 * MaxJumpHeight / (MaxFallTime * MaxFallTime);
}

float UCrawlerMovement::GetAcceleration()
{
	return (IsCrawling()) ? Acceleration : AerialAcceleration;
}

float UCrawlerMovement::GetDeceleration()
{
	return (IsCrawling()) ? Deceleration : AerialDeceleration;
}


float UCrawlerMovement::GetMaxSpeed()
{
	return IsCrawling() ? MaxSpeedOnSurface : MaxSpeedInAir;
}

bool UCrawlerMovement::IsThisExceedingMaxSpeed(float MaxSpeed, FVector Velo) const
{
	MaxSpeed = FMath::Max(0.f, MaxSpeed);
	const float MaxSpeedSquared = FMath::Square(MaxSpeed);

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return (Velo.SizeSquared() > MaxSpeedSquared * OverVelocityPercent);
}


bool UCrawlerMovement::ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotationQuat)
{
	bPositionCorrected |= Super::ResolvePenetrationImpl(Adjustment, Hit, NewRotationQuat);
	return bPositionCorrected;
}



bool UCrawlerMovement::ExploreEnvironmentWithRays(
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

	const FVector O = UpdatedComponent->GetComponentLocation();
	const FVector F = FVector::ForwardVector;
	const FVector R = FVector::RightVector;
	const FVector U = FVector::UpVector;

	// We write to these values in the following loop
	int HitCountSum = 0;
	FVector NormalSum = FVector(0, 0, 0);
	FVector LocationSum = FVector(0, 0, 0);
	float MaxClimb = 0;
	

	float MinDistance = SurfaceRayLength + 10.f; // We need to attach the MovingTarget to the closest HitActor
	FHitResult Hit;

	// Probe for hits, accumulate normal and location sums, calculate climb value
	for (float x = -1.f; x <= 1.f; x += delta)
	{
		for (float y = -1.f; y <= 1.f; y += delta)
		{
			for (float z = -1.f; z <= 1.f; z += delta)
			{
				FVector Ray = (F * x + R * y + U * z).GetSafeNormal();
				if (GetWorld()->LineTraceSingleByChannel(Hit, O, O + Ray * SurfaceRayLength, TraceChannel, CollisionParameters))
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
					const float UpDotRay = FVector::DotProduct(UpdatedComponent->GetUpVector(), -Ray);
					const float RightDotRay = FVector::DotProduct(FVector::CrossProduct(MovementDirection, UpdatedComponent->GetUpVector()), -Ray);
					const float VertThreshold = 0.4;
					const float HoriThreshold = 0.4;
					if (MoveDotRay < 0 && fabsf(UpDotRay) < VertThreshold && fabsf(RightDotRay) < HoriThreshold)
					{
						MaxClimb = fmaxf(MaxClimb, fabsf(MoveDotRay) - fabsf(UpDotRay));
						//MaxClimb = fminf(1.f, MaxClimb + fabsf(MoveDotRay) - fabsf(UpDotRay));
					}

					// Attach the MovingTargetActor to the closest Hit
					if (Hit.Distance < MinDistance)
					{
						MinDistance = Hit.Distance;
						MobileTargetActor->AttachToActor(Hit.Actor.Get(), FAttachmentTransformRules::KeepWorldTransform);
						//UpdatedComponent->GetOwner()->AttachToActor(Hit.Actor.Get(), AttachmentRules);
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


void UCrawlerMovement::SetLatchPoint(FVector Location, FVector Normal)
{
	LatchPoint = Location;
	LatchNormal = Normal;
	MobileTargetActor->SetActorLocationAndRotation(Location, Normal.Rotation());
}


bool UCrawlerMovement::IsLookingToCling()
{
	return (CrawlerState == ECrawlerState::Crawling) || (CrawlerState == ECrawlerState::Falling);
}


FVector UCrawlerMovement::GetClingVector(const FVector& EndLocation, const FVector& EndNormal)
{
	// Try to move towards a point above EndLocation while rotating in line with EndNormal

	const FVector TargetLocation = EndLocation + EndNormal * IdealDistanceToSurface;

	FVector Direction = TargetLocation - UpdatedComponent->GetComponentLocation();
	const float Distance = Direction.Size();

	float Force = fmaxf(0, Distance - IdealDistanceThreshold) / IdealDistanceTolerance;
	Force *= Force;

	return Direction.GetSafeNormal() * Force * ClingMultiplier;
}


FVector UCrawlerMovement::GetClimbVector(const FVector& Normal, float ClimbFactor)
{
	// The climb vector should be proportional to the movement speed
	return Normal * ClimbFactor * ClimbMultiplier * (Velocity.Size() / MaxSpeedOnSurface);
}



FVector ProjectToPlane(const FVector& U, const FVector& N)
{
	return U - ((FVector::DotProduct(U, N)) / N.SizeSquared()) * N;
}

FQuat FindLookAtQuat(const FVector& EyePosition, const FVector& LookAtPosition, const FVector& UpVector)
{
	const FVector XAxis = (LookAtPosition - EyePosition).GetSafeNormal();
	const FVector YAxis = (UpVector ^ XAxis).GetSafeNormal();
	const FVector ZAxis = XAxis ^ YAxis;

	return FRotationMatrix::MakeFromXZ(XAxis, ZAxis).ToQuat();
}
void UCrawlerMovement::RotateTowardsNormal(FVector Normal, float t)
{
	// We will calculate a forward vector based on the model rotation, the normal and the camera
	//const FVector CamForward = CameraBoom->GetComponentRotation().Vector();
	const FVector CamForward = CameraForward;  ///////////////////////////////////////////////// TODO CHANGE THIS when rebuilding camera /
	const FVector ModelUp = UpdatedComponent->GetUpVector();
	const FVector ModelForward = ProjectToPlane(CamForward, ModelUp).GetSafeNormal();

	// We want to align our forward vector with the plane given by the normal
	const FVector PlaneForward = ProjectToPlane(ModelForward, Normal).GetSafeNormal();

	// We also want to flatten our forward vector against the plane defined by the model's pitch axis
	// This prevents the model from turning sideways (on its yaw axis)
	const FVector PitchAxis = FVector::CrossProduct(ModelForward, ModelUp).GetSafeNormal();
	FVector FinalForward = ProjectToPlane(PlaneForward, PitchAxis);

	// Calculate the rotation given by our forward vector and the normal (interpreted as up)
	const FVector O = UpdatedComponent->GetComponentLocation();
	FQuat LookAtQuat = FindLookAtQuat(O, O + FinalForward * 1000, Normal);

	// Apply the rotation to RootComponent
	FQuat RootQuat = UpdatedComponent->GetComponentQuat();
	FQuat FinalQuat = FQuat::Slerp(RootQuat, LookAtQuat, t);
	UpdatedComponent->SetRelativeRotation(FinalQuat);
}


// Delte these later
void UCrawlerMovement::MarkSpot(FVector Point, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	float length = 10.f;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, IsPersistant, Duration, 0, 1.f);
			}
		}
	}
}

void UCrawlerMovement::MarkLine(FVector Start, FVector End, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	DrawDebugLine(GetWorld(), Start, End, Colour, IsPersistant, Duration, 0, 1.f);
}