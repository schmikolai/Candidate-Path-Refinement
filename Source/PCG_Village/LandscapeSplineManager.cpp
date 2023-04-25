// Fill out your copyright notice in the Description page of Project Settings.


#include "LandscapeSplineManager.h"

// Sets default values
ALandscapeSplineManager::ALandscapeSplineManager()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Erstelle einen SplineComponent und befestige ihn an der Root Component
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);
}

void ALandscapeSplineManager::BeginPlay()
{
	Super::BeginPlay();

	// Add your custom logic here
}

void ALandscapeSplineManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Add your custom logic here
}
