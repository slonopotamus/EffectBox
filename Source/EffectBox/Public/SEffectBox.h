#pragma once

#if ENGINE_MAJOR_VERSION < 5
#include "StrongObjectPtr.h"
#endif

#include "Widgets/SCompoundWidget.h"

class FWidgetRenderer;
class SVirtualWindow;
class UTextureRenderTarget2D;

class EFFECTBOX_API SEffectBox final : public SCompoundWidget
{
	typedef SCompoundWidget Super;

	TSharedPtr<SVirtualWindow> VirtualWindow;
	TSharedPtr<SDPIScaler> DPIScaler;

	FWidgetRenderer* WidgetRenderer = nullptr;

	TStrongObjectPtr<UTextureRenderTarget2D> RenderTarget{nullptr};

	TStrongObjectPtr<UMaterialInstanceDynamic> DynamicEffect{nullptr};

	bool bIsDesignTime = false;

	virtual int32 OnPaint(
	    const FPaintArgs& Args,
	    const FGeometry& AllottedGeometry,
	    const FSlateRect& MyCullingRect,
	    FSlateWindowElementList& OutDrawElements,
	    int32 LayerId,
	    const FWidgetStyle& InWidgetStyle,
	    bool bParentEnabled) const override;

	virtual bool ComputeVolatility() const override;

	mutable FSlateBrush SurfaceBrush;

public:
	SLATE_BEGIN_ARGS(SEffectBox)
	{
		_Visibility = EVisibility::HitTestInvisible;
		_AllowFastUpdate = true;
		_IsDesignTime = false;
		_TextureParameter = NAME_None;
		_EffectMaterial = nullptr;
	}
	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_ARGUMENT(bool, IsDesignTime)
	SLATE_ARGUMENT(bool, AllowFastUpdate)
	SLATE_ARGUMENT(FName, TextureParameter)
	SLATE_ARGUMENT(UMaterialInterface*, EffectMaterial)
	SLATE_END_ARGS()

	SEffectBox();
	virtual ~SEffectBox() override;

	void Construct(const FArguments& Args);

	void SetContent(const TSharedRef<SWidget>& InContent);

	void SetEffectMaterial(UMaterialInterface* EffectMaterial, const FName& TextureParameter);

	UMaterialInstanceDynamic* GetEffectMaterial() const;

	void SetAllowFastUpdate(bool bAllowFastUpdate);
};
