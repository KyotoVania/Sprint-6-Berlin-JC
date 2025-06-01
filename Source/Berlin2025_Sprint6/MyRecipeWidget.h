// MyRecipeWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FusionRecipe.h"
#include "MyRecipeWidget.generated.h"

class UTextBlock;
class UCheckBox;  // Remplace UImage
class UBorder;

UCLASS()
class BERLIN2025_SPRINT6_API UMyRecipeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Fonction d'initialisation appelée par le HUD manager
	UFUNCTION(BlueprintCallable, Category = "Recipe Widget")
	void InitializeRecipe(const FDisplayRecipeInfo& RecipeInfo, const FName& RecipeName);

	UFUNCTION(BlueprintCallable, Category = "Recipe Widget")
	void UpdateRecipeDisplay(const FDisplayRecipeInfo& UpdatedRecipeInfo);

	// Getters BlueprintPure pour les bindings UMG
	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	FText GetInput1DisplayName() const { return CurrentRecipeInfo.Input1DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	FText GetInput2DisplayName() const { return CurrentRecipeInfo.Input2DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	FText GetOutputDisplayName() const { return CurrentRecipeInfo.OutputDisplayName; }

	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	FText GetSkillGrantedDisplayName() const { return CurrentRecipeInfo.SkillGrantedDisplayName; }

	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	bool GetPlayerHasInput1() const { return CurrentRecipeInfo.bPlayerHasInput1; }

	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	bool GetPlayerHasInput2() const { return CurrentRecipeInfo.bPlayerHasInput2; }

	UFUNCTION(BlueprintPure, Category = "Recipe Widget")
	bool CanPlayerCraftRecipe() const { return CurrentRecipeInfo.bPlayerHasInput1 && CurrentRecipeInfo.bPlayerHasInput2; }

protected:
	virtual void NativePreConstruct() override;

	// Widgets liés automatiquement depuis le Blueprint
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Input1NameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Input2NameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> OutputNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> SkillNameText;

	// NOUVEAU : CheckBox pour indiquer la possession des ingrédients
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> Input1PossessionCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> Input2PossessionCheckBox;

	// Optionnel : Bordure principale pour l'état général de la recette
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> RecipeContainerBorder;

	// Couleurs pour indiquer le statut
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe Widget|Style")
	FLinearColor CanCraftColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.3f); // Vert transparent

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe Widget|Style")
	FLinearColor CannotCraftColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.3f); // Gris transparent

private:
	// Données de la recette courante
	UPROPERTY()
	FDisplayRecipeInfo CurrentRecipeInfo;

	// Nom de la recette (pour debug/identification)
	UPROPERTY()
	FName RecipeName;

	// Fonctions internes pour mettre à jour l'affichage
	void UpdateTextDisplays();
	void UpdateVisualStates();
	void UpdateCheckBoxStates();
	void UpdateBorderColors();
};