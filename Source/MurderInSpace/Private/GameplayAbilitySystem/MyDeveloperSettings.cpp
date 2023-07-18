#include "GameplayAbilitySystem/MyDeveloperSettings.h"

void UMyDeveloperSettings::AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData)
{
    if(!Table->FindRowUnchecked(RowName))
    {
        Table->AddRow(RowName, RowData);
    }
}