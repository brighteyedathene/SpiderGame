// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanMovement.h"

#include "DrawDebugHelpers.h"


UHumanMovement::UHumanMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxSpeed = 150;
	Acceleration = 300.f;
	Deceleration = 300.f;
	MaxTurnSpeed = 180.f;
	BackwardsSpeedMultiplier = 0.3f;
	TurningBoost = 8.0f;
	bPositionCorrected = false;

	GroundRayStartOffset = -45.f;
	GroundRootOffset = -90.0f;
	GroundRayLength = 80.f;

	ResetMoveState();


}

void UHumanMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime) || bMovementDisabled)
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
		FVector ControlDirection = GetPendingInputVector().GetSafeNormal();

		// apply input for local players but also for AI that's not following a navigation path at the moment
		if (Controller->IsLocalPlayerController() == true || Controller->IsFollowingAPath() == false || bUseAccelerationForPaths)
		{
			ApplyControlInputToVelocity(DeltaTime);
		}

		// if it's not player controller, but we do have a controller, then it's AI
		// (that's not following a path) and we need to limit the speed
		else if (IsExceedingMaxSpeed(MaxSpeed) == true)
		{
			Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;
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

		TurnToFaceDirection(DeltaTime, Velocity);
		StickToGround();

		// Finalize
		UpdateComponentVelocity();
	}
};

void UHumanMovement::ApplyControlInputToVelocity(float DeltaTime)
{
	const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed(ControlAcceleration.GetSafeNormal()) * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);
	


	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			Velocity = Velocity + (ControlAcceleration * Velocity.Size() - Velocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = Velocity;
			const float VelSize = FMath::Max(Velocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && Velocity.SizeSquared() < FMath::Square(MaxPawnSpeed))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);


	ConsumeInputVector();
}

bool UHumanMovement::ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotationQuat)
{
	bPositionCorrected |= Super::ResolvePenetrationImpl(Adjustment, Hit, NewRotationQuat);
	return bPositionCorrected;
}

float UHumanMovement::GetMaxSpeed(FVector Direction)
{
	float HeadingDotDirecion = FVector::DotProduct(UpdatedComponent->GetForwardVector(), Direction);
	return MaxSpeed * fmaxf(BackwardsSpeedMultiplier, HeadingDotDirecion);
}

FVector UHumanMovement::GetVelocity()
{
	return Velocity;
}

float ReduceAngle180(float Angle)
{
	while (Angle > 180)
		Angle -= 360;

	while (Angle < -180)
		Angle += 360;

	return Angle;
}

void UHumanMovement::TurnToFaceDirection(float DeltaTime, FVector Direction)
{
	if (Direction.IsNearlyZero())
		return;

	float CurrentYaw = UpdatedComponent->GetComponentRotation().Yaw;
	float TargetYaw = Direction.Rotation().Yaw;
	float YawDiff = ReduceAngle180(TargetYaw - CurrentYaw);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("CurrentYaw = %f"), CurrentYaw));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("TargetYaw = %f"), TargetYaw));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("YawDiff = %f"), YawDiff));

	float MaxTurnThisFrame = MaxTurnSpeed * DeltaTime;
	CurrentTurnSpeed = FMath::Clamp(YawDiff, -MaxTurnThisFrame, MaxTurnThisFrame);

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("TurnSpeed = %f"), CurrentTurnSpeed));

	UpdatedComponent->AddRelativeRotation(FRotator(0, CurrentTurnSpeed, 0), true);
}

void UHumanMovement::StickToGround()
{
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(GetOwner());
	ECollisionChannel TraceChannel = ECC_GameTraceChannel5; // should be the 'Floor' channel


	FVector Start = UpdatedComponent->GetComponentLocation() + FVector::UpVector * GroundRayStartOffset;
	FVector End = Start - FVector::UpVector * GroundRayLength;
	FVector FeetPosition = UpdatedComponent->GetComponentLocation() + FVector::UpVector * GroundRootOffset;

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, CollisionParameters))
	{
		//UpdatedComponent->SetWorldLocation(Hit.ImpactPoint + FVector::UpVector * HeightOffGround);
		
		FVector GravityVector = Hit.ImpactPoint - FeetPosition;

		UpdatedComponent->AddWorldOffset(FVector::UpVector * GravityVector.Z);
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, AActor::GetDebugName(Hit.Actor.Get()));
	}

}