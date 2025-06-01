// CrystalProperties.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CrystalType.h" // Notre enum ECrystalType
#include "CrystalProperties.generated.h"

// Forward declarations
class UStaticMesh;
class UMaterialInterface;
class UParticleSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FCrystalPropertiesRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	ECrystalType CrystalType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UStaticMesh> CrystalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UMaterialInterface> CrystalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UParticleSystem> CrystalVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<USoundBase> CrystalSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties", meta = (ClampMin = "0.0"))
	float LightIntensity;

	// Ajoute d'autres propriétés si nécessaire (ex: paramètres spécifiques pour le VFX, vitesse de rotation, etc.)

	FCrystalPropertiesRow()
		: CrystalType(ECrystalType::Anger)
		,CrystalMesh(nullptr)
		, CrystalMaterial(nullptr)
		, CrystalVFX(nullptr)
		, CrystalSFX(nullptr)
		, LightColor(FLinearColor::White)
		, LightIntensity(1000.0f)
	{}
};