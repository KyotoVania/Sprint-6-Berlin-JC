// MyFPSPlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyInteractableInterface.h" // Notre interface d'interaction
#include "InputActionValue.h"      // Nécessaire pour Enhanced Input
#include "Engine/DataTable.h" // Pour UDataTable
#include "FusionRecipe.h"
#include "MyFusionRecipeHUD.h" // Pour le HUD des recettes de fusion

#include "MyFPSPlayerCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class USceneComponent;
class AMyItemCrystal; // Forward declaration pour le type de cristal
class UInputMappingContext; // Forward declaration
class UInputAction;       // Forward declaration

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillSeeHiddenPlatformsChanged, bool, bNewVisibilityState);


UCLASS()
class BERLIN2025_SPRINT6_API AMyFPSPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyFPSPlayerCharacter();

	UFUNCTION(BlueprintCallable, Category = "Skills")
	void UnlockSkill(ESkillType SkillToUnlock);

	
	UPROPERTY(BlueprintAssignable, Category = "Skills|Events")
	FOnSkillSeeHiddenPlatformsChanged OnSkillSeeHiddenPlatformsChanged;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Fusion", meta = (ToolTip = "Table de propriétés des cristaux à utiliser pour les cristaux fusionnés."))
	TObjectPtr<UDataTable> DefaultCrystalPropertiesTable;
	// ----- Composants -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components|Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;

	// Anciennement CrystalAttachmentPointComponent, maintenant spécifique pour chaque main
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Inventory")
	TObjectPtr<USceneComponent> RightHandAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Inventory")
	TObjectPtr<USceneComponent> LeftHandAttachPoint;

	// ----- Interaction -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Config")
	float LineTraceLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Config")
	bool bShouldPerformRaycast;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TScriptInterface<IMyInteractableInterface> LookedAtInteractableActor;

	void PerformLineTrace();
	void UpdateLookedAtInteractable(TScriptInterface<IMyInteractableInterface> NewLookedAt);

	// ----- Inventaire (Gestion des deux mains) -----
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<AMyItemCrystal> RightHandCrystal;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<AMyItemCrystal> LeftHandCrystal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop")
	float DropDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop")
	float DropImpulseStrength;

	// ----- Compétences -----
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skills")
	bool bCanSeeHiddenPlatforms;


	// ----- Actions & Input -----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Input Action pour interagir avec la main droite."))
	TObjectPtr<UInputAction> InteractRightHandInputAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Input Action pour interagir avec la main gauche."))
	TObjectPtr<UInputAction> InteractLeftHandInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Input Action pour fusionner les cristaux tenus."))
	TObjectPtr<UInputAction> FuseCrystalsInputAction;
	
	// Fonctions handler pour les nouvelles inputs
	void HandleInteractRightHand(const FInputActionValue& Value);
	void HandleInteractLeftHand(const FInputActionValue& Value);
	void HandleFuseCrystals(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ToolTip = "Classe du Widget Blueprint pour le HUD des recettes de fusion."))
	TSubclassOf<UMyFusionRecipeHUD> FusionRecipeHUDWidgetClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UMyFusionRecipeHUD> FusionRecipeHUDInstance;

	// Input Action pour afficher/cacher le HUD des recettes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleRecipesHUDInputAction;
public:
	// ----- Fonctions d'inventaire mises à jour -----

	
	/** Tente de ramasser un cristal. Détermine automatiquement quelle main utiliser. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool PickupCrystal(AMyItemCrystal* CrystalToPickup);

	/**
	 * Tente de placer le cristal tenu dans la main spécifiée sur un piédestal (ou autre interactable).
	 * Renvoie le cristal qui était tenu pour que le piédestal puisse le gérer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	AMyItemCrystal* PlaceDownCrystalFromHand(bool bIsRightHand); // true pour droite, false pour gauche

	/** Lâche le cristal tenu dans la main spécifiée dans le monde. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropCrystalFromHand(bool bIsRightHand);


	// ----- Getters -----
	UFUNCTION(BlueprintPure, Category = "Inventory")
	AMyItemCrystal* GetRightHandCrystal() const { return RightHandCrystal; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	AMyItemCrystal* GetLeftHandCrystal() const { return LeftHandCrystal; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	USceneComponent* GetRightHandAttachPoint() const { return RightHandAttachPoint; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	USceneComponent* GetLeftHandAttachPoint() const { return LeftHandAttachPoint; }

	UFUNCTION(BlueprintPure, Category = "Skills")
	bool CanSeeHiddenPlatforms() const { return bCanSeeHiddenPlatforms; }

	// Setter pour la compétence (sera appelé par le piédestal/puzzle manager)
	UFUNCTION(BlueprintCallable, Category="Skills")
	void SetCanSeeHiddenPlatforms(bool bNewValue) { bCanSeeHiddenPlatforms = bNewValue; }


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Fusion", meta = (ToolTip = "DataTable contenant les recettes de fusion des cristaux."))
	TObjectPtr<UDataTable> FusionRecipeTable;
// Fonctions internes pour simplifier la logique de Pickup/Drop
private:
	void HandleToggleRecipesHUD(const FInputActionValue& Value);

	/** Logique interne pour ramasser un cristal dans une main spécifique si elle est vide. */
	bool PickupCrystalToHand(AMyItemCrystal* CrystalToPickup, TObjectPtr<AMyItemCrystal>& HandCrystalVariable, USceneComponent* AttachPoint);
	
	/** Logique interne pour détacher un cristal d'une main. */
	AMyItemCrystal* DetachCrystalFromHand(TObjectPtr<AMyItemCrystal>& HandCrystalVariable);

	/** Logique interne pour lâcher un cristal dans le monde. */
	void DropCrystalIntoWorld(AMyItemCrystal* CrystalToDrop);
};