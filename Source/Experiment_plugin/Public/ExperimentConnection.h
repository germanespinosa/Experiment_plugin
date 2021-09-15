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
	void GetCells();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	FVector GetSpawnLocation();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	void ProcessServerMessage();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool StartEpisode(APawn *PreyPawn, int ParticipantId);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool EndEpisode();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	float TimeStamp();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool SetState(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool Disconnect();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool Connect();

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool GetUpdates();

	UPROPERTY(BlueprintReadWrite)
	float Speed = .2;

	UPROPERTY(BlueprintReadWrite)
	float TurningSpeed = 90;

	UPROPERTY(BlueprintReadWrite)
	int UpdatesPerSecond = 10;

	UPROPERTY(BlueprintReadWrite)
	AActor* Predator;
	
	UPROPERTY(BlueprintReadWrite)
	APawn* Prey;

	UPROPERTY(BlueprintReadWrite)
	bool EpisodeInProgress;

	UPROPERTY(BlueprintReadWrite)
	bool PreyIsCaught;

	UPROPERTY(BlueprintReadWrite)
	bool UpdateWorld;

	UPROPERTY(BlueprintReadWrite)
	bool ShowVisibility;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> VisibilityCone;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> Columns;

	UPROPERTY(BlueprintReadWrite)
	FString WorldName;

	bool SendMessage(const FServerCommand &Message);

	bool SendEmptyMessage(const FString Command);

	bool SendIntMessage(const FString Command, int Content);

	bool SendFloatMessage(const FString Command, float Content);

	bool SendStringMessage(const FString Command, const FString Content);

	bool GetResponse(FServerCommand &Message);

	bool WaitResponse(FServerCommand& Message, int TimeOut = 0);

	FString CleanMessage(const FString& str);

	void ShowMarker(int id);

	void HideMarker(int id);

	TArray<FCell> Cells;

	FVector Destination;
	bool Contact;
	int PredatorSpawnCellId;
	int DestinationCellId;
	bool IsPreyVisible;
	FString RemoteIp;
	int RemotePort;
	UConnection *Connection = NewObject<UConnection>();
	FDateTime EpisodeStartTime;
};