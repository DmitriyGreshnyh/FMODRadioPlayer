// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "RadioPlayerComponent.generated.h"



USTRUCT(BlueprintType)
struct FMusicMeta
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Path;

	bool operator==(const FMusicMeta& Other) const
	{
		return Name == Other.Name && Path == Other.Path;
	}

	bool IsEmpty() const
	{
		return Name == "" && Path == "";
	}
};


USTRUCT(BlueprintType)
struct FRadioStation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Url;

	bool operator==(const FRadioStation& Other) const
	{
		return Name == Other.Name &&
			Url == Other.Url;
	}

	bool IsEmpty() const
	{
		return Name == "" && Url == "";
	}

};

UCLASS(Blueprintable, BlueprintType)
class URadioPlayerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (FMusicMeta)", CompactNodeTitle = "==", Keywords = "== equal"))
		static bool EqualEqual_MusicMeta(const FMusicMeta& A, const FMusicMeta& B)
	{
		return A == B;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (FRadioStation)", CompactNodeTitle = "==", Keywords = "== equal"))
		static bool EqualEqual_RadioStation(const FRadioStation& A, const FRadioStation& B)
	{
		return A == B;
	}
};



UCLASS( ClassGroup=(Custom), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent) )
class URadioPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FMusicMeta> GetTrackList() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerMusic")
	void UpdateTrackList();

	UFUNCTION(BlueprintCallable, Category = "PlayerMusic")
	void LoadStations();

	UFUNCTION(BlueprintCallable, Category = "PlayerMusic")
	void SaveStations();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerMusic")
	TArray<FRadioStation> GetRadioStations() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerMusic")
	void AddRadioStation(const FRadioStation& RadioStation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerMusic")
	FString GetMusicFolderPath() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerMusic")
	FString GetRadioStationsJsonPath() const;

protected:

	UFUNCTION(BlueprintCallable)
	void RemoveRadioStation(const FRadioStation& RadioStation);

private:

	TArray<FMusicMeta> m_trackList;

	static bool IsSupportedAudioFile(const FString& FileName);

	TArray<FRadioStation> RadioStations;

	bool SaveRadioStationsToJson(const TArray<FRadioStation>& Stations, const FString& FilePath);

	bool LoadRadioStationsFromJson(TArray<FRadioStation>& OutRadioStations, const FString& FilePath);

	FString m_musicFolderName = "Music";

	FString m_stationsJsonPath = "radio_stations.json";
};

