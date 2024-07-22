#include "EffectBox.h"

#include "SEffectBox.h"

UEffectBox::UEffectBox()
{
	Visibility = ESlateVisibility::HitTestInvisible;
}

TSharedRef<SWidget> UEffectBox::RebuildWidget()
{
	MyEffectBox = SNew(SEffectBox)
	                  .IsDesignTime(IsDesignTime())
	                      [GetContentSlot() && GetContentSlot()->Content
	                           ? GetContentSlot()->Content->TakeWidget()
	                           : SNullWidget::NullWidget];

	return MyEffectBox.ToSharedRef();
}

void UEffectBox::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyEffectBox.Reset();
}

#if WITH_EDITOR
const FText UEffectBox::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "SpecialFX", "Special Effects");
}
#endif

UMaterialInstanceDynamic* UEffectBox::GetEffectMaterial() const
{
	if (MyEffectBox)
	{
		return MyEffectBox->GetEffectMaterial();
	}

	return nullptr;
}

void UEffectBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!MyEffectBox)
	{
		return;
	}

	MyEffectBox->SetEffectMaterial(EffectMaterial, TextureParameter);
}

void UEffectBox::OnSlotAdded(UPanelSlot* InSlot)
{
	if (MyEffectBox)
	{
		MyEffectBox->SetContent(InSlot->Content ? InSlot->Content->TakeWidget() : SNullWidget::NullWidget);
	}
}

void UEffectBox::OnSlotRemoved(UPanelSlot* InSlot)
{
	if (MyEffectBox.IsValid())
	{
		MyEffectBox->SetContent(SNullWidget::NullWidget);
	}
}
