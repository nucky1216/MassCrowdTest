// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "SmartObjectSubsystem.h"
#include "Construction.generated.h"


/**
 * 
 */
UCLASS()
class MASSTEST_API UConstruction : public UMassObserverProcessor
{
	GENERATED_BODY()
		UConstruction();


	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	TObjectPtr<USmartObjectSubsystem> SmartObjectSubsystem;

	float IncrementHeight = 100.f;

	FMassEntityQuery EntityQuery;

};

USTRUCT()
struct MASSTEST_API FConstructionFloor :public FMassTag
{
	GENERATED_BODY()

};