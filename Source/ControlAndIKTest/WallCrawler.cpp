// Fill out your copyright notice in the Description page of Project Settings.

#include "WallCrawler.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"

#include "Human.h"
#include "MobileTargetActor.h"


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

	FollowCamRailStart = CreateDefaultSubobject<USceneComponent>(TEXT("FollowCamRailStart"));
	FollowCamRailStart->SetupAttachment(RootComponent);
	FollowCamRailEnd = CreateDefaultSubobject<USceneComponent>(TEXT("FollowCamRailEnd"));
	FollowCamRailEnd->SetupAttachment(RootComponent);

	YawFactor = 6.0f;
	MaxOrbitDistance = 200.f;
	MinOrbitDistance = 0.0f;
	ZoomSpeed = 0.05f;
	FollowCameraDistance = 30.f;
	MaxFollowCameraDistance = 30.f;

	BiteRayStart = CreateDefaultSubobject<USceneComponent>(TEXT("BiteRayStart"));
	BiteRayStart->SetupAttachment(RootComponent);
	BiteRayLength = 10.f;
	BiteBaseDamage = 1.f;
	BiteForceReleaseDistance = 20.f;

	MaxHealth = 100.f;
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AWallCrawler::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AWallCrawler::JumpReleased);
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AWallCrawler::RollPressed);
	PlayerInputComponent->BindAction("Roll", IE_Released, this, &AWallCrawler::RollReleased);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AWallCrawler::Bite);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AWallCrawler::BiteRelease);


	PlayerInputComponent->BindAction("ChangeCameraMode", IE_Pressed, this, &AWallCrawler::CycleCameraModes);
	//PlayerInputComponent->BindAxis("Zoom", this, &AWallCrawler::Zoom); // Handled in blueprint!!

}

// Called when the game starts or when spawned
void AWallCrawler::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	BiteTargetActor = GetWorld()->SpawnActor<AMobileTargetActor>(AMobileTargetActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

}

// Called every frame
void AWallCrawler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Find out what camera we're using
	//AActor* CurrentCamOwner = GEngine->GetFirstLocalPlayerController(GetWorld())->PlayerCameraManager->GetViewTarget();
	//if (CurrentCamOwner == FollowCamera->GetOwner())
	//{
	//	UpdateCameraFollow();
	//	MoveStrafe(CurrentCamOwner->GetActorQuat());
	//}
	//else
	//{
	//	UpdateCameraFixed();
	//	MoveRotate(CurrentCamOwner->GetActorQuat());
	//}

	ContinueBite();

	switch (CameraMode)
	{
	case ECameraMode::Follow:
		UpdateCameraFollow();
		if (!BiteDown)
			MoveStrafe(CameraBoom->GetComponentQuat());
		break;

	case ECameraMode::Orbit:
		UpdateCameraOrbit();
		if(!BiteDown)
			MoveRotate(CameraBoom->GetComponentQuat());
		break;

	}


	if (CrawlerMovement->IsCrawling())
	{
		CrawlerGaitControl->UpdateGait(CrawlerMovement->GetVelocity() * GetWorld()->GetDeltaSeconds());
	}
	else if (CrawlerMovement->IsRolling())
	{
		CrawlerGaitControl->Slide(CrawlerMovement->GetVelocity() * GetWorld()->GetDeltaSeconds());
	}

	// Clear all Inputs 
	FlushInput();

}


#pragma region Camera


void AWallCrawler::CycleCameraModes()
{
	if (CameraMode == ECameraMode::Follow)
	{
		CameraMode = ECameraMode::Orbit;
		//FQuat OldRotation = CameraBoom->GetRelativeRotationFromWorld(FQuat::Identity);
		FRotator OldRotator = CameraBoom->GetComponentRotation();
		LocalPitch = -OldRotator.Pitch;
		LocalYaw = OldRotator.Yaw;
	}
	else
	{
		CameraMode = ECameraMode::Follow;
		LocalPitch = 0;
		LocalYaw = 0;
	}
}

void AWallCrawler::UpdateCameraFollow()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Using my camera!"));
	const float FOLLOW_CAM_MAX_PITCH = 89.f;
	const float FOLLOW_CAM_MIN_PITCH = -89.f;

	// Rotation control...
	LocalPitch += InputPitch;
	LocalYaw += InputYaw;
	if (LocalPitch > FOLLOW_CAM_MAX_PITCH)
		LocalPitch = FOLLOW_CAM_MAX_PITCH;
	if (LocalPitch < FOLLOW_CAM_MIN_PITCH)
		LocalPitch = FOLLOW_CAM_MIN_PITCH;

	if (LocalYaw > 360.0f)
		LocalYaw -= 360.0f;
	if (LocalYaw < 0.0f)
		LocalYaw += 360.0f;


	if (bDead)
	{
		FRotator CameraRotation = FRotator(-LocalPitch, LocalYaw, 0);
		CameraBoom->SetRelativeRotation(CameraRotation);
		FlushInput();
		return;
	}
	else
	{
		FRotator CameraRotation = FRotator(-LocalPitch, InputYaw * YawFactor, 0);
		CameraBoom->SetRelativeRotation(CameraRotation);
	}

	float PitchFactor = FMath::GetMappedRangeValueClamped(FVector2D(FOLLOW_CAM_MIN_PITCH, FOLLOW_CAM_MAX_PITCH), FVector2D(0.f, 1.f), LocalPitch);
	FVector CameraPosition = FollowCamRailStart->GetComponentLocation() * PitchFactor + FollowCamRailEnd->GetComponentLocation() * (1 - PitchFactor);
	CameraBoom->SetWorldLocation(CameraPosition);
	FollowCameraDistance = fminf(MaxFollowCameraDistance, PitchFactor * MaxFollowCameraDistance);

}

void AWallCrawler::UpdateCameraOrbit()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Using my camera!"));

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


	if (bDead)
	{
		FRotator CameraRotation = FRotator(-LocalPitch, LocalYaw, 0);
		CameraBoom->SetWorldRotation(CameraRotation);
		FlushInput();
		return;
	}
	else
	{
		FRotator CameraRotation = FRotator(-LocalPitch, LocalYaw, 0);
		CameraBoom->SetWorldRotation(CameraRotation);
	}

}

void AWallCrawler::UpdateCameraFixed()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Using another camera..."));
}


void AWallCrawler::MoveStrafe(const FQuat & CameraQuat)
{
	FVector ViewForward = CameraBoom->GetComponentQuat().GetForwardVector() + CameraBoom->GetComponentQuat().GetUpVector() * (LocalPitch / 90);
	//FVector ViewForward = CameraWorldRotation.Quaternion().GetForwardVector() + CameraWorldRotation.Quaternion().GetUpVector() * (CameraBoom->RelativeRotation.Pitch / 90);
	CrawlerMovement->SetViewForward(ViewForward);


	const FQuat CamQuat = CameraBoom->GetComponentQuat();
	const FVector CameraForward = FRotationMatrix::Make(CamQuat).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix::Make(CamQuat).GetUnitAxis(EAxis::Y);

	FVector FDirection = FVector::VectorPlaneProject(CameraForward, RootComponent->GetUpVector()).GetSafeNormal();
	FVector RDirection = FVector::VectorPlaneProject(CameraRight, RootComponent->GetUpVector()).GetSafeNormal();

	float MovementIntensity = fmaxf(fabsf(InputForward), fabsf(InputRight));
	FVector MovementDirection = InputForward * FDirection + InputRight * RDirection;
	MovementDirection.Normalize();

	AddMovementInput(MovementDirection, MovementIntensity);

}

void AWallCrawler::MoveRotate(const FQuat & CameraQuat)
{
	const FVector CameraForward = FRotationMatrix::Make(CameraQuat).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix::Make(CameraQuat).GetUnitAxis(EAxis::Y);
	const FVector CameraUp = FRotationMatrix::Make(CameraQuat).GetUnitAxis(EAxis::Z);

	FVector FProjection = FVector::VectorPlaneProject(CameraForward, RootComponent->GetUpVector()).GetSafeNormal();
	FVector RProjection = FVector::VectorPlaneProject(CameraRight, RootComponent->GetUpVector()).GetSafeNormal();
	FVector UProjection = FVector::VectorPlaneProject(CameraUp, RootComponent->GetUpVector()).GetSafeNormal();


	float ModelUpDotCameraForward = FVector::DotProduct(RootComponent->GetUpVector(), CameraForward);
	float ModelUpDotCameraRight = FVector::DotProduct(RootComponent->GetUpVector(), CameraRight);
	float ModelUpDotCameraUp = FVector::DotProduct(RootComponent->GetUpVector(), CameraUp);

	float ModelForwardDotCameraUp = FVector::DotProduct(RootComponent->GetForwardVector(), CameraUp);
	//float ModelUpDotCameraUp = FVector::DotProduct(RootComponent->GetUpVector(), CameraUp);
	float ModelRightDotCameraUp = FVector::DotProduct(RootComponent->GetRightVector(), CameraUp);

	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, FString::Printf(TEXT("ModelUp . CamForward: %f   ModelUp . CamRight: %f   ModelUp . CamUp: %f"), ModelUpDotCameraForward, ModelUpDotCameraRight, ModelUpDotCameraUp));

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
	//	FString::Printf(TEXT("ModelUp Dot F, R, U = %f, %f, %f"), ModelUpDotCameraForward, ModelUpDotCameraRight,  ModelUpDotCameraUp));



	float ForwardModifier = fmaxf(0, ModelUpDotCameraUp);
	FVector FDirection = (ForwardModifier * FProjection + (1 - ForwardModifier) * UProjection).GetSafeNormal();
	float RightModifier = fmaxf(0, ModelUpDotCameraForward);
	FVector RDirection = (RightModifier * UProjection + (1 - RightModifier) * RProjection).GetSafeNormal();
	//if (ModelUpDotCameraUp < 0)
	//{
	//	FDirection = UDirection;
	//}

	FVector R = RootComponent->GetComponentLocation();
	RDirection = FVector::VectorPlaneProject(FVector::CrossProduct(CameraUp, FDirection), RootComponent->GetUpVector()).GetSafeNormal() * FMath::Sign(ModelUpDotCameraUp);

	//FDirection = FDirection * fmaxf(FMath::Sign(ModelUpDotCameraForward), FMath::Sign(ModelUpDotCameraUp));


	//MarkLine(R, R + FDirection * 15, FColor::Red, 0);
	//MarkLine(R, R + RDirection * 15, FColor::Green, 0);

	float MovementIntensity = fmaxf(fabsf(InputForward), fabsf(InputRight));
	FVector MovementDirection = InputForward * FDirection + InputRight * RDirection;
	MovementDirection.Normalize();

	AddMovementInput(MovementDirection, MovementIntensity);


	FVector ViewForward = MovementIntensity * MovementDirection + (1 - MovementIntensity) * RootComponent->GetForwardVector();
	CrawlerMovement->SetViewForward(ViewForward);
}

#pragma endregion Camera



#pragma region Visibility

/* Visibility Interface functions **/
FVector AWallCrawler::GetVisionTargetLocation_Implementation() 
{
	return RootComponent->GetComponentLocation();
}

#pragma endregion Visibility



#pragma region Health

float AWallCrawler::GetHealth_Implementation()
{
	return CurrentHealth;
}

void AWallCrawler::UpdateHealth_Implementation(float Delta)
{
	CurrentHealth = fminf(CurrentHealth + Delta, MaxHealth);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("ouch I recieved %f damage! Now my health is %f"), Delta, CurrentHealth));
	if (IsDead_Implementation())
	{
		Die_Implementation();
	}
}

bool AWallCrawler::IsDead_Implementation()
{
	return CurrentHealth <= 0 || bDead;
}
void AWallCrawler::Die_Implementation()
{
	if (!bDead)
	{
		bDead = true;
		
		EndBite();

		TArray<UStaticMeshComponent*> StaticMeshes;
		GetComponents<UStaticMeshComponent>(StaticMeshes);
		for (auto & Mesh : StaticMeshes)
		{
			Mesh->SetSimulatePhysics(true);
			Mesh->SetCollisionProfileName("PhysicsBody");
		}

		MySphereComponent->SetSimulatePhysics(false); // everything except the root
		MySphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		for (auto & Leg : CrawlerGaitControl->Legs)
		{
			Leg->Die();
		}

		CrawlerMovement->bShouldUpdate = false;

		Execute_DeathNotice_BPEvent(this);
		//DeathNotice_BPEvent();

		//GetCom
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("UWAAA!<dead>"));
	}
}
#pragma endregion Health



#pragma region Attack

void AWallCrawler::Bite()
{
	if (IsDead_Implementation())
		return;

	FHitResult Hit = TryGetBiteTarget();
	if (Hit.IsValidBlockingHit())
	{
		AHuman* Human = Cast<AHuman>(Hit.Actor.Get());
		if (Human)
		{
			float Damage = Human->GetBiteDamageForBone(Hit.BoneName);
			if (Damage > 0)
			{
				CurrentBiteDPS = Damage * BiteBaseDamage;
				BiteVictim = Human;
			}
		}
		BiteTargetActor->AttachToComponent(Hit.Component.Get(), FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
		BiteTargetActor->SetActorLocationAndRotation(Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	BiteDown = true;
}

void AWallCrawler::BiteRelease()
{
	EndBite();
}

void AWallCrawler::ContinueBite()
{

	if (BiteVictim)
	{
		//MarkLine(BiteRayStart->GetComponentLocation(), BiteTargetActor->GetActorLocation(), FColor::Red, 0);
		if (FVector::Distance(BiteTargetActor->GetActorLocation(), BiteRayStart->GetComponentLocation()) > BiteForceReleaseDistance)
		{
			EndBite();
		}
		else
		{
			BiteVictim->UpdateHealth_Implementation(-CurrentBiteDPS * GetWorld()->GetDeltaSeconds());
		}
	}
	
}

void AWallCrawler::EndBite()
{
	if (BiteVictim)
	{
		BiteVictim = nullptr;
	}
	BiteDown = false;
}


FHitResult AWallCrawler::TryGetBiteTarget()
{
	//MarkLine(BiteRayStart->GetComponentLocation(), BiteRayStart->GetComponentLocation() - RootComponent->GetUpVector() * BiteRayLength, FColor::Green, 0);

	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);

	FVector RayOigin = BiteRayStart->GetComponentLocation();
	FVector ForwardBias = RootComponent->GetForwardVector() * 5.f;
	FVector RightBias = RootComponent->GetRightVector() * 5.f;

	FHitResult Hit;
	if ((GetWorld()->LineTraceSingleByChannel(Hit,
			RayOigin,
			RayOigin - RootComponent->GetUpVector() * BiteRayLength,
			ECC_Visibility, CollisionParameters)

		|| GetWorld()->LineTraceSingleByChannel(Hit,
			RayOigin,
			RayOigin - RootComponent->GetUpVector() * BiteRayLength + ForwardBias,
			ECC_Visibility, CollisionParameters)

		|| GetWorld()->LineTraceSingleByChannel(Hit,
			RayOigin,
			RayOigin - RootComponent->GetUpVector() * BiteRayLength - ForwardBias,
			ECC_Visibility, CollisionParameters)
		))
	{
		AHuman* Human = Cast<AHuman>(Hit.Actor.Get());
		if (Human)
		{
			if (Human->GetBiteDamageForBone(Hit.BoneName) > 0)
			{
				return Hit;
			}
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("nuthin"));
	FHitResult NoHit;
	return NoHit;
}

bool AWallCrawler::HasPotentialBiteTarget()
{
	FHitResult Hit = TryGetBiteTarget();
	if (Hit.IsValidBlockingHit())
	{
		return true;
	}
	else
	{
		return false;
	}	
}

#pragma endregion Attack



#pragma region Input
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

void AWallCrawler::JumpPressed()
{
	EndBite();

	CrawlerMovement->MaybeStartJump();
}
void AWallCrawler::JumpReleased()
{
	CrawlerMovement->MaybeEndJump();
}
void AWallCrawler::RollPressed()
{
	if (CrawlerMovement->bCanRoll)
		CrawlerMovement->StartRoll();
	else
	{
		if (!bDead)
			Die_Implementation();
		else
			Reset();
	}
}
void AWallCrawler::RollReleased()
{
	CrawlerMovement->EndRoll();
}
#pragma endregion Input



void AWallCrawler::MarkSpot(FVector Point, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	float length = 1.f;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				DrawDebugLine(GetWorld(), Point, Point + (FVector(x, y, z) * length), Colour, IsPersistant, Duration, 0, 0.1f);
			}
		}
	}
}

void AWallCrawler::MarkLine(FVector Start, FVector End, FColor Colour, float Duration)
{
	bool IsPersistant = Duration > 0;
	DrawDebugLine(GetWorld(), Start, End, Colour, IsPersistant, Duration, 0, 0.1f);
}