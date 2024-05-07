// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "AgentTrait.h"
#include "MassEntityConfigAsset.h"
#include "GatherBD.generated.h"

/**
 * 
 */
UCLASS()
class MASSTEST_API UGatherBD : public USmartObjectMassBehaviorDefinition
{
	GENERATED_BODY()

public:
	virtual void Activate(FMassCommandBuffer& CommandBuffer,const FMassBehaviorEntityContext& EntityContext) const;

	virtual void Deactivate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const;

	UPROPERTY(EditDefaultsOnly, Category = SmartObject)
		int ResourceAmount = 1;

	UPROPERTY(EditDefaultsOnly, Category = SmartObject)
		TEnumAsByte<EResourceType> ResourceType = Tree;
	UPROPERTY(EditDefaultsOnly, Category = SmartObject)
		UMassEntityConfigAsset* ItemConfig;
};
