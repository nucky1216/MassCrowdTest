// Fill out your copyright notice in the Description page of Project Settings.


#include "GatherBD.h"
#include "MassSpawnerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "ItemTrait.h"
#include "SmartObjectComponent.h"
#include "SmartObjectSubsystem.h"
#include "MassCommonFragments.h"
#include "MassSmartObjectFragments.h"
void UGatherBD::Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	UE_LOG(LogTemp, Log, TEXT("[GatherBD] Activate!"));
	FAgentFragment& Agent = EntityContext.EntityView.GetFragmentData<FAgentFragment>();

	Agent.ResouceHandle.Reset();


}

void UGatherBD::Deactivate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
	UE_LOG(LogTemp, Log, TEXT("[GatherBD] Deactivate!"));
	UMassSpawnerSubsystem* SpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(EntityContext.SmartObjectSubsystem.GetWorld());

	if (EntityContext.SmartObjectSubsystem.GetWorld() && UGameplayStatics::GetPlayerPawn(EntityContext.SmartObjectSubsystem.GetWorld(), 0))
	{
		TArray<FMassEntityHandle> Items;
		const FMassEntityTemplate* EntityTemplate = ItemConfig->GetConfig().GetOrCreateEntityTemplate(*UGameplayStatics::GetPlayerPawn(EntityContext.SmartObjectSubsystem.GetWorld(), 0), *ItemConfig);
		SpawnerSubsystem->SpawnEntities(*EntityTemplate, 4, Items);

		for (const FMassEntityHandle& ItemHandle : Items)
		{
			const FVector& SpawnLocation = EntityContext.EntityView.GetFragmentDataPtr<FTransformFragment>()->GetTransform().GetLocation();
			

			UE_LOG(LogTemp, Log, TEXT("Get SpawnLocation from Context.FragmentDataPtr:(%s)"), *SpawnLocation.ToString());

			FItemFragment ItemFragment;
			ItemFragment.ItemType = ResourceType;
			ItemFragment.OldLocation = SpawnLocation;
			CommandBuffer.PushCommand(FCommandAddFragmentInstance(ItemHandle, FConstStructView::Make(ItemFragment)));
		}
		const FMassSmartObjectUserFragment& SOUser = EntityContext.EntityView.GetFragmentData<FMassSmartObjectUserFragment>();
		if (USmartObjectComponent* SOComp = EntityContext.SmartObjectSubsystem.GetSmartObjectComponent(SOUser.ClaimHandle))
		{
			CommandBuffer.PushCommand(FDeferredCommand([SOComp, EntityContext](UMassEntitySubsystem& System)
				{
					UE_LOG(LogTemp, Log, TEXT("DeActivated1 SmartObject Registerd:%d"),SOComp->IsRegistered());
					EntityContext.SmartObjectSubsystem.UnregisterSmartObject(*SOComp);
					SOComp->GetOwner()->Destroy();
					UE_LOG(LogTemp, Log, TEXT("DeActivated2 SmartObject Registerd:%d"), SOComp->IsRegistered());
				}));
		}

	}	
	
}
