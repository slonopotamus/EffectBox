#pragma once

#include "Components/ContentWidget.h"

#include "EffectBox.generated.h"

class SEffectBox;

UCLASS(meta = (DisableNativeTick))
class EFFECTBOX_API UEffectBox : public UContentWidget
{
	GENERATED_BODY()

	UEffectBox();

	TSharedPtr<SEffectBox> MyEffectBox;

	/**
	 * The texture sampler parameter of the @EffectMaterial, that we'll set to the render target.
	 */
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName TextureParameter;

	/**
	 * The effect to apply to the render target.  We will set the texture sampler based on the name set in the @TextureParameter property.
	 *
	 * If you want to adjust transparency of the final image, make sure you set Blend Mode to @BLEND_AlphaComposite (Pre-Multiplied Alpha)
	 * and make sure to multiply the alpha you apply across the surface to the color and the alpha of the render target, otherwise
	 * you won't see the expected color.
	 */
	UPROPERTY(EditAnywhere, Category = "Effect")
	UMaterialInterface* EffectMaterial = nullptr;

	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

public:
	UFUNCTION(BlueprintCallable)
	UMaterialInstanceDynamic* GetEffectMaterial() const;
};
