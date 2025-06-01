// MyMasterDoor.cpp
#include "MyMasterDoor.h"
#include "Components/StaticMeshComponent.h" // Si besoin
#include "Kismet/GameplayStatics.h"     // Pour PlaySoundAtLocation

AMyMasterDoor::AMyMasterDoor()
{
	// Si vous héritez de AActor et voulez un mesh par défaut
	// DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	// RootComponent = DoorMesh;
	PrimaryActorTick.bCanEverTick = false; // A moins que vous ayez des animations à gérer au tick
}

void AMyMasterDoor::OpenDoor()
{
	UE_LOG(LogTemp, Log, TEXT("Porte %s: OUVRE-TOI !"), *GetName());
	if(OpeningSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OpeningSound, GetActorLocation());
	}
	// Ici, ajoutez la logique pour l'animation d'ouverture, le déplacement, etc.
	// Par exemple, désactiver la collision et cacher :
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// Ou si vous avez une animation Timeline : MyTimeline->Play();
}

void AMyMasterDoor::CloseDoor()
{
	UE_LOG(LogTemp, Log, TEXT("Porte %s: FERME-TOI !"), *GetName());
	// Logique inverse
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}