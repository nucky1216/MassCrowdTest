// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentTrait.h"
#include "BuildingSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include <Kismet/GameplayStatics.h>

void UAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);

	BuildContext.AddFragment<FAgentFragment>();

	BuildContext.AddTag<FAgent>();
	
	BuildContext.AddTag<FMassInNavigationObstacleGridTag>();

	const FConstSharedStruct AgentFragment = EntitySubsystem->GetOrCreateConstSharedFragment(UE::StructUtils::GetStructCrc32(FConstStructView::Make(AgentParameters)),AgentParameters);
	BuildContext.AddConstSharedFragment(AgentFragment);
}

UAgentInitializer::UAgentInitializer()
{
	ObservedType = FAgentFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;


}

void UAgentInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	
	NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
		{
			UBuildingSubsystem* BuildingSubsystem = GetWorld()->GetSubsystem<UBuildingSubsystem>();
			TArrayView<FMassMoveTargetFragment> MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			FMassMovementParameters MoveParms=Context.GetConstSharedFragment<FMassMovementParameters>();
			TArrayView<FAgentFragment> Agents = Context.GetMutableFragmentView<FAgentFragment>();

			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				BuildingSubsystem->AddAgent(Context.GetEntity(EntityIndex));
				FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIndex];
				FVector CurLoc = Transforms[EntityIndex].GetTransform().GetLocation();
				FAgentFragment& Agent = Agents[EntityIndex];
				FVector TargetLoc = FVector(0,0,0);
				

				MoveTarget.SlackRadius = 25.f;
				MoveTarget.DesiredSpeed.Set(MoveParms.DefaultDesiredSpeed);
				

				MoveTarget.CreateNewAction(EMassMovementAction::Move, *this->GetWorld());
				MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

				
				if (NavSys && UGameplayStatics::GetPlayerPawn(this->GetWorld(), 0))
				{
					
					
				 /* UNavigationPath* NavPath=NavSys->FindPathToLocationSynchronously(this,CurLoc,TargetLoc, UGameplayStatics::GetPlayerPawn(this->GetWorld(), 0));
					NavPath->
					UE_LOG(LogTemp, Log, TEXT("NaviSys is Valid Num:%d"),NavPath.Get);*/

					FPathFindingQuery Query(
						nullptr,
						*NavSys->GetDefaultNavDataInstance(),
						CurLoc,
						TargetLoc
					);
					
					FPathFindingResult PathResult = NavSys->FindPathSync(Query);

					const UEnum* EnumPtr = StaticEnum<ENavigationQueryResult::Type>();
					UE_LOG(LogTemp, Log, TEXT("NaviSys is Valid Finded:%s"), *EnumPtr->GetNameStringByValue(PathResult.Result));
					if (PathResult.IsSuccessful())
					{
						const TArray<FNavPathPoint>& PathPoints = PathResult.Path->GetPathPoints();
						UE_LOG(LogTemp, Log, TEXT("NaviPathPoints Num:%d"), PathPoints.Num());
						
						for (auto NPoint : PathPoints)
						{
							Agent.PathPoints.Add(NPoint.Location);
						}

						Agent.PathIdx = 0;
						if (PathPoints.Num() > 1)
						{
							MoveTarget.Center = Agent.PathPoints[Agent.PathIdx]; // 设置下一个路径点为目标
							MoveTarget.Forward = (MoveTarget.Center - CurLoc).GetSafeNormal();
						}
					}
				}
				
				FString EntityInfor = Context.GetEntity(EntityIndex).DebugGetDescription();
				UE_LOG(LogTemp, Log, TEXT("Entity[%d] Init MoveTarget.DesireSpeed:%f!, Center:%s CurLoc:%s,EntityHandle:%s")
					,EntityIndex,MoveTarget.DesiredSpeed.Get(),*MoveTarget.Center.ToString(),*CurLoc.ToString(),*EntityInfor);
			}
		}));
}

void UAgentInitializer::ConfigureQueries()
{

	EntityQuery.AddRequirement<FAgentFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddConstSharedRequirement<FAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);

	EntityQuery.AddTagRequirement<FAgent>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	
}

UAgentTestProcessor::UAgentTestProcessor()
{
	bAutoRegisterWithProcessingPhases = true;

	//ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::PrePhysics;
	UE_LOG(LogTemp, Log, TEXT("Test Processor Construct!"));
}

void UAgentTestProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
	
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [DeltaTime, this](FMassExecutionContext& Context)
		{
			const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMassMoveTargetFragment> MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
			TArrayView<FAgentFragment> Agents = Context.GetMutableFragmentView<FAgentFragment>();
			TArrayView<const FMassEntityHandle> EntityHandles = Context.GetEntities();
			

			for (int EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{

				SignalSubsystem->DelaySignalEntity(UE::Mass::Signals::FollowPointPathDone, Context.GetEntity(EntityIndex), 1.5f);

				//UE_LOG(LogTemp, Log, TEXT("Search Entity%d"), EntityIndex);
				FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIndex];
				FTransform& Transform = TransformList[EntityIndex].GetMutableTransform();
				FAgentFragment& Agent = Agents[EntityIndex];
				const FMassEntityHandle EntityHandle = EntityHandles[EntityIndex];
				
				MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
				MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

				const UEnum* EnumPtr = StaticEnum<EMassMovementAction>();
				FString ActionName = EnumPtr->GetNameStringByValue(static_cast<int64>(MoveTarget.GetCurrentAction()));

				if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius+100.f)
				{
					UE_LOG(LogTemp, Log, TEXT("Entity[%d] CurAction:%s GetAtTarget:%d/%d"),
						EntityHandle.Index, *ActionName, Agent.PathIdx,Agent.PathPoints.Num());
					
					if (Agent.PathPoints.IsEmpty())
					{
						Context.Defer().DestroyEntity(EntityHandle);
						UE_LOG(LogTemp, Log, TEXT("Entity[%d] Ready to Destroy!"), EntityHandle.Index);
					}
					else
					{
						FVector NextLoc = Agent.PathPoints[0];
						Agent.PathPoints.RemoveAt(0);
						MoveTarget.Center = NextLoc;
						MoveTarget.Forward = (MoveTarget.Center - Transform.GetTranslation()).GetSafeNormal();
					}

				}

				
			}
		});
}

void UAgentTestProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FAgentFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddConstSharedRequirement<FAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);

	EntityQuery.AddTagRequirement<FAgent>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UAgentTestProcessor::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	
	
}
