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

bool UExperimentConnection::WaitResponse(FServerCommand& Message, int TimeOut) {
	auto StartTime = FDateTime::UtcNow();
	Message.Command = "";
	while (Message.Command == "" && GetResponse(Message) && (TimeOut == 0 || (FDateTime::UtcNow() - EpisodeStartTime).GetTotalMilliseconds() < TimeOut));
	return Message.Command != "";
}

bool UExperimentConnection::StartEpisode(APawn *PreyPawn, int ParticipantId) {
	Prey = PreyPawn;
	PreyIsCaught = false;
	for (int Id = 0; Id < 331; Id++) VisibilityCone[Id]->SetActorHiddenInGame(true);
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

float angle_difference(float a1, float a2, bool &s) {
	if (a1 > a2) {
		auto d = a1 - a2;
		if (d < 180) {
			s = true;
			return d;
		}
		else {
			s = false;
			return 360 + a2 - a1;
		}
	}
	else {
		auto d = a2 - a1;
		if (d < 180) {
			s = false;
			return d;
		}
		else {
			s = true;
			return 360 + a1 - a2;
		}
	}
}

float angle_adjustment(float thetha, float goal, float limit) {
	bool s;
	float dif = angle_difference(thetha, goal, s);
	float adjustment;
	if (dif > limit) {
		adjustment = limit;
	}
	else {
		adjustment = dif;
	}
	if (s) return -adjustment;
	else return adjustment;
}

bool UExperimentConnection::SetState(float DeltaTime) {
	static float AcumDelta = 0;
	AcumDelta += DeltaTime;
	if (Prey == nullptr || Predator == nullptr) return false;
	auto PreyLocation = Prey->GetActorLocation();
	auto PredatorLocation = Predator->GetActorLocation();

	auto PreyOrientation = Prey->GetViewRotation();
	auto PredatorOrientation = Predator->GetActorRotation();
	
	auto Yaw = PredatorOrientation.Yaw;
	ProcessServerMessage();
	auto Direction = (Destination - PredatorLocation).GetSafeNormal();
	auto NewLocation = PredatorLocation + (Direction * Speed * DeltaTime);
	FRotator DestinationRotation;
	if (IsPreyVisible) {
		DestinationRotation = (PreyLocation - PredatorLocation).GetSafeNormal().Rotation();
	}
	else {
		DestinationRotation = (Destination - PredatorLocation).GetSafeNormal().Rotation();
	}
	auto YawError = DestinationRotation.Yaw - Yaw;
	auto RotationAdjustment = angle_adjustment(Yaw, DestinationRotation.Yaw, (TurningSpeed * DeltaTime));
	auto NewRotation = PredatorOrientation;
	NewRotation.Yaw = NewRotation.Yaw + RotationAdjustment ;
	Predator->SetActorLocationAndRotation(NewLocation, NewRotation);
	if (UpdatesPerSecond > 0 && AcumDelta < 1/(float)UpdatesPerSecond) return true;
	AcumDelta = 0;
	SendStringMessage("set_game_state", FString::Printf(TEXT("[%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]"), 
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
	if (ShowVisibility)	SendEmptyMessage("get_visibility");
	return true;
}

bool UExperimentConnection::Disconnect()
{
	return Connection->Disconnect();
}

bool UExperimentConnection::Connect()
{
	return Connection->Connect(RemoteIp, RemotePort);
}

void UExperimentConnection::ShowMarker(int id) {
	auto Marker = VisibilityCone[id];
	Marker->SetHidden(false);
}

void UExperimentConnection::HideMarker(int id) {
	auto Marker = VisibilityCone[id];
	Marker->SetHidden(true);
}

void UExperimentConnection::ProcessServerMessage() {
	FServerCommand Response;
	if (!GetResponse(Response)) return;
	if (Response.Command == "set_destination_cell") {
		FPredatorInstruction Instruction;
		FJsonObjectConverter::JsonObjectStringToUStruct(Response.Content, &Instruction, 0, 0);
		DestinationCellId = Instruction.next_step;
		auto& DestinationCell = Cells[DestinationCellId];
		IsPreyVisible = Instruction.contact;
		Destination.X = DestinationCell.location.x;
		Destination.Y = DestinationCell.location.y;
		Destination.Z = Predator->GetActorLocation().Z;
	} else
	if (Response.Command == "set_turning_speed") {
		TurningSpeed = FCString::Atof(*Response.Content);
	}
	else 
	if (Response.Command == "set_speed") {
		Speed = FCString::Atof(*Response.Content);
	} else 
	if (Response.Command == "set_spawn_cell") {
		PredatorSpawnCellId = FCString::Atoi(*Response.Content);
	} else
	if (Response.Command == "set_visibility") {
		if (ShowVisibility) {
			Response.Content.RemoveFromEnd("]");
			Response.Content.RemoveFromStart("[");
			TArray<FString> VisibleCellIds;
			Response.Content.ParseIntoArray(VisibleCellIds, TEXT(","), false);
			for (int Id = 0; Id < 331; Id++) VisibilityCone[Id]->SetActorHiddenInGame(true);
			for (auto& VisibleCellId : VisibleCellIds) {
				auto CellId = FCString::Atoi(*VisibleCellId);
				VisibilityCone[CellId]->SetActorHiddenInGame(false);
			}
		}
	} else
	if (Response.Command == "set_prey_caught") {
		PreyIsCaught = true;
	}
	if (Response.Command == "set_occlusions") {
		FOcclusions Occlusions;
		FJsonObjectConverter::JsonObjectStringToUStruct(Response.Content, &Occlusions, 0, 0);
		for (auto& Cell : Cells) Cell.occluded = false;
		for (auto OcclusionId : Occlusions.OcclusionIds) Cells[OcclusionId].occluded = true;
		for (unsigned int i = 0; i < 331; i++) {
			Columns[i]->SetActorHiddenInGame(!Cells[i].occluded);
		}
	}
	if (Response.Command == "set_spawn_cell") {
		PredatorSpawnCellId = FCString::Atoi(*Response.Content);
	}
	if (Response.Command == "show_visibility") {
		ShowVisibility = true;
	}
	if (Response.Command == "hide_visibility") {
		ShowVisibility = false;
	}
}

FVector UExperimentConnection::GetSpawnLocation() {
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
}

bool UExperimentConnection::GetUpdates() {
	return SendEmptyMessage("get_updates");
}

FString UExperimentConnection::CleanMessage(const FString& str) {
	return str.Replace(TEXT("\r\n"), TEXT("")).Replace(TEXT("\t"), TEXT(""));
}
