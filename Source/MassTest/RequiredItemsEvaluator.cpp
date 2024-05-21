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
	//UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] Evaluating.."));

	FAgentFragment& Agent = Context.GetExternalData(AgentHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	UBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	//AgentEntity Location
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

	
	//UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] SOHandle Valid :%d"), SOHandle.IsValid());
	if (!EntitySubsystem.IsEntityValid(EntityHandle) && Agent.QueuedItems.Num() > 0)
	{
		EntityHandle = Agent.QueuedItems.Pop();
		if (EntitySubsystem.IsEntityValid(EntityHandle))
		{
			UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] Next ItemFound"));
			bFoundItemHandle = true;
			FItemFragment* ItemFragment = EntitySubsystem.GetFragmentDataPtr<FItemFragment>(EntityHandle);
			if (ItemFragment)
			{
				ItemFragment->bClaimed = true;
			}
			return;
		}
	}
	//未获取任何资源的Agent是否在等待命令
	if (Agent.QueuedItems.Num() <= 0 && !Agent.BuildingHandle.IsValid())
	{
		if (BuildingSubsystem.GetQueuedBuidlings() > 0)
		{
			FMassEntityHandle TreeHandle;
			FMassEntityHandle RockHandle;
			//UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] Detect Location:%s"),*Location.ToString());
			if (BuildingSubsystem.FindItem(Location, 3000.f, Rock, RockHandle))
			{
				UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] Found Rock!"));
				if (BuildingSubsystem.FindItem(Location, 3000.f, Tree, TreeHandle))
				{
					UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] Found Tree"));
					
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
	//建筑物有效，并且已采集到所需的资源，智能对象句柄设为建筑物的SO句柄
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
		//UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] QueuedResource Num:%d"), QueuedResources.Num());

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
	Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMoveToEntityTaskData, ItemHandle));

	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParameterHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMoveToEntityTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transisition) const
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

	
	//UE_LOG(LogTemp, Log, TEXT("[RequireEvaluator] CollectImte:MoveToEntityTask Entered:%s"), *Context.GetDebugInfoString());
	return EStateTreeRunStatus::Running;
}


EStateTreeRunStatus FMoveToEntityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	UBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::FollowPointPathDone, MassContext.GetEntity(),1);

	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FTransform& Transform = Context.GetExternalData(TransformHandle).GetTransform();

	MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
	MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

	//到达物品周围，销毁该物品
	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius + 100.f)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoveToEntityTask] 0-ItemHandle:%d "), EntitySubsystem.IsEntityValid(ItemHandle));
		EntitySubsystem.Defer().DestroyEntity(ItemHandle);
		BuildingSubsystem.ItemHashGrid.RemovePoint(ItemHandle, MoveTarget.Center);
		//UE_LOG(LogTemp, Log, TEXT("[MoveToEntityTask] CollectItem:MoveToEntity-> Collect Item Finished:%s "),*ItemHandle.DebugGetDescription());
		UE_LOG(LogTemp, Log, TEXT("[MoveToEntityTask] 1-ItemHandle:%d "), EntitySubsystem.IsEntityValid(ItemHandle));
		return EStateTreeRunStatus::Succeeded;

	}

	return EStateTreeRunStatus::Running;
}

bool FClaimSmartObjectTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkInstanceDataProperty(SmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FClaimSmartObjectTaskData, SOHandle));
	Linker.LinkInstanceDataProperty(ClaimResultHandle, STATETREE_INSTANCEDATA_PROPERTY(FClaimSmartObjectTaskData, ClaimResult));

	Linker.LinkExternalData(SmartObjectUserHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	return true;
}

EStateTreeRunStatus FClaimSmartObjectTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{

	
	const FSmartObjectHandle& SOHandle = Context.GetInstanceData(SmartObjectHandle);
	EMassSmartObjectClaimResult& ClaimResult = Context.GetInstanceData(ClaimResultHandle);

	FSmartObjectRequestFilter Filter;
	Filter.BehaviorDefinitionClass = USmartObjectBehaviorDefinition::StaticClass();

	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FMassSmartObjectUserFragment& SOUser = Context.GetExternalData(SmartObjectUserHandle);

	FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem.Claim(SOHandle, Filter);

	
	if (!ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;

	SOUser.ClaimHandle = ClaimHandle;
	SOUser.InteractionStatus = EMassSmartObjectInteractionStatus::Unset;
	const FTransform Transform = SmartObjectSubsystem.GetSlotTransform(SOUser.ClaimHandle).Get(FTransform::Identity);
	SOUser.TargetLocation = Transform.GetLocation();
	SOUser.TargetDirection = Transform.GetRotation().Vector();
	
	ClaimResult = EMassSmartObjectClaimResult::Succeeded;

	UE_LOG(LogTemp, Log, TEXT("[RequiredEvaluator]ClaimSmartObjectTask: Entered!"));

	return EStateTreeRunStatus::Succeeded;
}

bool FMoveTargetTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParamtersHandle);

	return true;
}

EStateTreeRunStatus FMoveTargetTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassSmartObjectUserFragment& SOUserFragment = Context.GetExternalData(SOUserHandle);
	const FMassMovementParameters& MoveParameters = Context.GetExternalData(MoveParamtersHandle);

	if (!SOUserFragment.ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;

	MoveTarget.Center = SOUserFragment.TargetLocation;
	MoveTarget.Forward = SOUserFragment.TargetDirection;
	MoveTarget.SlackRadius = 25.f;
	MoveTarget.DesiredSpeed.Set(MoveParameters.DefaultDesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
	//UE_LOG(LogTemp, Log, TEXT("[RequiredEvaluator]MoveTargetTask: Entered!"));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMoveTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::FollowPointPathDone, MassContext.GetEntity(), 1);

	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FTransform& Transform = Context.GetExternalData(TransformHandle).GetTransform();

	MoveTarget.DistanceToGoal = (MoveTarget.Center -Transform.GetLocation()).Length();
	MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

	//UE_LOG(LogTemp, Log, TEXT("[RequiredEvaluator]MoveTargetTask:Tick MoveTargetDistToGoal: %d"), MoveTarget.DistanceToGoal);

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius + 100.f)
	{
		UE_LOG(LogTemp, Log, TEXT("[RequiredEvaluator]MoveTargetTask Tick Successed!"));
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Running;
}
