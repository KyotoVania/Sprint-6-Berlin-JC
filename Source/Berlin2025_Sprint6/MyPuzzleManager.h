// MyPuzzleManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPuzzleManager.generated.h"

class AMyItemCrystalPedestal; // Forward declaration
class AMyMasterDoor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleCompleted);

UCLASS()
class BERLIN2025_SPRINT6_API AMyPuzzleManager : public AActor
{
	GENERATED_BODY()

public:
	AMyPuzzleManager();

protected:
	virtual void BeginPlay() override;

public:
	// Liste des piédestaux à surveiller pour ce puzzle
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle Config")
	TArray<AMyItemCrystalPedestal*> PedestalsToMonitor;

	// Optionnel: La porte à ouvrir lorsque le puzzle est complété
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle Config", meta = (ToolTip = "La porte à ouvrir une fois le puzzle résolu."))
	AActor* DoorToOpen; // Vous pouvez utiliser AActor* ou une classe de porte plus spécifique si vous en avez une

	// Délégué qui sera diffusé quand le puzzle est complété
	UPROPERTY(BlueprintAssignable, Category = "Puzzle Events")
	FOnPuzzleCompleted OnPuzzleCompleted;

private:
	// Fonction appelée lorsqu'un piédestal change d'état
	UFUNCTION() // Important pour que le BindDynamic fonctionne
	void HandlePedestalStateChanged(bool bIsNowCorrect);

	// Vérifie si tous les piédestaux sont dans l'état correct
	void CheckPuzzleCompletion();

	// Compteur pour le nombre de piédestaux actuellement corrects
	int32 CorrectPedestalsCount;
};