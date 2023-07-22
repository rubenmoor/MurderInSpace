#include "GameplayAbilitySystem/MyDeveloperSettings.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Input/MyInputAction.h"

#if WITH_EDITOR
UMyDeveloperSettings::UMyDeveloperSettings()
{
    auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    AssetRegistryModule.OnAssetAdded().AddLambda([this] (const FAssetData& AssetData)
    {
        if(AssetData.IsInstanceOf(UMyInputAction::StaticClass(), EResolveClass::Yes))
        {
            MyInputActions.Add(AssetData.GetAsset());
        }
    });
    AssetRegistryModule.OnAssetRemoved().AddLambda([this] (const FAssetData& AssetData)
    {
        
        if(AssetData.IsInstanceOf(UMyInputAction::StaticClass()))
        {
            MyInputActions.Remove(AssetData.GetAsset());
        }
    });
}

void UMyDeveloperSettings::ResetMyInputActions()
{
    MyInputActions.Reset();
    const auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    TArray<FAssetData> AssetData;
    AssetRegistryModule.GetAssetsByClass(UMyInputAction::StaticClass()->GetClassPathName(), AssetData);
    for(auto AssetDatum : AssetData)
    {
        MyInputActions.Add(AssetDatum.GetAsset());
    }
}
#endif

void UMyDeveloperSettings::AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData)
{
    if(!Table->FindRowUnchecked(RowName))
    {
        Table->AddRow(RowName, RowData);
    }
}
