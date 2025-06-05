// MyItemCrystal.h
#pragma once

#include "CoreMinimal.h"
#include "MyMasterItem.h"
#include "CrystalType.h"
#include "SkillType.h"
#include "CrystalProperties.h"
#include "MyItemCrystal.generated.h"

// Forward declarations
class UPointLightComponent;
class UParticleSystemComponent;
class URotatingMovementComponent;
class UTimelineComponent;
class USoundBase;
class UDataTable;
class UCurveFloat;
class USoundAttenuation;  // Nouveau: pour les paramètres d'atténuation
class AMyFPSPlayerCharacter;

UCLASS(Blueprintable, BlueprintType)
class BERLIN2025_SPRINT6_API AMyItemCrystal : public AMyMasterItem
{
	GENERATED_BODY()

public:
	AMyItemCrystal();

	virtual void Interact_Implementation(AActor* InteractorActor) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // Nouveau: pour nettoyer le son

	// ----- Composants -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> PointLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> CrystalVFXComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTimelineComponent> CrystalTimeline;

	// ----- Configuration du Cristal -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal Config")
	ECrystalType CrystalType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crystal Config")
	TObjectPtr<UDataTable> CrystalPropertiesTable;

	// Nouveau: Paramètres d'atténuation pour le son d'ambiance
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Crystal Config|Audio", meta = (ToolTip = "Paramètres d'atténuation pour le son d'ambiance du cristal."))
	TObjectPtr<USoundAttenuation> AmbienceSoundAttenuation;

	// ----- Flottaison -----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crystal Config|Floatation")
	TObjectPtr<UCurveFloat> FloatationCurve;

	// Fonction appelée par le Timeline pour la flottaison
	UFUNCTION()
	void TimelineUpdate_Floatation(float Value);

	// Son chargé depuis la DataTable (utilisé pour l'ambiance et potentiellement interaction)
	UPROPERTY(BlueprintReadOnly, Category="Crystal Properties")
	TObjectPtr<USoundBase> LoadedCrystalSFX;
	void SetupCrystalFromDataTable();
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Crystal Properties")
	ESkillType PotentialSkillToGrant;

	UFUNCTION(BlueprintCallable, Category = "Crystal Audio")
	void StartAmbienceSound();
	
	UFUNCTION(BlueprintCallable, Category = "Crystal Audio")
	void StopAmbienceSound();

	UFUNCTION(BlueprintCallable, Category = "Crystal Audio")
	bool IsAmbienceSoundPlaying() const;
	UPROPERTY()
	bool bHasBeenPickedUpByPlayer;
	UFUNCTION(BlueprintCallable, Category = "Crystal State")
	void MarkAsPickedUpByPlayer() { bHasBeenPickedUpByPlayer = true; }
    
	UFUNCTION(BlueprintCallable, Category = "Crystal State")
	bool ShouldHaveActiveTimeline() const;
	FVector InitialRelativeLocation;

private:
	void InitializeFloatationTimeline();
	bool ShouldPlayAmbienceSound() const;


	// Nouveau: Référence vers l'instance du son d'ambiance en cours
	UPROPERTY()
	class UAudioComponent* CurrentAmbienceAudioComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Crystal Properties")
	ECrystalType GetCrystalType() const { return CrystalType; }

	UFUNCTION(BlueprintPure, Category="Crystal Properties")
	USoundBase* GetCrystalSound() const { return LoadedCrystalSFX; }
};