#include"Connection.h"
#include "Networking.h"


UConnection::UConnection() {
	Host = nullptr;
	AddToRoot();
}

bool UConnection::Connect(FString ServerIp, int ServerPort) {
	if (IsConnected()) return false;
	FIPv4Address ip;
	FIPv4Address::Parse(ServerIp, ip);
	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(ServerPort);
	FSocket *HostT = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	if (HostT->Connect(*addr)) {
		Host = HostT;
		return true;
	}
	return false;
}

bool UConnection::IsConnected() {
	return (Host != nullptr);
}

bool UConnection::Disconnect() {
	if (!IsConnected()) return false;
	Host->Close();
	Host = nullptr;
	return true;
}

bool UConnection::Send(const FString& message) {
	if (!IsConnected()) return false;
	const TCHAR* seriallizedChar = message.GetCharArray().GetData();
	int32 size = FCString::Strlen(seriallizedChar) + 1;
	int32 sent = 0;
	if (!Host->Send((uint8*)TCHAR_TO_UTF8(seriallizedChar), size, sent))
	{
		Disconnect();
		return false;
	}
	return true;
}

bool UConnection::Receive(FString &ServerMessage) {
	if (!IsConnected()) {
		ServerMessage = "";
		return false;
	}
	TArray<uint8> ReceiveData;
	uint8 element = 0;
	int32 read = 0;
	uint32 PendingData = 0;
	if (!Host->HasPendingData(PendingData)) return true;
	ReceiveData.Init(element, PendingData);
	if (Host->Recv(ReceiveData.GetData(), ReceiveData.Num(), read)) {
		if (read)
			ServerMessage = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceiveData.GetData())));
		else
			ServerMessage = "";
		return true;
	}
	Disconnect();
	return false;
}