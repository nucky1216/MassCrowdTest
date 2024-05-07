// Fill out your copyright notice in the Description page of Project Settings.


#include "ConstructLevel.h"
#include "Construction.h"

void UConstructLevel::Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);
	CommandBuffer.PushCommand(FCommandAddTag(EntityContext.EntityView.GetEntity(), FConstructionFloor::StaticStruct()));
}

void UConstructLevel::Deactivate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
}
