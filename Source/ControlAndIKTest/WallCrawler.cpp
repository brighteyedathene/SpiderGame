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

	YawFactor = 6.0f;

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

}

// Called when the game starts or when spawned
void AWallCrawler::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// Get the ragdoll sections
	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);
	for (auto Mesh : StaticMeshes)
	{

		if (Mesh->GetFName() == RagdollTag)
		{
			RagdollMeshes.Add(Mesh);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Added!"));
		}
	}
	
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

	FRotator CameraRotation = FRotator(-LocalPitch, InputYaw * YawFactor, 0);
	CameraBoom->SetRelativeRotation(CameraRotation);

	if (bDead)
	{
		InputPitch = 0;
		InputForward = 0;
		InputRight = 0;
		return;
	}

	// Movement control...
	// - This involves calculating camera forward/right 
	// - and projecting it onto the current surface (given by the RootComponent orientation or world up)
	const FRotator Rotation = CameraBoom->GetComponentRotation();
	const FVector CameraForward = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

	// TODO change this so that camera movement isn't tied to rotation speed in the movement component!
	CrawlerMovement->SetCameraRotation(CameraBoom->GetComponentQuat(), LocalPitch); 

	FVector FDirection = FVector::VectorPlaneProject(CameraForward, RootComponent->GetUpVector()).GetSafeNormal();
	FVector RDirection = FVector::VectorPlaneProject(CameraRight, RootComponent->GetUpVector()).GetSafeNormal();

	float MovementIntensity = fmaxf(fabsf(InputForward), fabsf(InputRight));
	FVector MovementDirection = InputForward * FDirection + InputRight * RDirection;
	MovementDirection.Normalize();

	AddMovementInput(MovementDirection, MovementIntensity);

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


#pragma region Health

float AWallCrawler::GetHealth_Implementation()
{
	return CurrentHealth;
}

void AWallCrawler::UpdateHealth_Implementation(float Delta)
{
	CurrentHealth = fminf(CurrentHealth + Delta, MaxHealth);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("ouch I recieved %f damage! Now my health is %f"), Delta, CurrentHealth));
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
		
		TArray<UStaticMeshComponent*> StaticMeshes;
		GetComponents<UStaticMeshComponent>(StaticMeshes);
		//for (auto Mesh : RagdollMeshes)
		for (auto Mesh : StaticMeshes)
		{
			Mesh->SetSimulatePhysics(true);
		}
		MySphereComponent->SetSimulatePhysics(false); // everything except the root
		MySphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		for (auto Leg : CrawlerGaitControl->Legs)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("About to kill a leg"));
			Leg->Die();
			//Leg->DetachRootComponentFromParent();
		}

		CrawlerMovement->bShouldUpdate = false;

		//GetCom
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("UWAAA!<dead>"));
	}
}
#pragma endregion Health


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
	CrawlerMovement->MaybeStartJump();
}
void AWallCrawler::JumpReleased()
{
	CrawlerMovement->MaybeEndJump();
}
void AWallCrawler::RollPressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Roll pressed"));
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
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Roll released"));
	CrawlerMovement->EndRoll();
}
#pragma endregion Input

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