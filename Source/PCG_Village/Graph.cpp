#include "Graph.h"
#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Containers/Map.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"

TArray<FGraphEdge> UGraphLibrary::FindMinimumSpanningTree(TArray<FVector> points) {
    int n = points.Num();
    TArray<FGraphEdge> edges;

    // Erzeuge eine Liste aller Kanten im Graphen
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            float dist = FVector::Distance(points[i], points[j]);
            edges.Add(FGraphEdge(i, j, dist));
        }
    }

    // Sortiere die Kanten nach Gewicht
    edges.Sort();

    // Initialisiere eine Map, um die Gruppen der Punkte zu speichern
    TMap<int, TSet<int>> groups;

    // Initialisiere die Gruppen mit jeweils einem Punkt
    for (int i = 0; i < n; i++) {
        groups.Add(i, TSet<int>({ i }));
    }

    TArray<FGraphEdge> tree;

    // Füge die Kanten hinzu, solange sie keinen Zyklus erzeugen
    for (FGraphEdge e : edges) {
        int group1 = -1, group2 = -1;

        // Finde die Gruppen der beiden Punkte
        for (auto& pair : groups) {
            if (pair.Value.Contains(e.u)) {
                group1 = pair.Key;
            }
            if (pair.Value.Contains(e.v)) {
                group2 = pair.Key;
            }
        }

        if (group1 != group2) {
            // Vereinige die Gruppen
            groups[group1].Append(groups[group2]);
            groups.Remove(group2);

            // Füge die Kante zum minimalen Spannbaum hinzu
            tree.Add(e);
        }
    }

    return tree;
}
