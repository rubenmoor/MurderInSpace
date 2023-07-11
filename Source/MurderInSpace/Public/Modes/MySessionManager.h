#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MySessionManager.generated.h"

#define SETTING_CUSTOMNAME "CUSTOMNAME"

struct FLocalPlayerContext;
/**
 * using a game instance subsystem to manage LAN and online sessions 
 */
UCLASS()
class MURDERINSPACE_API UMySessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	bool CreateSession(const FLocalPlayerContext& LPC, struct FHostSessionConfig SessionConfig, std::function<void(FName, bool)> Callback);
	void LeaveSession(std::function<void(FName, bool)> Callback);
	void LeaveSession();
	bool StartSession(std::function<void(FName, bool)> Callback);
	bool FindSessions(const FLocalPlayerContext& LPC, std::function<void(bool)> Callback);
	bool JoinSession(const FLocalPlayerContext& LPC, const FOnlineSessionSearchResult& Result, std::function<void(FName, EOnJoinSessionCompleteResult::Type)> Callback);

	// show the login browser window for EOS
	void ShowLoginScreen(const FLocalPlayerContext& LPC);

	TArray<FOnlineSessionSearchResult> GetSearchResult() const { return LastSessionSearch->SearchResults; }

	static UMySessionManager* Get(UGameInstance* GI) { return GI->GetSubsystem<UMySessionManager>(); }

protected:
	// event handlers
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	IOnlineSessionPtr GetSessionInterface() const;
	
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
};
