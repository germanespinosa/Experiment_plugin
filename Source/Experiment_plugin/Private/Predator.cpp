// Fill out your copyright notice in the Description page of Project Settings.

#include "Predator.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"


bool APredator::SocketCreate(FString IPStr, int32 port)
{
	FIPv4Address::Parse(IPStr, ip);

	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);

	Host = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	if (Host->Connect(*addr))
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

bool APredator::SocketReceive()
{
	TArray<uint8> ReceiveData;
	uint8 element = 0;
	ReceiveData.Init(element, 1024u);
	int32 read = 0;
	if (Host->Recv(ReceiveData.GetData(), ReceiveData.Num(), read)) {
		const FString ReceivedUE4String = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceiveData.GetData())));
		FString log = "Server:" + ReceivedUE4String;
		UE_LOG(LogTemp, Warning, TEXT("*** %s"), *log);
		//m_RecvThread = FRunnableThread::Create(new FReceiveThread(Host), TEXT("RecvThread"));
		return true;
	}
	return false;
}

bool APredator::SocketSend(FString message)
{
	TCHAR* seriallizedChar = message.GetCharArray().GetData();
	int32 size = FCString::Strlen(seriallizedChar) + 1;
	int32 sent = 0;

	if (Host->Send((uint8*)TCHAR_TO_UTF8(seriallizedChar), size, sent))
	{
		//		UE_LOG(LogTemp, Warning, TEXT("___Send Succeed!"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("___Send Failed!"));
		return false;
	}
}

FString APredator::StringFromBinaryArray(TArray<uint8> BinaryArray)
{
	return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
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
	SocketCreate(ServerIpAddress, ServerPort);
}

FString APredator::Clean(FString str)
{
	return str.Replace(TEXT("\r\n"), TEXT("")).Replace(TEXT("\t"), TEXT(""));
}

// Called every frame
void APredator::Tick(float DeltaTime)
{
	static float AcumDelta = 0;
	Super::Tick(DeltaTime);
	AcumDelta += DeltaTime;
	if (USceneComponent* RootComp = GetRootComponent()) {
		if (APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) {
			RootComp->SetWorldRotation((RootComp->GetComponentLocation() - LocalPawn->GetActorLocation()).GetSafeNormal().Rotation());
			auto direction = LocalPawn->GetActorLocation() - RootComp->GetComponentLocation();
			CurrentLocation += direction * speed * DeltaTime;
			SetActorLocation(CurrentLocation);
			if (AcumDelta >= 1.0 / double(UpdatesPerSecond)) {
				AcumDelta = 0;
				FServerCommand ServerCommand;
				ServerCommand.Command = TEXT("update_game_status");
				FGameStatus GameStatus;
				auto PreyLocation = LocalPawn->GetActorLocation();
				GameStatus.PreyLocation.x = PreyLocation.X;
				GameStatus.PreyLocation.y = PreyLocation.Y;
				auto PredatorLocation = RootComp->GetComponentLocation();
				GameStatus.PredatorLocation.x = PredatorLocation.X;
				GameStatus.PredatorLocation.y = PredatorLocation.Y;
				FJsonObjectConverter::UStructToJsonObjectString(GameStatus, ServerCommand.Content);
				ServerCommand.Content = Clean(ServerCommand.Content);
				FString Buffer;
				FJsonObjectConverter::UStructToJsonObjectString(ServerCommand, Buffer);
				Buffer = Clean(Buffer);
				SocketSend(Buffer);
				SocketReceive();
			}
		}
	}

}