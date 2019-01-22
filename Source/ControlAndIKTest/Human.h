// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthInterface.h"
#include "VisibleInterface.h"
#include "StrikeBox.h"
#include "MobileTargetActor.h"

#include "Stimulus.h"

#include "Human.generated.h"

class UHumanMovement;
class UHumanGaitControl;
class UHumanSenseComponent;
class ATargetPoint;
class UBehaviorTree;
class UCapsuleComponent;
class AAssassinationGameState;
class AHumanAIController;

UCLASS()
class CONTROLANDIKTEST_API AHuman : public APawn, public IHealthInterface, public IVisibleInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHuman();

	UPROPERTY(EditAnywhere)
	UHumanMovement* HumanMovement;

	UPROPERTY(EditAnywhere)
	UHumanGaitControl* HumanGaitControl;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UHumanSenseComponent* HumanSense;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	void MoveForward(float value);
	void MoveRight(float value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;



#pragma region GlobalReferencing

	// keeping a reference to avoid casting for every use
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameState")
	//AAssassinationGameState* AssassinationState;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	AAssassinationGameState* GetAssassinationState();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	AHumanAIController* GetHumanAI();

#pragma endregion GlobalReferencing



#pragma region InnerFeeling

public:
	UPROPERTY(Transient, BlueprintReadOnly, Category = IndividualFeeling)
	bool bCrawlerInSight;

	UPROPERTY(Transient, BlueprintReadOnly, Category = IndividualFeeling)
	bool bDeadBodyInSight;

	UPROPERTY(Transient, BlueprintReadOnly, Category = IndividualFeeling)
	bool bHumanInTroubleInSight;

	UPROPERTY(Transient, BlueprintReadOnly, Category = IndividualFeeling)
	bool bCrawlerOnSensitiveArea;

	UPROPERTY(Transient, BlueprintReadOnly, Category = IndividualFeeling)
	bool bCrawlerOnBody;

	UPROPERTY(Transient, BlueprintReadOnly, Category = IndividualFeeling)
	float Tension;

	/** How long before sensing something spooky causes a global alert */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IndividualFeeling)
	float TensionThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IndividualFeeling)
	float TensionCooldownRate;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = IndividualFeeling)
	bool IsAlert() { return Tension >= TensionThreshold; };


	UPROPERTY(Transient, BlueprintReadWrite, Category = IndividualFeeling)
	TArray<UStimulus*> Stimuli;

	UFUNCTION(BlueprintCallable, Category = IndividualFeeling)
	void AddNewStimulus(EStimulusType Type, FVector Location, AActor* Actor = nullptr);

	UFUNCTION(BlueprintCallable, Category = IndividualFeeling)
	void ProcessStimuli();

	void PrintStimuli();

	void PrintStatusVariables();

	UFUNCTION(BlueprintCallable, Category = IndividualFeeling)
	void UpdateTension(float DeltaTime);

	float GetTensionMin();

	// Unset all flags from senses so that tension can be released
	void ClearInnerFeeling();

#pragma endregion InnerFeeling



#pragma region Vision

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vision)
	USceneComponent* EyeLocationMarker;

	UFUNCTION(BlueprintCallable, Category = Vision)
	void UpdateVision();

	bool IsThisActorVisible(AActor* Target);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vision)
	float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vision)
	float IdentifyCrawlerRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vision)
	float AlertFieldOfView;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vision)
	float AlertIdentifyCrawlerRange;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	float GetEffectiveIdentifyCrawlerRange();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	float GetEffectiveFieldOfView();

	/** Angle (degrees) by which to rotate the eyes downward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vision)
	float EyeTilt;



	UPROPERTY(Transient, BlueprintReadOnly, Category = Vision)
	FVector LookTarget;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = Vision)
	int LookTargetPriority;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Vision)
	bool bLookTargetValid;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Vision)
	bool IsLookTargetValid() { return bLookTargetValid; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Vision)
	FVector GetLookTarget();

	UFUNCTION(BlueprintCallable, Category = Vision)
	void TryUpdateLookTarget(FVector NewTarget, int Priority);

#pragma endregion Vision



#pragma region Visibility

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
	USceneComponent* VisionTargetMarker;

	/** Has this human been seen by another while not living */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Visibility")
	bool bSeenAsDeadBody;

	UFUNCTION(BlueprintCallable, Category = "Visibility")
	void MarkAsSeenDead() { bSeenAsDeadBody = true; };

	/** Has this human been seen by another while not living */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Visibility")
	bool bSeenInTrouble;

	UFUNCTION(BlueprintCallable, Category = "Visibility")
	void MarkAsSeenInTrouble() { bSeenInTrouble = true; };

	UFUNCTION(BlueprintCallable, Category = "Visibility")
	void MarkAsNoLongerInTrouble() { bSeenInTrouble = false; };

	/* Visibility Interface functions **/
	virtual FVector GetVisionTargetLocation_Implementation() override;

#pragma endregion Visibility



#pragma region Health

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCapsuleComponent* HumanCollider;

	/** Health interface functions */
	virtual float GetHealth_Implementation() override;
	virtual void UpdateHealth_Implementation(float Delta) override;
	virtual bool IsDead_Implementation() override;
	virtual void Die_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	float GetBiteDamageForBone(FName BoneName);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	TMap<FName, float> LocationDamageMap;

	UFUNCTION(BlueprintCallable, Category = Health)
	void SetStunnedFor(float StunDuration) { StunRemaining = fmaxf(StunRemaining, StunDuration); };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	bool IsStunned() { return StunRemaining > 0; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Health)
	float GetStunRemaing() { return StunRemaining/StunDecayDuration; };


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	float MaxHealth;

protected:

	float CurrentHealth;

	bool bDead;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Health)
	float StunRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health)
	float StunDecayDuration;

#pragma endregion Health


#pragma region Attack

public:

	TArray<UStrikeBox*> StrikeBoxes;
	TMap<ELimb, UStrikeLimb*> StrikeLimbMap;

	UFUNCTION(BlueprintCallable, Category = Senses)
	void UpdateActiveStrikeBox();

	UFUNCTION(BlueprintCallable, Category = Senses)
	UStrikeBox* FindActiveStrikeBox();

	/** Returns true if a crawler is latched to this human */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	bool IsCrawlerOnBody() { return bCrawlerOnBody; };

	/** Sets bCrawlerOnBody */
	UFUNCTION(BlueprintCallable, Category = Strike)
	void CheckCrawlerOnBody();

	/** Returns the crawler's latched bone name or "None" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	FName GetCrawlerBoneName();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	EStrikePosition StrikePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	UStrikeBox* ActiveStrikeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	UStrikeLimb* ActiveStrikeLimb;

	/* This value augments strike progression for IK weight and animation blending - Should always be greater than 1!! 
	* Effectively speeds up the motion of the strike without affecting the overall time taken
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	float EffectiveProgressMultiplier;

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bStrikeLockedIn;

	void BeginStrike();

	void ContinueStrike(float DeltaTime);

	UPROPERTY(Transient, BlueprintReadWrite)
	float StrikeTimer;

	/** Can be attached to strike boxes to more accurately track movement of limbs during strike animation 
	* WITHHOUT resorting to perfect accuracy on crawler!
	*/
	AMobileTargetActor* StrikeTargetTracker;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	FVector GetCurrentStrikeTarget();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	bool IsStriking();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	bool HasActiveStrikeBox() { return ActiveStrikeBox != nullptr; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	float GetStrikeProgress();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	float GetStrikeIKWeight();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Strike)
	ELimb GetStrikeLimbType();

#pragma endregion Attack


#pragma region Gait

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetForwardMovement();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetRightMovement();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetGaitFraction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetSpeedFraction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	FTransform GetLeftFootIKTransform();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	FTransform GetRightFootIKTransform();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetLeftFootIKBlend();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Gait)
	float GetRightFootIKBlend();

#pragma endregion Gait



#pragma region Movement

	UFUNCTION(BlueprintCallable, Category = Movement)
	void TurnToFaceDirection(FVector Direction);

	UFUNCTION(BlueprintCallable, Category = Movement)
	void TurnToFaceVelocity();

#pragma endregion Movement




#pragma region Sound

	UFUNCTION(BlueprintImplementableEvent, Category = Sound)
		void StrikeBegan_BPEvent();

#pragma endregion Sound

};
