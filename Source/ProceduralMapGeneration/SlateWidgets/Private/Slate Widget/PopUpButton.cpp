// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PopUpButton.h"

#include "SlateOptMacros.h"
#include "Brushes/SlateColorBrush.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPopUpButton::Construct(const FArguments& InArgs)
{
	ButtonStyle = MakeShared<FButtonStyle>();
	ButtonStyle.Get()->SetNormal(FSlateColorBrush(FColor(0,0,0,0))); //Make everything 0 later on
	ButtonStyle.Get()->SetHovered(FSlateColorBrush(FColor(0,0,0,0))); //Make everything 0 later on
	ButtonStyle.Get()->SetPressed(FSlateColorBrush(FColor(0,0,0,0))); //Make everything 0 later on

	FSlateFontInfo FontInfo = FSlateFontInfo(FCoreStyle::Get().GetFontStyle(FName("EmbossedText")));
	FontInfo.TypefaceFontName = FName("Regular");
	FontInfo.Size = 12;
	
	ChildSlot
	[
		SNew(SVerticalBox)

		//1. 
		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SBox)
			
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.WidthOverride(230)
			.HeightOverride(50)
			.Padding(FMargin(0))
			[
				SNew(SButton)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				.ContentPadding(FMargin(0,0,0,0))
				.OnHovered(this,&SPopUpButton::OnHovered)
				.OnUnhovered(this, &SPopUpButton::OnUnHovered)
				.OnPressed(this,&SPopUpButton::OnPressed)
				.ButtonStyle(ButtonStyle.Get())
				[
					SNew(SBorder)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderBackgroundColor(FSlateColor(FColor(0,0,0,0.3)))
					[
						ConstructTextBlock(FontInfo)
					]
				]
				
			]
		]
	];
	
}

TSharedRef<STextBlock> SPopUpButton::ConstructTextBlock(FSlateFontInfo FontInfo)
{
	TextBlock = SNew(STextBlock)
       .Justification(ETextJustify::Center)
       .Text(FText::FromString("Start Test with selected room"))
       .Font(FontInfo)
       .RenderOpacity(0.5);

	return TextBlock.ToSharedRef();
}

void SPopUpButton::OnHovered()
{
	UE_LOG(LogTemp, Display, TEXT("Hovered"));
	TextBlock.Get()->SetRenderOpacity(1);
	// ButtonStyle.Get()->SetHovered(FSlateColorBrush(FColor(0,0,0,0.5)));

}

void SPopUpButton::OnUnHovered()
{
	UE_LOG(LogTemp, Display, TEXT("UnHovered"));
	TextBlock.Get()->SetRenderOpacity(0.5);
}

void SPopUpButton::OnPressed()
{
	TextBlock.Get()->SetRenderOpacity(1);

}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
