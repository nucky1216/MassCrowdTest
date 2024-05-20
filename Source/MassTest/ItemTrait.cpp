// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"

void UItemTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FItemFragment>();
}

UItemInitializerProcessor::UItemInitializerProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ObservedType = FTransformFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UItemInitializerProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
		{
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();

			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				FItemFragment& Item = ItemFragments[EntityIndex];
				FTransform& Transform = Transforms[EntityIndex].GetMutableTransform();

				//UE_LOG(LogTemp, Log, TEXT("ItemInitializerProcessor->Get Entity Transform.Location:%s"), *Transform.GetLocation().ToString());
				if (Transform.GetLocation() == FVector::Zero())
				{
					Item.OldLocation.X += FMath::FRandRange(-100.f, 100.f);
					Item.OldLocation.Y += FMath::FRandRange(-100.f, 100.f);
					Transform.SetLocation(Item.OldLocation);
				}
				
				BuildingSubsystem->ItemHashGrid.InsertPoint(Context.GetEntity(EntityIndex), Item.OldLocation);
				//UE_LOG(LogTemp, Log, TEXT("ItemInitializerProcessor->Item[%d]=%d Added to HashGrid"),EntityIndex, Item.ItemType.GetValue());
		
				Context.Defer().AddTag<FItemAddedToGrid>(Context.GetEntity(EntityIndex));
			}

		});
}

void UItemInitializerProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement <FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
}

void UItemInitializerProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<UBuildingSubsystem>(Owner.GetWorld());
}

UItemProcessor::UItemProcessor()
{
}

void UItemProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
		{
			TConstArrayView<FTransformFragment> Transforms= Context.GetFragmentView<FTransformFragment>();
			TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();

			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				FItemFragment& Item = ItemFragments[EntityIndex];
				const FVector& Location = Transforms[EntityIndex].GetTransform().GetLocation();

				//UE_LOG(LogTemp, Log, TEXT("ItemProcessor->Get Entity Transform.Location:%s"), *Location.ToString());

				BuildingSubsystem->ItemHashGrid.UpdatePoint(Context.GetEntity(EntityIndex), Item.OldLocation, Location);
				Item.OldLocation = Location;
			}

		});
}

void UItemProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FItemAddedToGrid>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UItemProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<UBuildingSubsystem>(Owner.GetWorld());
}
