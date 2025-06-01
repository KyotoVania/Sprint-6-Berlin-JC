// MyItemCrystal.h
#pragma once

#include "CoreMinimal.h"
#include "MyMasterItem.h"
#include "CrystalType.h"
#include "SkillType.h"
#include "CrystalProperties.h" // Inclure notre nouvelle structure
#include "MyItemCrystal.generated.h"

// Forward declarations
class UPointLightComponent;
class UParticleSystemComponent;
class URotatingMovementComponent;
class UTimelineComponent;
class USoundBase;
class UDataTable;
class UCurveFloat;
class UAudioComponent;     // << AJOUTÉ forward declaration
class USphereComponent;    // << AJOUTÉ forward declaration
class AMyFPSPlayerCharacter; // Déjà présent, bien

UCLASS(Blueprintable, BlueprintType)
class BERLIN2025_SPRINT6_API AMyItemCrystal : public AMyMasterItem
{
	GENERATED_BODY()

public:
	AMyItemCrystal();

	virtual void Interact_Implementation(AActor* InteractorActor) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ----- Composants -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> PointLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> CrystalVFXComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTimelineComponent> CrystalTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AmbienceAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> AmbienceTriggerSphere;

	// ----- Configuration du Cristal -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal Config")
	ECrystalType CrystalType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crystal Config")
	TObjectPtr<UDataTable> CrystalPropertiesTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Crystal Config|Audio", meta = (ToolTip = "Rayon de la sphère pour déclencher le son d'ambiance (LoadedCrystalSFX)."))
    float AmbienceSoundRadius;

	// ----- Flottaison -----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crystal Config|Floatation")
	TObjectPtr<UCurveFloat> FloatationCurve;

	// Fonction appelée par le Timeline pour la flottaison
	UFUNCTION() // Déjà UFUNCTION, bien
	void TimelineUpdate_Floatation(float Value);

	// Son chargé depuis la DataTable (utilisé pour l'ambiance et potentiellement interaction)
	UPROPERTY(BlueprintReadOnly, Category="Crystal Properties")
	TObjectPtr<USoundBase> LoadedCrystalSFX;
	void SetupCrystalFromDataTable();
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Crystal Properties")
	ESkillType PotentialSkillToGrant;
private:
	void InitializeFloatationTimeline();
	void InitializeAmbienceSystem(); // CORRIGÉ: Nom correspond à l'implémentation
	bool ShouldPlayAmbienceSound() const;

	FVector InitialRelativeLocation;

    // Fonctions pour gérer l'overlap - DOIVENT ÊTRE UFUNCTION()
	UFUNCTION()
	void OnAmbienceSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAmbienceSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	UFUNCTION(BlueprintPure, Category = "Crystal Properties")
	ECrystalType GetCrystalType() const { return CrystalType; }

	UFUNCTION(BlueprintPure, Category="Crystal Properties")
	USoundBase* GetCrystalSound() const { return LoadedCrystalSFX; }
};