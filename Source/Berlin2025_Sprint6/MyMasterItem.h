// MyMasterItem.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyInteractableInterface.h" // Inclure notre nouvelle interface
#include "MyMasterItem.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UDecalComponent;
class USceneComponent;

UCLASS(Blueprintable, BlueprintType)
class BERLIN2025_SPRINT6_API AMyMasterItem : public AActor, public IMyInteractableInterface // Implémenter l'interface
{
	GENERATED_BODY()

public:
	AMyMasterItem();

	virtual void BeginPlay() override;

	// Composants
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SM_Shape;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDecalComponent> Decal_Outline;

	// Variables Globales
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterItem|Config")
	FVector DecalSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterItem|Config")
	FVector ShapeScale;

	// Fonctions pour gérer l'outline
	UFUNCTION(BlueprintCallable, Category = "MasterItem|Interaction")
	virtual void EnableOutline();

	UFUNCTION(BlueprintCallable, Category = "MasterItem|Interaction")
	virtual void DisableOutline();

	// Implémentation des fonctions de l'interface
	virtual void StartLookAtInteractable_Implementation() override;
	virtual void StopLookAtInteractable_Implementation() override;

private:
	bool bIsOutlineEnabled; // Pour gérer le DoOnce

public:
	virtual void Tick(float DeltaTime) override;
};