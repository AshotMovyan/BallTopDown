// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "..//Actors/Wall.h"
#include "MyPlayer.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class USphereComponent;
class UArrowComponent;
class UPlayerStart;
class ATopDownGameGameModeBase;

UCLASS()
class TOPDOWNGAME_API AMyPlayer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UArrowComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
		USphereComponent* SphereColl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		float RotateSpeed; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float MaxLaunchIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		float MaxTimeLaunchVelocty;		//	The time it takes for the ball to get maximum tension to launch			

	void TurnAtRate(float value);

	void LaunchPressed();
	void LaunchReleased();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void FellOutOfWorld(const UDamageType& DmgType) override;

private:
	FRotator CurrentRotation;
	FTimerHandle TimerHandle;
	float CurrentLaunchIntensity;
	FVector CurrentForward;
	ATopDownGameGameModeBase* MyMode;
};
