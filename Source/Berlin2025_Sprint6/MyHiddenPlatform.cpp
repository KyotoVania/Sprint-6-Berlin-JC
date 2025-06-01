// MyHiddenPlatform.cpp
#include "MyHiddenPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyFPSPlayerCharacter.h" // Assurez-vous d'inclure le header de votre personnage

AMyHiddenPlatform::AMyHiddenPlatform()
{
    PrimaryActorTick.bCanEverTick = false; // Plus besoin de Tick pour ça !

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    RootComponent = PlatformMesh;

    // État initial (sera potentiellement écrasé dans BeginPlay par l'état actuel de la compétence)
    PlatformMesh->SetVisibility(false);
    PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMyHiddenPlatform::BeginPlay()
{
    Super::BeginPlay();

    // Récupérer le personnage joueur
    AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    CachedPlayerCharacter = Cast<AMyFPSPlayerCharacter>(PlayerActor);

    if (CachedPlayerCharacter)
    {
        // S'abonner au delegate du joueur
        CachedPlayerCharacter->OnSkillSeeHiddenPlatformsChanged.AddDynamic(this, &AMyHiddenPlatform::HandlePlayerSkillChanged);

        // Vérifier immédiatement l'état actuel de la compétence au cas où elle serait déjà active
        // (par exemple, si la plateforme est spawnée après que la compétence ait été acquise)
        HandlePlayerSkillChanged(CachedPlayerCharacter->CanSeeHiddenPlatforms()); //
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AMyHiddenPlatform (%s): Impossible de trouver AMyFPSPlayerCharacter pour s'abonner aux événements de compétence."), *GetName());
    }
}

void AMyHiddenPlatform::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Très important : Se désabonner du delegate pour éviter les crashs si la plateforme est détruite avant le joueur
    if (CachedPlayerCharacter)
    {
        CachedPlayerCharacter->OnSkillSeeHiddenPlatformsChanged.RemoveDynamic(this, &AMyHiddenPlatform::HandlePlayerSkillChanged);
    }
}

void AMyHiddenPlatform::HandlePlayerSkillChanged(bool bNewVisibilityState)
{
    UE_LOG(LogTemp, Log, TEXT("AMyHiddenPlatform (%s): Reçu HandlePlayerSkillChanged, nouvel état de visibilité: %s"), *GetName(), bNewVisibilityState ? TEXT("True") : TEXT("False"));
    PlatformMesh->SetVisibility(bNewVisibilityState);
    PlatformMesh->SetCollisionEnabled(bNewVisibilityState ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    // Ajoutez ici d'autres logiques si nécessaire (ex: jouer un son/VFX de révélation/dissimulation)
}