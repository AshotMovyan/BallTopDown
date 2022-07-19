// Fill out your copyright notice in the Description page of Project Settings.


#include "..//Pawns/MyPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "..//TopDownGameGameModeBase.h"
#include "..//Actors/Wall.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#define print(String) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT(String))


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
	MaxTimeLaunchVelocty = 0.6f;
}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	if (SphereColl)
	{
		SphereColl->OnComponentHit.AddDynamic(this, &AMyPlayer::OnHit);
	}

	MyMode = Cast<ATopDownGameGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

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
	
	if (able_to_input())
	{
		print("aaaaaaaaaa");
		PlayerInputComponent->BindAxis("TurnAtRate", this, &AMyPlayer::TurnAtRate);
		PlayerInputComponent->BindAction("Launch", IE_Pressed, this, &AMyPlayer::LaunchPressed);
		PlayerInputComponent->BindAction("Launch", IE_Released, this, &AMyPlayer::LaunchReleased);
	}


}

void AMyPlayer::FellOutOfWorld(const UDamageType& DmgType)
{
	if (MyMode)
	{
		Destroy();
		MyMode->RestartPlayer(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}
}


void AMyPlayer::TurnAtRate(float value)
{
	if (able_to_input())
	{
		CurrentRotation = FRotator(0.0f, value * RotateSpeed, 0.0f);
	}
}

void AMyPlayer::LaunchPressed()
{
	if (able_to_input())
	{
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AMyPlayer::LaunchReleased, MaxTimeLaunchVelocty, true);
	}
}

void AMyPlayer::LaunchReleased()
{
	if (able_to_input())
	{
		CurrentLaunchIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, MaxTimeLaunchVelocty), FVector2D(0.0f, MaxLaunchIntensity),
			GetWorldTimerManager().GetTimerElapsed(TimerHandle));

		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::SanitizeFloat(CurrentLaunchIntensity));

		GetWorldTimerManager().ClearTimer(TimerHandle);

		CurrentForward = Arrow->GetForwardVector();
		SphereColl->AddImpulse(CurrentForward * CurrentLaunchIntensity);
	}
}

void AMyPlayer::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AWall* Wall = Cast<AWall>(OtherActor);
	if (Wall)
	{
		FRotator MirrorVectorByNormalRotation = UKismetMathLibrary::FindLookAtRotation(Hit.TraceEnd, CurrentForward -	//	Get mirror vector and make rotation from it used mirror formule.
			((UKismetMathLibrary::Dot_VectorVector(Hit.ImpactNormal, CurrentForward)) * 2 * Hit.ImpactNormal));			//	( r = d − 2n*(d⋅n) ) Where r-mirror vector, n-hit normal, d-current vector. 
		Arrow->SetRelativeRotation(FRotator(Arrow->GetRelativeRotation().Pitch, MirrorVectorByNormalRotation.Yaw, Arrow->GetRelativeRotation().Roll));
		CurrentForward = Arrow->GetForwardVector();
	}
}

void AMyPlayer::RootComponentSimulatePhysics(bool IsSimulate)
{
	SphereColl->SetSimulatePhysics(IsSimulate);
}

bool AMyPlayer::able_to_input()
{
	bool can = !IsAttacks;
	return can;
}
