// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentTrait.h"
#include "BuildingSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "MassCommonFragments.h"

void UAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);

	BuildContext.AddFragment<FAgentFragment>();

	BuildContext.AddTag<FAgent>();

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
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
		{
			UBuildingSubsystem* BuildingSubsystem = GetWorld()->GetSubsystem<UBuildingSubsystem>();

			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				BuildingSubsystem->AddAgent(Context.GetEntity(EntityIndex));

				UE_LOG(LogTemp, Log, TEXT("ObeserveProcessor Execute! Idx:%d"), EntityIndex);
			}
		}));
}

void UAgentInitializer::ConfigureQueries()
{

	EntityQuery.AddRequirement<FAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FAgent>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
}

//UAgentTestProcessor::UAgentTestProcessor()
//{
//	//bAutoRegisterWithProcessingPhases = true;
//	//ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::PrePhysics;
//}
//
//void UAgentTestProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
//{
//	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
//	
//	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [DeltaTime](FMassExecutionContext& Context)
//		{
//			const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();
//			
//			for (int i = 0; i < Context.GetNumEntities(); ++i)
//			{
//				UE_LOG(LogTemp, Log, TEXT("Search Entity%d"),i);
//			}
//		});
//}
//
//void UAgentTestProcessor::ConfigureQueries()
//{
//	EntityQuery.AddRequirement<FAgentFragment>(EMassFragmentAccess::ReadWrite);
//	//EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
//	EntityQuery.AddConstSharedRequirement<FAgentParameters>(EMassFragmentPresence::All);
//	EntityQuery.AddTagRequirement<FAgent>(EMassFragmentPresence::All);
//	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
//}
