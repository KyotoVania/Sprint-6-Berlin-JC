// MyRecipeListItemWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "MyRecipeListItemWidget.generated.h"

class UMyRecipeData; // Forward declaration
class UTextBlock;    // Forward declaration pour les bindings C++ optionnels
class UCheckBox;     // Forward declaration pour les bindings C++ optionnels
struct FDisplayRecipeInfo; // Forward declaration

UCLASS()
class BERLIN2025_SPRINT6_API UMyRecipeListItemWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

protected:
    // Implémentation de l'interface IUserObjectListEntry
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    // Variable pour stocker les données de la recette pour cet item.
    // BlueprintReadOnly pour que le Blueprint UMG enfant puisse y accéder pour les bindings.
    UPROPERTY(BlueprintReadOnly, Category = "Recipe Item", meta = (ExposeOnSpawn = "true"))
    TObjectPtr<UMyRecipeData> CurrentRecipeData;

public:
    // Fonctions BlueprintPure optionnelles pour des bindings plus propres depuis UMG.
    // Vous pouvez aussi binder directement à CurrentRecipeData.RecipeInfo.Propriété dans le BP UMG.

    UFUNCTION(BlueprintPure, Category = "Recipe Item Display")
    FText GetInput1DisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Recipe Item Display")
    bool GetPlayerHasInput1() const;

    UFUNCTION(BlueprintPure, Category = "Recipe Item Display")
    FText GetInput2DisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Recipe Item Display")
    bool GetPlayerHasInput2() const;

    UFUNCTION(BlueprintPure, Category = "Recipe Item Display")
    FText GetOutputDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Recipe Item Display")
    FText GetSkillGrantedDisplayName() const;

    // Si vous voulez binder des UTextBlock, UCheckBox etc. directement en C++ (plus avancé)
    // UPROPERTY(meta = (BindWidget))
    // TObjectPtr<UTextBlock> Input1NameText;

    // UPROPERTY(meta = (BindWidget))
    // TObjectPtr<UCheckBox> Input1OwnedCheckBox;

private:
    // Helper pour éviter la duplication de code dans les getters si CurrentRecipeData est null
    const FDisplayRecipeInfo* GetSafeRecipeInfo() const;
};