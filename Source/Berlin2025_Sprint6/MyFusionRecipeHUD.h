// MyFusionRecipeHUD.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FusionRecipe.h" // Pour FFusionRecipeRow et FDisplayRecipeInfo
#include "MyFusionRecipeHUD.generated.h"

class AMyFPSPlayerCharacter;
class UDataTable;
class UMyRecipeData; // << AJOUTÉ Forward declaration
enum class ECrystalType : uint8;
enum class ESkillType : uint8;

UCLASS()
class BERLIN2025_SPRINT6_API UMyFusionRecipeHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Fusion HUD")
	void RefreshRecipeDisplay(); // Déjà existant

	// MODIFIÉ: Utilise TArray de UMyRecipeData* au lieu de FDisplayRecipeInfo directement
	UPROPERTY(BlueprintReadOnly, Category = "Fusion HUD")
	TArray<TObjectPtr<UMyRecipeData>> DisplayableRecipeObjectList;

protected:
	virtual void NativeConstruct() override; // Déjà existant

	TWeakObjectPtr<AMyFPSPlayerCharacter> OwningPlayerCharacter; // Déjà existant
	TWeakObjectPtr<UDataTable> PlayerFusionRecipeTable; // Déjà existant

	FText GetFormattedCrystalName(ECrystalType CrystalType) const; // Déjà existant
	FText GetFormattedSkillName(ESkillType SkillType) const;   // Déjà existant
};