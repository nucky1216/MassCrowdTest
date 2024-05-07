// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"
#include "BuildingSubsystem.h"
// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SmartObjectComp = CreateDefaultSubobject<USmartObjectComponent>(TEXT("SmartObjectComponent"));
	SetRootComponent(SmartObjectComp);

}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	FTimerDelegate TimerDelegate;
	FTimerHandle TimerHandle;

	TimerDelegate.BindLambda([this]()
		{
			GetWorld()->GetSubsystem<UBuildingSubsystem>()->AddBuilding(SmartObjectComp->GetRegisteredHandle(), Floors);
		});
	GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
	
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

