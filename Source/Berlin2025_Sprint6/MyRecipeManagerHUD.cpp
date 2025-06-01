// MyRecipeManagerHUD.cpp
#include "MyRecipeManagerHUD.h"
#include "MyFPSPlayerCharacter.h"
#include "MyItemCrystal.h"
#include "MyRecipeWidget.h"
#include "Engine/DataTable.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "CrystalType.h"
#include "SkillType.h"

void UMyRecipeManagerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitializePlayerReferences();
	RefreshAllRecipes();
}

void UMyRecipeManagerHUD::InitializePlayerReferences()
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (PlayerPawn)
	{
		OwningPlayerCharacter = Cast<AMyFPSPlayerCharacter>(PlayerPawn);
		if (OwningPlayerCharacter.IsValid())
		{
			PlayerFusionRecipeTable = OwningPlayerCharacter->FusionRecipeTable;
		}
	}

	if (!OwningPlayerCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UMyRecipeManagerHUD: Impossible de récupérer le AMyFPSPlayerCharacter"));
	}

	if (!PlayerFusionRecipeTable.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UMyRecipeManagerHUD: FusionRecipeTable non assignée dans le joueur"));
	}

	if (!RecipeWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UMyRecipeManagerHUD: RecipeWidgetClass non assignée"));
	}

	if (!RecipeScrollBox)
	{
		UE_LOG(LogTemp, Error, TEXT("UMyRecipeManagerHUD: RecipeScrollBox non trouvée (vérifiez le binding dans le Blueprint)"));
	}
}

void UMyRecipeManagerHUD::RefreshAllRecipes()
{
	ClearAllRecipes();
	
	if (!OwningPlayerCharacter.IsValid() || !PlayerFusionRecipeTable.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyRecipeManagerHUD: Impossible de rafraîchir - références invalides"));
		return;
	}

	PopulateRecipeWidgets();
}

void UMyRecipeManagerHUD::ClearAllRecipes()
{
	// Supprimer tous les widgets existants du container
	if (RecipeScrollBox)
	{
		RecipeScrollBox->ClearChildren();
	}

	// Vider le cache
	CreatedRecipeWidgets.Empty();
}

void UMyRecipeManagerHUD::PopulateRecipeWidgets()
{
	if (!RecipeWidgetClass || !RecipeScrollBox)
	{
		UE_LOG(LogTemp, Error, TEXT("UMyRecipeManagerHUD: RecipeWidgetClass ou RecipeScrollBox non configurés"));
		return;
	}

	FString ContextString(TEXT("RecipeManagerHUD"));
	TArray<FName> RowNames = PlayerFusionRecipeTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FFusionRecipeRow* CurrentRecipe = PlayerFusionRecipeTable->FindRow<FFusionRecipeRow>(RowName, ContextString);
		if (CurrentRecipe)
		{
			UMyRecipeWidget* NewRecipeWidget = CreateRecipeWidget(CurrentRecipe, RowName);
			if (NewRecipeWidget)
			{
				RecipeScrollBox->AddChild(NewRecipeWidget);
				CreatedRecipeWidgets.Add(NewRecipeWidget);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UMyRecipeManagerHUD: %d recettes créées"), CreatedRecipeWidgets.Num());
}

UMyRecipeWidget* UMyRecipeManagerHUD::CreateRecipeWidget(const FFusionRecipeRow* RecipeData, const FName& RecipeName)
{
	if (!RecipeData || !RecipeWidgetClass)
	{
		return nullptr;
	}

	UMyRecipeWidget* NewWidget = CreateWidget<UMyRecipeWidget>(this, RecipeWidgetClass);
	if (NewWidget)
	{
		// Créer les informations formatées pour ce widget
		FDisplayRecipeInfo DisplayInfo = CreateDisplayInfoFromRecipe(RecipeData);
		
		// Initialiser le widget avec les données
		NewWidget->InitializeRecipe(DisplayInfo, RecipeName);
		
		UE_LOG(LogTemp, Log, TEXT("UMyRecipeManagerHUD: Widget créé pour la recette %s"), *RecipeName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UMyRecipeManagerHUD: Échec de création du widget pour %s"), *RecipeName.ToString());
	}

	return NewWidget;
}

FDisplayRecipeInfo UMyRecipeManagerHUD::CreateDisplayInfoFromRecipe(const FFusionRecipeRow* RecipeData) const
{
	FDisplayRecipeInfo DisplayInfo;
	
	// Formater les noms des cristaux et compétences
	DisplayInfo.Input1DisplayName = GetFormattedCrystalName(RecipeData->InputCrystalType1);
	DisplayInfo.Input2DisplayName = GetFormattedCrystalName(RecipeData->InputCrystalType2);
	DisplayInfo.OutputDisplayName = GetFormattedCrystalName(RecipeData->OutputFusedCrystalType);
	DisplayInfo.SkillGrantedDisplayName = GetFormattedSkillName(RecipeData->SkillGrantedByOutputCrystal);

	// Vérifier si le joueur possède les ingrédients
	DisplayInfo.bPlayerHasInput1 = DoesPlayerHaveCrystalType(RecipeData->InputCrystalType1);
	DisplayInfo.bPlayerHasInput2 = DoesPlayerHaveCrystalType(RecipeData->InputCrystalType2);

	return DisplayInfo;
}

bool UMyRecipeManagerHUD::DoesPlayerHaveCrystalType(ECrystalType CrystalType) const
{
	if (!OwningPlayerCharacter.IsValid())
	{
		return false;
	}

	AMyItemCrystal* RightHandCrystal = OwningPlayerCharacter->GetRightHandCrystal();
	AMyItemCrystal* LeftHandCrystal = OwningPlayerCharacter->GetLeftHandCrystal();

	bool bHasInRightHand = (RightHandCrystal && RightHandCrystal->GetCrystalType() == CrystalType);
	bool bHasInLeftHand = (LeftHandCrystal && LeftHandCrystal->GetCrystalType() == CrystalType);

	return bHasInRightHand || bHasInLeftHand;
}

FText UMyRecipeManagerHUD::GetFormattedCrystalName(ECrystalType CrystalType) const
{
	const UEnum* EnumPtr = StaticEnum<ECrystalType>();
	if (EnumPtr)
	{
		return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(CrystalType));
	}
	return FText::FromString(TEXT("Cristal Inconnu"));
}

FText UMyRecipeManagerHUD::GetFormattedSkillName(ESkillType SkillType) const
{
	const UEnum* EnumPtr = StaticEnum<ESkillType>();
	if (EnumPtr)
	{
		return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(SkillType));
	}
	return FText::FromString(TEXT("Compétence Inconnue"));
}