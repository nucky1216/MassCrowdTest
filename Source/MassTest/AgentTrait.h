// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "SmartObjectTypes.h"
#include "MassObserverProcessor.h"
//#include "BuildingSubsystem.h"
#include "AgentTrait.generated.h"

class UBuildingSubsystem;

UENUM()
enum EResourceType
{
	Tree,
	Rock
};

USTRUCT(BlueprintType)
struct MASSTEST_API FAgentFragment :public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MassTest")
		TMap<TEnumAsByte<EResourceType>, int> Inventory;// 库存<类型，数量>;

	UPROPERTY()
		FSmartObjectHandle BuildingHandle;
	UPROPERTY()
		FSmartObjectHandle ResouceHandle;
	UPROPERTY()
		TArray<FMassEntityHandle> QueuedItems;
};

USTRUCT()
struct MASSTEST_API FAgentParameters : public FMassSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
		TMap<TEnumAsByte<EResourceType>, int> DefaultRequiredResources;
};
/**
 * 
 */
UCLASS()
class MASSTEST_API UAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

		virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
	
	UPROPERTY(EditAnywhere)
		FAgentParameters AgentParameters;
};

UCLASS()
class MASSTEST_API UAgentInitializer :public UMassObserverProcessor
{
	GENERATED_BODY()

	UAgentInitializer();
	
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;

	TObjectPtr<UBuildingSubsystem> MovementSubsystem;
	TObjectPtr<USmartObjectSubsystem> SmartObjectSubsystem;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct MASSTEST_API FAgent : public FMassTag
{
	GENERATED_BODY()
};