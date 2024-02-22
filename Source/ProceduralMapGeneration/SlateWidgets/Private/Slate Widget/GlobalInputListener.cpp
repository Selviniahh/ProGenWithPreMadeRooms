#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/GlobalInputListener.h"

//Static stuff initialization
TMap<FKey, bool> FMyInputProcessor::KeyPressedMap;
bool FMyInputProcessor::IsAltUp = true;
float FMyInputProcessor::MovementSpeed = 500.0f;

bool FMyInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();
	
	PressedKey = Key;

	if (KeyPressedMap.Contains(Key))
	{
		KeyPressedMap[InKeyEvent.GetKey()] = true;
	}
	
	// Return false to allow the event to continue through the Slate event pipeline
	return IInputProcessor::HandleKeyDownEvent(SlateApp, InKeyEvent);
}

bool FMyInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	PressedMouseKey = MouseEvent.GetEffectingButton();
	return IInputProcessor::HandleMouseMoveEvent(SlateApp, MouseEvent);
}

auto FMyInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) -> void
{
}

bool FMyInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
	float ScrollData = InWheelEvent.GetWheelDelta(); //-1 or 1 

	SpeedAdjustFactor = (MovementSpeed < 101) ? 20.f : 100.f;
	
	if (MovementSpeed == 100.f && ScrollData > 0)
	{
		SpeedAdjustFactor = 100.f;
	}
	MovementSpeed += ScrollData * SpeedAdjustFactor;
	
	//Clamp the speed
	MovementSpeed = FMath::Clamp(MovementSpeed,20,1000);
	
	// UE_LOG(LogTemp, Log, TEXT("Scrolled: %f"), MovementSpeed);
	return IInputProcessor::HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGestureEvent);
}

bool FMyInputProcessor::IsKeyPressed(const FKey& Key)
{
	return KeyPressedMap.Contains(Key) && KeyPressedMap[Key];
}

void FMyInputProcessor::FillMap()
{
	KeyPressedMap.Add(EKeys::W, false);
	KeyPressedMap.Add(EKeys::A, false);
	KeyPressedMap.Add(EKeys::S, false);
	KeyPressedMap.Add(EKeys::D, false);
	KeyPressedMap.Add(EKeys::Q, false);
	KeyPressedMap.Add(EKeys::E, false);
	KeyPressedMap.Add(EKeys::LeftShift, false);
}

bool FMyInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (KeyPressedMap.Contains(InKeyEvent.GetKey()))
	{
		KeyPressedMap[InKeyEvent.GetKey()] = false;
	}

	IsAltUp = (InKeyEvent.GetKey() == EKeys::LeftAlt);
	
	return IInputProcessor::HandleKeyUpEvent(SlateApp, InKeyEvent);
}
