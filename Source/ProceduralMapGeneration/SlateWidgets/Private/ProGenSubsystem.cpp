// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapGeneration/SlateWidgets/Public/ProGenSubsystem.h"

void UProGenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("ProGenSubsystem Initialized"));

}

void UProGenSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("ProGenSubsystem Deinitialized"));
	Super::Deinitialize();
}
