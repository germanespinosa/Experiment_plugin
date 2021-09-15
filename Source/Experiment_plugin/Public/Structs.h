#pragma once
#include "Structs.generated.h"

USTRUCT(Blueprintable)
struct FLocation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		float x;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		float y;
};

USTRUCT(Blueprintable)
struct FPredatorInstruction
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
	int destination;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
	int next_step;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
	bool contact;
};

USTRUCT(Blueprintable)
struct FOcclusions
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int> OcclusionIds;
};


USTRUCT(Blueprintable)
struct FCoordinates
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		int x;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		int y;
};

USTRUCT(Blueprintable)
struct FCell
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		int id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		int cell_type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FCoordinates coordinates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FLocation location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		bool occluded;
};



USTRUCT(Blueprintable)
struct FExperimentState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FLocation PreyLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FLocation PredatorLocation;
};

USTRUCT(Blueprintable)
struct FServerCommand
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString Command;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString Content;
};

