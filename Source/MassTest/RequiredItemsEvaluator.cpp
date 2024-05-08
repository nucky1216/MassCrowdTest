// Fill out your copyright notice in the Description page of Project Settings.


#include "RequiredItemsEvaluator.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "MassStateTreeExecutionContext.h"
#include "BuildingSubsystem.h"
#include "SmartObjectSubsystem.h"
#include "ItemTrait.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectFragments.h"

void FRequiredItemsEvaluator::Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	if (EvalType == EStateTreeEvaluationType::Tick)
		return ;
	FAgentFragment& Agent = Context.GetExternalData(AgentHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	UBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	const FVector& Location = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();

	FSmartObjectRequestFilter& Filter = Context.GetInstanceData(FilterHandle);
	bool& bFoundSmartObject = Context.GetInstanceData(FoundSmartObjectHandle);
	bool& bFoundItemHandle = Context.GetInstanceData(FoundItemHandle);
	FSmartObjectHandle& SOHandle = Context.GetInstanceData(SmartObjectHandle);
	FMassEntityHandle& EntityHandle = Context.GetInstanceData(ItemHandle);

	bFoundSmartObject = false;
	bFoundItemHandle = false;

	Filter.BehaviorDefinitionClass = USmartObjectBehaviorDefinition::StaticClass();

	FSmartObjectRequest Request;
	Request.QueryBox = FBox::BuildAABB(Location, FVector(5000.0f));

	FGameplayTagQueryExpression Query;
	Query.AllTagsMatch();


	if (!EntitySubsystem.IsEntityValid(EntityHandle) && Agent.QueuedItems.Num() > 0)
	{
		EntityHandle = Agent.QueuedItems.Pop();
		if (EntitySubsystem.IsEntityValid(EntityHandle))
		{
			bFoundItemHandle = true;
			FItemFragment* ItemFragment = EntitySubsystem.GetFragmentDataPtr<FItemFragment>(EntityHandle);
			if (ItemFragment)
			{
				ItemFragment->bClaimed = true;
			}
		}
	}
	//Agent是否在等待命令
	if (Agent.QueuedItems.Num() <= 0 && !Agent.BuildingHandle.IsValid())
	{
		if (BuildingSubsystem.GetQueuedBuidlings() > 0)
		{
			FMassEntityHandle TreeHandle;
			FMassEntityHandle RockHandle;

			if (BuildingSubsystem.FindItem(Location, 5000.f, Rock, RockHandle))
			{
				if (BuildingSubsystem.FindItem(Location, 5000.f, Tree, TreeHandle))
				{
					bFoundItemHandle = true;
					BuildingSubsystem.CalimFloor(Agent.BuildingHandle);

					Agent.QueuedItems.Emplace(TreeHandle);
					Agent.QueuedItems.Emplace(RockHandle);

					EntityHandle = Agent.QueuedItems.Pop();
					FItemFragment* ItemFragment = EntitySubsystem.GetFragmentDataPtr<FItemFragment>(EntityHandle);
					if (ItemFragment)
					{
						ItemFragment->bClaimed = true;
					}

					return;
				}
			}
		}
	}

	if (Agent.BuildingHandle.IsValid() && Agent.QueuedItems.Num() == 0)
	{
		bFoundSmartObject = true;
		SOHandle = Agent.BuildingHandle;
		return;
	}

	//检查排队的资源
	TArray<FSmartObjectHandle> QueuedResources;
	BuildingSubsystem.GetQueuedResources(QueuedResources);
	if (QueuedResources.Num() > 0 && !Agent.ResouceHandle.IsValid())
	{
		FSmartObjectHandle ResourceHandle;
		bFoundSmartObject = true;
		BuildingSubsystem.ClaimResources(ResourceHandle);
		Agent.ResouceHandle = ResourceHandle;
		SOHandle = ResourceHandle;
	}
}

bool FRequiredItemsEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AgentHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);

	Linker.LinkInstanceDataProperty(FoundSmartObjectHandle,STATETREE_INSTANCEDATA_PROPERTY(FequiredItemsEvaluatorData,bFoundSmartObject));
	Linker.LinkInstanceDataProperty(FilterHandle, STATETREE_INSTANCEDATA_PROPERTY(FequiredItemsEvaluatorData, Filter));
	Linker.LinkInstanceDataProperty(FoundItemHandle,STATETREE_INSTANCEDATA_PROPERTY(FequiredItemsEvaluatorData, bFoundItemHandle));
	Linker.LinkInstanceDataProperty(SmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FequiredItemsEvaluatorData, SmartObjectHandle));
	Linker.LinkInstanceDataProperty(ItemHandle, STATETREE_INSTANCEDATA_PROPERTY(FequiredItemsEvaluatorData, ItemHandle));

	return true;
}

bool FMoveToEntityTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMoveToENtityTaskData, ItemHandle));

	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParameterHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMoveToEntityTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transisition)
{
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassMovementParameters& MoveParameters = Context.GetExternalData(MoveParameterHandle);
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);

	if (!EntitySubsystem.IsEntityValid(ItemHandle))
		return EStateTreeRunStatus::Failed;

	const FVector& Location = EntitySubsystem.GetFragmentDataChecked<FTransformFragment>(ItemHandle).GetTransform().GetLocation();

	MoveTarget.Center = Location;
	MoveTarget.SlackRadius = 25.f;
	MoveTarget.DesiredSpeed.Set(MoveParameters.DefaultDesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMoveToEntityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	//39:13
	return EStateTreeRunStatus();
}
