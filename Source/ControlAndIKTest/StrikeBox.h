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
	LeftArm UMETA(DisplayName = "LeftArm"),
	RightArm UMETA(DisplayName = "RightArm"),
	Neck UMETA(DisplayName = "Neck"),
	LowSurface UMETA(DisplayName = "LowSurface"),
	HighSurface UMETA(DisplayName = "HighSurface")
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTROLANDIKTEST_API UStrikeBox : public UBoxComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStrikeBox();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	EStrikePosition StrikePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	ELimb LimbType;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Strike)
	bool CrawlerInside;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	FName SocketName;

	/* Higher number means higher priority when choosing a pose */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	int Priority;

	/* How quickly can this box detect a crawler? 0 means it never will. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Strike)
	float Sensitivity;

	// declare overlap begin function
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// declare overlap end function
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Disable();
	
	inline static bool StrikeBoxCompare(UStrikeBox& One, UStrikeBox& Two) { return One.Priority < Two.Priority; }


	static TMap<EStrikePosition, FName> SocketMap;
};
