// MyFusionRecipeHUD.cpp
#include "MyFusionRecipeHUD.h"
#include "MyFPSPlayerCharacter.h" //
#include "MyItemCrystal.h"        //
#include "Engine/DataTable.h"     //
#include "Kismet/GameplayStatics.h" //
#include "FusionRecipe.h"         //
#include "SkillType.h"            //
#include "CrystalType.h"          //
#include "MyRecipeData.h"         // << AJOUTÉ Include pour notre classe wrapper

void UMyFusionRecipeHUD::NativeConstruct()
{
    Super::NativeConstruct();

    APawn* PlayerPawn = GetOwningPlayerPawn(); //
    if (PlayerPawn)
    {
        OwningPlayerCharacter = Cast<AMyFPSPlayerCharacter>(PlayerPawn); //
        if (OwningPlayerCharacter.IsValid())
        {
            PlayerFusionRecipeTable = OwningPlayerCharacter->FusionRecipeTable; //
        }
    }
    RefreshRecipeDisplay(); //
}

FText UMyFusionRecipeHUD::GetFormattedCrystalName(ECrystalType CrystalType) const
{
    const UEnum* EnumPtr = StaticEnum<ECrystalType>(); //
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(CrystalType)); //
    }
    return FText::FromString(TEXT("Cristal Inconnu")); //
}

FText UMyFusionRecipeHUD::GetFormattedSkillName(ESkillType SkillType) const
{
    const UEnum* EnumPtr = StaticEnum<ESkillType>(); //
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(SkillType)); //
    }
    return FText::FromString(TEXT("Compétence Inconnue")); //
}

void UMyFusionRecipeHUD::RefreshRecipeDisplay()
{
    // DisplayableRecipeList.Empty(); // ANCIEN
    DisplayableRecipeObjectList.Empty(); // NOUVEAU

    if (!OwningPlayerCharacter.IsValid() || !PlayerFusionRecipeTable.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UMyFusionRecipeHUD: Joueur ou FusionRecipeTable invalide. Impossible de rafraîchir.")); //
        return;
    }

    AMyItemCrystal* RightHandHeldCrystal = OwningPlayerCharacter->GetRightHandCrystal(); //
    AMyItemCrystal* LeftHandHeldCrystal = OwningPlayerCharacter->GetLeftHandCrystal();   //

    // Utilisation d'une valeur par défaut pour ECrystalType si la main est vide pour éviter les crashs,
    // mais la présence du cristal est vérifiée avant d'utiliser le type.
    // La valeur par défaut ici n'a pas d'importance tant que RightHandHeldCrystal/LeftHandHeldCrystal est vérifié.
    ECrystalType RightHandType = RightHandHeldCrystal ? RightHandHeldCrystal->GetCrystalType() : ECrystalType::Joy; //
    ECrystalType LeftHandType = LeftHandHeldCrystal ? LeftHandHeldCrystal->GetCrystalType() : ECrystalType::Joy;     //

    FString ContextString(TEXT("RecipeHUD")); //
    TArray<FName> RowNames = PlayerFusionRecipeTable->GetRowNames(); //

    for (const FName& RowName : RowNames)
    {
        FFusionRecipeRow* CurrentRecipe = PlayerFusionRecipeTable->FindRow<FFusionRecipeRow>(RowName, ContextString); //
        if (CurrentRecipe)
        {
            FDisplayRecipeInfo TempInfo; // Structure temporaire pour stocker les infos formatées
            TempInfo.Input1DisplayName = GetFormattedCrystalName(CurrentRecipe->InputCrystalType1); //
            TempInfo.Input2DisplayName = GetFormattedCrystalName(CurrentRecipe->InputCrystalType2); //
            TempInfo.OutputDisplayName = GetFormattedCrystalName(CurrentRecipe->OutputFusedCrystalType); //
            TempInfo.SkillGrantedDisplayName = GetFormattedSkillName(CurrentRecipe->SkillGrantedByOutputCrystal); //

            bool bPlayerHasType1 = (RightHandHeldCrystal && RightHandType == CurrentRecipe->InputCrystalType1) ||
                                   (LeftHandHeldCrystal && LeftHandType == CurrentRecipe->InputCrystalType1); //
            bool bPlayerHasType2 = (RightHandHeldCrystal && RightHandType == CurrentRecipe->InputCrystalType2) ||
                                   (LeftHandHeldCrystal && LeftHandType == CurrentRecipe->InputCrystalType2); //
            
            TempInfo.bPlayerHasInput1 = bPlayerHasType1; //
            TempInfo.bPlayerHasInput2 = bPlayerHasType2; //

            // Créer l'objet UMyRecipeData
            UMyRecipeData* RecipeDataObject = NewObject<UMyRecipeData>(this); // 'this' (UMyFusionRecipeHUD) est un bon Outer pour ces objets.
            if (RecipeDataObject)
            {
                RecipeDataObject->RecipeInfo = TempInfo; // Copier les données dans l'objet wrapper
                DisplayableRecipeObjectList.Add(RecipeDataObject); // Ajouter l'objet wrapper à la liste
            }
        }
    }
    // Le Blueprint UMG qui hérite de cette classe pourra maintenant lire DisplayableRecipeObjectList
    // et le UListView pourra l'utiliser pour générer ses items.
}