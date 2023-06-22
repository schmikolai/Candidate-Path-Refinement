// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PathRefinement.generated.h"

UENUM()
enum class ECostFunction
{
	EuclideanDistance,
	SlopeMultiplication
};

USTRUCT(BlueprintType)
struct FPathRefinementSettings
{
public:
	GENERATED_BODY()
		FPathRefinementSettings() = default;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "1", EditConditionHides))
		int Iterations = 5;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float MinEdgeLength = 500.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"))
		int RefinementStepsPerIteration = 2;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int NumCandidatePoints = 4;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float CandidatePointScaling = 0.1f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cost")
		ECostFunction CostFunction = ECostFunction::SlopeMultiplication;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cost", meta = (EditCondition="IsEnumValueEqual(CostFunction, ECostFunction::SlopeMultiplication)", ClampMin = "0.0", ClampMax = "1.0"))
		float SlopeFactorExponent = 0.25f;
};

USTRUCT(BlueprintType)
struct FWorldParams
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FVector2D WorldSize;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UTextureRenderTarget2D* Heightmap;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int HeightmapResolution;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int LandscapeResolution;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float LandscapeZScale;
};

UCLASS()
class PCG_VILLAGE_API UPathRefinement : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Pathways")
	static void PathRefinement(USplineComponent* SplineComponent, FPathRefinementSettings Settings, FWorldParams WorldParams, bool preserveSplineSegments = false);
private:
	static TArray<FVector> GenerateCandidatePoints(FVector V, FVector Prev, FVector Next, FPathRefinementSettings& Settings, FWorldParams& WorldParams);
	static FVector GetBestCandidatePoint(FVector Prev, FVector Next, TArray<FVector>& CandidatePoints, FPathRefinementSettings& Settings);
	static float GetZAtWorldLocation(FVector V, FWorldParams& WorldParams);
	static void AddSplinePointToTail(TDoubleLinkedList<FVector>& Points, USplineComponent* SplineComponent, int PointIndex, FWorldParams& WorldParams);
	static void SubdivideEdges(TDoubleLinkedList<FVector>* Points, FPathRefinementSettings& Settings, FWorldParams& WorldParams);
	static void RefinementStep(TDoubleLinkedList<FVector>* Points, FPathRefinementSettings& Settings, FWorldParams& WorldParams);
	static float Cost(FVector V1, FVector V2, FPathRefinementSettings& Settings);
};
