#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h" // Nécessaire pour UENUM
#include "SkillType.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	None                        UMETA(DisplayName = "Aucune Compétence"),
	CanSeeHiddenPlatforms       UMETA(DisplayName = "Peut Voir Plateformes Cachées"),
	// Ajoutez ici d'autres compétences futures, par exemple :
	// CanBreakFragileBarriers  UMETA(DisplayName = "Peut Briser Barrières Fragiles"),
	// CanActivateAncientGlyphs UMETA(DisplayName = "Peut Activer Glyphes Anciens")
};