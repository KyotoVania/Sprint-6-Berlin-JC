// MyHiddenPlatform.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyHiddenPlatform.generated.h"

class UStaticMeshComponent;

UCLASS()
class BERLIN2025_SPRINT6_API AMyHiddenPlatform : public AActor
{
	GENERATED_BODY()

public:	
	AMyHiddenPlatform();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	// Fonction appelée lorsque le delegate du joueur est diffusé
	UFUNCTION() // Important pour que BindDynamic fonctionne
	void HandlePlayerSkillChanged(bool bNewVisibilityState);

private:
	// Pour garder une référence au joueur et se désabonner proprement
	TObjectPtr<class AMyFPSPlayerCharacter> CachedPlayerCharacter;
};