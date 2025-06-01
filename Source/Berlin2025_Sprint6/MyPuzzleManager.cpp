// MyPuzzleManager.cpp
#include "MyPuzzleManager.h"
#include "MyItemCrystalPedestal.h"
// Incluez ici votre classe de porte si vous en utilisez une spécifique (par ex. "MyMasterDoor.h")
// #include "MyMasterDoor.h" // Exemple

AMyPuzzleManager::AMyPuzzleManager()
{
    PrimaryActorTick.bCanEverTick = false; // Pas besoin de Tick par défaut
    CorrectPedestalsCount = 0;
}

void AMyPuzzleManager::BeginPlay()
{
    Super::BeginPlay();

    CorrectPedestalsCount = 0; // Réinitialiser au cas où
    int32 InitialCorrectPedestals = 0;

    if (PedestalsToMonitor.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("APuzzleManager (%s): Aucun piédestal assigné dans PedestalsToMonitor."), *GetName());
        return;
    }

    for (AMyItemCrystalPedestal* Pedestal : PedestalsToMonitor)
    {
        if (Pedestal)
        {
            // S'abonner à l'événement OnPedestalStateChanged de chaque piédestal
            Pedestal->OnPedestalStateChanged.AddDynamic(this, &AMyPuzzleManager::HandlePedestalStateChanged);

            // Vérifier l'état initial au cas où des cristaux seraient déjà placés correctement au démarrage du niveau
            if (Pedestal->bIsCorrectCrystalOnPedestal) // Assurez-vous que bIsCorrectCrystalOnPedestal est accessible (public ou via getter)
            {
                InitialCorrectPedestals++;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("APuzzleManager (%s): Un piédestal null a été trouvé dans PedestalsToMonitor."), *GetName());
        }
    }
    CorrectPedestalsCount = InitialCorrectPedestals; // Mettre à jour le compteur initial
    UE_LOG(LogTemp, Log, TEXT("APuzzleManager (%s): Initialisé avec %d piédestaux. %d sont initialement corrects."), *GetName(), PedestalsToMonitor.Num(), CorrectPedestalsCount);

    // Vérifier si le puzzle est déjà complété au démarrage
    CheckPuzzleCompletion();
}

void AMyPuzzleManager::HandlePedestalStateChanged(bool bIsNowCorrect)
{
    // Cette fonction sera appelée par le délégué FOnPedestalStateChanged de AMyItemCrystalPedestal.
    // Le paramètre bIsNowCorrect nous dit si le piédestal qui a envoyé l'événement est maintenant correct.

    // Pour être plus précis, nous allons recalculer le nombre de piédestaux corrects
    // plutôt que de simplement incrémenter/décrémenter, pour éviter des erreurs si les événements
    // ne sont pas parfaitement synchronisés ou si l'état initial était complexe.
    int32 CurrentCorrectCount = 0;
    for (AMyItemCrystalPedestal* Pedestal : PedestalsToMonitor)
    {
        if (Pedestal && Pedestal->bIsCorrectCrystalOnPedestal) // Accès direct à bIsCorrectCrystalOnPedestal
        {
            CurrentCorrectCount++;
        }
    }
    CorrectPedestalsCount = CurrentCorrectCount;

    UE_LOG(LogTemp, Log, TEXT("APuzzleManager (%s): Un piédestal a changé d'état. Total piédestaux corrects: %d/%d"), *GetName(), CorrectPedestalsCount, PedestalsToMonitor.Num());

    CheckPuzzleCompletion();
}

void AMyPuzzleManager::CheckPuzzleCompletion()
{
    if (PedestalsToMonitor.Num() == 0) return; // Pas de piédestaux, pas de puzzle

    if (CorrectPedestalsCount == PedestalsToMonitor.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("APuzzleManager (%s): Puzzle COMPLÉTÉ! Tous les %d piédestaux sont corrects."), *GetName(), PedestalsToMonitor.Num());
        OnPuzzleCompleted.Broadcast();

        // Logique pour ouvrir la porte
        if (DoorToOpen)
        {
            UE_LOG(LogTemp, Log, TEXT("APuzzleManager (%s): Tentative d'ouverture de la porte: %s"), *GetName(), *DoorToOpen->GetName());
            // Ici, vous appelleriez une fonction sur votre porte pour l'ouvrir.
            // Par exemple, si votre porte a une fonction OpenDoor() :
            // Cast<AMyMasterDoor>(DoorToOpen)->OpenDoor();
            // Ou si c'est un acteur générique, vous pourriez vouloir détruire l'acteur,
            // ou activer/désactiver sa collision, ou jouer une animation via une interface.
            // Pour l'instant, un simple log :
            DoorToOpen->SetActorHiddenInGame(true); // Exemple simple: cacher la porte
            DoorToOpen->SetActorEnableCollision(false); // Exemple: désactiver la collision
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("APuzzleManager (%s): Puzzle complété, mais aucune porte n'est assignée à DoorToOpen."), *GetName());
        }

        // Optionnel: Se désabonner des piédestaux si le puzzle ne peut être résolu qu'une fois.
        for (AMyItemCrystalPedestal* Pedestal : PedestalsToMonitor)
        {
            if (Pedestal)
            {
                Pedestal->OnPedestalStateChanged.RemoveDynamic(this, &AMyPuzzleManager::HandlePedestalStateChanged);
            }
        }
    }
    // else: le puzzle n'est pas (ou plus) complété.
}