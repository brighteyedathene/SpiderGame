// Fill out your copyright notice in the Description page of Project Settings.

#include "CrawlerMovement.h"

/* This already has a body apparently
*/
UCrawlerMovement::UCrawlerMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxSpeed = 1200.f;
	Acceleration = 4000.f;
	Deceleration = 8000.f;
	TurningBoost = 8.0f;
	TerminalVelocity = 9000.f;
	GravityAcceleration = 980.f;
	bPositionCorrected = false;

	ResetMoveState();
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
		// apply input for local players but also for AI that's not following a navigation path at the moment
		if (Controller->IsLocalPlayerController() == true || Controller->IsFollowingAPath() == false || bUseAccelerationForPaths)
		{
			ApplyControlInputToVelocity(DeltaTime);
		}
		
		bPositionCorrected = false;

		// Move actor
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

}


void UCrawlerMovement::ApplyControlInputToVelocity(float DeltaTime)
{
	const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;

	FVector WorkingVelocity = (IsFalling()) ? FVector(Velocity.X, Velocity.Y, 0.f) : Velocity;


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
			const float VelSize = FMath::Max(WorkingVelocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
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
	WorkingVelocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	WorkingVelocity = WorkingVelocity.GetClampedToMaxSize(NewMaxSpeed);

	// Apply gravity
	if (IsFalling())
	{
		WorkingVelocity.Z = fmaxf(Velocity.Z - GravityAcceleration * DeltaTime, -TerminalVelocity);
	}

	Velocity = WorkingVelocity;
	ConsumeInputVector();
}

void UCrawlerMovement::SetFalling(bool ApplyIfTrue)
{
	bFalling = ApplyIfTrue;
}

bool UCrawlerMovement::IsFalling()
{
	return bFalling;
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