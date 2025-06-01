// MyMasterItem.cpp
#include "MyMasterItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h" // Si tu comptes modifier le matériau du décalque dynamiquement

AMyMasterItem::AMyMasterItem()
{
	PrimaryActorTick.bCanEverTick = false; // Désactivé par défaut, active si besoin

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	SM_Shape = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Shape"));
	SM_Shape->SetupAttachment(RootComponent);

	Decal_Outline = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal_Outline"));
	Decal_Outline->SetupAttachment(SM_Shape); // Attaché au mesh pour suivre ses transformations
	Decal_Outline->SetVisibility(false); // Caché par défaut

	// Valeurs par défaut si besoin
	DecalSize = FVector(10.0f, 128.0f, 128.0f); // Exemple, ajuste selon tes besoins
	ShapeScale = FVector(1.0f);
	bIsOutlineEnabled = false;
}

void AMyMasterItem::BeginPlay()
{
	Super::BeginPlay();
	SM_Shape->SetRelativeScale3D(ShapeScale);
	Decal_Outline->DecalSize = DecalSize;
	// Tu peux aussi définir le matériau du décalque ici si besoin
	// Decal_Outline->SetDecalMaterial(...);
}

void AMyMasterItem::EnableOutline()
{
	if (!bIsOutlineEnabled)
	{
		Decal_Outline->SetVisibility(true);
		bIsOutlineEnabled = true;
		// Logique du "DoOnce" est gérée par bIsOutlineEnabled
	}
}

void AMyMasterItem::DisableOutline()
{
	if (bIsOutlineEnabled) // Si tu veux que ça agisse comme un reset du DoOnce
	{
		Decal_Outline->SetVisibility(false);
		bIsOutlineEnabled = false;
	}
}

// Implémentation C++ de la fonction d'interface
void AMyMasterItem::StartLookAtInteractable_Implementation()
{
	// Ce qui était branché à "Event Start Look At Interactable" dans ton BP_MasterItem
	EnableOutline();
}

// Implémentation C++ de la fonction d'interface
void AMyMasterItem::StopLookAtInteractable_Implementation()
{
	// Ce qui était branché à "Event Stop Look At Interactable" dans ton BP_MasterItem
	DisableOutline();
}

void AMyMasterItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}