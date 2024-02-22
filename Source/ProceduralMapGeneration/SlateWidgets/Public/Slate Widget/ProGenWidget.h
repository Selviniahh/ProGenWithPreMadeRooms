#pragma once

#include "CoreMinimal.h"
#include "SlateMaterialBrush.h"
#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "Widgets/SCompoundWidget.h"

class SPopUpButton;
class UPluginSettings;
class ARoomActor;
class AProceduralGen;
/**
 * 
 */
class PROCEDURALMAPGENERATION_API SProGenWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProGenWidget)
		{}
	
	SLATE_ARGUMENT(UMaterialInterface*, Material)
	SLATE_ARGUMENT(AActor*, SceneCapActor)
	SLATE_END_ARGS()

	void RetrieveProGenActor();
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual ~SProGenWidget() override;
	static void HandleRenderViewMovement(float InDeltaTime, AActor* SceneCapActor, USceneCaptureComponent2D* SceneCapComponent, FVector& CurrentVelocity);
	TSharedRef<SHorizontalBox> ConstructTextInput(const FText& Text, const FText& HintText, const FSlateFontInfo& PropertyTextFont, const FOnTextChanged& OnTextChangedDelegate);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	TSharedPtr<FSlateMaterialBrush> MaterialBrush; // Member variable
	mutable  UPluginSettings* PluginSetting;
	AActor* SceneCapInst;
	FVector CurrentInputVelocity; // Member variable to store current velocity
	USceneCaptureComponent2D* SceneCapComp;

	FReply DoubleClicked(const FGeometry& Geo, const FPointerEvent& PointerEvent) const;
	
#pragma region ListView
	TSharedRef<SListView<TWeakObjectPtr<ARoomActor>>> ConstructListView();
	void HighlightAvailableRoomRows(TWeakObjectPtr<ARoomActor> RoomActor);
	void SetDirectionMaps(TWeakObjectPtr<ARoomActor> RoomActor);
	TSharedRef<ITableRow> OnGenerateListViewRow(TWeakObjectPtr<ARoomActor> RoomActor, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TWeakObjectPtr<ARoomActor> RoomActor, ESelectInfo::Type SelectedType);
	EVisibility MakeTestButtonVisibility(TWeakObjectPtr<ARoomActor> SelectedRoom) const;
	FReply OnButtonClicked();

	TSharedPtr<SListView<TWeakObjectPtr<ARoomActor>>> ConstructedAssetListView;
	TMap<FString, TSharedPtr<FTableRowStyle>> RowStyle;
	TMap<FString,TSharedPtr<FSlateBrush>> SlateEnterBrushMap;
	TMap<FString,TSharedPtr<FSlateBrush>> SlateExitBrushMap;
	TArray<TWeakObjectPtr<ARoomActor>> ProGenRooms;
	TWeakObjectPtr<ARoomActor> PreviousFirstSelectedItem;
	int PrevNumOfSelection;
	TArray<TWeakObjectPtr<ARoomActor>> SelectionOrder;
	FName RoomManagerTabName = TEXT("RoomManagerTab");
	
#pragma endregion ListView

/*I couldn't handle matrix transformation and all those stuff so pop up is not working. */
#pragma region unmanaged
	TSharedRef<SMenuAnchor> ConstructMenuAnchor(const TSharedRef<SPopUpButton>& PopUpButton);
	TOptional<TTransform2<float>> SetPopUpLocation() const;

	mutable FGeometry Geometry; //Still don't know what mutable was muting
	TSharedPtr<SMenuAnchor> MenuAnchor;
#pragma endregion unmanaged

	
	AActor* SceneCapActor;

	//Settings for scene capture orientation
	int Multiplayer = 444;
	int InterpSpeed = 444;

	int TestVariable = 1;
};
