// MyItemCrystalPedestal.cpp
#include "MyItemCrystalPedestal.h"
#include "MyItemCrystal.h"
#include "MyFPSPlayerCharacter.h" // Inclure pour le cast
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h" // Si vous l'utilisez activement
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/RotatingMovementComponent.h" // Pour désactiver la rotation du cristal
#include "Components/TimelineComponent.h" // Pour désactiver la flottaison du cristal


AMyItemCrystalPedestal::AMyItemCrystalPedestal()
{
	// Le SM_Shape hérité de AMyMasterItem sera notre PedestalMeshComponent par défaut.
	// PedestalMeshComponent est assigné dans AMyMasterItem (SM_Shape)
	// Si vous avez créé un PedestalMeshComponent séparé ici, assurez-vous qu'il est bien celui que vous voulez utiliser.
	// Pour cet exemple, nous utilisons SM_Shape comme PedestalMeshComponent.
	PedestalMeshComponent = SM_Shape; 

	CrystalAttachmentPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CrystalAttachmentPoint"));
	if (PedestalMeshComponent) // S'assurer que PedestalMeshComponent est valide avant d'attacher
	{
		CrystalAttachmentPointComponent->SetupAttachment(PedestalMeshComponent);
	} else {
		CrystalAttachmentPointComponent->SetupAttachment(RootComponent); // Fallback au root si SM_Shape n'est pas encore là
	}


	PedestalLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PedestalLight"));
	if (PedestalMeshComponent)
	{
		PedestalLightComponent->SetupAttachment(PedestalMeshComponent);
	} else {
		PedestalLightComponent->SetupAttachment(RootComponent);
	}


	ActivatedStateMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActivatedStateMesh"));
	if (PedestalMeshComponent)
	{
		ActivatedStateMeshComponent->SetupAttachment(PedestalMeshComponent);
	} else {
		ActivatedStateMeshComponent->SetupAttachment(RootComponent);
	}
	ActivatedStateMeshComponent->SetVisibility(true); // Caché par défaut
	ActivatedStateMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	// Optionnel: InteractionTriggerSphere (si vous l'utilisez pour autre chose que le line trace du joueur)
	InteractionTriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionTriggerSphere"));
	if (PedestalMeshComponent)
	{
		InteractionTriggerSphere->SetupAttachment(PedestalMeshComponent);
	} else {
		InteractionTriggerSphere->SetupAttachment(RootComponent);
	}

	InteractionTriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

	// Valeurs par défaut
	ExpectedCrystalType = ECrystalType::Joy; // Exemple
	ExpectedTypeHintColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f); // Bleu clair par défaut pour "hint"
	CorrectCrystalPlacedColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Vert pour "correct"
	IncorrectCrystalPlacedColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Rouge pour "incorrect"
	
	bIsCrystalPlaced = false;
	bIsCorrectCrystalOnPedestal = false;
	CurrentCrystalOnPedestal = nullptr;
	SuccessSound = nullptr;
	FailSound = nullptr;
	ActivationVFX = nullptr;
	DeactivationVFX = nullptr;
}

void AMyItemCrystalPedestal::BeginPlay()
{
	Super::BeginPlay();
	// Assurer l'état initial
	bIsCrystalPlaced = false;
	bIsCorrectCrystalOnPedestal = false;
	CurrentCrystalOnPedestal = nullptr;
	UpdatePedestalVisuals();
	
}
void AMyItemCrystalPedestal::Interact_Implementation(AActor* InteractorActor)
{
	Super::Interact_Implementation(InteractorActor); // Appeler l'implémentation parente si elle fait quelque chose

	AMyFPSPlayerCharacter* PlayerCharacter = Cast<AMyFPSPlayerCharacter>(InteractorActor);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystalPedestal::Interact_Implementation: InteractorActor n'est pas un AMyFPSPlayerCharacter."));
		return;
	}

	if (bIsCrystalPlaced) // Si un cristal est déjà sur le piédestal
	{
		// Ce cas est maintenant spécifiquement lorsque le joueur interagit (probablement avec une main vide,
		// comme géré par AMyFPSPlayerCharacter::HandleInteractRightHand/LeftHand) pour PRENDRE le cristal.
		// Nous vérifions si le joueur PEUT ramasser (a au moins une main de libre).
		if (!PlayerCharacter->GetRightHandCrystal() || !PlayerCharacter->GetLeftHandCrystal()) //
		{
			AMyItemCrystal* CrystalTaken = RemoveCrystal();
			if (CrystalTaken)
			{
				// PlayerCharacter->PickupCrystal s'occupera de le placer dans une main disponible.
				PlayerCharacter->PickupCrystal(CrystalTaken); //
				UE_LOG(LogTemp, Log, TEXT("Joueur %s a repris %s du piédestal %s"), *PlayerCharacter->GetName(), *CrystalTaken->GetName(), *GetName());
			}
		}
		else
		{
			// Les deux mains du joueur sont pleines, et il a essayé d'interagir avec un piédestal occupé.
			UE_LOG(LogTemp, Log, TEXT("Piédestal %s est occupé. Le joueur %s a les deux mains pleines."), *GetName(), *PlayerCharacter->GetName());
			if (FailSound) //
			{
				UGameplayStatics::PlaySoundAtLocation(this, FailSound, GetActorLocation());
			}
		}
	}
	else // Si le piédestal est vide
	{
		// Ce cas (placer un cristal sur un piédestal vide) devrait maintenant être principalement géré par
		// AMyFPSPlayerCharacter::HandleInteractRightHand/LeftHand qui appellent directement TryPlaceCrystal sur ce piédestal.
		// Si Interact_Implementation est appelée ici, c'est probablement une interaction qui n'implique pas le placement d'un cristal
		// (ex: le joueur interagit avec un piédestal vide avec les mains vides) ou un cas non géré.

		// On affiche ce que le joueur tient pour information.
		FString RightHandCrystalName = PlayerCharacter->GetRightHandCrystal() ? PlayerCharacter->GetRightHandCrystal()->GetName() : TEXT("rien"); //
		FString LeftHandCrystalName = PlayerCharacter->GetLeftHandCrystal() ? PlayerCharacter->GetLeftHandCrystal()->GetName() : TEXT("rien"); //

		UE_LOG(LogTemp, Log, TEXT("Piédestal %s est vide. Joueur tient : Main D (%s), Main G (%s)."), *GetName(), *RightHandCrystalName, *LeftHandCrystalName);
		
		// Si le joueur a interagi avec un piédestal vide ET qu'il n'a rien dans les mains, on peut jouer un son d'échec/vide.
		if (FailSound && !PlayerCharacter->GetRightHandCrystal() && !PlayerCharacter->GetLeftHandCrystal()) //
		{
			 UGameplayStatics::PlaySoundAtLocation(this, FailSound, GetActorLocation());
		}
	}
}

bool AMyItemCrystalPedestal::TryPlaceCrystal(AMyItemCrystal* CrystalToPlace)
{
	if (bIsCrystalPlaced || !CrystalToPlace)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryPlaceCrystal: Impossible de placer. Piédestal occupé (%s) ou cristal invalide."), bIsCrystalPlaced ? TEXT("true") : TEXT("false"));
		return false;
	}

	CurrentCrystalOnPedestal = CrystalToPlace;
	bIsCrystalPlaced = true;

	// Attacher le cristal au piédestal
	// Le joueur a déjà appelé PlaceDownCrystal, donc le cristal est "libre" logiquement.
	// Nous nous assurons qu'il est détaché de tout parent précédent avant de l'attacher ici.
	CrystalToPlace->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CrystalToPlace->AttachToComponent(CrystalAttachmentPointComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	CrystalToPlace->SetActorRelativeLocation(FVector::ZeroVector);
	CrystalToPlace->SetActorRelativeRotation(FRotator::ZeroRotator);

	// Désactiver la physique/collision et mouvement du cristal puisqu'il est sur le piédestal
	if (CrystalToPlace->SM_Shape)
	{
		CrystalToPlace->SM_Shape->SetSimulatePhysics(false);
		CrystalToPlace->SM_Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Ou QueryOnly si besoin pour le line trace futur
	}
	if (CrystalToPlace->RotatingMovementComponent)
	{
		CrystalToPlace->RotatingMovementComponent->Deactivate();
	}
	if (CrystalToPlace->CrystalTimeline)
	{
		CrystalToPlace->CrystalTimeline->Stop();
        if(CrystalToPlace->SM_Shape)
        {
            FVector RelativeLoc = CrystalToPlace->SM_Shape->GetRelativeLocation();
            RelativeLoc.Z = 0; 
            CrystalToPlace->SM_Shape->SetRelativeLocation(RelativeLoc);
        }
	}
  
    CrystalToPlace->DisableOutline();


	HandleCrystalPlaced(CrystalToPlace);
	return true;
}

AMyItemCrystal* AMyItemCrystalPedestal::RemoveCrystal()
{
	if (!bIsCrystalPlaced || !CurrentCrystalOnPedestal)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveCrystal: Rien à retirer. Piédestal vide (%s) ou cristal invalide."), bIsCrystalPlaced ? TEXT("false") : TEXT("true"));
		return nullptr;
	}

	AMyItemCrystal* CrystalToRemove = CurrentCrystalOnPedestal;

	// Préparer le cristal à être repris (sera géré par PickupCrystal du joueur)
	// Le détachement se fait ici.
	CrystalToRemove->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	// La réactivation de la physique/collision/mouvement sera faite par PickupCrystal du joueur
    // ou si le cristal est juste "droppé" dans le monde.

	CurrentCrystalOnPedestal = nullptr; // Important : faire ceci *avant* HandleCrystalRemoved si HandleCrystalRemoved y accède
	bIsCrystalPlaced = false;
	// bIsCorrectCrystalOnPedestal sera mis à false dans HandleCrystalRemoved

	HandleCrystalRemoved();

	return CrystalToRemove;
}
void AMyItemCrystalPedestal::HandleCrystalPlaced(AMyItemCrystal* PlacedCrystal) //
{
    if (!PlacedCrystal) return;

    bIsCorrectCrystalOnPedestal = (PlacedCrystal->GetCrystalType() == ExpectedCrystalType); //
    UpdatePedestalVisuals(); //

    if (bIsCorrectCrystalOnPedestal)
    {
        UE_LOG(LogTemp, Log, TEXT("Piédestal %s: Cristal CORRECT (%s) placé."), *GetName(), *UEnum::GetValueAsString(PlacedCrystal->GetCrystalType())); //
        if (SuccessSound) UGameplayStatics::PlaySoundAtLocation(this, SuccessSound, GetActorLocation()); //
        if (ActivationVFX) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ActivationVFX, CrystalAttachmentPointComponent->GetComponentLocation(), CrystalAttachmentPointComponent->GetComponentRotation()); //

        // --- NOUVELLE LOGIQUE DE DÉBLOCAGE DE COMPÉTENCE ---
        if (PlacedCrystal->PotentialSkillToGrant != ESkillType::None)
        {
            // Essayer de récupérer le personnage joueur qui a interagi (ou le joueur 0 par défaut)
            AMyFPSPlayerCharacter* PlayerCharacter = Cast<AMyFPSPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
            if (PlayerCharacter)
            {
                PlayerCharacter->UnlockSkill(PlacedCrystal->PotentialSkillToGrant);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Piédestal %s: Impossible de trouver AMyFPSPlayerCharacter pour débloquer la compétence %s."),
                       *GetName(), *UEnum::GetValueAsString(PlacedCrystal->PotentialSkillToGrant));
            }
        }
        // --- FIN DE LA NOUVELLE LOGIQUE ---
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Piédestal %s: Cristal INCORRECT (%s) placé. Attendu: %s."), *GetName(), *UEnum::GetValueAsString(PlacedCrystal->GetCrystalType()), *UEnum::GetValueAsString(ExpectedCrystalType)); //
        if (FailSound) UGameplayStatics::PlaySoundAtLocation(this, FailSound, GetActorLocation()); //
    }
    OnPedestalStateChanged.Broadcast(bIsCorrectCrystalOnPedestal); //
}	
void AMyItemCrystalPedestal::HandleCrystalRemoved()
{
	bIsCorrectCrystalOnPedestal = false; 
	UpdatePedestalVisuals();

	if (DeactivationVFX) 
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeactivationVFX, CrystalAttachmentPointComponent->GetComponentLocation(), CrystalAttachmentPointComponent->GetComponentRotation());
	}
	// UE_LOG(LogTemp, Log, TEXT("Piédestal %s: Cristal retiré."), *GetName());
	OnPedestalStateChanged.Broadcast(false); // Toujours false car le piédestal est maintenant vide ou incorrect
}


void AMyItemCrystalPedestal::UpdatePedestalVisuals()
{
	if (PedestalLightComponent)
	{
		if (!bIsCrystalPlaced) // Aucun cristal sur le piédestal
		{
			PedestalLightComponent->SetLightColor(ExpectedTypeHintColor);
		}
		else // Un cristal est placé
		{
			if (bIsCorrectCrystalOnPedestal)
			{
				PedestalLightComponent->SetLightColor(CorrectCrystalPlacedColor);
			}
			else
			{
				PedestalLightComponent->SetLightColor(IncorrectCrystalPlacedColor);
			}
		}
	}
}