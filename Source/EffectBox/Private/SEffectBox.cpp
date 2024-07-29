#include "SEffectBox.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/SWidgetUtils.h"

static const TAutoConsoleVariable<bool> CVarEnableEffectBox(TEXT("Slate.EnableEffectBox"), true, TEXT("Controls whether EffectBox renders to render target"));

SEffectBox::SEffectBox()
{
	SetCanTick(false);
}

SEffectBox::~SEffectBox()
{
	if (VirtualWindow)
	{
		FSlateApplication::Get().UnregisterVirtualWindow(VirtualWindow.ToSharedRef());
	}

	if (WidgetRenderer)
	{
		BeginCleanup(WidgetRenderer);
	}
}

void SEffectBox::Construct(const FArguments& Args)
{
	bIsDesignTime = Args._IsDesignTime;

	if (CVarEnableEffectBox.GetValueOnGameThread())
	{
		WidgetRenderer = new FWidgetRenderer(true);
		if (auto* RT = WidgetRenderer->CreateTargetFor(FVector2D::UnitVector, TF_Bilinear, true))
		{
			RenderTarget.Reset(RT);
			SurfaceBrush.SetResourceObject(RT);
		}

		if (!bIsDesignTime)
		{
			DPIScaler = SNew(SDPIScaler);
			VirtualWindow = SNew(SVirtualWindow).Visibility(EVisibility::HitTestInvisible);
			VirtualWindow->SetAllowFastUpdate(Args._AllowFastUpdate);
			VirtualWindow->SetContent(DPIScaler.ToSharedRef());
			FSlateApplication::Get().RegisterVirtualWindow(VirtualWindow.ToSharedRef());
		}
	}

	SetEffectMaterial(Args._EffectMaterial, Args._TextureParameter);
	SetContent(Args._Content.Widget);
}

void SEffectBox::SetContent(const TSharedRef<SWidget>& InContent)
{
	if (DPIScaler)
	{
		DPIScaler->SetContent(InContent);
	}
	else
	{
		ChildSlot.AttachWidget(InContent);
	}
}

void SEffectBox::SetEffectMaterial(UMaterialInterface* EffectMaterial, const FName& TextureParameter)
{
	if (EffectMaterial)
	{
		DynamicEffect.Reset(Cast<UMaterialInstanceDynamic>(EffectMaterial));
		if (!DynamicEffect)
		{
			DynamicEffect.Reset(UMaterialInstanceDynamic::Create(EffectMaterial, GetTransientPackage()));
		}
	}
	else
	{
		DynamicEffect = nullptr;
	}

	SurfaceBrush.SetResourceObject(DynamicEffect.Get());

	if (DynamicEffect)
	{
		DynamicEffect->SetTextureParameterValue(TextureParameter, RenderTarget.Get());
	}
}

UMaterialInstanceDynamic* SEffectBox::GetEffectMaterial() const
{
	return DynamicEffect.Get();
}

int32 SEffectBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	SCOPE_CYCLE_SWIDGET(this);

	if (!RenderTarget)
	{
		return Super::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

	const auto RenderSize = AllottedGeometry.GetAbsoluteSize();
	int32 RenderTargetWidth = FMath::RoundToInt(FMath::Abs(RenderSize.X));
	int32 RenderTargetHeight = FMath::RoundToInt(FMath::Abs(RenderSize.Y));

	static const auto* CVarMaxWidth = IConsoleManager::Get().FindConsoleVariable(TEXT("WidgetComponent.MaximumRenderTargetWidth"));
	if (ensure(CVarMaxWidth))
	{
		RenderTargetWidth = FMath::Min(RenderTargetWidth, CVarMaxWidth->GetInt());
	}
	RenderTargetWidth = FMath::Clamp<int32>(RenderTargetWidth, 1, GetMax2DTextureDimension());

	static const auto* CVarMaxHeight = IConsoleManager::Get().FindConsoleVariable(TEXT("WidgetComponent.MaximumRenderTargetHeight"));
	if (ensure(CVarMaxHeight))
	{
		RenderTargetHeight = FMath::Min(RenderTargetHeight, CVarMaxHeight->GetInt());
	}
	RenderTargetHeight = FMath::Clamp<int32>(RenderTargetHeight, 1, GetMax2DTextureDimension());

	RenderTarget->ResizeTarget(RenderTargetWidth, RenderTargetHeight);

	SurfaceBrush.ImageSize = FVector2D(RenderTargetWidth, RenderTargetHeight);

	if (VirtualWindow)
	{
		DPIScaler->SetDPIScale(GetPrepassLayoutScaleMultiplier());
		VirtualWindow->Resize(SurfaceBrush.ImageSize);

		if (AllottedGeometry.AbsolutePosition != VirtualWindow->GetPositionInScreen())
		{
			VirtualWindow->SetCachedScreenPosition(AllottedGeometry.AbsolutePosition);
		}

		constexpr float WindowScale = 1;

		const FPaintArgs PaintArgs = Args.WithNewHitTestGrid(VirtualWindow->GetHittestGrid()).WithNewParent(nullptr);

		FSlateInvalidationContext Context(OutDrawElements, InWidgetStyle);
		Context.bParentEnabled = bParentEnabled;
		Context.bAllowFastPathUpdate = VirtualWindow->Advanced_IsInvalidationRoot();
		Context.LayoutScaleMultiplier = WindowScale;
		Context.PaintArgs = &PaintArgs;
		Context.IncomingLayerId = LayerId;

#if ENGINE_MAJOR_VERSION < 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4)
		VirtualWindow->ProcessWindowInvalidation();
		VirtualWindow->SlatePrepass(Context.LayoutScaleMultiplier);
#endif

		auto WindowRef = VirtualWindow.ToSharedRef();

		static const auto* CVarDeferRendering = IConsoleManager::Get().FindConsoleVariable(TEXT("Slate.DeferRetainedRenderingRenderThread"));
		WidgetRenderer->DrawInvalidationRoot(WindowRef, RenderTarget.Get(), *VirtualWindow, Context, ensure(CVarDeferRendering) && CVarDeferRendering->GetBool());
	}
	else
	{
		// This is a workaround to make outlines show properly during design time
		WidgetRenderer->DrawWidget(RenderTarget->GameThread_GetRenderTargetResource(), ChildSlot.GetWidget(), GetPrepassLayoutScaleMultiplier(), SurfaceBrush.ImageSize, Args.GetDeltaTime());
	}

	const FLinearColor ComputedColorAndOpacity(InWidgetStyle.GetColorAndOpacityTint() * GetColorAndOpacity() * SurfaceBrush.GetTint(InWidgetStyle));
	// Retainer widget uses pre-multiplied alpha, so pre-multiply the color by the alpha to respect opacity.
	const FLinearColor PremultipliedColorAndOpacity(ComputedColorAndOpacity * ComputedColorAndOpacity.A);

	FSlateDrawElement::MakeBox(
	    OutDrawElements,
	    LayerId,
	    AllottedGeometry.ToPaintGeometry(),
	    &SurfaceBrush,
	    // We always write out the content in gamma space, so when we render the final version we need to
	    // render without gamma correction enabled.
	    ESlateDrawEffect::PreMultipliedAlpha | ESlateDrawEffect::NoGamma,
	    FLinearColor(PremultipliedColorAndOpacity.R, PremultipliedColorAndOpacity.G, PremultipliedColorAndOpacity.B, PremultipliedColorAndOpacity.A));

	return LayerId;
}

bool SEffectBox::ComputeVolatility() const
{
	return RenderTarget.IsValid();
}
