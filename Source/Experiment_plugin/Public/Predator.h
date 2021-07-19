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
struct FGameStatus
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


UCLASS()
class EXPERIMENT_PLUGIN_API APredator : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	APredator();
	UPROPERTY(EditAnywhere, Category = ServerConfig)
		FString ServerIpAddress = "192.168.137.39";
	UPROPERTY(EditAnywhere, Category = ServerConfig)
		int32 ServerPort = 65123;
	UPROPERTY(EditAnywhere, Category = ServerConfig)
		int32 UpdatesPerSecond = 10;

	FString Clean(FString str);
	bool SocketCreate(FString IPStr, int32 port);
	bool SocketSend(FString message);
	bool SocketReceive();
	FString StringFromBinaryArray(TArray<uint8> BinaryArray);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FVector CurrentLocation;
	float speed;
	FSocket* Host;
	FIPv4Address ip;
	FRunnableThread* m_RecvThread;
};
