#include "PieceStats.h"
#include "Components/TextBlock.h"

void UPieceStats::SetHealthValue(int32 Health)
{
	HealthText->SetText(FText::AsNumber(Health));
}

void UPieceStats::SetAttackValue(int32 Attack)
{
	AttackText->SetText(FText::AsNumber(Attack));
}
