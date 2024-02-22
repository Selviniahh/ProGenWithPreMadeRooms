// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/ProGenWidget.h"
#include "EngineUtils.h"
#include "SlateOptMacros.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/AssetManager.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "Widgets/Layout/SScrollBox.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/GlobalInputListener.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PopUpButton.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/RoomManager.h"
#include "Widgets/Views/SListView.h"
#include "Styling/SlateBrush.h"

class SRoomManager;
struct FStreamableManager;
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SProGenWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	SceneCapActor = InArgs._SceneCapActor;
	
	//Set the slate window focus
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EFocusCause::SetDirectly);

	//Load the material
	if (UMaterialInterface* Material = InArgs._Material)
		MaterialBrush = MakeShared<FSlateMaterialBrush>(*Material, FVector2D(100, 100)); // Initialize member variable

	//Set the fonts 
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;
	FSlateFontInfo PropertyTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 12;


	//Access to the progen actor if progen instance is dragged to the scene
	PluginSetting = GetMutableDefault<UPluginSettings>();
	RetrieveProGenActor();
	
	AProceduralGen* ProceduralGen = Cast<AProceduralGen>(PluginSetting->ProGenClass.Get()->GetDefaultObject());
	PluginSetting->ProGenInst = ProceduralGen;

	//Init button slate
	TSharedRef<SPopUpButton> PopUpButton = SNew(SPopUpButton);

	//Init RowStyle
	TSharedPtr<FTableRowStyle> DefaultTableRowStyle = MakeShared<FTableRowStyle>(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"));
	for (auto RoomActor : ProGenRooms)
	{
		RowStyle.Add(RoomActor.Get()->GetName(),DefaultTableRowStyle);
	}

	//Access Scene capture component. THe reason not creating actor from C++, Scene capture component details are not exposed in BP editor
	SceneCapInst = PluginSetting->SceneCapActorInst.Get();
	TArray<USceneCaptureComponent2D*> SceneCapCompArray;
	SceneCapInst->GetComponents(SceneCapCompArray);
	SceneCapComp = SceneCapCompArray[0];
	
	ChildSlot
	[
		// Top-Level Vertical Box
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			ConstructMenuAnchor(PopUpButton)
		]
		
		// TextBlock Slot
		+ SVerticalBox::Slot()
		  .AutoHeight() // Adjust as needed for your text size
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .Padding(FMargin(0, 10, 0, 15))
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Procedural Map Generation")))
			.Justification(ETextJustify::Center)
			.Font(TitleTextFont) // Set font size to 24 and use bold weight
			.OnDoubleClicked(this, &SProGenWidget::DoubleClicked)
		]

		// HorizontalBox Slot
		+ SVerticalBox::Slot()
		.FillHeight(1) // Takes remaining space
		[
			SNew(SHorizontalBox)

			// Left Section (VerticalBox with Image)
			+ SHorizontalBox::Slot()
			  .VAlign(VAlign_Fill) // Adjust width ratio as needed
			  .HAlign(HAlign_Fill) // Adjust width ratio as needed
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				  .VAlign(VAlign_Fill)
				  .HAlign(HAlign_Fill)
				[
					SNew(SImage)
					.Image(MaterialBrush.Get())
				]
			]

			// Right Section (VerticalBox with TextBlocks and TextBoxes)
			+ SHorizontalBox::Slot()
			  .VAlign(VAlign_Fill)
			  .HAlign(HAlign_Fill) 
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					ConstructListView()
				]
			]
		]
	];
}

SProGenWidget::~SProGenWidget()
{
	
}

void SProGenWidget::HandleRenderViewMovement(const float InDeltaTime, AActor* SceneCapActor, USceneCaptureComponent2D* SceneCapComponent, FVector& CurrentVelocity)
{
	TMap<FKey, bool> KeyMap = FMyInputProcessor::KeyPressedMap;
	FKey MyKey = FMyInputProcessor::PressedKey;
	FVector Direction = FVector::ZeroVector;
	
	//Simulate actual add input movement as much as possible
	if (FMyInputProcessor::KeyPressedMap.Contains(MyKey))
	{
		if		(KeyMap[EKeys::A]) Direction = FVector(-1, 0, 0);
		else if (KeyMap[EKeys::D]) Direction = FVector(1, 0, 0);
		else if (KeyMap[EKeys::W]) Direction = FVector(0, -1, 0);
		else if (KeyMap[EKeys::S]) Direction = FVector(0, 1, 0);
		else if (KeyMap[EKeys::Q]) Direction = FVector(0, 0, -1);
		else if (KeyMap[EKeys::E]) Direction = FVector(0, 0, 1);
		
		// Check for specific diagonal combinations
		if (KeyMap[EKeys::A] && KeyMap[EKeys::W]) Direction = FVector(-1, -1, 0).GetSafeNormal();
		if (KeyMap[EKeys::W] && KeyMap[EKeys::D]) Direction = FVector(1, -1, 0).GetSafeNormal();
		if (KeyMap[EKeys::A] && KeyMap[EKeys::S]) Direction = FVector(-1, 1, 0).GetSafeNormal();
		if (KeyMap[EKeys::D] && KeyMap[EKeys::S]) Direction = FVector(1, 1, 0).GetSafeNormal();
	}

	if (SceneCapActor)
	{
		FVector TargetVelocity = Direction * FMyInputProcessor::MovementSpeed;
		// Smoothly interpolate to the target velocity
		CurrentVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, InDeltaTime, 10.0f);

		//Ortho camera movement
		if (KeyMap[EKeys::Q] || KeyMap[EKeys::E])
		{
			SceneCapComponent->OrthoWidth =  SceneCapComponent->OrthoWidth + TargetVelocity.Z / 100;
			UE_LOG(LogTemp, Display, TEXT("orto: %s"), *TargetVelocity.ToString());

		}
		//Location movement 
		else
		{
			// Apply the current velocity to update position
			FVector NewLocation = SceneCapActor->GetActorLocation() + CurrentVelocity * InDeltaTime;
			NewLocation.Z = SceneCapActor->GetActorLocation().Z;
			SceneCapActor->SetActorLocation(NewLocation);	
		}
	}
}

void SProGenWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	HandleRenderViewMovement(InDeltaTime,SceneCapActor,SceneCapComp,CurrentInputVelocity);
}

FReply SProGenWidget::DoubleClicked(const FGeometry& Geo, const FPointerEvent& PointerEvent) const
{
	UE_LOG(LogTemp, Display, TEXT("DoubleClicked: %s"));

	return FReply::Handled();
}

TSharedRef<SListView<TWeakObjectPtr<ARoomActor>>> SProGenWidget::ConstructListView()
{
	ConstructedAssetListView = SNew(SListView<TWeakObjectPtr<ARoomActor>>)
	.ListItemsSource(&ProGenRooms)
	.OnGenerateRow(this, &SProGenWidget::OnGenerateListViewRow)
	.OnSelectionChanged(SharedThis(this), &SProGenWidget::OnSelectionChanged);
	
	return ConstructedAssetListView.ToSharedRef();
}

TSharedRef<ITableRow> SProGenWidget::OnGenerateListViewRow(TWeakObjectPtr<ARoomActor> RoomActor, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!RoomActor.IsValid()) return SNew(STableRow<TWeakObjectPtr<ARoomActor>>, OwnerTable);
	SetDirectionMaps(RoomActor);

	HighlightAvailableRoomRows(RoomActor);
	
	TSharedRef<STableRow<TWeakObjectPtr<ARoomActor>>> ListViewRowWidget = SNew(STableRow<TWeakObjectPtr<ARoomActor>>, OwnerTable)
		.Style(RowStyle[RoomActor->GetName()].Get())
		[
			SNew(SHorizontalBox)

			//First slot for ActorName
			+ SHorizontalBox::Slot()
			  .Padding(FMargin(10, 10, 10, 10))
			  .HAlign(HAlign_Center)
			  .VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
				.Text(FText::FromString(RoomActor->GetName()))
				.Justification(ETextJustify::Center)
				]
			]

			//Second slot for two image
			+ SHorizontalBox::Slot()
			  .Padding(FMargin(10, 10, 10, 10))
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				  .AutoWidth() // Use AutoWidth to size the slot to the content
				  .VAlign(VAlign_Center) // Vertically center the image in the slot
				  .HAlign(HAlign_Center) // Horizontally center the image in the slot
				  .Padding(FMargin(0,0,10,0))
				[
					SNew(SImage)
					.Image(SlateEnterBrushMap[RoomActor.Get()->GetName()].Get())
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth() // Similarly, use AutoWidth for the second image
				  .VAlign(VAlign_Center) // Vertically center the image
				  .HAlign(HAlign_Center) // Horizontally center the image
				  .Padding(FMargin(0,0,10,0))
				[
					SNew(SImage)
					.Image(SlateExitBrushMap[RoomActor->GetName()].Get())
				]
			]

			//Third slot for Tested Text and button
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .VAlign(VAlign_Center)
				  .HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Tested"))
				]

				+ SHorizontalBox::Slot()
				  .Padding(FMargin(6, 0, 0, 0))
				  .AutoWidth()
				  .VAlign(VAlign_Center)
				  .HAlign(HAlign_Center)
				[
					SNew(SCheckBox)
					.IsEnabled(false)
				]
			]

			//Fourth slot for Make test textbox and button (I am not sure to entire delete here) 
			+ SHorizontalBox::Slot()
			  .Padding(FMargin(10, 10, 10, 10))
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					 .VAlign(VAlign_Center)
				[
				SNew(SButton)
					.OnClicked(this, &SProGenWidget::OnButtonClicked)
					.Text(FText::FromString(TEXT("Make Test")))
					.Visibility(this,&SProGenWidget::MakeTestButtonVisibility,RoomActor)
				]
			]
		];
	
	return ListViewRowWidget;
}

void SProGenWidget::HighlightAvailableRoomRows(TWeakObjectPtr<ARoomActor> RoomActor)
{
	if (ConstructedAssetListView.Get()->GetNumItemsSelected() == 1)
	{
		Direction FirstRoomExitSocDir = ConstructedAssetListView.Get()->GetSelectedItems()[0]->ExitSocketDirection;
		Direction ExpectedDirection = AProceduralGen::ExpectedDirection(FirstRoomExitSocDir);

		if (RoomActor->EnterSocketDirection == ExpectedDirection)
		{
			//Highlight the row
			FTableRowStyle* HighlightStyle = RowStyle[RoomActor->GetName()].Get();
			FSlateColor HighlightColor = FSlateColor(FLinearColor(1.0f, 1.0f, 0.8f, 0.3f)); // Light yellow with some transparency

			HighlightStyle->SetEvenRowBackgroundBrush(FSlateColorBrush(HighlightColor));
			HighlightStyle->SetOddRowBackgroundBrush(FSlateColorBrush(HighlightColor));
			HighlightStyle->SetEvenRowBackgroundHoveredBrush(FSlateColorBrush(HighlightColor));
			HighlightStyle->SetOddRowBackgroundHoveredBrush(FSlateColorBrush(HighlightColor));	
		}
	}
	else //Reset to default
	{
		TSharedPtr<FTableRowStyle> NewStyle = MakeShared<FTableRowStyle>(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"));
		RowStyle[RoomActor->GetName()] = NewStyle;
	}
}

void SProGenWidget::OnSelectionChanged(TWeakObjectPtr<ARoomActor> RoomActor, ESelectInfo::Type SelectedType)
{
	/*Track the num of selection and empty selection order if selection has changed. This is important to make selection not exceed over 2 elements*/
	int CurrentSelection = ConstructedAssetListView.Get()->GetNumItemsSelected();
	if (PrevNumOfSelection != CurrentSelection)
	{
		SelectionOrder.Empty();
		PrevNumOfSelection = CurrentSelection;
	}
	
	/*if first selected element changed, undo all highlights*/
	TSharedPtr<FTableRowStyle> NewStyle = MakeShared<FTableRowStyle>(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"));
	TWeakObjectPtr<ARoomActor> CurrentFirstSelectedItem = ConstructedAssetListView.Get()->GetSelectedItems().Num() > 0 ? ConstructedAssetListView.Get()->GetSelectedItems()[0] : nullptr;
	if (PreviousFirstSelectedItem != CurrentFirstSelectedItem)
	{
		SelectionOrder.Empty();
		for (auto& StyleEntry : RowStyle)
		{
			 StyleEntry.Value = MakeShared<FTableRowStyle>(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"));
		}
		
		//Assign Previous selected one
		PreviousFirstSelectedItem = CurrentFirstSelectedItem;
	}

	/*Selection multiple rows with Ctrl cannot capture the most recent selection*/
	TArray<TWeakObjectPtr<ARoomActor>> CurrentSelectedRooms = ConstructedAssetListView.Get()->GetSelectedItems();
	if (CurrentSelectedRooms.Contains(RoomActor))
	{
		CurrentSelectedRooms.Remove(RoomActor);

		if (!CurrentSelectedRooms.IsEmpty() && ConstructedAssetListView.Get()->GetNumItemsSelected() == 2)
		{
			SelectionOrder.Add(RoomActor);
			SelectionOrder.Add(CurrentSelectedRooms[0]);
		}
	}
	
	ConstructedAssetListView.Get()->RebuildList();
}

TSharedRef<SHorizontalBox> SProGenWidget::ConstructTextInput(const FText& Text, const FText& HintText, const FSlateFontInfo& PropertyTextFont, const FOnTextChanged& OnTextChangedDelegate)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		  .AutoWidth()
		  .VAlign(VAlign_Center)
		  .HAlign(HAlign_Center)
		  .Padding(FMargin(15, 0, 15, 0))
		[
			SNew(STextBlock)
					.Text(Text)
					.Font(PropertyTextFont)
		]

		+ SHorizontalBox::Slot()
		  .AutoWidth()
		  .VAlign(VAlign_Center)
		  .HAlign(HAlign_Center)
		[
			SNew(SEditableTextBox)
					.Font(PropertyTextFont)
					.HintText(HintText)
					.OnTextChanged(OnTextChangedDelegate)
		];
}

EVisibility SProGenWidget::MakeTestButtonVisibility(TWeakObjectPtr<ARoomActor> SelectedRoom) const
{
	if (ConstructedAssetListView.Get()->GetNumItemsSelected() == 2 && ConstructedAssetListView.Get()->GetSelectedItems()[1] == SelectedRoom)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Hidden;
	}
}

//TODO: If the directions are incorrect, Pop up message box

FReply SProGenWidget::OnButtonClicked()
{
	/*If selection is incorrect open msg dialog box*/
	Direction FirstRoomExitSocDir = SelectionOrder[0]->ExitSocketDirection;
	Direction SecRoomEnterSocDir = SelectionOrder[1]->EnterSocketDirection;
	Direction ExpectedDirection = AProceduralGen::ExpectedDirection(FirstRoomExitSocDir);

	//If first selection is NoExit room, abort
	if (SelectionOrder[0]->NoExit)
	{
		FText Message = FText::FromString("Room " + SelectionOrder[0]->GetName() + " marked as NoExit room. No exit rooms cannot be selected as first room");
		FMessageDialog::Open(EAppMsgType::Ok,Message);
		return FReply::Handled();
	}

	//If selection is incorrect abort
	if (SecRoomEnterSocDir != ExpectedDirection)
	{
		FString FirstRoomExitSocDirStr = UEnum::GetValueAsString(FirstRoomExitSocDir);
		FString SecRoomEnterSocDirStr = UEnum::GetValueAsString(SecRoomEnterSocDir);
		FString ExpectedDirectionStr = UEnum::GetValueAsString(ExpectedDirection);
		FText Message = FText::FromString("Incorrect Test Selection. \n\nFirst Room's exit socket direction is " + FirstRoomExitSocDirStr + "\n"
		"Second Room's enter socket direction is " + SecRoomEnterSocDirStr + "\n"
		"Second Selected room expected direction is: " + ExpectedDirectionStr);
		FMessageDialog::Open(EAppMsgType::Ok,Message);
	}
	//Everything checks out create a new tab
	else
	{
		//Create the content for the window
		TSharedRef<SRoomManager> RoomManagerContent = SNew(SRoomManager)
		.RoomFirst(SelectionOrder[0].Get())
		.RoomSecond(SelectionOrder[1].Get());

		// Create and open the standalone window
		TSharedRef<SWindow> RoomManagerWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Room Manager")))
		.ClientSize(FVector2D(1920,1080))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			RoomManagerContent
		];

		FSlateApplication::Get().AddWindow(RoomManagerWindow);

		
	}
	return FReply::Handled();
}

void SProGenWidget::SetDirectionMaps(TWeakObjectPtr<ARoomActor> RoomActor)
{
	FSoftObjectPath DirPath;
	switch (RoomActor->EnterSocketDirection)
	{
	case HorizontalRight:
		DirPath = PluginSetting->Right;
		break;
	case HorizontalLeft:
		DirPath = PluginSetting->Left;
		break;
	case VerticalUp:
		DirPath = PluginSetting->Up;
		break;
	case VerticalDown:
		DirPath = PluginSetting->Down;
		break;
	default: ;
	}
	TSharedPtr<FSlateBrush> SlateBrush = MakeShared<FSlateBrush>();
	if (UTexture2D* TextureRaw = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *DirPath.ToString())))
	{
		SlateBrush.Get()->SetResourceObject(TextureRaw);
		SlateBrush.Get()->ImageSize = FVector2D(16,16);
		SlateBrush.Get()->DrawAs = ESlateBrushDrawType::Image;
		SlateBrush.Get()->ImageType = ESlateBrushImageType::FullColor;
	}
	
	SlateEnterBrushMap.Add(RoomActor.Get()->GetName(),SlateBrush);
	
	FSoftObjectPath DirPathEnd;
	switch (RoomActor->ExitSocketDirection)
	{
	case HorizontalRight:
		DirPathEnd = PluginSetting->Right;
		break;
	case HorizontalLeft:
		DirPathEnd = PluginSetting->Left;
		break;
	case VerticalUp:
		DirPathEnd = PluginSetting->Up;
		break;
	case VerticalDown:
		DirPathEnd = PluginSetting->Down;
		break;
	default: ;
	}

	//If room is no exit, assign no exit image
	if (RoomActor->NoExit)
		DirPathEnd = PluginSetting->NoExit;
	
	TSharedPtr<FSlateBrush> SlateBrushEnd = MakeShared<FSlateBrush>();
	if (UTexture2D* TextureRaw = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *DirPathEnd.ToString())))
	{
		SlateBrushEnd.Get()->SetResourceObject(TextureRaw);
		SlateBrushEnd.Get()->ImageSize = FVector2D(16,16);
		SlateBrushEnd.Get()->DrawAs = ESlateBrushDrawType::Image;
		SlateBrushEnd.Get()->ImageType = ESlateBrushImageType::FullColor;
	}
	SlateExitBrushMap.Add(RoomActor.Get()->GetName(),SlateBrushEnd);
}


void SProGenWidget::RetrieveProGenActor()
{
	//try to load from world outliner
	AProceduralGen* ProceduralGen = nullptr;
	for (TActorIterator<AProceduralGen> Actor(GEditor->GetEditorWorldContext().World()); Actor; ++Actor)
	{
		ProceduralGen = *Actor;
	}

	//If not found in world outliner, try to load from project settings sync
	if (!ProceduralGen)
	{
		if (PluginSetting->ProGenClass.IsPending())
		{
			PluginSetting->ProGenClass.LoadSynchronous();
		}
		ProceduralGen = Cast<AProceduralGen>(PluginSetting->ProGenClass.Get()->GetDefaultObject());
	}

	for (auto RoomDesign : ProceduralGen->RoomDesigns)
	{
		ARoomActor* RoomActor = Cast<ARoomActor>(RoomDesign->GetDefaultObject());
		ProGenRooms.Add(RoomActor);
	}
	
	//After loading Rooms, fill the test case map for each room
	for (auto ProGenRoom : ProGenRooms)
	{
		PluginSetting->AllTestCases.Add(ProGenRoom->GetClass()->GetName(), ETestCase::Undefined);
		PluginSetting->IsRoomsGenerated = true;
	}

}
#pragma region unmanaged

TSharedRef<SMenuAnchor> SProGenWidget::ConstructMenuAnchor(const TSharedRef<SPopUpButton>& PopUpButton)
{
	MenuAnchor = SNew(SMenuAnchor)
	   .MenuContent(PopUpButton)
		.Placement(MenuPlacement_ComboBox)
		.FitInWindow(true)
		.RenderTransform(SharedThis(this),&SProGenWidget::SetPopUpLocation);

	return MenuAnchor.ToSharedRef();
}

TOptional<TTransform2<float>> SProGenWidget::SetPopUpLocation() const
{
	// Get the absolute mouse cursor position.
	FVector2D MousePos = FSlateApplication::Get().GetCursorPos();
	FVector2D LocalMousePos = Geometry.AbsoluteToLocal(MousePos);
	return TTransform2<float>(LocalMousePos);
}
#pragma endregion unmanaged

END_SLATE_FUNCTION_BUILD_OPTIMIZATION