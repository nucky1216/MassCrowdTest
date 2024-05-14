// Fill o"ut your copyright notice in the Description page of Project Settings.


#include "Construction.h"
#include "AgentTrait.h"
#include "SmartObjectComponent.h"
#include "MassSmartObjectFragments.h"
#include "Components/InstancedStaticMeshComponent.h"

UConstruction::UConstruction()
{
	ObservedType = FConstructionFloor::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UConstruction::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
		{
			TArrayView<FAgentFragment> Agents = Context.GetMutableFragmentView<FAgentFragment>();
			TConstArrayView<FMassSmartObjectUserFragment> SOUsers = Context.GetFragmentView<FMassSmartObjectUserFragment>();

			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				FAgentFragment& Agent = Agents[EntityIndex];
				const FMassSmartObjectUserFragment& SOUser = SOUsers[EntityIndex];

				if (const USmartObjectComponent* SmartObjectComponent = SmartObjectSubsystem->GetSmartObjectComponent(SOUser.ClaimHandle))
				{
					const AActor* Actor = SmartObjectComponent->GetOwner();
					UInstancedStaticMeshComponent* InstancedStaticMeshComp = Actor->FindComponentByClass<UInstancedStaticMeshComponent>();
					FTransform Transform;
					Transform.SetLocation(FVector(0, 0, IncrementHeight * InstancedStaticMeshComp->GetInstanceCount()));
					InstancedStaticMeshComp->AddInstance(Transform);

					Agent.BuildingHandle = FSmartObjectHandle::Invalid;
					Context.Defer().RemoveTag<FConstructionFloor>(Context.GetEntity(EntityIndex));

					int* ResourceAmount = Agent.Inventory.Find(EResourceType::Rock);
					if (ResourceAmount)
						*ResourceAmount -= 1;
					ResourceAmount = Agent.Inventory.Find(EResourceType::Tree);
					if (ResourceAmount)
						*ResourceAmount -= 1;




				}
			}
		});
}

void UConstruction::ConfigureQueries()
{
	EntityQuery.AddRequirement<FAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassSmartObjectUserFragment>(EMassFragmentAccess::ReadOnly);
}

void UConstruction::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
}
