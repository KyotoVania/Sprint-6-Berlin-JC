// MyItemCrystal.cpp
#include "MyItemCrystal.h"

#include "MyFPSPlayerCharacter.h"
#include "MyItemCrystalPedestal.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"  // Nouveau
#include "Components/AudioComponent.h"
#include "UObject/ConstructorHelpers.h" 
#include "Kismet/GameplayStatics.h"

AMyItemCrystal::AMyItemCrystal()
{
    PrimaryActorTick.bCanEverTick = true; 
	PotentialSkillToGrant = ESkillType::None;

    PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    PointLightComponent->SetupAttachment(SM_Shape);

    CrystalVFXComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CrystalVFX"));
    CrystalVFXComponent->SetupAttachment(SM_Shape);

    RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
    RotatingMovementComponent->UpdatedComponent = SM_Shape;

    CrystalTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CrystalTimeline"));

    // Valeurs par défaut
    CrystalType = ECrystalType::Joy; 
    CrystalPropertiesTable = nullptr;
    FloatationCurve = nullptr;
    LoadedCrystalSFX = nullptr;
    AmbienceSoundAttenuation = nullptr;  // Sera configuré dans l'éditeur ou par code
    CurrentAmbienceAudioComponent = nullptr;
}

void AMyItemCrystal::BeginPlay()
{
    Super::BeginPlay();

    SetupCrystalFromDataTable();
    InitializeFloatationTimeline();

    if (SM_Shape)
    {
        InitialRelativeLocation = SM_Shape->GetRelativeLocation();
    }
    
    // Démarrer automatiquement le son d'ambiance si les conditions sont remplies
    if (ShouldPlayAmbienceSound())
    {
        StartAmbienceSound();
    }
}

void AMyItemCrystal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Nettoyer le son d'ambiance lors de la destruction
    StopAmbienceSound();
    Super::EndPlay(EndPlayReason);
}

void AMyItemCrystal::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Vérifier périodiquement si les conditions pour jouer le son ont changé
    static float LastSoundCheck = 0.0f;
    LastSoundCheck += DeltaTime;
    
    if (LastSoundCheck >= 0.5f) // Vérifier toutes les 0.5 secondes
    {
        LastSoundCheck = 0.0f;
        
        bool ShouldPlay = ShouldPlayAmbienceSound();
        bool IsCurrentlyPlaying = IsAmbienceSoundPlaying();
        
        if (ShouldPlay && !IsCurrentlyPlaying)
        {
            StartAmbienceSound();
        }
        else if (!ShouldPlay && IsCurrentlyPlaying)
        {
            StopAmbienceSound();
        }
    }
}

void AMyItemCrystal::SetupCrystalFromDataTable()
{
    if (!CrystalPropertiesTable)
    {
       UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystal: CrystalPropertiesTable non assigné pour %s"), *GetNameSafe(this));
       return;
    }

    TArray<FName> RowNames = CrystalPropertiesTable->GetRowNames();
    FCrystalPropertiesRow* FoundRowData = nullptr;

    for (const FName& RowName : RowNames)
    {
       FCrystalPropertiesRow* CurrentRow = CrystalPropertiesTable->FindRow<FCrystalPropertiesRow>(RowName, TEXT("CrystalSetupContext"));
       if (CurrentRow && CurrentRow->CrystalType == this->CrystalType) 
       {
          FoundRowData = CurrentRow;
          break; 
       }
    }

    if (!FoundRowData)
    {
       const UEnum* EnumPtr = StaticEnum<ECrystalType>();
       FString CrystalTypeString = EnumPtr ? EnumPtr->GetNameStringByValue(static_cast<int64>(this->CrystalType)) : TEXT("INVALID_TYPE");
       UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystal: Aucune ligne avec CrystalType '%s' trouvée dans CrystalPropertiesTable pour %s"), *CrystalTypeString, *GetNameSafe(this));
       return;
    }

    if (SM_Shape)
    {
       if (FoundRowData->CrystalMesh)
       {
          SM_Shape->SetStaticMesh(FoundRowData->CrystalMesh);
       }
       if (FoundRowData->CrystalMaterial)
       {
          SM_Shape->SetMaterial(0, FoundRowData->CrystalMaterial);
       }
    }

    if (PointLightComponent)
    {
       PointLightComponent->SetLightColor(FoundRowData->LightColor);
       PointLightComponent->SetIntensity(FoundRowData->LightIntensity);
    }

    if (CrystalVFXComponent && FoundRowData->CrystalVFX)
    {
       CrystalVFXComponent->SetTemplate(FoundRowData->CrystalVFX);
       CrystalVFXComponent->ActivateSystem(true);
    }

    LoadedCrystalSFX = FoundRowData->CrystalSFX;
}

void AMyItemCrystal::InitializeFloatationTimeline()
{
    if (CrystalTimeline && FloatationCurve)
    {
       FOnTimelineFloat TimelineUpdateDelegate;
       TimelineUpdateDelegate.BindUFunction(this, FName("TimelineUpdate_Floatation"));

       CrystalTimeline->AddInterpFloat(FloatationCurve, TimelineUpdateDelegate);
       CrystalTimeline->SetLooping(true);
       CrystalTimeline->PlayFromStart();
    }
    else
    {
       if (!FloatationCurve)
       {
          UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystal: FloatationCurve non assigné pour %s"), *GetNameSafe(this));
       }
    }
}

void AMyItemCrystal::TimelineUpdate_Floatation(float Value)
{
    if (SM_Shape) 
    {
       FVector NewLocation = InitialRelativeLocation;
       NewLocation.Z += Value; 
       SM_Shape->SetRelativeLocation(NewLocation);
    }
}

bool AMyItemCrystal::ShouldPlayAmbienceSound() const
{
	AActor* ParentActor = GetAttachParentActor();
	if (ParentActor)
	{
		// Ne pas jouer le son si le cristal est attaché à un piédestal
		if (Cast<AMyItemCrystalPedestal>(ParentActor))
		{
			return false;
		}
		// Si attaché à autre chose (comme le joueur), le son peut jouer.
	}
	// Si pas attaché (au sol), le son peut jouer.
	return true;
}

void AMyItemCrystal::StartAmbienceSound()
{
    if (!LoadedCrystalSFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystal %s: Impossible de démarrer le son d'ambiance - LoadedCrystalSFX est null"), *GetNameSafe(this));
        return;
    }

    // Arrêter le son existant s'il y en a un
    StopAmbienceSound();

    // Utiliser SpawnSoundAttached pour créer le son avec atténuation
    CurrentAmbienceAudioComponent = UGameplayStatics::SpawnSoundAttached(
        LoadedCrystalSFX,           // Le son à jouer
        SM_Shape,                   // Composant auquel attacher le son (le mesh du cristal)
        NAME_None,                  // Pas de socket spécifique
        FVector::ZeroVector,        // Pas d'offset de location
        FRotator::ZeroRotator,      // Pas d'offset de rotation
        EAttachLocation::KeepRelativeOffset,  // Type d'attachement
        true,                       // bStopWhenAttachedToDestroyed
        1.0f,                       // VolumeMultiplier
        1.0f,                       // PitchMultiplier
        0.0f,                       // StartTime
        AmbienceSoundAttenuation,   // Paramètres d'atténuation
        nullptr,                    // Pas de concurrency settings
        false                       // bAutoDestroy - false pour pouvoir contrôler l'arrêt
    );

    if (CurrentAmbienceAudioComponent)
    {
        // Configurer pour jouer en boucle
        CurrentAmbienceAudioComponent->SetIntParameter(FName("Loop"), 1);
        UE_LOG(LogTemp, Log, TEXT("Cristal %s: Son d'ambiance démarré avec SpawnSoundAttached"), *GetNameSafe(this));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cristal %s: Échec du démarrage du son d'ambiance avec SpawnSoundAttached"), *GetNameSafe(this));
    }
}

void AMyItemCrystal::StopAmbienceSound()
{
    if (CurrentAmbienceAudioComponent && IsValid(CurrentAmbienceAudioComponent))
    {
        CurrentAmbienceAudioComponent->Stop();
        CurrentAmbienceAudioComponent = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Cristal %s: Son d'ambiance arrêté"), *GetNameSafe(this));
    }
}

bool AMyItemCrystal::IsAmbienceSoundPlaying() const
{
    return CurrentAmbienceAudioComponent && IsValid(CurrentAmbienceAudioComponent) && CurrentAmbienceAudioComponent->IsPlaying();
}

void AMyItemCrystal::Interact_Implementation(AActor* InteractorActor)
{
    UE_LOG(LogTemp, Log, TEXT("Crystal %s interacted with by %s"), *GetNameSafe(this), *GetNameSafe(InteractorActor));

    AMyFPSPlayerCharacter* PlayerCharacter = Cast<AMyFPSPlayerCharacter>(InteractorActor);
    if (PlayerCharacter)
    {
       if (!PlayerCharacter->GetRightHandCrystal() || !PlayerCharacter->GetLeftHandCrystal())
       {
          if (PlayerCharacter->PickupCrystal(this))
          {
             UE_LOG(LogTemp, Log, TEXT("Player %s picked up crystal %s"), *GetNameSafe(PlayerCharacter), *GetNameSafe(this));
             DisableOutline();
             // Le son continuera automatiquement car le cristal est maintenant attaché au joueur
          }
          else
          {
             UE_LOG(LogTemp, Warning, TEXT("Player %s failed to pick up crystal %s (PickupCrystal returned false)."), *GetNameSafe(PlayerCharacter), *GetNameSafe(this));
          }
       }
       else
       {
          FString RightCrystalName = PlayerCharacter->GetRightHandCrystal() ? PlayerCharacter->GetRightHandCrystal()->GetName() : TEXT("Nothing");
          FString LeftCrystalName = PlayerCharacter->GetLeftHandCrystal() ? PlayerCharacter->GetLeftHandCrystal()->GetName() : TEXT("Nothing");

          UE_LOG(LogTemp, Log, TEXT("Player %s has both hands full (Right: %s, Left: %s). Cannot pick up %s."), 
             *GetNameSafe(PlayerCharacter), 
             *RightCrystalName,
             *LeftCrystalName,
             *GetNameSafe(this));
       }
    }
    else
    {
       UE_LOG(LogTemp, Warning, TEXT("Interact_Implementation on AMyItemCrystal called by a non-AMyFPSPlayerCharacter actor: %s"), *GetNameSafe(InteractorActor));
    }
}