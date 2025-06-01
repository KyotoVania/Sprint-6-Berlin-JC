// FusionRecipe.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CrystalType.h"     // Votre énumération ECrystalType existante
#include "SkillType.h"       // Notre nouvelle énumération ESkillType
#include "FusionRecipe.generated.h"

USTRUCT(BlueprintType)
struct FFusionRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Cristal 1 nécessaire pour la fusion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fusion Config")
	ECrystalType InputCrystalType1;

	// Cristal 2 nécessaire pour la fusion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fusion Config")
	ECrystalType InputCrystalType2;

	// Type du cristal résultant de la fusion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fusion Config")
	ECrystalType OutputFusedCrystalType;

	// Compétence que le cristal fusionné (OutputFusedCrystalType) peut débloquer
	// Cette compétence sera accordée lorsque le cristal fusionné est utilisé/placé correctement.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fusion Config")
	ESkillType SkillGrantedByOutputCrystal;

	// Constructeur par défaut pour initialiser les valeurs
	FFusionRecipeRow()
		: InputCrystalType1(ECrystalType::Joy) // Valeurs par défaut, à ajuster
		, InputCrystalType2(ECrystalType::Hope)
		, OutputFusedCrystalType(ECrystalType::Joy) // Doit idéalement être un type "fusionné" distinct
		, SkillGrantedByOutputCrystal(ESkillType::None)
	{}
};


USTRUCT(BlueprintType)
struct FDisplayRecipeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	FText Input1DisplayName; // Nom lisible du cristal 1

	// UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	// FSlateBrush Input1Icon; // Pour l'icône, plus complexe, on verra plus tard

	UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	bool bPlayerHasInput1; // Le joueur possède-t-il cet ingrédient ?

	UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	FText Input2DisplayName; // Nom lisible du cristal 2

	// UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	// FSlateBrush Input2Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	bool bPlayerHasInput2; // Le joueur possède-t-il cet ingrédient ?

	UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	FText OutputDisplayName; // Nom lisible du cristal résultat

	// UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	// FSlateBrush OutputIcon;

	UPROPERTY(BlueprintReadOnly, Category = "Recipe Info")
	FText SkillGrantedDisplayName; // Nom de la compétence accordée

	FDisplayRecipeInfo() : bPlayerHasInput1(false), bPlayerHasInput2(false) {}
};