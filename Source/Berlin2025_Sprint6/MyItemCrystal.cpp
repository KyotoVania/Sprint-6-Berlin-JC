// MyItemCrystal.cpp
#include "MyItemCrystal.h"

#include "MyFPSPlayerCharacter.h" // Bien pour le Cast
#include "MyItemCrystalPedestal.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"   // Bien pour AmbienceTriggerSphere
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h" // Assurez-vous que cet include est présent ! Il manquait dans votre extrait.
#include "UObject/ConstructorHelpers.h" 
#include "Kismet/GameplayStatics.h"

AMyItemCrystal::AMyItemCrystal()
{
    PrimaryActorTick.bCanEverTick = true; 
	PotentialSkillToGrant = ESkillType::None;

    // SM_Shape est créé dans AMyMasterItem, donc pas besoin de le recréer ici.
    // S'assurer qu'il est bien le composant racine par défaut ou attaché correctement.
    // Dans AMyMasterItem, RootComponent = DefaultSceneRoot, et SM_Shape est attaché à DefaultSceneRoot. C'est OK.

    PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    PointLightComponent->SetupAttachment(SM_Shape); // Attacher au mesh visible

    CrystalVFXComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CrystalVFX"));
    CrystalVFXComponent->SetupAttachment(SM_Shape); // Attacher au mesh visible

    RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
    RotatingMovementComponent->UpdatedComponent = SM_Shape; // Le SM_Shape est celui qui tourne

    // Les composants pour l'ambiance
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudio"));
    AmbienceAudioComponent->SetupAttachment(RootComponent); // Attaché à la racine de l'acteur
    AmbienceAudioComponent->bAutoActivate = false; 

    AmbienceTriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmbienceTriggerSphere"));
    AmbienceTriggerSphere->SetupAttachment(RootComponent); // Attaché à la racine de l'acteur
    AmbienceTriggerSphere->SetCollisionProfileName(TEXT("Trigger")); // Pour les overlaps
    AmbienceTriggerSphere->SetSphereRadius(500.0f); // Valeur par défaut, sera ajustée par AmbienceSoundRadius

    CrystalTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CrystalTimeline"));

    // Valeurs par défaut
    CrystalType = ECrystalType::Joy; 
    CrystalPropertiesTable = nullptr;
    FloatationCurve = nullptr;
    LoadedCrystalSFX = nullptr; // Sera chargé par SetupCrystalFromDataTable
    AmbienceSoundRadius = 500.0f; 
}

void AMyItemCrystal::BeginPlay()
{
    Super::BeginPlay();

    SetupCrystalFromDataTable();    // Doit être appelé avant InitializeAmbienceSystem si ce dernier utilise LoadedCrystalSFX
    InitializeFloatationTimeline();
    InitializeAmbienceSystem();     // Configure l'AudioComponent et les overlaps

    if (SM_Shape) // SM_Shape est le composant mesh principal
    {
        InitialRelativeLocation = SM_Shape->GetRelativeLocation();
    }
    
    // Appliquer le rayon configuré si AmbienceTriggerSphere est valide
    if (AmbienceTriggerSphere) { 
       AmbienceTriggerSphere->SetSphereRadius(AmbienceSoundRadius);
    }
}

void AMyItemCrystal::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Le TimelineComponent gère son propre Tick.
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

    LoadedCrystalSFX = FoundRowData->CrystalSFX; // C'est ce son qui sera utilisé pour l'ambiance
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

void AMyItemCrystal::InitializeAmbienceSystem()
{
    if (LoadedCrystalSFX && AmbienceAudioComponent)
    {
       AmbienceAudioComponent->SetSound(LoadedCrystalSFX);
       
    }
    else
    {
       if (!LoadedCrystalSFX) UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystal %s: LoadedCrystalSFX (pour ambiance) est null. Vérifiez la DataTable et l'assignation."), *GetNameSafe(this));
       if (!AmbienceAudioComponent) UE_LOG(LogTemp, Warning, TEXT("AMyItemCrystal %s: AmbienceAudioComponent est null."), *GetNameSafe(this));
    }

    if (AmbienceTriggerSphere)
    {
       AmbienceTriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyItemCrystal::OnAmbienceSphereBeginOverlap);
       AmbienceTriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AMyItemCrystal::OnAmbienceSphereEndOverlap);
       AmbienceTriggerSphere->SetSphereRadius(AmbienceSoundRadius); // S'assurer que le rayon est appliqué
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
	// La condition IsSimulatingPhysics() est retirée ici pour permettre au son de jouer même si droppé et en mouvement.
	// Le son sera géré par l'entrée/sortie de la sphère du joueur.
	return true;
}

void AMyItemCrystal::OnAmbienceSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyFPSPlayerCharacter* PlayerCharacter = Cast<AMyFPSPlayerCharacter>(OtherActor);
	if (PlayerCharacter && AmbienceAudioComponent && LoadedCrystalSFX)
	{
		if (ShouldPlayAmbienceSound()) // Utilisation de la nouvelle fonction helper
		{
			if (!AmbienceAudioComponent->IsPlaying())
			{
				AmbienceAudioComponent->Play();
				UE_LOG(LogTemp, Log, TEXT("Cristal %s: Son d'ambiance DÉMARRÉ (overlap & conditions remplies) pour %s"), *GetNameSafe(this), *GetNameSafe(OtherActor));
			}
		}
		else
		{
			// Si les conditions ne sont pas remplies (ex: sur un piédestal) et qu'il jouait, on l'arrête.
			if (AmbienceAudioComponent->IsPlaying())
			{
				AmbienceAudioComponent->Stop();
				UE_LOG(LogTemp, Log, TEXT("Cristal %s: Son d'ambiance ARRÊTÉ (conditions non remplies à l'overlap) pour %s"), *GetNameSafe(this), *GetNameSafe(OtherActor));
			}
		}
	}
}
void AMyItemCrystal::OnAmbienceSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AMyFPSPlayerCharacter* PlayerCharacter = Cast<AMyFPSPlayerCharacter>(OtherActor);
    if (PlayerCharacter && AmbienceAudioComponent && AmbienceAudioComponent->IsPlaying())
    {
        AmbienceAudioComponent->Stop(); // Ou FadeOut(0.5f, 0.0f); pour une transition douce
        UE_LOG(LogTemp, Log, TEXT("Cristal %s: Son d'ambiance (LoadedCrystalSFX) arrêté pour %s"), *GetNameSafe(this), *GetNameSafe(OtherActor));
    }
}
    void AMyItemCrystal::Interact_Implementation(AActor* InteractorActor)
{
    UE_LOG(LogTemp, Log, TEXT("Crystal %s interacted with by %s"), *GetNameSafe(this), *GetNameSafe(InteractorActor));

    AMyFPSPlayerCharacter* PlayerCharacter = Cast<AMyFPSPlayerCharacter>(InteractorActor);
    if (PlayerCharacter)
    {
       // Vérifie si le joueur a au moins une main de libre pour ramasser ce cristal
       if (!PlayerCharacter->GetRightHandCrystal() || !PlayerCharacter->GetLeftHandCrystal()) //
       {
          // PlayerCharacter->PickupCrystal va tenter de le mettre dans la main droite, puis la gauche.
          if (PlayerCharacter->PickupCrystal(this)) //
          {
             UE_LOG(LogTemp, Log, TEXT("Player %s picked up crystal %s"), *GetNameSafe(PlayerCharacter), *GetNameSafe(this));
             DisableOutline(); //
          }
          else
          {
             // Cela pourrait arriver si PickupCrystal a une logique interne qui échoue pour une autre raison,
             // ou si entre-temps les deux mains se sont remplies (peu probable dans un appel direct comme ici).
             UE_LOG(LogTemp, Warning, TEXT("Player %s failed to pick up crystal %s (PickupCrystal returned false)."), *GetNameSafe(PlayerCharacter), *GetNameSafe(this));
          }
       }
       else // Les deux mains du joueur sont pleines
       {
          // Construction du message pour le log, indiquant ce que tient le joueur
          FString RightCrystalName = PlayerCharacter->GetRightHandCrystal() ? PlayerCharacter->GetRightHandCrystal()->GetName() : TEXT("Nothing"); //
          FString LeftCrystalName = PlayerCharacter->GetLeftHandCrystal() ? PlayerCharacter->GetLeftHandCrystal()->GetName() : TEXT("Nothing"); //

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