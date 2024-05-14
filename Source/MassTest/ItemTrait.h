// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "AgentTrait.h"
#include "BuildingSubsystem.h"
#include "MassRepresentationSubsystem.h"
#include "ItemTrait.generated.h"

/**
 * 
 */
UCLASS()
class MASSTEST_API UItemTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

		virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

};

USTRUCT()
struct MASSTEST_API FItemFragment :public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
		TEnumAsByte<EResourceType> ItemType;
	
	UPROPERTY()
		FVector OldLocation;

	UPROPERTY()
		bool bClaimed = false;

};

USTRUCT()
struct MASSTEST_API FItemAddedToGrid :public FMassTag
{
	GENERATED_BODY()

};

UCLASS()
class MASSTEST_API UItemInitializerProcessor :public UMassObserverProcessor
{
	GENERATED_BODY()

		UItemInitializerProcessor();
public:
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

	UPROPERTY()
		UBuildingSubsystem* BuildingSubsystem;

	UPROPERTY()
		TObjectPtr<UMassRepresentationSubsystem> RepresentationSubsystem;
};

UCLASS()
class MASSTEST_API UItemProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UItemProcessor();
public:
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

	UPROPERTY()
		UBuildingSubsystem* BuildingSubsystem;

	UPROPERTY()
		TObjectPtr<UMassRepresentationSubsystem> RepresentationSubsystem;

};