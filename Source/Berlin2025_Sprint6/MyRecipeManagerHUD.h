// MyRecipeManagerHUD.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FusionRecipe.h"
#include "MyRecipeManagerHUD.generated.h"

class AMyFPSPlayerCharacter;
class UDataTable;
class UMyRecipeWidget;
class UVerticalBox;
class UScrollBox;
enum class ECrystalType : uint8;
enum class ESkillType : uint8;

UCLASS()
class BERLIN2025_SPRINT6_API UMyRecipeManagerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Recipe Manager")
	void RefreshAllRecipes();

	UFUNCTION(BlueprintCallable, Category = "Recipe Manager")
	void ClearAllRecipes();

protected:
	virtual void NativeConstruct() override;

	// Widget Class pour chaque recette individuelle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe Manager|Config")
	TSubclassOf<UMyRecipeWidget> RecipeWidgetClass;

	// Container pour organiser les widgets de recettes (assigné depuis le Blueprint)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> RecipeScrollBox;

	// Alternative si vous préférez une VerticalBox
	// UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	// TObjectPtr<UVerticalBox> RecipeContainer;

private:
	// Référence vers le joueur propriétaire
	TWeakObjectPtr<AMyFPSPlayerCharacter> OwningPlayerCharacter;

	// Référence vers la DataTable des recettes
	TWeakObjectPtr<UDataTable> PlayerFusionRecipeTable;

	// Cache des widgets de recettes créés
	UPROPERTY()
	TArray<TObjectPtr<UMyRecipeWidget>> CreatedRecipeWidgets;

	// Fonctions utilitaires
	void InitializePlayerReferences();
	UMyRecipeWidget* CreateRecipeWidget(const FFusionRecipeRow* RecipeData, const FName& RecipeName);
	void PopulateRecipeWidgets();
	
	// Fonctions de formatage (reprises de votre code existant)
	FText GetFormattedCrystalName(ECrystalType CrystalType) const;
	FText GetFormattedSkillName(ESkillType SkillType) const;
	
	// Vérification de possession des ingrédients
	bool DoesPlayerHaveCrystalType(ECrystalType CrystalType) const;
	FDisplayRecipeInfo CreateDisplayInfoFromRecipe(const FFusionRecipeRow* RecipeData) const;
};