// MyInteractableInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "MyInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UMyInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class BERLIN2025_SPRINT6_API IMyInteractableInterface
{
	GENERATED_BODY()

public:
	// Fonction appelée quand le joueur commence à regarder l'objet
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void StartLookAtInteractable();

	// Fonction appelée quand le joueur arrête de regarder l'objet
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void StopLookAtInteractable();

	// Fonction appelée quand le joueur interagit avec l'objet
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* InteractorActor); 
	// InteractorActor est l'acteur qui initie l'interaction (généralement le joueur)
};