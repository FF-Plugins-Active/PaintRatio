// Copyright Epic Games, Inc. All Rights Reserved.

#include "PaintRatioBPLibrary.h"
#include "PaintRatio.h"

// UE Includes
#include "Kismet/KismetRenderingLibrary.h"
#include "Math/Color.h"

UPaintRatioBPLibrary::UPaintRatioBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UPaintRatioBPLibrary::GetPaintRatio(FPaintRatio DelegateRatio, UCanvasRenderTarget2D* InCRT, bool bNormalize, FLinearColor InWantedColor)
{
	if (IsValid(InCRT) == false)
	{
		return false;
	}

	TArray<FColor> Array_Colors;
	UKismetRenderingLibrary::ReadRenderTarget(GEngine->GetCurrentPlayWorld(), InCRT, Array_Colors, bNormalize);

	AsyncTask(ENamedThreads::GameThread, [DelegateRatio, Array_Colors, InCRT, InWantedColor]()
		{
			float Count_PaintedPixels = 0;
			for (int32 PixelIndex = 0; PixelIndex < Array_Colors.Num(); PixelIndex++)
			{
				// We need to delete alpha value because ReinterpretAsLinear() gives color without alpha.
				if (Array_Colors[PixelIndex].ReinterpretAsLinear().ToFColor(true).ToHex() == InWantedColor.ToFColor(true).WithAlpha(0).ToHex())
				{
					Count_PaintedPixels = Count_PaintedPixels + 1;
				}
			}

			float OutRatio = Count_PaintedPixels / ((float)InCRT->SizeX * (float)InCRT->SizeY);

			AsyncTask(ENamedThreads::GameThread, [DelegateRatio, OutRatio, InWantedColor]()
				{
					DelegateRatio.ExecuteIfBound(OutRatio);
				}
			);
		}
	);

	return true;
}