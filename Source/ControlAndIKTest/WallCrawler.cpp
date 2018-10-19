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
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AWallCrawler::DropPressed);

}

// Called when the game starts or when spawned
void AWallCrawler::BeginPlay()
{
	Super::BeginPlay();
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

	FVector FDirection = FVector::VectorPlaneProject(CameraForward, RootComponent->GetUpVector()).GetSafeNormal();
	FVector RDirection = FVector::VectorPlaneProject(CameraRight, RootComponent->GetUpVector()).GetSafeNormal();

	float MovementIntensity = fmaxf(fabsf(InputForward), fabsf(InputRight));
	FVector MovementDirection = InputForward * FDirection + InputRight * RDirection;
	MovementDirection.Normalize();

	// TODO change this so that camera movement isn't tied to rotation speed in the movement component!
	CrawlerMovement->SetCameraForward(CameraBoom->GetForwardVector()); 
	AddMovementInput(MovementDirection, MovementIntensity);

	if (CrawlerMovement->IsCrawling())
	{
		CrawlerGaitControl->UpdateGait(CrawlerMovement->GetVelocity());
	}

	// Clear all Inputs 
	FlushInput();

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

void AWallCrawler::JumpPressed()
{
	CrawlerMovement->MaybeStartJump();
}
void AWallCrawler::JumpReleased()
{
	CrawlerMovement->MaybeEndJump();
}
void AWallCrawler::DropPressed()
{

}
void AWallCrawler::DropReleased()
{

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