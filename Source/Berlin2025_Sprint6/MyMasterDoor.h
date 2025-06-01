// MyMasterDoor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyMasterDoor.generated.h"

UCLASS()
class BERLIN2025_SPRINT6_API AMyMasterDoor : public AActor // ou AStaticMeshActor
{
	GENERATED_BODY()
public:
	AMyMasterDoor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh; // Si vous héritez de AActor

public:
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual void CloseDoor(); // Si besoin

	// Peut-être des UPROPERTY pour des sons, des timelines d'animation, etc.
	UPROPERTY(EditAnywhere, Category="Door")
	USoundBase* OpeningSound;
};