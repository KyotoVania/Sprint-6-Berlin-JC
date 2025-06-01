// MyRecipeWidget.cpp
#include "MyRecipeWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"  // Nouveau include
#include "Components/Border.h"

void UMyRecipeWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	// Configuration initiale
	if (IsDesignTime())
	{
		// Données de test pour le designer
		FDisplayRecipeInfo TestInfo;
		TestInfo.Input1DisplayName = FText::FromString(TEXT("Cristal Test 1"));
		TestInfo.Input2DisplayName = FText::FromString(TEXT("Cristal Test 2"));
		TestInfo.OutputDisplayName = FText::FromString(TEXT("Cristal Fusionné"));
		TestInfo.SkillGrantedDisplayName = FText::FromString(TEXT("Compétence Test"));
		TestInfo.bPlayerHasInput1 = true;
		TestInfo.bPlayerHasInput2 = false;
		
		UpdateRecipeDisplay(TestInfo);
	}
}

void UMyRecipeWidget::InitializeRecipe(const FDisplayRecipeInfo& RecipeInfo, const FName& InRecipeName)
{
	CurrentRecipeInfo = RecipeInfo;
	RecipeName = InRecipeName;
	
	UpdateTextDisplays();
	UpdateVisualStates();
	
	UE_LOG(LogTemp, Log, TEXT("UMyRecipeWidget: Recette initialisée - %s"), *RecipeName.ToString());
}

void UMyRecipeWidget::UpdateRecipeDisplay(const FDisplayRecipeInfo& UpdatedRecipeInfo)
{
	CurrentRecipeInfo = UpdatedRecipeInfo;
	
	UpdateTextDisplays();
	UpdateVisualStates();
}

void UMyRecipeWidget::UpdateTextDisplays()
{
	if (Input1NameText)
	{
		Input1NameText->SetText(CurrentRecipeInfo.Input1DisplayName);
	}

	if (Input2NameText)
	{
		Input2NameText->SetText(CurrentRecipeInfo.Input2DisplayName);
	}

	if (OutputNameText)
	{
		OutputNameText->SetText(CurrentRecipeInfo.OutputDisplayName);
	}

	if (SkillNameText)
	{
		SkillNameText->SetText(CurrentRecipeInfo.SkillGrantedDisplayName);
	}
}

void UMyRecipeWidget::UpdateVisualStates()
{
	UpdateCheckBoxStates();
	UpdateBorderColors();
}

void UMyRecipeWidget::UpdateCheckBoxStates()
{
	// Mettre à jour les CheckBox pour indiquer la possession des ingrédients
	if (Input1PossessionCheckBox)
	{
		Input1PossessionCheckBox->SetIsChecked(CurrentRecipeInfo.bPlayerHasInput1);
		// Optionnel : désactiver l'interaction utilisateur (CheckBox en lecture seule)
		Input1PossessionCheckBox->SetIsEnabled(false);
	}

	if (Input2PossessionCheckBox)
	{
		Input2PossessionCheckBox->SetIsChecked(CurrentRecipeInfo.bPlayerHasInput2);
		// Optionnel : désactiver l'interaction utilisateur (CheckBox en lecture seule)
		Input2PossessionCheckBox->SetIsEnabled(false);
	}
}

void UMyRecipeWidget::UpdateBorderColors()
{
	// Mettre à jour la bordure principale du container selon si la recette peut être craftée
	if (RecipeContainerBorder)
	{
		bool bCanCraft = CanPlayerCraftRecipe();
		FLinearColor ContainerColor = bCanCraft ? CanCraftColor : CannotCraftColor;
		RecipeContainerBorder->SetBrushColor(ContainerColor);
	}
}