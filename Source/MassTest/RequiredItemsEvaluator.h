// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgentTrait.h"
#include "MassCommonFragments.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassSmartObjectRequest.h"
#include "MassSmartObjectTypes.h"
#include "RequiredItemsEvaluator.generated.h"


struct FMassMoveTargetFragment;
struct FTransformFragment;
struct FMassSmartObjectUserFragment;
class UMassSignalSubsystem;
struct FMassMovementParameters;

USTRUCT()
struct MASSTEST_API FequiredItemsEvaluatorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
			FSmartObjectRequestFilter Filter;
	UPROPERTY(EditAnywhere, Category = Output)
		FSmartObjectHandle SmartObjectHandle;

	UPROPERTY(EditAnywhere, Category = Output)
		FMassEntityHandle ItemHandle;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bFoundSmartObject = false;

	UPROPERTY(EditAnywhere, Category = Output)
		bool bFoundItemHandle = false;
};

USTRUCT()
struct MASSTEST_API FRequiredItemsEvaluator :public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

	virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FequiredItemsEvaluatorData::StaticStruct(); }

	TStateTreeExternalDataHandle<FAgentFragment> AgentHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<UMassEntitySubsystem> EntitySubsystemHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<UBuildingSubsystem> BuildingSubsystemHandle;

	TStateTreeExternalDataHandle<bool> FoundSmartObjectHandle;
	TStateTreeExternalDataHandle<FSmartObjectRequestFilter> FilterHandle;
	TStateTreeExternalDataHandle<bool> FoundItemHandle;
	TStateTreeExternalDataHandle<FSmartObjectHandle> SmartObjectHandle;
	TStateTreeExternalDataHandle<FMassEntityHandle> ItemHandle;
};