#include "ExperimentConnection.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

static UExperimentConnection* ExperimentConnection;

UExperimentConnection::UExperimentConnection(){
	Prey = nullptr;
	Predator = nullptr;
}

float UExperimentConnection::TimeStamp() {
	return ((float)(FDateTime::UtcNow() - EpisodeStartTime).GetTotalMilliseconds() / 1000);
}


UExperimentConnection* UExperimentConnection::GetExperimentConnection(FString ServerIp, int ServerPort)
{
	ExperimentConnection = NewObject<UExperimentConnection>();
	ExperimentConnection->RemoteIp = ServerIp;
	ExperimentConnection->RemotePort = ServerPort;
	ExperimentConnection->AddToRoot();
	return ExperimentConnection;
}

UExperimentConnection* UExperimentConnection::GetActiveExperimentConnection()
{
	return ExperimentConnection;
}


FCell UExperimentConnection::GetCell(int CellId) {
	return Cells[CellId];
}

void UExperimentConnection::SetPredatorActor(AActor* PredatorActor)
{
	Predator = PredatorActor;
}

bool UExperimentConnection::WaitResponse(FServerCommand& message) {
	message.Command = "";
	while (message.Command == "" && GetResponse(message));
	return message.Command != "";
}

bool UExperimentConnection::GetPredatorSpawnCell(bool Wait) {
	if (!SendEmptyMessage("get_spawn_cell")) return false;
	if (!Wait) return true;
	FServerCommand Response;
	if (WaitResponse(Response) && Response.Command == "set_spawn_cell") {
		PredatorSpawnCellId = FCString::Atoi(*Response.Content);
		return true;
	}
	return false;
}

bool UExperimentConnection::StartEpisode(APawn *PreyPawn, int ParticipantId) {
	Prey = PreyPawn;
	PreyIsCaught = false;
	if (!SendIntMessage("start_episode",ParticipantId)) return false;
	EpisodeStartTime = FDateTime::UtcNow();
	return true;
}

bool UExperimentConnection::EndEpisode() {
	if (!Connection->IsConnected()) return false;
	return SendEmptyMessage("end_episode");
}

bool UExperimentConnection::SendMessage(const FServerCommand& Message) {
	FString Buffer;
	FJsonObjectConverter::UStructToJsonObjectString(Message, Buffer);
	Buffer = CleanMessage(Buffer);
	return Connection->Send(Buffer);
}

bool UExperimentConnection::SendEmptyMessage(const FString command) {
	FServerCommand Message;
	Message.Command = command;
	Message.Content = "";
	return SendMessage(Message);
}

bool UExperimentConnection::SendIntMessage(const FString command, int content) {
	FServerCommand Message;
	Message.Command = command;
	Message.Content = FString::FromInt(content);
	return SendMessage(Message);
}

bool UExperimentConnection::SendFloatMessage(const FString command, float content) {
	FServerCommand Message;
	Message.Command = command;
	Message.Content = FString::SanitizeFloat(content);
	return SendMessage(Message);
}

bool UExperimentConnection::SendStringMessage(const FString command, const FString content) {
	FServerCommand Message;
	Message.Command = command;
	Message.Content = content;
	return SendMessage(Message);
}

bool UExperimentConnection::SetState(float DeltaTime) {
	if (Prey == nullptr || Predator == nullptr) return false;
	auto PreyLocation = Prey->GetActorLocation();
	auto PredatorLocation = Predator->GetActorLocation();
	auto PreyOrientation = Prey->GetViewRotation();
	auto PredatorOrientation = Predator->GetActorRotation();
	static float AcumDelta = 0;
	ProcessServerMessage();
	AcumDelta += DeltaTime;
	FVector Direction = (Destination - PredatorLocation).GetSafeNormal();
	auto NewLocation = PredatorLocation + (Direction * Speed * DeltaTime);
	auto NewRotation = (PreyLocation - PredatorLocation).GetSafeNormal().Rotation();
	Predator->SetActorLocationAndRotation(NewLocation, NewRotation);

	if (UpdatesPerSecond > 0 && AcumDelta < 1/(float)UpdatesPerSecond) return true;
	AcumDelta = 0;
	return SendStringMessage("set_game_state", FString::Printf(TEXT("[%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]"), 
		TimeStamp(), 
		PreyLocation.X, 
		PreyLocation.Y, 
		PreyLocation.Z, 
		PreyOrientation.Roll, 
		PreyOrientation.Pitch, 
		PreyOrientation.Yaw, 
		PredatorLocation.X, 
		PredatorLocation.Y, 
		PredatorLocation.Z, 
		PredatorOrientation.Roll, 
		PredatorOrientation.Pitch, 
		PredatorOrientation.Yaw));	
}

bool UExperimentConnection::Disconnect()
{
	return Connection->Disconnect();
}

bool UExperimentConnection::Connect()
{
	return Connection->Connect(RemoteIp, RemotePort);
}

void UExperimentConnection::ProcessServerMessage() {
	FServerCommand Response;
	if (!GetResponse(Response)) return;
	if (Response.Command == "set_destination_cell") {
		auto DestinationCellId = FCString::Atoi(*Response.Content);
		auto& DestinationCell = Cells[DestinationCellId];
		Destination.X = DestinationCell.location.x;
		Destination.Y = DestinationCell.location.y;
		Destination.Z = Predator->GetActorLocation().Z;
		UE_LOG(LogTemp, Warning, TEXT("New predator destination (%f, %f, %f)"), Destination.X, Destination.Y, Destination.Z);
	} else
	if (Response.Command == "set_speed") {
		Speed = FCString::Atof(*Response.Content);
		UE_LOG(LogTemp, Warning, TEXT("New predator speed (%f)"), Speed);
	} else 
	if (Response.Command == "set_spawn_cell") {
		PredatorSpawnCellId = FCString::Atoi(*Response.Content);
		UE_LOG(LogTemp, Warning, TEXT("New predator spawn cell (%d)"), PredatorSpawnCellId);
		Connection->Disconnect();
	} else
	if (Response.Command == "set_prey_caught") {
		PreyIsCaught = true;
		UE_LOG(LogTemp, Warning, TEXT("Prey has been caught"));
	} else
	if (Response.Command == "set_update_world") {
		UpdateWorld = true;
		UE_LOG(LogTemp, Warning, TEXT("A new world will be loaded when the next episodes ends"));
	}

}

FVector UExperimentConnection::GetSpawnLocation() {
	GetPredatorSpawnCell(true);
	FCell SpawnCell = GetCell(PredatorSpawnCellId);
	FVector SpawnLocation;
	SpawnLocation.X = SpawnCell.location.x;
	SpawnLocation.Y = SpawnCell.location.y;
	SpawnLocation.Z = Predator->GetActorLocation().Z;
	return SpawnLocation;
}

bool UExperimentConnection::GetResponse(FServerCommand& Response) {
	Response.Command = "";
	Response.Content = "";
	return Connection->Receive(Response);
}

void UExperimentConnection::GetCells() {
	if (Cells.Num() == 0) { //initialize the cell location and occlusions
		Cells.Reserve(331);
		FServerCommand Response;
		for (int i = 0; i < 331; i++) {
			auto NewCellId = Cells.Emplace();
			auto &NewCell = Cells[NewCellId];
			SendIntMessage("get_cell", i);
			if (WaitResponse(Response) && Response.Command=="set_cell") {
				FJsonObjectConverter::JsonObjectStringToUStruct(Response.Content, &NewCell, 0, 0);
			}
		}
	}
	else {
		FServerCommand Response;
		SendEmptyMessage("get_occlusions");
		FOcclusions Occlusions;
		if (WaitResponse(Response) && Response.Command == "set_occlusions") {
			FJsonObjectConverter::JsonObjectStringToUStruct(Response.Content, &Occlusions, 0, 0);
			for (auto& Cell : Cells) Cell.occluded = false;
			for (auto OcclusionId : Occlusions.OcclusionIds) Cells[OcclusionId].occluded = true;
		}
	}
}

FString UExperimentConnection::CleanMessage(const FString& str) {
	return str.Replace(TEXT("\r\n"), TEXT("")).Replace(TEXT("\t"), TEXT(""));
}
