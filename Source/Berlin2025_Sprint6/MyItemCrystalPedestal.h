// MyItemCrystalPedestal.h
#pragma once

#include "CoreMinimal.h"
#include "MyMasterItem.h" // Notre classe de base pour les objets interactifs
#include "CrystalType.h"    // Pour l'enum ECrystalType
#include "MyItemCrystalPedestal.generated.h"

// Forward declarations
class AMyItemCrystal;
class AMyFPSPlayerCharacter; // Ajout pour le cast dans Interact_Implementation
class UPointLightComponent;
class USphereComponent;
class USceneComponent;
class UStaticMeshComponent;
class USoundBase;
class UParticleSystem;

// Délégué pour informer d'un changement d'état du piédestal (correct ou non)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPedestalStateChanged, bool, bIsNowCorrect);

UCLASS(Blueprintable, BlueprintType)
class BERLIN2025_SPRINT6_API AMyItemCrystalPedestal : public AMyMasterItem
{
	GENERATED_BODY()

public:
	AMyItemCrystalPedestal();

protected:
	virtual void BeginPlay() override;

	// ----- Composants -----
public: // Rendre public pour accès facile depuis le BP si nécessaire, sinon protected
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PedestalMeshComponent; // Le mesh principal du piédestal (SM_Shape de MyMasterItem)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> CrystalAttachmentPointComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> PedestalLightComponent;

	// Optionnel: un mesh qui s'active/change quand le bon cristal est placé
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ActivatedStateMeshComponent;

	// Optionnel: une sphère pour détecter la proximité du joueur pour l'interaction ou des effets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InteractionTriggerSphere;

	// ----- État du Piédestal -----
	UPROPERTY(BlueprintReadOnly, Category = "Pedestal State")
	bool bIsCrystalPlaced;
protected:
	// ----- Configuration du Piédestal -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config")
	ECrystalType ExpectedCrystalType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|Visuals", meta = (ToolTip = "Couleur de la lumière quand le piédestal est vide, indiquant le type attendu."))
	FLinearColor ExpectedTypeHintColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|Visuals", meta = (ToolTip = "Couleur de la lumière quand le cristal correct est placé."))
	FLinearColor CorrectCrystalPlacedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|Visuals", meta = (ToolTip = "Couleur de la lumière quand un cristal incorrect est placé."))
	FLinearColor IncorrectCrystalPlacedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|Audio")
	TObjectPtr<USoundBase> SuccessSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|Audio")
	TObjectPtr<USoundBase> FailSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|VFX")
	TObjectPtr<UParticleSystem> ActivationVFX; // Particule à jouer lors du succès

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal Config|VFX")
	TObjectPtr<UParticleSystem> DeactivationVFX; // Optionnel: Particule à jouer lors du retrait



	UPROPERTY(BlueprintReadOnly, Category = "Pedestal State")
	TObjectPtr<AMyItemCrystal> CurrentCrystalOnPedestal;

public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Pedestal State")
	bool bIsCorrectCrystalOnPedestal; // Pour savoir si le cristal actuellement placé est le bon

	// ----- Interaction -----
	// Implémentation de la fonction d'interface
	virtual void Interact_Implementation(AActor* InteractorActor) override;

	/** Appelé pour tenter de placer un cristal sur le piédestal. */
	UFUNCTION(BlueprintCallable, Category = "Pedestal Interaction")
	bool TryPlaceCrystal(AMyItemCrystal* CrystalToPlace);

	/** Appelé pour retirer le cristal du piédestal. Renvoie le cristal retiré. */
	UFUNCTION(BlueprintCallable, Category = "Pedestal Interaction")
	AMyItemCrystal* RemoveCrystal();

	// ----- Événements -----
	UPROPERTY(BlueprintAssignable, Category = "Pedestal Events")
	FOnPedestalStateChanged OnPedestalStateChanged;
	
protected:
	// ----- Logique Interne -----
	/** Gère la logique après qu'un cristal a été physiquement placé (vérification, effets, etc.). */
	void HandleCrystalPlaced(AMyItemCrystal* PlacedCrystal);

	/** Gère la logique après qu'un cristal a été retiré. */
	void HandleCrystalRemoved();

	/** Met à jour l'apparence du piédestal (lumière, mesh d'activation). */
	void UpdatePedestalVisuals(); // Changé pour ne pas prendre de paramètre, utilise bIsCorrectCrystalOnPedestal
	
	// Surcharge des fonctions d'interface (pour l'outline, par exemple) si un comportement différent de AMyMasterItem est souhaité.
	// virtual void StartLookAtInteractable_Implementation() override;
	// virtual void StopLookAtInteractable_Implementation() override;
};