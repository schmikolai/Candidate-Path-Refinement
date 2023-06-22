// Fill out your copyright notice in the Description page of Project Settings.


#include "PathRefinement.h"
#include "Containers/List.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/DateTime.h"


float UPathRefinement::Cost(FVector V1, FVector V2, FPathRefinementSettings& Settings)
{
	switch (Settings.CostFunction)
	{
	case ECostFunction::EuclideanDistance:
		return FVector::Dist(V1, V2);
	case ECostFunction::SlopeMultiplication:
		float FlatDist = FVector::Dist2D(V1, V2);
		float ZDist = FMath::Abs(V2.Z - V1.Z);
		float slopeAngle = FMath::Atan(ZDist / FlatDist);
		return FlatDist * FMath::Pow(slopeAngle, Settings.SlopeFactorExponent);
	}
	return 0.f;
}

float UPathRefinement::GetZAtWorldLocation(FVector V, FWorldParams& WorldParams)
{
	FVector2D UV = (FVector2D(V) + WorldParams.WorldSize / 2) / WorldParams.WorldSize;
	UV *= static_cast<double>(WorldParams.LandscapeResolution) / WorldParams.HeightmapResolution;
	FColor val = UKismetRenderingLibrary::ReadRenderTargetUV(NULL, WorldParams.Heightmap, UV.X, UV.Y);
	uint16 HeightRG = (static_cast<uint16>(val.R) << 8) | static_cast<uint16>(val.G);
	float height = (HeightRG - 32768.f) * WorldParams.LandscapeZScale / 128.f;
	return height;
}

void UPathRefinement::AddSplinePointToTail(TDoubleLinkedList<FVector>& Points, USplineComponent* SplineComponent, int PointIndex, FWorldParams& WorldParams)
{
	FVector p = SplineComponent->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
	p.Z = GetZAtWorldLocation(p, WorldParams);
	Points.AddTail(p);
}

TArray<FVector> UPathRefinement::GenerateCandidatePoints(FVector V, FVector Prev, FVector Next, FPathRefinementSettings& Settings, FWorldParams& WorldParams)
{
	TArray<FVector> CandidatePoints;
	CandidatePoints.Add(V);
	FVector2D Prev2D = FVector2D(Prev);
	FVector2D Next2D = FVector2D(Next);
	FVector2D Tangent = Next2D - Prev2D;
	FVector Normal = FVector(Tangent.Y, -Tangent.X, 0) * Settings.CandidatePointScaling;
	for (int i = 1; i <= Settings.NumCandidatePoints; i++)
	{
		FVector a = V + Normal * i;
		a.Z = GetZAtWorldLocation(a, WorldParams);
		CandidatePoints.Add(a);
		FVector b = V - Normal * i;
		b.Z = GetZAtWorldLocation(b, WorldParams);
		CandidatePoints.Add(b);
	}

	return CandidatePoints;
}

FVector UPathRefinement::GetBestCandidatePoint(FVector Prev, FVector Next, TArray<FVector>& CandidatePoints, FPathRefinementSettings& Settings)
{
	float MinCost = 9999999.0f;
	FVector BestCandidate = FVector::Zero();

	for (int i = 0; i < CandidatePoints.Num(); i++)
	{
		float SegmentCost = Cost(Prev, CandidatePoints[i], Settings) + Cost(CandidatePoints[i], Next, Settings);
		if (SegmentCost < MinCost)
		{
			MinCost = SegmentCost;
			BestCandidate = CandidatePoints[i];
		}
	}

	return BestCandidate;
}

void UPathRefinement::SubdivideEdges(TDoubleLinkedList<FVector>* Points, FPathRefinementSettings& Settings, FWorldParams& WorldParams)
{
	auto CurrentNode = Points->GetHead();
	auto NextNode = CurrentNode->GetNextNode();
	while (NextNode != nullptr)
	{
		FVector V1 = CurrentNode->GetValue();
		FVector V2 = NextNode->GetValue();
		float distance = FVector::Dist2D(V1, V2);
		if (distance > Settings.MinEdgeLength * 2)
		{
			FVector Center = V1 + (V2 - V1) * 0.5;
			Center.Z = GetZAtWorldLocation(Center, WorldParams);
			Points->InsertNode(Center, NextNode);
		}
		CurrentNode = NextNode;
		NextNode = NextNode->GetNextNode();
	}
}

void UPathRefinement::RefinementStep(TDoubleLinkedList<FVector>* Points, FPathRefinementSettings& Settings, FWorldParams& WorldParams)
{
	auto CurrentNode = Points->GetHead();
	auto NextNode = CurrentNode->GetNextNode();
	auto PrevNode = CurrentNode->GetPrevNode();
	while (NextNode != nullptr)
	{
		if (PrevNode != nullptr)
		{
			FVector V = CurrentNode->GetValue();
			FVector Prev = PrevNode->GetValue();
			FVector Next = NextNode->GetValue();
			auto CandidatePoints = GenerateCandidatePoints(V, Prev, Next, Settings, WorldParams);
			FVector Best = GetBestCandidatePoint(Prev, Next, CandidatePoints, Settings);
			Points->RemoveNode(CurrentNode);
			Points->InsertNode(Best, NextNode);
		}
		CurrentNode = NextNode;
		PrevNode = NextNode->GetPrevNode();
		NextNode = NextNode->GetNextNode();
	}
}

void UPathRefinement::PathRefinement(USplineComponent* SplineComponent, FPathRefinementSettings Settings, FWorldParams WorldParams, bool preserveSplineSegments)
{
	int NumberOfSplinePoints = SplineComponent->GetNumberOfSplinePoints();
	if (NumberOfSplinePoints < 2)
	{
		return;
	}

	TDoubleLinkedList<FVector> Points = TDoubleLinkedList<FVector>();

	if (preserveSplineSegments)
	{
		for (int i = 0; i < NumberOfSplinePoints; i++)
		{
			AddSplinePointToTail(Points, SplineComponent, i, WorldParams);
		}
		if (SplineComponent->IsClosedLoop())
		{
			AddSplinePointToTail(Points, SplineComponent, 0, WorldParams);
		}
	}
	else
	{
		AddSplinePointToTail(Points, SplineComponent, 0, WorldParams);
		AddSplinePointToTail(Points, SplineComponent, NumberOfSplinePoints - 1, WorldParams);
	}

	double StartTime = FPlatformTime::Seconds();

	for (int i = 0; i < Settings.Iterations; i++)
	{
		SubdivideEdges(&Points, Settings, WorldParams);

		for (int j = 0; j < Settings.RefinementStepsPerIteration; j++)
		{
			RefinementStep(&Points, Settings, WorldParams);
		}

	}

	double EndTime = FPlatformTime::Seconds();
	double Duration = EndTime - StartTime;
	UE_LOG(LogTemp, Warning, TEXT("Path refinement duration: %f s"), Duration)

	SplineComponent->ClearSplinePoints(false);
	auto CurrentNode = Points.GetHead();
	while (CurrentNode != nullptr)
	{
		SplineComponent->AddSplinePoint(CurrentNode->GetValue(), ESplineCoordinateSpace::World, false);
		CurrentNode = CurrentNode->GetNextNode();
	}
	SplineComponent->UpdateSpline();
}
