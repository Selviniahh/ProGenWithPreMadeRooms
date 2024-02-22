// Fill out your copyright notice in the Description page of Project Settings.
//NOTE: Widget spawns the second room, it displays direction images too 

#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest/CorrScenarioManagerWidget.h"
#include "SlateMaterialBrush.h"
#include "SlateOptMacros.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/GlobalInputListener.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest/CorrScenarioManagerHandler.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SCorrScenarioManagerWidget::Construct(const FArguments& InArgs)
{
	FirstRoom = InArgs._FirstRoom;
	SecondRoom = InArgs._SecondRoom;
	SpawnedRooms = InArgs._SpawnedRooms;
	PlugSetting = GetDefault<UPluginSettings>();

	if (UMaterialInterface* Material = PlugSetting->SceneCapMaterial.Get())
		SceneCapImgBrush = MakeShared<FSlateMaterialBrush>(*Material, FVector2D(100, 100)); // Initialize member variable

	CorrScenarioHandler = NewObject<UCorrScenarioManagerHandler>();
	CorrScenarioHandler->Initialize(FirstRoom, SecondRoom, SpawnedRooms);
	
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
			.Image(SceneCapImgBrush.Get())
		]

		// Vertical Box Slot for Buttons and TextBoxes
		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .FillWidth(1.0)
		[
			SNew(SVerticalBox) // This Vertical Box will contain all the buttons and textboxes

			//NOTE: 1. Start all corridor scenarios 
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{ 
						ConstructTextBlock(CorrScenarioHandler->PropertyTextFont, FText::FromString("Spawn The Second Room at selected tile")),
						ConstructButton(FText::FromString("Spawn Room"), [this]()
						{
							return CorrScenarioHandler->HandleSecondRoomSpawning(true);
						}),
					})
			]

			//NOTE: 2. Undo all the spawned tiles and corridors
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(CorrScenarioHandler->PropertyTextFont, FText::FromString("Undo all the spawned tiles and corridors")),
						ConstructButton(FText::FromString("Undo tiles"), [this]()
						{
							CorrScenarioHandler->UndoTiles();
							return FReply::Handled();
						}),
					})
			]

			//NOTE: 3. Save the given path
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			                      .Padding(FMargin(15, 5, 15, 5))
			[
				HorizontalField(
					{
						ConstructTextBlock(CorrScenarioHandler->PropertyTextFont, FText::FromString("Directions"),nullptr,
							FText::FromString("First room's exit socket direction, Second room's enter direction (the one you'll spawn now )")),
						ConstructImage(FirstRoom->ExitSocketDirection, FirstImage, FText::FromString("First room's exit socket direction")),
						ConstructImage(SecondRoom->EnterSocketDirection, SecImage, FText::FromString("Second room's enter direction (the one you'll spawn now )")),
					})
			]
		]
	];
}

void SCorrScenarioManagerWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	CorrScenarioHandler->SpawnTiles();
	CorrScenarioHandler->HandleTileSelection(FirstRoom);
	CorrScenarioHandler->HandleSecondRoomSpawning();
	if (FMyInputProcessor::IsAltUp) CorrScenarioHandler->DoOnce = true;

}

SCorrScenarioManagerWidget::~SCorrScenarioManagerWidget()
{
	UE_LOG(LogTemp, Display, TEXT("sa"));
	CorrScenarioHandler->Destruct();
}

TSharedRef<SWidget> SCorrScenarioManagerWidget::ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, const FSlateColor* Color, const FText& ToolTipText)
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
	.ToolTipText(ToolTipText)
	[
		TextBlock
	];
}

TSharedRef<SButton> SCorrScenarioManagerWidget::ConstructButton(const FText& Text, const TFunction<FReply()>& ButtonClick)
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

TSharedRef<SWidget> SCorrScenarioManagerWidget::ConstructImage(const TEnumAsByte<Direction> Direction, TSharedPtr<FSlateBrush>& Image, const FText& Text)
{
	Image = CorrScenarioHandler->GetDirectionMaps(Direction, Image);

	TSharedRef<SImage> RefImage = SNew(SImage)
		.Image(Image.Get());

	TSharedRef<SButton> ImageBlock = SNew(SButton)
	.ButtonStyle(FCoreStyle::Get(), "NoBorder")
	.ToolTip(SNew(SToolTip).Text(Text))
	.ToolTipText(Text)
	.Content()
	[
		RefImage
	];
	
	return 	ImageBlock;
}

TSharedRef<SHorizontalBox> SCorrScenarioManagerWidget::HorizontalField(std::initializer_list<TSharedRef<SWidget>> Content)
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
