// Fill out your copyright notice in the Description page of Project Settings.

#include "Predator.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Misc/Timespan.h"

bool SocketCreate(FString IPStr, int32 port, FSocket** Host)
{
	FIPv4Address ip;
	FIPv4Address::Parse(IPStr, ip);

	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);

	*Host = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	if ((*Host)->Connect(*addr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Connect Succeed!"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Connect Failed!"));
		return false;
	}
}

bool SocketSend(FString message, FSocket* Host)
{
	TCHAR* seriallizedChar = message.GetCharArray().GetData();
	int32 size = FCString::Strlen(seriallizedChar) + 1;
	int32 sent = 0;

	if (Host->Send((uint8*)TCHAR_TO_UTF8(seriallizedChar), size, sent))
	{
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("___Send Failed!"));
		return false;
	}
}


FString SocketReceive(FSocket *Host)
{
	TArray<uint8> ReceiveData;
	uint8 element = 0;
	ReceiveData.Init(element, 1024u);
	int32 read = 0;
	if (Host->Recv(ReceiveData.GetData(), ReceiveData.Num(), read)) {
		const FString ServerMessage = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceiveData.GetData())));
		FString log = "Server:" + ServerMessage;
		return ServerMessage;
	}
	return FString();
}

FString CleanMessage(FString str)
{
	return str.Replace(TEXT("\r\n"), TEXT("")).Replace(TEXT("\t"), TEXT(""));
}


void APredator::ProcessServerMessage(FString message) {
	FServerCommand Command;
	FJsonObjectConverter::JsonObjectStringToUStruct(message, &Command, 0, 0);
	if (Command.Command == "update_predator_destination") {
		FLocation DestinationLocation;
		FJsonObjectConverter::JsonObjectStringToUStruct(Command.Content, &DestinationLocation, 0, 0);
		UE_LOG(LogTemp, Warning, TEXT("New predator destination (%f,%f)"), DestinationLocation.x, DestinationLocation.y);
		Destination.X = DestinationLocation.x;
		Destination.Y = DestinationLocation.y;
	}
	if (Command.Command == "update_predator_speed") {
		speed = FCString::Atof(*Command.Content);
		UE_LOG(LogTemp, Warning, TEXT("New predator speed (%f)"), speed);
	}
	if (Command.Command == "update_predator_location") {
		FLocation NewLocation;
		FJsonObjectConverter::JsonObjectStringToUStruct(Command.Content, &NewLocation, 0, 0);
		UE_LOG(LogTemp, Warning, TEXT("New predator location  (%f,%f)"), NewLocation.x, NewLocation.y);
		CurrentLocation.X = NewLocation.x;
		CurrentLocation.Y = NewLocation.y;
		SetActorLocation(CurrentLocation);
	}

}

// Sets default values
APredator::APredator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CurrentLocation = this->GetActorLocation(); // To save where ever the actor is in the viewport
	speed = .2f;   // change this to whatever
}

// Called when the game starts or when spawned
void APredator::BeginPlay()
{
	Super::BeginPlay();
	if (Debug) Prey = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	Connected = SocketCreate(ServerIpAddress, ServerPort, &Host);
	ExperimentStartTime = FDateTime::UtcNow();
	InEpisode = false;
}


// Called every frame
void APredator::Tick(float DeltaTime)
{
	static float AcumDelta = 0;
	Super::Tick(DeltaTime);
	AcumDelta += DeltaTime;
	if (USceneComponent* RootComp = GetRootComponent()) {
		if (APawn* LocalPawn = Prey) {
			if (!InEpisode) Episode++;
			InEpisode = true;
			auto PreyLocation = LocalPawn->GetActorLocation();
			Destination.Z = PreyLocation.Z;
			RootComp->SetWorldRotation((RootComp->GetComponentLocation() - LocalPawn->GetActorLocation()).GetSafeNormal().Rotation());
			FVector direction = Destination - RootComp->GetComponentLocation();
			CurrentLocation += direction * speed * DeltaTime;
			SetActorLocation(CurrentLocation);
			if (Connected && AcumDelta >= 1.0 / double(UpdatesPerSecond)) {
				auto TimeStamp = ((float)(FDateTime::UtcNow() - ExperimentStartTime).GetTotalMilliseconds() / 1000);
				FServerCommand ServerCommand;
				ServerCommand.Command = TEXT("update_game_state");
				auto PredatorLocation = RootComp->GetComponentLocation();
				auto PreyOrientation = LocalPawn->GetViewRotation();
				auto PredatorOrientation = RootComp->GetComponentRotation();
				ServerCommand.Content = FString::Printf(TEXT("[%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]"), Episode, TimeStamp, PreyLocation.X, PreyLocation.Y, PreyLocation.Z, PreyOrientation.Roll, PreyOrientation.Pitch, PreyOrientation.Yaw, PredatorLocation.X, PredatorLocation.Y, PredatorLocation.Z, PredatorOrientation.Roll, PredatorOrientation.Pitch, PredatorOrientation.Yaw);
				FString Buffer;
				FJsonObjectConverter::UStructToJsonObjectString(ServerCommand, Buffer);
				Buffer = CleanMessage(Buffer);
				SocketSend(Buffer, Host);
				FString ServerMessage = SocketReceive(Host);
				if (!ServerMessage.IsEmpty()) {
					ProcessServerMessage(ServerMessage);
				}
				AcumDelta = 0;
			}
		}
		else {
			InEpisode = false;
		}
	}

}