// MyRecipeListItemWidget.cpp
#include "MyRecipeListItemWidget.h"
#include "MyRecipeData.h"
#include "FusionRecipe.h" // Pour FDisplayRecipeInfo
// #include "Components/TextBlock.h" // Décommentez si vous utilisez BindWidget pour TextBlock
// #include "Components/CheckBox.h"  // Décommentez si vous utilisez BindWidget pour CheckBox

void UMyRecipeListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    // IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject); // Appelez le parent si nécessaire
    CurrentRecipeData = Cast<UMyRecipeData>(ListItemObject);

    // Si vous n'utilisez pas de bindings Blueprint et que vous voulez mettre à jour
    // les widgets C++ (ceux avec meta = (BindWidget)) :
    // if (Input1NameText && CurrentRecipeData)
    // {
    //     Input1NameText->SetText(CurrentRecipeData->RecipeInfo.Input1DisplayName);
    // }
    // if (Input1OwnedCheckBox && CurrentRecipeData)
    // {
    //     Input1OwnedCheckBox->SetIsChecked(CurrentRecipeData->RecipeInfo.bPlayerHasInput1);
    // }
    // etc.
}

const FDisplayRecipeInfo* UMyRecipeListItemWidget::GetSafeRecipeInfo() const
{
    if (CurrentRecipeData)
    {
        return &(CurrentRecipeData->RecipeInfo);
    }
    return nullptr;
}

FText UMyRecipeListItemWidget::GetInput1DisplayName() const
{
    const FDisplayRecipeInfo* Info = GetSafeRecipeInfo();
    return Info ? Info->Input1DisplayName : FText::GetEmpty();
}

bool UMyRecipeListItemWidget::GetPlayerHasInput1() const
{
    const FDisplayRecipeInfo* Info = GetSafeRecipeInfo();
    return Info ? Info->bPlayerHasInput1 : false;
}

FText UMyRecipeListItemWidget::GetInput2DisplayName() const
{
    const FDisplayRecipeInfo* Info = GetSafeRecipeInfo();
    return Info ? Info->Input2DisplayName : FText::GetEmpty();
}

bool UMyRecipeListItemWidget::GetPlayerHasInput2() const
{
    const FDisplayRecipeInfo* Info = GetSafeRecipeInfo();
    return Info ? Info->bPlayerHasInput2 : false;
}

FText UMyRecipeListItemWidget::GetOutputDisplayName() const
{
    const FDisplayRecipeInfo* Info = GetSafeRecipeInfo();
    return Info ? Info->OutputDisplayName : FText::GetEmpty();
}

FText UMyRecipeListItemWidget::GetSkillGrantedDisplayName() const
{
    const FDisplayRecipeInfo* Info = GetSafeRecipeInfo();
    return Info ? Info->SkillGrantedDisplayName : FText::GetEmpty();
}