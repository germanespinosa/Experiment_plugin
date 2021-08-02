#pragma once
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "JsonObjectConverter.h"
#include "Connection.generated.h"

UCLASS()
class EXPERIMENT_PLUGIN_API UConnection : public UObject
{
	GENERATED_BODY()

public:
	UConnection();
	bool Send(const FString &message);
	bool Receive(FString &);
	template<typename OutStructType>
	bool Receive(OutStructType& OutStruct) {
		FString Buffer;
		if (Receive(Buffer)) {
			if (!Buffer.IsEmpty()) {
				FJsonObjectConverter::JsonObjectStringToUStruct(Buffer, &OutStruct, 0, 0);
			}
			return true;
		}
		return false;
	}
	bool Connect(FString ServerIp, int ServerPort);
	bool IsConnected();
	bool Disconnect();
	FSocket* Host;
};



