// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Landscape.h"
#include "LandscapeSplineManager.generated.h"

UCLASS()
class PCG_VILLAGE_API ALandscapeSplineManager : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ALandscapeSplineManager();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ALandscape* Landscape = nullptr;
private:
	UPROPERTY(VisibleAnywhere)
	USplineComponent* SplineComponent;
};
