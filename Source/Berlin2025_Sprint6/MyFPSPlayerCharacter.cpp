// MyFPSPlayerCharacter.cpp
#include "MyFPSPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "MyItemCrystal.h" //
#include "MyItemCrystalPedestal.h" //
#include "Components/TimelineComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h" //
#include "InputAction.h"         //
#include "Components/CapsuleComponent.h"

AMyFPSPlayerCharacter::AMyFPSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	FusionRecipeTable = nullptr;
	DefaultCrystalPropertiesTable = nullptr;
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent()); // Attaché à la capsule pour suivre la rotation du corps
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Positionner la caméra (ajuster selon besoin)
	FirstPersonCameraComponent->bUsePawnControlRotation = true; 

	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);         
	FirstPersonMeshComponent->SetupAttachment(FirstPersonCameraComponent); 
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->CastShadow = false;
	// Position des bras à ajuster pour qu'ils soient visibles et alignés avec la caméra
	FirstPersonMeshComponent->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
    FirstPersonMeshComponent->SetRelativeRotation(FRotator(0.f, 0.f, -90.f)); // Ajuster selon le mesh des bras

	// Création des points d'attache pour les cristaux
	RightHandAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightHandAttachPoint"));
	// Attachez-le au FirstPersonMeshComponent si vous avez un socket de main droite, sinon à la caméra.
	// Exemple: RightHandAttachPoint->SetupAttachment(FirstPersonMeshComponent, FName("RightHandSocket"));
	RightHandAttachPoint->SetupAttachment(FirstPersonCameraComponent); // Ou FirstPersonMeshComponent
	RightHandAttachPoint->SetRelativeLocation(FVector(20.0f, 15.0f, -20.0f)); // À ajuster PRÉCISÉMENT pour la main droite

	LeftHandAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandAttachPoint"));
	// Attachez-le au FirstPersonMeshComponent si vous avez un socket de main gauche, sinon à la caméra.
	// Exemple: LeftHandAttachPoint->SetupAttachment(FirstPersonMeshComponent, FName("LeftHandSocket"));
	LeftHandAttachPoint->SetupAttachment(FirstPersonCameraComponent); // Ou FirstPersonMeshComponent
	LeftHandAttachPoint->SetRelativeLocation(FVector(20.0f, -15.0f, -20.0f)); // À ajuster PRÉCISÉMENT pour la main gauche


	LineTraceLength = 500.0f;
	bShouldPerformRaycast = true;
	RightHandCrystal = nullptr;
	LeftHandCrystal = nullptr;
	LookedAtInteractableActor = nullptr;
	
	PlayerInputMappingContext = nullptr;
	// InteractInputAction = nullptr; // L'ancienne action, à déprécier
	InteractRightHandInputAction = nullptr;
	InteractLeftHandInputAction = nullptr;
	FuseCrystalsInputAction = nullptr;

	DropDistance = 100.0f; 
	DropImpulseStrength = 0.0f;
	FusionRecipeHUDWidgetClass = nullptr;
	FusionRecipeHUDInstance = nullptr;
	ToggleRecipesHUDInputAction = nullptr;
	bCanSeeHiddenPlatforms = false;
}

void AMyFPSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			if (PlayerInputMappingContext) //
			{
				Subsystem->AddMappingContext(PlayerInputMappingContext, 0);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AMyFPSPlayerCharacter: PlayerInputMappingContext n'est pas assigné !"));
			}
		}
	}
}

void AMyFPSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldPerformRaycast) //
	{
		PerformLineTrace(); //
	}
}

void AMyFPSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		// Lier les nouvelles actions
		if (InteractRightHandInputAction) //
		{
			EnhancedInputComponent->BindAction(InteractRightHandInputAction, ETriggerEvent::Triggered, this, &AMyFPSPlayerCharacter::HandleInteractRightHand);
		}
		if (InteractLeftHandInputAction) //
		{
			EnhancedInputComponent->BindAction(InteractLeftHandInputAction, ETriggerEvent::Triggered, this, &AMyFPSPlayerCharacter::HandleInteractLeftHand);
		}
		if (FuseCrystalsInputAction) //
		{
			EnhancedInputComponent->BindAction(FuseCrystalsInputAction, ETriggerEvent::Triggered, this, &AMyFPSPlayerCharacter::HandleFuseCrystals);
		}
		if (ToggleRecipesHUDInputAction)
		{
			EnhancedInputComponent->BindAction(ToggleRecipesHUDInputAction, ETriggerEvent::Triggered, this, &AMyFPSPlayerCharacter::HandleToggleRecipesHUD);
		}
		// L'ancien binding pour InteractInputAction peut être supprimé ou commenté
		// if (InteractInputAction)
		// {
		//	EnhancedInputComponent->BindAction(InteractInputAction, ETriggerEvent::Triggered, this, &AMyFPSPlayerCharacter::HandleInteractInput);
		// }
	}
}
void AMyFPSPlayerCharacter::HandleToggleRecipesHUD(const FInputActionValue& Value)
{
	if (!FusionRecipeHUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyFPSPlayerCharacter (%s): FusionRecipeHUDWidgetClass non assigné !"), *GetName());
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	if (FusionRecipeHUDInstance && FusionRecipeHUDInstance->IsVisible())
	{
		FusionRecipeHUDInstance->RemoveFromParent(); // Ou SetVisibility(ESlateVisibility::Collapsed);
		// Restaurer le mode d'input du jeu et cacher la souris
		FInputModeGameOnly GameInputMode;
		PlayerController->SetInputMode(GameInputMode);
		PlayerController->SetShowMouseCursor(false);
	}
	else
	{
		if (!FusionRecipeHUDInstance) // Créer si la première fois
		{
			FusionRecipeHUDInstance = CreateWidget<UMyFusionRecipeHUD>(PlayerController, FusionRecipeHUDWidgetClass);
		}

		if (FusionRecipeHUDInstance)
		{
			FusionRecipeHUDInstance->RefreshRecipeDisplay(); // Mettre à jour les données avant d'afficher
			FusionRecipeHUDInstance->AddToViewport();
			// Changer le mode d'input pour UI et montrer la souris
			FInputModeGameAndUI UIInputMode;
			UIInputMode.SetWidgetToFocus(FusionRecipeHUDInstance->TakeWidget()); // Donner le focus au widget
			UIInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(UIInputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}
void AMyFPSPlayerCharacter::PerformLineTrace() //
{
	FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation(); //
	FVector EndLocation = StartLocation + (FirstPersonCameraComponent->GetForwardVector() * LineTraceLength); //
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this); 

	TScriptInterface<IMyInteractableInterface> NewInteractable = nullptr; //

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UMyInteractableInterface::StaticClass())) //
		{
			NewInteractable = HitActor;
		}
	}
	
	UpdateLookedAtInteractable(NewInteractable); //
}

void AMyFPSPlayerCharacter::UpdateLookedAtInteractable(TScriptInterface<IMyInteractableInterface> NewLookedAt) //
{
	if (LookedAtInteractableActor != NewLookedAt) //
	{
		if (LookedAtInteractableActor) //
		{
			IMyInteractableInterface::Execute_StopLookAtInteractable(LookedAtInteractableActor.GetObject()); //
		}
		LookedAtInteractableActor = NewLookedAt; //
		if (LookedAtInteractableActor) //
		{
			IMyInteractableInterface::Execute_StartLookAtInteractable(LookedAtInteractableActor.GetObject()); //
		}
	}
}

// --- Implémentation des nouvelles fonctions d'inventaire et d'interaction ---

bool AMyFPSPlayerCharacter::PickupCrystalToHand(AMyItemCrystal* CrystalToPickup, TObjectPtr<AMyItemCrystal>& HandCrystalVariable, USceneComponent* AttachPoint)
{
	if (!CrystalToPickup || HandCrystalVariable != nullptr) // Si le cristal est invalide ou si la main est déjà pleine
	{
		return false;
	}

	HandCrystalVariable = CrystalToPickup;

	// Désactiver la physique et la collision du cristal ramassé
	if (HandCrystalVariable->SM_Shape) //
	{
		HandCrystalVariable->SM_Shape->SetSimulatePhysics(false);
		HandCrystalVariable->SM_Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
    
	// Stopper la rotation et la flottaison
	if (HandCrystalVariable->RotatingMovementComponent) //
	{
		HandCrystalVariable->RotatingMovementComponent->Deactivate();
	}
	if (HandCrystalVariable->CrystalTimeline) //
	{
		HandCrystalVariable->CrystalTimeline->Stop();
		if(HandCrystalVariable->SM_Shape)  // // Remettre à Z relatif initial si modifié par flottaison
		{
			FVector InitialRelativeLoc = HandCrystalVariable->SM_Shape->GetRelativeLocation();
            // Ceci suppose que la position de base Z de SM_Shape par rapport à son parent (le root du cristal) est 0
            // ou une valeur connue. Si InitialRelativeLocation était stockée dans AMyItemCrystal, ce serait mieux.
			InitialRelativeLoc.Z = 0; 
			HandCrystalVariable->SM_Shape->SetRelativeLocation(InitialRelativeLoc);
		}
	}

	// Attacher le cristal au point d'attache de la main
	HandCrystalVariable->AttachToComponent(AttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	HandCrystalVariable->SetActorRelativeLocation(FVector::ZeroVector);
	HandCrystalVariable->SetActorRelativeRotation(FRotator::ZeroRotator);
	if(HandCrystalVariable->SM_Shape) HandCrystalVariable->SM_Shape->SetRelativeRotation(FRotator::ZeroRotator); //

	// Assurez-vous que l'outline est désactivé car il est maintenant tenu
    HandCrystalVariable->DisableOutline(); //


	UE_LOG(LogTemp, Log, TEXT("Joueur a ramassé %s dans une main."), *HandCrystalVariable->GetName());
	return true;
}

bool AMyFPSPlayerCharacter::PickupCrystal(AMyItemCrystal* CrystalToPickup)
{
	if (!CrystalToPickup) return false;

	// Essayer de ramasser dans la main droite d'abord, puis la gauche
	if (RightHandCrystal == nullptr)
	{
		return PickupCrystalToHand(CrystalToPickup, RightHandCrystal, RightHandAttachPoint);
	}
	else if (LeftHandCrystal == nullptr)
	{
		return PickupCrystalToHand(CrystalToPickup, LeftHandCrystal, LeftHandAttachPoint);
	}

	UE_LOG(LogTemp, Warning, TEXT("Les deux mains sont pleines, impossible de ramasser %s."), *CrystalToPickup->GetName());
	return false; // Les deux mains sont pleines
}

AMyItemCrystal* AMyFPSPlayerCharacter::DetachCrystalFromHand(TObjectPtr<AMyItemCrystal>& HandCrystalVariable)
{
    if (!HandCrystalVariable)
    {
        return nullptr;
    }
    AMyItemCrystal* CrystalToDetach = HandCrystalVariable;
    // Le détachement de l'acteur sera géré par la fonction appelante (PlaceDown ou Drop)
    // CrystalToDetach->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // Fait dans PlaceDown/Drop

    HandCrystalVariable = nullptr; // Le joueur ne le tient plus dans cette main
    UE_LOG(LogTemp, Log, TEXT("Cristal %s détaché de la main."), *CrystalToDetach->GetName());
    return CrystalToDetach;
}

AMyItemCrystal* AMyFPSPlayerCharacter::PlaceDownCrystalFromHand(bool bIsRightHand)
{
	AMyItemCrystal* CrystalToPlace = nullptr;
	if (bIsRightHand && RightHandCrystal)
	{
		CrystalToPlace = DetachCrystalFromHand(RightHandCrystal);
	}
	else if (!bIsRightHand && LeftHandCrystal)
	{
		CrystalToPlace = DetachCrystalFromHand(LeftHandCrystal);
	}

	if (CrystalToPlace)
	{
		// Le cristal est maintenant "libre" logiquement. Le piédestal ou une autre logique
		// s'occupera de son attachement physique s'il est placé avec succès.
		// Si l'action de placer échoue, il faudra probablement le redropper ou le reprendre.
		// Pour l'instant, on le détache juste. Le piédestal appellera AttachToComponent.
        CrystalToPlace->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		UE_LOG(LogTemp, Log, TEXT("Joueur tente de placer %s."), *CrystalToPlace->GetName());
	}
	return CrystalToPlace;
}

void AMyFPSPlayerCharacter::DropCrystalIntoWorld(AMyItemCrystal* CrystalToDrop)
{
    if (!CrystalToDrop) return;

    CrystalToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation(); //
    FVector CameraForward = FirstPersonCameraComponent->GetForwardVector(); //
    FVector DropLocation = CameraLocation + (CameraForward * DropDistance); //
    FRotator DropRotation = CrystalToDrop->GetActorRotation(); 

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); 
    QueryParams.AddIgnoredActor(CrystalToDrop); 

    if(GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, DropLocation, ECC_WorldStatic, QueryParams))
    {
        DropLocation = HitResult.ImpactPoint + (CameraForward * 5.0f); // Un petit offset pour ne pas dropper dans le sol
    }
    
    CrystalToDrop->SetActorLocationAndRotation(DropLocation, DropRotation);

    if (CrystalToDrop->SM_Shape) //
    {
        CrystalToDrop->SM_Shape->SetSimulatePhysics(true);
        CrystalToDrop->SM_Shape->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
        if(DropImpulseStrength > 0.0f && CrystalToDrop->SM_Shape->IsAnySimulatingPhysics()) //
        {
            CrystalToDrop->SM_Shape->AddImpulse(CameraForward * DropImpulseStrength, NAME_None, true); //
        }
    }
    // Réactiver la rotation et la flottaison si souhaité quand l'objet est au sol
    if (CrystalToDrop->RotatingMovementComponent) //
	{
		CrystalToDrop->RotatingMovementComponent->Activate(true);
	}
	if (CrystalToDrop->CrystalTimeline && CrystalToDrop->FloatationCurve) //
	{
        // S'assurer que SM_Shape a sa position Z relative correcte avant de redémarrer la flottaison
        if(CrystalToDrop->SM_Shape) //
        {
            // FVector RelativeLoc = CrystalToDrop->SM_Shape->GetRelativeLocation();
            // RelativeLoc.Z = 0; // Ou la valeur de base stockée dans InitialRelativeLocation de AMyItemCrystal
            // CrystalToDrop->SM_Shape->SetRelativeLocation(RelativeLoc);
            // NOTE: InitialRelativeLocation doit être correctement initialisée dans AMyItemCrystal::BeginPlay et utilisée ici.
        }
		CrystalToDrop->CrystalTimeline->PlayFromStart();
	}
    CrystalToDrop->DisableOutline(); //

    UE_LOG(LogTemp, Log, TEXT("Joueur a lâché %s dans le monde."), *CrystalToDrop->GetName());
}


void AMyFPSPlayerCharacter::DropCrystalFromHand(bool bIsRightHand)
{
	AMyItemCrystal* CrystalToDrop = nullptr;
	if (bIsRightHand && RightHandCrystal)
	{
		CrystalToDrop = DetachCrystalFromHand(RightHandCrystal);
	}
	else if (!bIsRightHand && LeftHandCrystal)
	{
		CrystalToDrop = DetachCrystalFromHand(LeftHandCrystal);
	}

	if (CrystalToDrop)
	{
		DropCrystalIntoWorld(CrystalToDrop);
	}
}


// --- Fonctions Handler pour les nouvelles Inputs ---

void AMyFPSPlayerCharacter::HandleInteractRightHand(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Interaction Main Droite Pressée"));
	if (LookedAtInteractableActor) //
	{
		// Si on regarde un piédestal ET qu'on tient un cristal dans la main droite
		AMyItemCrystalPedestal* LookedAtPedestal = Cast<AMyItemCrystalPedestal>(LookedAtInteractableActor.GetObject());
		if (LookedAtPedestal && RightHandCrystal)
		{
			AMyItemCrystal* CrystalFromHand = PlaceDownCrystalFromHand(true); // true pour droite
			if (CrystalFromHand)
			{
				if (LookedAtPedestal->TryPlaceCrystal(CrystalFromHand)) //
				{
					UE_LOG(LogTemp, Log, TEXT("Joueur a placé %s de la main droite sur %s"), *CrystalFromHand->GetName(), *LookedAtPedestal->GetName());
				}
				else
				{
					// Le piédestal a refusé, reprendre le cristal (ou le dropper)
					UE_LOG(LogTemp, Warning, TEXT("Placement de %s (main droite) sur %s refusé. Reprise auto."), *CrystalFromHand->GetName(), *LookedAtPedestal->GetName());
					PickupCrystalToHand(CrystalFromHand, RightHandCrystal, RightHandAttachPoint); // Reprise dans la même main
				}
			}
		}
		// Si on regarde un cristal ramassable ET que la main droite est vide
		else if (Cast<AMyItemCrystal>(LookedAtInteractableActor.GetObject()) && !RightHandCrystal)
		{
			PickupCrystalToHand(Cast<AMyItemCrystal>(LookedAtInteractableActor.GetObject()), RightHandCrystal, RightHandAttachPoint);
		}
		// Si on regarde un piédestal qui a un cristal dessus ET que la main droite est vide
		else if (LookedAtPedestal && LookedAtPedestal->bIsCrystalPlaced && !RightHandCrystal) //
		{
			 IMyInteractableInterface::Execute_Interact(LookedAtInteractableActor.GetObject(), this); // Laisse le piédestal gérer la reprise
		}
		// Autres cas d'interaction avec la main droite ?
	}
	else if (RightHandCrystal) // Si on ne regarde rien d'interactif mais qu'on tient un cristal main droite
	{
		DropCrystalFromHand(true); // Lâcher le cristal de la main droite
	}
}

void AMyFPSPlayerCharacter::HandleInteractLeftHand(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Interaction Main Gauche Pressée"));
	// Logique similaire à HandleInteractRightHand, mais pour LeftHandCrystal et LeftHandAttachPoint
	if (LookedAtInteractableActor) //
	{
		AMyItemCrystalPedestal* LookedAtPedestal = Cast<AMyItemCrystalPedestal>(LookedAtInteractableActor.GetObject());
		if (LookedAtPedestal && LeftHandCrystal)
		{
			AMyItemCrystal* CrystalFromHand = PlaceDownCrystalFromHand(false); // false pour gauche
			if (CrystalFromHand)
			{
				if (LookedAtPedestal->TryPlaceCrystal(CrystalFromHand)) //
				{
					UE_LOG(LogTemp, Log, TEXT("Joueur a placé %s de la main gauche sur %s"), *CrystalFromHand->GetName(), *LookedAtPedestal->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Placement de %s (main gauche) sur %s refusé. Reprise auto."), *CrystalFromHand->GetName(), *LookedAtPedestal->GetName());
					PickupCrystalToHand(CrystalFromHand, LeftHandCrystal, LeftHandAttachPoint); // Reprise dans la même main
				}
			}
		}
		else if (Cast<AMyItemCrystal>(LookedAtInteractableActor.GetObject()) && !LeftHandCrystal)
		{
			PickupCrystalToHand(Cast<AMyItemCrystal>(LookedAtInteractableActor.GetObject()), LeftHandCrystal, LeftHandAttachPoint);
		}
		else if (LookedAtPedestal && LookedAtPedestal->bIsCrystalPlaced && !LeftHandCrystal) //
		{
			 IMyInteractableInterface::Execute_Interact(LookedAtInteractableActor.GetObject(), this); // Laisse le piédestal gérer la reprise
		}
	}
	else if (LeftHandCrystal) 
	{
		DropCrystalFromHand(false); 
	}
}
void AMyFPSPlayerCharacter::HandleFuseCrystals(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Log, TEXT("Touche Fusion Pressée"));
    if (RightHandCrystal && LeftHandCrystal) //
    {
        if (!FusionRecipeTable)
        {
            UE_LOG(LogTemp, Error, TEXT("AMyFPSPlayerCharacter (%s): FusionRecipeTable n'est pas assigné !"), *GetName());
            return;
        }

        ECrystalType TypeMainDroite = RightHandCrystal->GetCrystalType(); //
        ECrystalType TypeMainGauche = LeftHandCrystal->GetCrystalType(); //
        bool bRecipeFound = false;
        const FFusionRecipeRow* FoundRecipeData = nullptr; // Pour stocker la recette trouvée

        // Itérer sur toutes les lignes de la DataTable
        FString DataTableContextString(TEXT("FusionRecipeLookup"));
        TArray<FName> RowNames = FusionRecipeTable->GetRowNames();

        for (const FName& RowName : RowNames)
        {
            const FFusionRecipeRow* CurrentRecipe = FusionRecipeTable->FindRow<FFusionRecipeRow>(RowName, DataTableContextString);
            if (CurrentRecipe)
            {
                // Vérifier la recette dans les deux sens (ex: A+B ou B+A)
                if ((CurrentRecipe->InputCrystalType1 == TypeMainDroite && CurrentRecipe->InputCrystalType2 == TypeMainGauche) ||
                    (CurrentRecipe->InputCrystalType1 == TypeMainGauche && CurrentRecipe->InputCrystalType2 == TypeMainDroite))
                {
                    FoundRecipeData = CurrentRecipe;
                    bRecipeFound = true;
                    break; // Recette trouvée, on arrête de chercher
                }
            }
        }

        if (bRecipeFound && FoundRecipeData)
        {
            UE_LOG(LogTemp, Log, TEXT("Recette valide trouvée dans FusionRecipeTable! (%s + %s -> %s). Compétence potentielle: %s"),
                *UEnum::GetValueAsString(TypeMainDroite),
                *UEnum::GetValueAsString(TypeMainGauche),
                *UEnum::GetValueAsString(FoundRecipeData->OutputFusedCrystalType),
                *UEnum::GetValueAsString(FoundRecipeData->SkillGrantedByOutputCrystal));

            FVector SpawnLocation = (RightHandAttachPoint->GetComponentLocation() + LeftHandAttachPoint->GetComponentLocation()) / 2.0f; //
            FRotator SpawnRotation = GetActorRotation();

            // Détruire les cristaux originaux
            RightHandCrystal->Destroy();
            LeftHandCrystal->Destroy();
            RightHandCrystal = nullptr; //
            LeftHandCrystal = nullptr; //

            // Créer le nouveau cristal fusionné
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            AMyItemCrystal* NewFusedCrystal = GetWorld()->SpawnActor<AMyItemCrystal>(AMyItemCrystal::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams); //

        	if (NewFusedCrystal)
        	{
        		NewFusedCrystal->CrystalType = FoundRecipeData->OutputFusedCrystalType;
        		NewFusedCrystal->PotentialSkillToGrant = FoundRecipeData->SkillGrantedByOutputCrystal;

        		// --- NOUVELLE PARTIE : Assignation du CrystalPropertiesTable ---
        		if (DefaultCrystalPropertiesTable)
        		{
        			NewFusedCrystal->CrystalPropertiesTable = DefaultCrystalPropertiesTable; //
        		}
        		else
        		{
        			UE_LOG(LogTemp, Warning, TEXT("AMyFPSPlayerCharacter (%s): DefaultCrystalPropertiesTable non assigné. Le cristal fusionné %s risque de ne pas s'initialiser correctement."), *GetName(), *NewFusedCrystal->GetName());
        		}
        		// Optionnellement, faire de même pour FloatationCurve si c'est une propriété globale
        		// if (DefaultFloatationCurve) { NewFusedCrystal->FloatationCurve = DefaultFloatationCurve; }


        		// Maintenant, SetupCrystalFromDataTable (appelé dans BeginPlay du cristal, ou manuellement si besoin)
        		// devrait pouvoir trouver la table. Si BeginPlay a déjà eu lieu pour NewFusedCrystal
        		// (ce qui est le cas après SpawnActor), un appel manuel est plus sûr.
        		NewFusedCrystal->SetupCrystalFromDataTable(); //
        		// Vous devrez peut-être aussi appeler manuellement InitializeFloatationTimeline et InitializeAmbienceSystem
        		// si elles dépendent de propriétés chargées par SetupCrystalFromDataTable et que BeginPlay est passé.
        		// Alternativement, créez une fonction AMyItemCrystal::InitializeAfterSpawn(ECrystalType NewType, ESkillType NewSkill, UDataTable* PropertiesTable, UCurveFloat* Curve)

        		UE_LOG(LogTemp, Log, TEXT("Cristal fusionné %s créé (Type: %s, Compétence: %s)."),
					   *NewFusedCrystal->GetName(),
					   *UEnum::GetValueAsString(NewFusedCrystal->CrystalType),
					   *UEnum::GetValueAsString(NewFusedCrystal->PotentialSkillToGrant));

        		PickupCrystalToHand(NewFusedCrystal, RightHandCrystal, RightHandAttachPoint);
        	}
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Échec de la création du cristal fusionné après avoir trouvé une recette."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Aucune recette de fusion valide trouvée pour %s et %s."), *UEnum::GetValueAsString(TypeMainDroite), *UEnum::GetValueAsString(TypeMainGauche));
            // TODO: Jouer un son d'échec de fusion
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Pas assez de cristaux pour fusionner (besoin de 2). MainD: %s, MainG: %s"),
            RightHandCrystal ? *RightHandCrystal->GetName() : TEXT("Vide"),
            LeftHandCrystal ? *LeftHandCrystal->GetName() : TEXT("Vide"));
        // TODO: Jouer un son "besoin de deux cristaux"
    }
}

void AMyFPSPlayerCharacter::UnlockSkill(ESkillType SkillToUnlock)
{
    bool bPreviouslyUnlocked = false;
    bool bSkillStateChanged = false; // Pour savoir si on doit broadcaster
    bool bNewVisibilityState = bCanSeeHiddenPlatforms; //
    FString SkillNameAsString = UEnum::GetValueAsString(SkillToUnlock);

    switch (SkillToUnlock)
    {
        case ESkillType::None:
            return;

        case ESkillType::CanSeeHiddenPlatforms:
            bPreviouslyUnlocked = bCanSeeHiddenPlatforms; //
            if (!bPreviouslyUnlocked) { // Débloque seulement si ce n'est pas déjà le cas
                bCanSeeHiddenPlatforms = true; //
                bSkillStateChanged = true;
                bNewVisibilityState = true;
            }
            break;
        
            // ... logique pour bCanEmitEmotionalPulse ...
            // Si cette compétence affecte aussi la visibilité des plateformes (improbable), ajustez bSkillStateChanged et bNewVisibilityState
        default:
            UE_LOG(LogTemp, Warning, TEXT("AMyFPSPlayerCharacter (%s): Tentative de débloquer une compétence non gérée: %s"), *GetName(), *SkillNameAsString);
            return;
    }

    if (bSkillStateChanged) // Si l'état spécifique de CanSeeHiddenPlatforms a changé
    {
        OnSkillSeeHiddenPlatformsChanged.Broadcast(bNewVisibilityState);
    }

    // Logique de feedback pour n'importe quelle compétence débloquée
    if ((SkillToUnlock == ESkillType::CanSeeHiddenPlatforms && !bPreviouslyUnlocked) || 
        (SkillToUnlock != ESkillType::CanSeeHiddenPlatforms /*&& la compétence n'était pas déjà débloquée*/)) // Adaptez pour les autres booléens de compétence
    {
        UE_LOG(LogTemp, Log, TEXT("AMyFPSPlayerCharacter (%s): Compétence '%s' débloquée !"), *GetName(), *SkillNameAsString);
        // TODO: Ajouter un feedback visuel/sonore/UI pour le déblocage de compétence
    }
    else if (bPreviouslyUnlocked && SkillToUnlock == ESkillType::CanSeeHiddenPlatforms)
    {
        UE_LOG(LogTemp, Log, TEXT("AMyFPSPlayerCharacter (%s): Compétence '%s' était déjà débloquée."), *GetName(), *SkillNameAsString);
    }
}