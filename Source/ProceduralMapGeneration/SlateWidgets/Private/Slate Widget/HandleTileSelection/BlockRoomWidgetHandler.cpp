// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/HandleTileSelection/BlockRoomWidgetHandler.h"
#include "SlateMaterialBrush.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/Test/ProGenWidgetTests.h"
#include "UObject/SavePackage.h"

void UBlockRoomWidgetHandler::Initialize()
{
	PluginSetting = GetDefault<UPluginSettings>();
	SceneCapInst = PluginSetting->SceneCapActorInst.Get();

	if (UMaterialInterface* Material = PluginSetting->SceneCapMaterial.Get())
		SceneCapImgBrush = MakeShared<FSlateMaterialBrush>(*Material, FVector2D(100, 100)); // Initialize member variable

	TArray<USceneCaptureComponent2D*> SceneCapCompArray;
	SceneCapInst->GetComponents(SceneCapCompArray);
	SceneCapComp = SceneCapCompArray[0];

	FSlateFontInfo PropertyTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	PropertyTextFont.Size = 12;

	TSharedPtr<STextBlock> LocalTextBlock;

	BlockRoomWidgetHandler = NewObject<UBlockRoomWidgetHandler>(GetTransientPackage());
	TileHoverMat = PluginSetting->TileHoverMaterial.LoadSynchronous();
	TileUnhoverMat = PluginSetting->TileMaterial.LoadSynchronous();
	TileSelectMat = PluginSetting->TileSelectionMaterial.LoadSynchronous();

	UObject* BlockRoomObject = Cast<UObject>(BlockRoomWidgetHandler);
	FReferencerInformationList* RefInfoList = nullptr;
	if (IsReferenced(BlockRoomObject, GARBAGE_COLLECTION_KEEPFLAGS, EInternalObjectFlags::GarbageCollectionKeepFlags, true, RefInfoList))
	{
		for (auto RefInfo : RefInfoList->InternalReferences)
		{
			UE_LOG(LogTemp, Display, TEXT("EXPR: %s"), *RefInfo.Referencer->GetName());
		}
	}

	//Load the tile plane actor if not loaded before
	if (PluginSetting->TilePlaneActor.IsPending())
		PluginSetting->TilePlaneActor.LoadSynchronous();

	//Init the ProGen CDO 
	ProGenInst = PluginSetting->ProGenInst.Get();
}

void UBlockRoomWidgetHandler::OpenSelectedRoomBP(const ARoomActor* FirstRoom)
{
	if (UClass* RoomClass = FirstRoom->GetClass())
	{
		//Get the Asset Registry Module
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		//Add all blueprint types to the filter
		FARFilter Filter;
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		//USe the asset registry to get the list of all assets of type UBlueprint
		TArray<FAssetData> AssetList;
		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		for (const FAssetData& Asset : AssetList)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset());
			if (Blueprint && Blueprint->GeneratedClass == RoomClass)
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
				break;
			}
		}
	}
}

void UBlockRoomWidgetHandler::ChangeSelectedRoom(ARoomActor* FirstRoom)
{
	if (UClass* RoomClass = FirstRoom->GetClass())
	{
		//Get the Asset Registry Module
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		//Add all blueprint types to the filter
		FARFilter Filter;
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> AssetList;
		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		for (const FAssetData& Asset : AssetList)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()); //If the selected Blueprint is the FirstRoom
			if (Blueprint && Blueprint->GeneratedClass == RoomClass && Blueprint->GeneratedClass->IsChildOf(ARoomActor::StaticClass()))
			{
				//Set exclusion
				ARoomActor* DefaultRoomActor = Cast<ARoomActor>(Blueprint->GeneratedClass->GetDefaultObject());

				//Room's enter socket scene comp located at right but the tile is located at left so we need to offset the location before starting any calculations
				for (auto& Actor : SelectedActors)
				{
					FVector Location = Actor->GetActorLocation() - FirstRoom->GetActorLocation();
					FIntPoint SelectedIndex = ProGenInst->WorldToIndex(Location);
					ExclusionTiles.Add(SelectedIndex);
				}

				//Set the exclusion tiles. Exit exclusion set as empty as we don't need anymore configuring exclusions right inside editor UI 
				DefaultRoomActor->ExitExclusions.Empty();
				DefaultRoomActor->EnterExclusions = ExclusionTiles;
				DefaultRoomActor->EnterExclusionOffset = FIntPoint(0, 0);
				DefaultRoomActor->ExitExclusionOffset = FIntPoint(0, 0);
				
				ExclusionTiles.Empty();
				
				for (auto Tile : SpawnedTiles)
				{
					UMeshComponent* TileMesh = Cast<UMeshComponent>(Tile->GetComponentByClass(UMeshComponent::StaticClass()));
					TileMesh->SetMaterial(0, PluginSetting->TileMaterial.Get());
				}

				//Mark BP as dirty
				Blueprint->Modify();

				//Save the modified asset
				if (UPackage* Package = DefaultRoomActor->GetPackage())
				{
					FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
					UPackage::SavePackage(Package, nullptr, *PackageFileName, FSavePackageArgs());
				}
				
				// FBlueprintEditorUtils::RefreshVariables(Blueprint);
				FBlueprintEditorUtils::RefreshAllNodes(Blueprint);

				GEditor->RedrawLevelEditingViewports();
				UWorld* World = GEditor->GetEditorWorldContext().World();
		
				ProGenWidgetTest = NewObject<UProGenWidgetTests>(GetTransientPackage(), UProGenWidgetTests::StaticClass());
				FlushPersistentDebugLines(World);

				ProGenWidgetTest->Initialize(DefaultRoomActor,nullptr);
				FirstRoom->Destroy();
				ProGenWidgetTest->MakeOverlapTest();

				break;
			}
		}
	}
}

FReply UBlockRoomWidgetHandler::SpawnAllTiles(ARoomActor* FirstRoom)
{
	//If spawnedTiles are not empty, that means start Exclusion button double clicked before saving it. Respawning tiles shouldn't be allowed 
	if (!SpawnedTiles.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Tiles already spawned"));
		return FReply::Unhandled();
	}
	
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return FReply::Unhandled();

	AProceduralGen* ProGen = CastChecked<AProceduralGen>(PluginSetting->ProGenClass.Get()->GetDefaultObject());
	AActor* TileActor = CastChecked<AActor>(PluginSetting->TilePlaneActor.Get()->GetDefaultObject());

	//important to first init world tiles
	ProGen->InitWorldTiles();
	FRotator Rot = FRotator(0, 0, 0);
	ProGen->ForEachTileInRoom(FirstRoom, FirstRoom->Location, ProGen->DefaultRotation, [&](const int X, const int Y)
	{
		AActor* SpawnedTile = World->SpawnActor(TileActor->GetClass(), &ProGen->Tiles[X][Y].Location, &Rot);
		SpawnedTile->AddActorLocalOffset(FVector(0, 0, 50));
		float ScaleFactor = ProGen->TileSizeX / 100.0f; // Assuming TileSizeX is 16, this will be 0.16
		SpawnedTile->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
		SpawnedTiles.Add(SpawnedTile);
	});
	return FReply::Handled();
}

bool UBlockRoomWidgetHandler::HandleTileSelection(ARoomActor* FirstRoom)
{
	//Init raycast parameters
	int RaycastLength = 1000;
	FHitResult OutHit;
	if (!SceneCapInst->IsValidLowLevelFast()) return false;
	FVector StartLocation = SceneCapInst->GetActorLocation();
	FVector EndLocation = StartLocation + (SceneCapInst->GetActorForwardVector() * RaycastLength);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(FirstRoom);

	if (GEditor->GetEditorWorldContext().World()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_Visibility))
	{
		AActor* CurrentHitActor = OutHit.GetActor();
		if (CurrentHitActor && CurrentHitActor->ActorHasTag(FName("Tile")))
		{
			if (LastHitTile != CurrentHitActor)
			{
				// Revert the material of the last hit actor if tile changed and it weren't selected 
				if (LastHitTile && TileUnhoverMat)
				{
					UMeshComponent* LastMeshComponent = Cast<UMeshComponent>(LastHitTile->GetComponentByClass(UMeshComponent::StaticClass()));
					if (LastMeshComponent && !LastMeshComponent->ComponentTags.Contains("SelectedTile"))
					{
						LastMeshComponent->SetMaterial(0, TileUnhoverMat);
						if (SelectedActors.Contains(LastHitTile))
							SelectedActors.Remove(LastHitTile);
					}
					//If tile were selected but ctrl, is held, revert the material
					else if (FSlateApplication::Get().GetModifierKeys().IsControlDown() && LastMeshComponent->ComponentTags.Contains("SelectedTile"))
					{
						//WHen deselecting, if the selected tile deselected, remove from the array
						if (SelectedActors.Contains(LastHitTile))
							SelectedActors.Remove(LastHitTile);

						LastMeshComponent->SetMaterial(0, TileUnhoverMat);
					}
				}

				// Update the last hit actor and store its original material
				LastHitTile = CurrentHitActor;
				if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(CurrentHitActor->GetComponentByClass(UMeshComponent::StaticClass())))
				{
					//If shift pressed select the tile
					if (FSlateApplication::Get().GetModifierKeys().IsShiftDown()) //Is shift down
					{
						MeshComponent->SetMaterial(0, TileSelectMat);
						MeshComponent->ComponentTags.Add(FName("SelectedTile"));

						if (!SelectedActors.Contains(CurrentHitActor))
							SelectedActors.Add(CurrentHitActor);
					}
					else if (!MeshComponent->ComponentTags.Contains("SelectedTile")) //No shift selected, apply hover material
					{
						TileUnhoverMat = MeshComponent->GetMaterial(0);
						MeshComponent->SetMaterial(0, TileHoverMat);
					}
				}
			}
		}
		return true;
	}
	else
	{
		// No hit or hit a different actor, revert the material of the last hit actor if the tile weren't selected
		if (LastHitTile && TileUnhoverMat)
		{
			UMeshComponent* MeshComponent = Cast<UMeshComponent>(LastHitTile->GetComponentByClass(UMeshComponent::StaticClass()));
			if (!MeshComponent->ComponentTags.Contains("SelectedTile"))
			{
				MeshComponent->SetMaterial(0, TileUnhoverMat);
			}
		}
		LastHitTile = nullptr;
		TileUnhoverMat = nullptr;
		return false;
	}
}

FReply UBlockRoomWidgetHandler::TestOverlapWithSecondSelectedRoom(ARoomActor* FirstRoom, ARoomActor* SecondRoom)
{
	//If tiles are not destroyed, destroy all of them first
	if (!BlockRoomWidgetHandler->SpawnedTiles.IsEmpty())
	{
		for (auto& SpawnedTile : BlockRoomWidgetHandler->SpawnedTiles)
		{
			if (SpawnedTile)
				SpawnedTile->Destroy();
		}
	}

	//Spawn second room
	UWorld* World = GEditor->GetEditorWorldContext().World();
	SecondRoom =  World->SpawnActor<ARoomActor>(SecondRoom->GetClass(), FirstRoom->DoorSocketExit->GetComponentLocation(), PluginSetting->ProGenInst->DefaultRotation);
	if (PluginSetting->ProGenInst->IsColliding(SecondRoom,FirstRoom->DoorSocketExit->GetComponentLocation(),SecondRoom->GetActorRotation()))
	{
		//Handle this case later on 
		UE_LOG(LogTemp, Display, TEXT("Really overlapping"));
		return FReply::Unhandled();
	}
	else
	{
		return FReply::Handled();
	}
}

UBlockRoomWidgetHandler::~UBlockRoomWidgetHandler()
{
	for (auto& Element : SpawnedTiles)
	{
		Element->Destroy();
	}
}
