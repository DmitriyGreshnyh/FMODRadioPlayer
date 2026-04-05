// Fill out your copyright notice in the Description page of Project Settings.

#include "RadioPlayerComponent.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h" 

TArray<FMusicMeta> URadioPlayerComponent::GetTrackList() const
{
	return m_trackList;
}
//-------------------------------------------------------------------------------
FString URadioPlayerComponent::GetMusicFolderPath() const
{
	//Saved/Music
	return FPaths::ProjectSavedDir() / m_musicFolderName;
}
//-------------------------------------------------------------------------------
bool URadioPlayerComponent::IsSupportedAudioFile(const FString& FileName)
{
	//Checking enabled for FMOD audio files format
	static const TArray<FString> Extensions = { TEXT(".mp3"), TEXT(".wav"), TEXT(".ogg"), TEXT(".flac"), TEXT(".aiff") };
	for (const FString& Ext : Extensions)
	{
		if (FileName.EndsWith(Ext, ESearchCase::IgnoreCase))
			return true;
	}
	return false;
}
//-------------------------------------------------------------------------------
void URadioPlayerComponent::UpdateTrackList()
{
	m_trackList.Empty();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*GetMusicFolderPath()))
	{
		// Create folder if we don't have any
		PlatformFile.CreateDirectory(*GetMusicFolderPath());
		return;
	}

	TArray<FString> FoundFiles;
	PlatformFile.FindFiles(FoundFiles, *GetMusicFolderPath(), nullptr);

	for (const FString& FilePath : FoundFiles)
	{
		FString FileName = FPaths::GetCleanFilename(FilePath);
		if (IsSupportedAudioFile(FileName)) //If found file is Ok. Add it in TrackList
		{
			FMusicMeta TrackInfo;
			TrackInfo.Name = FileName;
			TrackInfo.Path = FilePath;
			//TODO gather more info from the audio file (lenght, author etc)

			m_trackList.Add(TrackInfo);
		}
	}
}
//-------------------------------------------------------------------------------
bool URadioPlayerComponent::LoadRadioStationsFromJson(TArray<FRadioStation>& OutRadioStations, const FString& FilePath)
{
	OutRadioStations.Empty();

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath)) //Trying to find JSON file
		return false;

	//This is needed to force JSON file to load as UTF-8. For Russian language support in rare scenarios
	FileData.Add(0);
	FString JsonString = UTF8_TO_TCHAR(reinterpret_cast<const char*>(FileData.GetData())); 

	// Parse JSON into the values array
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonArray))//Skip not Deserializable values
		return false;

	for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
	{
		if (!JsonValue.IsValid() || JsonValue->Type != EJson::Object)  //Skip not valid values
			continue; 
	
		TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
		FRadioStation Station;
		if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &Station)) //Convert valid JSON objects to FRadioStation 
		{
			OutRadioStations.Add(Station);
		}
	}
	return true;
}
//-------------------------------------------------------------------------------
bool URadioPlayerComponent::SaveRadioStationsToJson(const TArray<FRadioStation>& Stations, const FString& FilePath)
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FRadioStation& Station : Stations)
	{
		// Convert FRadioStation to JSON object
		TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Station);
		if (JsonObject.IsValid())
		{
			JsonArray.Add(MakeShared<FJsonValueObject>(JsonObject));
		}
	}

	//Create JSON string from JSON objects array
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonArray, Writer))
		return false;

	// Write JSON string to a file
	//ForceUTF8WithoutBOM is to force JSON file to save as UTF-8. For Russian language support in rare scenarios. 
	//But player still can convert it afterwards
	if (!FFileHelper::SaveStringToFile(OutputString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)) 
		return false;

	return true;
}
//-------------------------------------------------------------------------------
FString URadioPlayerComponent::GetRadioStationsJsonPath() const
{
	//Saved/radio_stations.json
	return FPaths::ProjectSavedDir() + m_stationsJsonPath;
}
//-------------------------------------------------------------------------------
void URadioPlayerComponent::LoadStations()
{
	LoadRadioStationsFromJson(RadioStations, GetRadioStationsJsonPath());
}
//-------------------------------------------------------------------------------
void URadioPlayerComponent::SaveStations()
{
	SaveRadioStationsToJson(RadioStations, GetRadioStationsJsonPath());
}
//-------------------------------------------------------------------------------
TArray<FRadioStation> URadioPlayerComponent::GetRadioStations() const
{
	return RadioStations;
}
//-------------------------------------------------------------------------------
void URadioPlayerComponent::AddRadioStation(const FRadioStation& RadioStation)
{
	RadioStations.Add(RadioStation);
	SaveStations();
}
//-------------------------------------------------------------------------------
void URadioPlayerComponent::RemoveRadioStation(const FRadioStation& RadioStation)
{
	RadioStations.Remove(RadioStation);
	SaveStations();
}