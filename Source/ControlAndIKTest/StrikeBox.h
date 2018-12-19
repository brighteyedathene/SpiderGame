// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "StrikeLimb.h"
#include "StrikeBox.generated.h"


UENUM(BlueprintType)
enum class EStrikePosition : uint8
{
	None UMETA(DisplayName = "None"),
	Chest UMETA(DisplayName = "Chest"),
	Groin UMETA(DisplayName = "Groin"),
	Neck UMETA(DisplayName = "Neck"),
	Face UMETA(DisplayName = "Face"),
	Occiput UMETA(DisplayName = "Occiput"),
	LeftArm UMETA(DisplayName = "LeftArm"),
	RightArm UMETA(DisplayName = "RightArm"),
	LeftShoulder UMETA(DisplayName = "LeftShoulder"),
	RightShoulder UMETA(DisplayName = "RightShoulder"),
	LeftThigh UMETA(DisplayName = "LeftThigh"),
	RightThigh UMETA(DisplayName = "RightThigh"),
	LeftFoot UMETA(DisplayName = "LeftFoot"),
	RightFoot UMETA(DisplayName = "RightFoot"),
	LowSurfaceLeft UMETA(DisplayName = "LowSurfaceLeft"),
	LowSurfaceRight UMETA(DisplayName = "LowSurfaceRight"),
	MidSurface UMETA(DisplayuName = "MidSurface"),
	HighSurface UMETA(DisplayName = "HighSurface")
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTROLANDIKTEST_API UStrikeBox : public UBoxComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStrikeBox();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StrikeSetup)
	EStrikePosition StrikePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StrikeSetup)
	ELimb LimbType;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool CrawlerInside;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StrikeSetup)
	bool bDetectionDependsOnBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StrikeSetup)
	TArray<FName> DetectionBoneNames;

	UFUNCTION()
	bool IsBoneValidForDetection(const FName & BoneName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StrikeSetup)
	FName SocketName;

	/* Higher number means lower priority when choosing a pose. 0 = first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StrikeSetup)
	int Priority;

	/* How quickly can this box detect a crawler? 0 means it never will. (Not in use!) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float Sensitivity;

	/* How long will the deadly part of the strike last. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float StrikeDuration;

	/* How long before another strike can be started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float CooldownDuration;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/* Tracks the normal size of the collider before expansion */
	FVector BaseScale;

	/* How much should this box expand when a crawler enters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float OverlapExpansionFactor;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// declare overlap begin function
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// declare overlap end function
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void Disable();

	inline static bool StrikeBoxCompare(UStrikeBox& One, UStrikeBox& Two) { return One.Priority < Two.Priority; }

	static TMap<EStrikePosition, FName> SocketMap;
};
