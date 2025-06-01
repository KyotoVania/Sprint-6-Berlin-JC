// MyRecipeData.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FusionRecipe.h" // Pour FDisplayRecipeInfo
#include "MyRecipeData.generated.h"

UCLASS(BlueprintType) // BlueprintType pour que le BP d'item puisse le référencer
class UMyRecipeData : public UObject
{
	GENERATED_BODY()
public:
	// BlueprintReadOnly pour que le BP d'item puisse lire ces infos
	UPROPERTY(BlueprintReadOnly, Category = "Recipe Data")
	FDisplayRecipeInfo RecipeInfo;
};