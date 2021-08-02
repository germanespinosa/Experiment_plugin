#pragma once

#include "Structs.h"
#include "Connection.h"
#include "ExperimentConnection.generated.h"


UCLASS()
class EXPERIMENT_PLUGIN_API UExperimentConnection: public UObject
{
	GENERATED_BODY()

public:
	UExperimentConnection();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	static UExperimentConnection *GetExperimentConnection(FString ServerIp, int ServerPort);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	static UExperimentConnection* GetActiveExperimentConnection();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	FCell GetCell(int CellId);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	void SetPredatorActor(AActor *PredatorActor);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool GetPredatorSpawnCell(bool Wait);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	void GetCells();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	FVector GetSpawnLocation();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	void ProcessServerMessage();


	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool StartEpisode(APawn *PreyPawn);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool EndEpisode();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool SpawnPredator();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	float TimeStamp();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool SetState(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool Disconnect();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool Connect();

	UPROPERTY(BlueprintReadWrite)
	float Speed = .2;

	UPROPERTY(BlueprintReadWrite)
	AActor* Predator;
	
	UPROPERTY(BlueprintReadWrite)
	APawn* Prey;

	bool SendMessage(const FServerCommand &Message);

	bool SendEmptyMessage(const FString command);

	bool SendIntMessage(const FString command, int content);

	bool SendFloatMessage(const FString command, float content);

	bool SendStringMessage(const FString command, const FString content);

	bool GetResponse(FServerCommand &message);

	bool WaitResponse(FServerCommand& message);

	FString CleanMessage(const FString& str);

	TArray<FCell> Cells;
	FVector Destination;
	int PredatorSpawnCellId;
	FString RemoteIp;
	int RemotePort;
	UConnection *Connection = NewObject<UConnection>();
	FDateTime EpisodeStartTime;
};