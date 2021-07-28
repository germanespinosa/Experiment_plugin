// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Predator.generated.h"

USTRUCT()
struct FLocation
{
	GENERATED_BODY()
public:
	UPROPERTY()
		double x;
	UPROPERTY()
		double y;
};


USTRUCT()
struct FCoordinates
{
	GENERATED_BODY()
public:
	UPROPERTY()
		int x;
	UPROPERTY()
		int y;
};

USTRUCT()
struct FCell
{
	GENERATED_BODY()
public:
	UPROPERTY()
		int id;
	UPROPERTY()
		int cell_type;
	UPROPERTY()
		FCoordinates coordinates;
	UPROPERTY()
		FLocation location;
	UPROPERTY()
		bool occluded;
};



USTRUCT()
struct FExperimentState
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FLocation PreyLocation;
	UPROPERTY()
		FLocation PredatorLocation;
};

USTRUCT()
struct FServerCommand
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString Command;
	UPROPERTY()
		FString Content;
};

bool SocketCreate(FString IPStr, int32 port, FSocket** Host);
bool SocketSend(FString message, FSocket* Host);
FString SocketReceive(FSocket* Host);
FString CleanMessage(FString str);



UCLASS()
class EXPERIMENT_PLUGIN_API APredator : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	APredator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		bool Debug;
	UPROPERTY(EditAnywhere, Category = ServerConfig)
		FString ServerIpAddress = "127.0.0.1";
	UPROPERTY(EditAnywhere, Category = ServerConfig)
		int32 ServerPort = 4000;
	UPROPERTY(EditAnywhere, Category = ServerConfig)
		int32 UpdatesPerSecond = 10;

	void ProcessServerMessage(FString message);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	bool Connected;
	virtual void Tick(float DeltaTime) override;
	FVector CurrentLocation;
	float speed;
	FSocket* Host;
	FVector Destination;
	UPROPERTY(BlueprintReadWrite)
	FDateTime ExperimentStartTime;
	UPROPERTY(BlueprintReadWrite)
	APawn* Prey;
	bool InEpisode;
	int32 Episode;
};
