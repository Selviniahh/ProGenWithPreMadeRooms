// Fill out your copyright notice in the Description page of Project Settings.
//NOTE: RoomManager class will initialize this class

#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest//AllCorridorTestWidget.h"
#include "SlateMaterialBrush.h" 
#include "SlateOptMacros.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/ProGenWidget.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest/CorridorTestHandler.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest/CorrScenarioManagerWidget.h"

class FAssetRegistryModule;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAllCorridorTestWidget::Construct(const FArguments& InArgs)
{
	FirstRoom = InArgs._FirstRoom;
	SecondRoom = InArgs._SecondRoom;
	PluginSetting = GetDefault<UPluginSettings>();
	CorrTestHandler = NewObject<UCorridorTestHandler>();
	CorrTestHandler->Initialize(FirstRoom, SecondRoom);

	if (UMaterialInterface* Material = PluginSetting->SceneCapMaterial.Get())
		SceneCapImgBrush = MakeShared<FSlateMaterialBrush>(*Material, FVector2D(100, 100)); // Initialize member variable

	FSlateFontInfo PropertyTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	PropertyTextFont.Size = 12;

	World = GEditor->GetEditorWorldContext().World();
	FlushPersistentDebugLines(World);

	//Access Scene capture component. THe reason not creating actor from C++, Scene capture component details are not exposed in BP editor
	SceneCapInst = PluginSetting->SceneCapActorInst.Get();
	TArray<USceneCaptureComponent2D*> SceneCapCompArray;
	SceneCapInst->GetComponents(SceneCapCompArray);
	
	ChildSlot
	[
		// Parent Horizontal Box
		SNew(SHorizontalBox)

		// Image Slot
		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .FillWidth(1.0)
		[
			SNew(SImage)
			.Image(SceneCapImgBrush.Get()) // Initialize member variable
		]

		// Vertical Box Slot for Buttons and TextBoxes
		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .FillWidth(1.0)
		[
			SNew(SVerticalBox) // This Vertical Box will contain all the buttons and textboxes

			//NOTE: 1. Start all corridor scenarios
			//REMIND: This not working so I decided to discard all 
			// + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			//                       .Padding(FMargin(15, 5, 15, 5))
			// [
			// 	HorizontalField(
			// 		{
			// 			ConstructTextBlock(PropertyTextFont, FText::FromString("Start All Corridor Scenarios")), 
			// 			ConstructButton(FText::FromString("Start Test"), [this]()
			// 			{
			// 				CorrTestHandler->Start(); //TODO: IDK Why yet but it makes couple second bottlenecks. Somehow fix it
			// 				return FReply::Handled();
			// 			}),
			// 		})
			// ]

			//NOTE: 2. Initialize Corridor Scenario manager widget
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(PropertyTextFont, FText::FromString(FirstRoom->GetClass()->GetName() + "Configure corridor scenarios")),
						ConstructButton(FText::FromString("Click me"), [this]()-> FReply
						{
							FText Title = FText::FromString("Corridor Scenario Manager for " + 	UEnum::GetValueAsString(FirstRoom->ExitSocketDirection) + TEXT(" -> ") + 
								UEnum::GetValueAsString(SecondRoom->EnterSocketDirection));
								
							//Initialize the window for corridor scenarios manager
							TSharedRef<SCorrScenarioManagerWidget> CorridorScenarioManagerWidget = SNew(SCorrScenarioManagerWidget)
							.FirstRoom(FirstRoom)
							.SecondRoom(SecondRoom)
							.SpawnedRooms(CorrTestHandler->SpawnedRooms);
							//TODO: Later on add other slate stuffs to pass here
							
							TSharedRef<SWindow> CorrScenarioManager = SNew(SWindow)
							.Title(Title)
							.ClientSize(FVector2D(1920,1080))
							.SupportsMaximize(true)
							.SupportsMinimize(true)
							[
								CorridorScenarioManagerWidget
							];

							FSlateApplication::Get().AddWindow(CorrScenarioManager);
							return FReply::Handled();
						}),
					})
			]

			//NOTE: 3. Set FirstRoom start offset 
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(PropertyTextFont, FText::FromString(FirstRoom->GetClass()->GetName() + " Start Offset")),
						ConstructTextBlock(PropertyTextFont, FText::FromString(FirstRoom->PathStartOffset.ToString())), //let's see how will this end up
						ConstructButton(FText::FromString("Change"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructButton(FText::FromString("Set"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructCheckBox(),
					})
			]

			//NOTE: 4. Set FirstRoom End Offset 
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(PropertyTextFont, FText::FromString(FirstRoom->GetClass()->GetName() + " End Offset")),
						ConstructTextBlock(PropertyTextFont, FText::FromString(FirstRoom->PathEndOffset.ToString())), //let's see how will this end up
						ConstructButton(FText::FromString("Change"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructButton(FText::FromString("Set"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructCheckBox(),
					})
			]

			//NOTE: 5. Set SecondRoom's Start Offset 
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(PropertyTextFont, FText::FromString(SecondRoom->GetClass()->GetName() + " Start Offset")),
						ConstructTextBlock(PropertyTextFont, FText::FromString(SecondRoom->PathStartOffset.ToString())), //let's see how will this end up
						ConstructButton(FText::FromString("Change"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructButton(FText::FromString("Set"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructCheckBox(),
					})
			]

			//NOTE: 6. Change first selected room to test first selected room's end offset 
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(PropertyTextFont, FText::FromString("Select different room to test first selected room's end offset")),
						ConstructButton(FText::FromString("Select Room"), [this]()-> FReply { return FReply::Handled(); }), //REMIND: change delegate later on
						ConstructCheckBox(),
					})
			]
		]
	];
	SceneCapComp = SceneCapCompArray[0];
}

//Destroy all the necessary actors when the widget closed 

void SAllCorridorTestWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	SProGenWidget::HandleRenderViewMovement(InDeltaTime, SceneCapInst, SceneCapComp, CurrentInputVelocity);
}

TSharedRef<SHorizontalBox> SAllCorridorTestWidget::HorizontalField(std::initializer_list<TSharedRef<SWidget>> Content)
{
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	for (const auto& Element : Content)
	{
		HorizontalBox->AddSlot()
		             .AutoWidth()
		             .HAlign(HAlign_Fill)
		             .VAlign(VAlign_Fill)
		             .Padding(FMargin(10, 5, 10, 5))
		[
			Element
		];
	}

	return HorizontalBox;
}

TSharedRef<SVerticalBox> SAllCorridorTestWidget::ConstructButtonAndItsTextBox(const FSlateFontInfo& PropertyTextFont, const FString& TextBoxText, const FString& ButtonText, const TFunction<FReply()>& ButtonClick)
{
	TSharedPtr<STextBlock> LocalTextBlock;

	TSharedRef<SVerticalBox> Slot = SNew(SVerticalBox)

		//2. slot textbox and button
		+ SVerticalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .AutoHeight()
		  .Padding(FMargin(10, 0, 0, 0))
		[
			SNew(SHorizontalBox)

			//1. test button textbox
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			  .HAlign(HAlign_Center)
			[
				ConstructTextBlock(PropertyTextFont, FText::FromString(TextBoxText)) //Let's see how it will be null init
			] //"If box component extends are incorrect"

			//Make test button
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			  .HAlign(HAlign_Center)
			  .Padding(FMargin(10, 0, 0, 0))
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateLambda(ButtonClick))
				[
					ConstructTextBlock(PropertyTextFont, FText::FromString(ButtonText))
				]
			]
		];
	return Slot;
}

TSharedRef<SWidget> SAllCorridorTestWidget::ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, const FSlateColor* Color)
{
	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
	   .Justification(ETextJustify::Center)
	   .Text(Text)
	   .Font(FontInfo)
		.Justification(ETextJustify::Center)
	   .RenderOpacity(0.5);

	if (Color) TextBlock.Get().SetColorAndOpacity(*Color);

	//Just wrapped in a button to make hover and unhover effect 
	return SNew(SButton)
	   .ButtonStyle(FCoreStyle::Get(), "NoBorder")
	   .HAlign(HAlign_Center)
	   .VAlign(VAlign_Center)
	   .OnHovered_Lambda([TextBlock]()
	                    {
		                    TextBlock->SetRenderOpacity(1.0); // Hover: increase opacity
	                    })
	   .OnUnhovered_Lambda([TextBlock]()
	                    {
		                    TextBlock->SetRenderOpacity(0.5); // Unhover: decrease opacity
	                    })
	[
		TextBlock
	];
}

TSharedRef<SButton> SAllCorridorTestWidget::ConstructButton(const FText& Text, const TFunction<FReply()>& ButtonClick)
{
	FSlateFontInfo PropertyTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	PropertyTextFont.Size = 12;
	TSharedPtr<STextBlock> TextBlock = SNew(STextBlock).Text(Text);

	TSharedRef<SButton> Button = SNew(SButton)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.OnClicked(FOnClicked::CreateLambda(ButtonClick))
	.ContentPadding(FMargin(5, 0, 5, 0))
	.Text(Text);


	Button->SetOnHovered(FSimpleDelegate::CreateLambda([Button]() { Button.Get().SetRenderOpacity(1); }));
	Button->SetOnUnhovered(FSimpleDelegate::CreateLambda([Button]() { Button.Get().SetRenderOpacity(0.5); }));
	return Button;
}

TSharedRef<SCheckBox> SAllCorridorTestWidget::ConstructCheckBox()
{
	return SNew(SCheckBox); //REMIND: Pass an enum that when checked or unchecked it will change it's room's enum for now it's ok to stay empty like this  
}

SAllCorridorTestWidget::~SAllCorridorTestWidget()
{
	
	CorrTestHandler->DestroySpawnedRooms();
}

void SAllCorridorTestWidget::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	SCompoundWidget::OnFocusLost(InFocusEvent);
}

FReply SAllCorridorTestWidget::Click()
{
	CorrTestHandler->Start();
	return FReply::Handled();
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
