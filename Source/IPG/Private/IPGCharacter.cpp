// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPGCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/IPGCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "IPG.h"
#include "AbilitySystemComponent.h"
#include "Player/IPGPlayerExtensionComponent.h"
#include "Player/IPGPreCMCTickComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#if UE_WITH_IRIS
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"   
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"   
#include "Iris/ReplicationSystem/ReplicationSystem.h"           
#endif

AIPGCharacter::AIPGCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UIPGCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	PlayerExtensionComponent = CreateDefaultSubobject<UIPGPlayerExtensionComponent>(TEXT("PlayerExtensionComponent"));
	PlayerExtensionComponent->RegisterAndCallWhenAbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &AIPGCharacter::OnAbilitySystemInitialized));
	PlayerExtensionComponent->RegisterWhenAbilitySystemUninitialized(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &AIPGCharacter::OnAbilitySystemUninitialized));

	PreCMCTickComponent = CreateDefaultSubobject<UIPGPreCMCTickComponent>(TEXT("PreCMCTickComponent"));
}

FIPGCharacterPropertiesForAnimation AIPGCharacter::GetPropertiesForAnimation_Implementation() const
{
	FIPGCharacterPropertiesForAnimation CharacterPropertiesForAnimation;
	
	UCharacterMovementComponent* CharMovement = GetCharacterMovement(); 
	if (!IsValid(CharMovement))
	{
		UE_LOG(LogTemp, Error, TEXT("Charcter movement component is not valid in AIPGCharacter::GetPropertiesForAnimation()"));
		return CharacterPropertiesForAnimation;
	}

	// Movement Mode
	EIPGMovementMode MovementMode;

	switch (CharMovement->MovementMode)
	{
	case EMovementMode::MOVE_None:
		MovementMode = EIPGMovementMode::OnGround;
		break;
	case EMovementMode::MOVE_Custom:
		MovementMode = EIPGMovementMode::OnGround;
		break;
	case EMovementMode::MOVE_Falling:
		MovementMode = EIPGMovementMode::InAir;
		break;
	case EMovementMode::MOVE_Flying:
		MovementMode = EIPGMovementMode::OnGround;
		break;
	case EMovementMode::MOVE_NavWalking:
		MovementMode = EIPGMovementMode::OnGround;
		break;
	case EMovementMode::MOVE_Swimming:
		MovementMode = EIPGMovementMode::InAir;
		break;
	case EMovementMode::MOVE_Walking:
		MovementMode = EIPGMovementMode::OnGround;
		break;
	default:
		break;

	}

	CharacterPropertiesForAnimation.InputState = CharacterInputState;
	CharacterPropertiesForAnimation.MovementMode = MovementMode;
	CharacterPropertiesForAnimation.Stance = CharMovement->IsCrouching() ? EIPGAnimStance::Crouch : EIPGAnimStance::Stand;
	CharacterPropertiesForAnimation.RotationMode = CharMovement->bOrientRotationToMovement ? EIPGRotationMode::OrientationToMovement : EIPGRotationMode::Strafe;
	CharacterPropertiesForAnimation.Gait = Gait;
	CharacterPropertiesForAnimation.ActorTransform = GetActorTransform(); 
	CharacterPropertiesForAnimation.Velocity = CharMovement->Velocity;
	CharacterPropertiesForAnimation.InputAcceleration = CharMovement->GetCurrentAcceleration(); 
	CharacterPropertiesForAnimation.CurrentMaxAcceleration = CharMovement->GetMaxAcceleration(); 
	CharacterPropertiesForAnimation.CurrentMaxDeceleration = CharMovement->BrakingDecelerationWalking;
	CharacterPropertiesForAnimation.OrientationIntent = GetActorRotation(); 
	CharacterPropertiesForAnimation.AimingRotation = IsLocallyControlled() ? GetControlRotation() : GetBaseAimRotation();
	CharacterPropertiesForAnimation.bJustLanded = bJustLanded;
	CharacterPropertiesForAnimation.LandVelocity = LandVelocity;
	CharacterPropertiesForAnimation.GroundNormal = CharMovement->CurrentFloor.HitResult.ImpactNormal;

	return CharacterPropertiesForAnimation;
}

void AIPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIPGCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AIPGCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIPGCharacter::Look);
	}
	else
	{
		UE_LOG(LogIPG, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	PlayerExtensionComponent->SetupPlayerInputComponent();
}

void AIPGCharacter::BeginPlay()
{
	Super::BeginPlay(); 

	PreCMCTickComponent->OnIPGPreCMCTickSignature.AddUObject(this, &AIPGCharacter::HandlePreCMCTick);

#if UE_WITH_IRIS
	if (!HasAuthority())
	{
		return;
	}

	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(this);
	UEngineReplicationBridge* ReplicationBridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(this);

	if (!ReplicationSystem || !ReplicationBridge)
	{
		return;
	}

	UE::Net::FNetRefHandle Handle = ReplicationBridge->GetReplicatedRefHandle(this);

	UE::Net::FNetObjectPrioritizerHandle PrioritizerHandle = ReplicationSystem->GetPrioritizerHandle(FName("FoVPrioritizer"));

	if (!ReplicationSystem->SetPrioritizer(Handle, PrioritizerHandle))
	{
		ReplicationSystem->SetStaticPriority(Handle, 1.0f);
	}
#endif
}

void AIPGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	// Iris Push Model
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(AIPGCharacter, CharacterInputState, Params);
}

void AIPGCharacter::HandlePreCMCTick()
{
	UpdateRotationPreCMC();

	UpdateMovementPreCMC();
}

void AIPGCharacter::UpdateRotationPreCMC()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!IsValid(CMC))
	{
		return;
	}

	// Wants to "Strafe" or "Aim"
	bool bWantsToStrafeOrAim = CharacterInputState.bWantsToStrafe || CharacterInputState.bWantsToAim;
	bool bUseControllerDesiredRotation = bWantsToStrafeOrAim ? true : false;
	bool bOrientRotationToMovement = bWantsToStrafeOrAim ? false : true;

	CMC->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
	CMC->bOrientRotationToMovement = bOrientRotationToMovement;

	// Falling
	FRotator RotationRate = CMC->IsFalling() ? FRotator(0.f, 0.f, 200.f) : FRotator(0.f, 0.f, -1.f);
	CMC->RotationRate = RotationRate;
}

void AIPGCharacter::UpdateMovementPreCMC()
{
	Gait = GetDesiredGait(); 

	// TODO 
}

EIPGGait AIPGCharacter::GetDesiredGait() const
{
	// TODO
	return EIPGGait::Walk;
}

void AIPGCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AIPGCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AIPGCharacter::OnAbilitySystemInitialized()
{
	// TODO
}

void AIPGCharacter::OnAbilitySystemUninitialized()
{
	// TODO
}

void AIPGCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AIPGCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AIPGCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AIPGCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

UAbilitySystemComponent* AIPGCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
