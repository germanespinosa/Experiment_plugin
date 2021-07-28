// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Predator.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Occlusion.generated.h"


class CellConnection {
public:
	CellConnection();
	bool Init(FString RemoteIp, int RemotePort);
	bool Check(int);
	bool Connected;
	FSocket* Host;
	FCell Cells[331];
	bool Updated[331];
	UClass *ActorComponent;
};

UCLASS()
class EXPERIMENT_PLUGIN_API AOcclusion : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOcclusion();
	int CellId;

	UFUNCTION(BlueprintCallable, Category = OcclusionMapping)
	void SetLocation();

	UFUNCTION(BlueprintCallable, Category = OcclusionMapping)
	static FVector GetLocation(int CellId);

	UFUNCTION(BlueprintCallable, Category = OcclusionMapping)
	static bool Occluded(int CellId);


	UFUNCTION(BlueprintCallable, Category = OcclusionMapping)
	static bool Connect(FString RemoteIp, int RemotePort);

	UFUNCTION(BlueprintCallable, Category = OcclusionMapping)
	static void Disconnect();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	FVector Location;
};
