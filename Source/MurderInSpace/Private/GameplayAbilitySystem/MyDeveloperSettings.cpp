#include "GameplayAbilitySystem/MyDeveloperSettings.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Input/MyInputAction.h"

void UMyDeveloperSettings::AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData)
{
    if(!Table->FindRowUnchecked(RowName))
    {
        Table->AddRow(RowName, RowData);
    }
}
