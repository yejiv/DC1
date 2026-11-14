#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDWarningRow.generated.h"

class UTextBlock;
class UBorder;

UCLASS()
class DC1_API UDC1HUDWarningRow : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void	SetWarning(const FText& InText, const FLinearColor& InColor);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Warning = { nullptr };


	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UBorder>		WarningImageBox = { nullptr };
};
