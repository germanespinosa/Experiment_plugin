// Fill out your copyright notice in the Description page of Project Settings.
#include "Occlusion.h"
#include "JsonObjectConverter.h"
#include <Components/SphereComponent.h>

CellConnection::CellConnection() {
	Connected = false;
}

bool CellConnection::Init(FString RemoteIp, int RemotePort) {
	if (!SocketCreate(RemoteIp, RemotePort, &Host)) return false;
	Connected = true;
	for (int i = 0; i < 331; i++) {
		FServerCommand ServerCommand;
		ServerCommand.Command = "get_cell";
		ServerCommand.Content = FString::FromInt(i);
		FString Buffer;
		FJsonObjectConverter::UStructToJsonObjectString(ServerCommand, Buffer);
		Buffer = CleanMessage(Buffer);
		SocketSend(Buffer, Host);
		while (!Check(i));
	}
	return true;
}

bool CellConnection::Check(int CellId) {
	if (Updated[CellId]) return true;
	while (true) {
		auto response = SocketReceive(Host);
		if (!response.IsEmpty()) {
			FServerCommand Command;
			FJsonObjectConverter::JsonObjectStringToUStruct(response, &Command, 0, 0);
			if (Command.Command == "set_cell") {
				FCell Cell;
				FJsonObjectConverter::JsonObjectStringToUStruct(Command.Content, &Cell, 0, 0);
				Cells[Cell.id] = Cell;
				Updated[Cell.id] = true;
				UE_LOG(LogTemp, Warning, TEXT("Cell info received %d (%f, %f) %d"), Cell.id, Cell.location.x, Cell.location.y, Cell.occluded);
				if (Cell.id == CellId) return true;
			}
		}
	}
	return false;
}

static CellConnection Connection;

static int InstanceCounter = 0;

bool AOcclusion::Connect(FString RemoteIp, int RemotePort) {
	InstanceCounter = 0;
	for (int i = 0; i < 331; i++) Connection.Updated[i] = false;
	return Connection.Init(RemoteIp, RemotePort);
}

void AOcclusion::Disconnect() {
	Connection.Host->Close();
}

bool AOcclusion::Occluded(int CellId) {
	return Connection.Cells[CellId].occluded;
}

FVector AOcclusion::GetLocation(int CellId) {
	FVector Location;
	Location.X = Connection.Cells[CellId].location.x;
	Location.Y = Connection.Cells[CellId].location.y;
	Location.Z = 0;
	UE_LOG(LogTemp, Warning, TEXT("Cell location set %d (%f,%f) %d"), CellId, Location.X, Location.Y, Connection.Cells[CellId].occluded);
	return Location;
}

void AOcclusion::SetLocation() {
}

// Sets default values
AOcclusion::AOcclusion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CellId = InstanceCounter++;
}

// Called when the game starts or when spawned
void AOcclusion::BeginPlay()
{
	Location.X = Connection.Cells[CellId].location.x;
	Location.Y = Connection.Cells[CellId].location.y;
	Location.Z = 0;
	UE_LOG(LogTemp, Warning, TEXT("Cell location set %d"), CellId);
	SetActorLocation(Location);
	Super::BeginPlay();
}

// Called every frame
void AOcclusion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

