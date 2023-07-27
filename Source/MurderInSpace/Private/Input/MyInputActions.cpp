#include "Input/MyInputActions.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Input/MyInputActionSet.h"

#if WITH_EDITOR
void UMyInputActions::RefreshMyInputActions()
{
    Map.Reset();
    const auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    TArray<FAssetData> AssetData;
    AssetRegistryModule.GetAssetsByClass(UMyInputActionSet::StaticClass()->GetClassPathName(), AssetData);
    for(auto AssetDatum : AssetData)
    {
        auto* Asset = Cast<UMyInputActionSet>(AssetDatum.GetAsset());
        Map.Add(Asset->GetFName(), Asset);
    }
    GetPackage()->MarkPackageDirty();
}
#endif
