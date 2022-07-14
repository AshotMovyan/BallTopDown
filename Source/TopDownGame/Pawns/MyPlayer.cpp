// Fill out your copyright notice in the Description page of Project Settings.


#include "..//Pawns/MyPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/ArrowComponent.h"
#include "Math/UnrealMathUtility.h"


// Sets default values
AMyPlayer::AMyPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Collision initialization
	SphereColl = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereColl->InitSphereRadius(40.0f);
	SphereColl->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = SphereColl;
	SphereColl->SetSimulatePhysics(true);
	SphereColl->SetLinearDamping(0.01f);
	if (SphereColl && SphereColl->GetBodyInstance())
	{
		//MeshComp->SetLinearDamping(1.0f);
		SphereColl->GetBodyInstance()->bLockXRotation = true;
		SphereColl->GetBodyInstance()->bLockYRotation = true;
		SphereColl->GetBodyInstance()->bLockZRotation = true;
		//MeshComp->SetConstraintMode(EDOFMode::Default);
	}

	// StaticMesh initialization
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/EditorMeshes/ArcadeEditorSphere.ArcadeEditorSphere"));
	if (MeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(MeshAsset.Object);
	}
	MeshComp->SetupAttachment(RootComponent);

	// SpringArm initialization
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 400.0f;
	SpringArmComp->SocketOffset = FVector(0.0f, 0.0f, 800.0f);

	// Camera initialization
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));

	// Arrow initialization
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(MeshComp);

	// Variables initialization
	RotateSpeed = 400.0f;
	MaxLaunchIntensity = 200000.0f;
	MaxTimeLaunchVelocty = 1.0f;
}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentRotation.IsZero() == false)
	{
		FRotator NewRotator = CurrentRotation * DeltaTime;
		FQuat NewQuaternion = FQuat(NewRotator);
		Arrow->AddLocalRotation(NewQuaternion);
	}

}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("TurnAtRate", this, &AMyPlayer::TurnAtRate);
	PlayerInputComponent->BindAction("Launch", IE_Pressed, this, &AMyPlayer::LaunchPressed);
	PlayerInputComponent->BindAction("Launch", IE_Released, this, &AMyPlayer::LaunchReleased);

}


void AMyPlayer::TurnAtRate(float value)
{
	CurrentRotation = FRotator(0.0f, value * RotateSpeed, 0.0f);
}

void AMyPlayer::LaunchPressed()
{
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AMyPlayer::LaunchReleased, MaxTimeLaunchVelocty, true);

}

void AMyPlayer::LaunchReleased()
{
	CurrentLaunchIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, MaxTimeLaunchVelocty), FVector2D(0.0f, MaxLaunchIntensity),
		GetWorldTimerManager().GetTimerElapsed(TimerHandle));

	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::SanitizeFloat(CurrentLaunchIntensity));

	GetWorldTimerManager().ClearTimer(TimerHandle);

	FVector Forward = Arrow->GetForwardVector();
	SphereColl->AddImpulse(Forward * CurrentLaunchIntensity);
}
