// CrystalType.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h" 
#include "CrystalType.generated.h"

UENUM(BlueprintType)
enum class ECrystalType : uint8
{
	Anger		UMETA(DisplayName = "Anger"),
	Fear		UMETA(DisplayName = "Fear"),
	Joy			UMETA(DisplayName = "Joy"),
	Hope		UMETA(DisplayName = "Hope"),
	Remission	UMETA(DisplayName = "Remission"),
	Sadness		UMETA(DisplayName = "Sadness")
};