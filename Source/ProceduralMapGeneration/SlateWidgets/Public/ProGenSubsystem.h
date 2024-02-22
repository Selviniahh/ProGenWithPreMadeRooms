// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "ProGenSubsystem.generated.h"

enum Direction : int;
/**
 * 
 */
UCLASS()
class PROCEDURALMAPGENERATION_API UProGenSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	//1. First Room Exit Dir, 2. Second room Enter Dir  
	TMap<TPair<TEnumAsByte<Direction>, TEnumAsByte<Direction>>, TArray<TArray<FIntPoint>>> CorridorTestScenarios;
};
