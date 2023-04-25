#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Graph.generated.h"

USTRUCT(BlueprintType)
struct FGraphEdge {
    GENERATED_BODY()
    UPROPERTY(BlueprintReadonly)
    int u;
    UPROPERTY(BlueprintReadonly)
    int v;
    float w;

    FGraphEdge() = default;
    FGraphEdge(int u, int v, float w) : u(u), v(v), w(w) {}

    bool operator<(const FGraphEdge& e) const {
        return w < e.w;
    }
};

/**
 * 
 */
UCLASS()
class PCG_VILLAGE_API UGraphLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Pathways")
    static TArray<FGraphEdge> FindMinimumSpanningTree(TArray<FVector> points);
};
