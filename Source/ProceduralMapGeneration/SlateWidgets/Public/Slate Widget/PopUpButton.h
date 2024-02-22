// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class PROCEDURALMAPGENERATION_API SPopUpButton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPopUpButton)
		{
		}

	SLATE_END_ARGS()

	TSharedPtr<FButtonStyle> ButtonStyle;
	
	TSharedRef<STextBlock> ConstructTextBlock(FSlateFontInfo FontInfo);
	TSharedPtr<STextBlock> TextBlock;
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void OnHovered();
	void OnUnHovered();
	void OnPressed();
};
