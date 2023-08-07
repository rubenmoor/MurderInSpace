#include "GameplayAbilitySystem/MyAttributeSetBase.h"

#include "GameplayAbilitySystem/MyDeveloperSettings.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"

TArray<FMyAttributeRow> UMyAttributeSetBase::GetAttributeInitialValueRows()
{
    return {};
}

void UMyAttributeSetBase::PostInitProperties()
{
    Super::PostInitProperties();

    auto* Settings = GetDefault<UMyDeveloperSettings>();
    if(Settings->InitialAttributeValues.IsNull())
    {
        UE_LOGFMT(LogMyGame, Error, "{0}: Data Table initial attribute values: not set; can't load defaults", GetFName());
    }
    else
    {
        auto* Table = Settings->InitialAttributeValues.LoadSynchronous();
        check(Table->RowStruct == FMyAttributeInitializationData::StaticStruct())

        // create rows in the table for initial attribute values to allow editing of the values in the editor
    	// or leave the existing rows as they are
        auto Rows = GetAttributeInitialValueRows();
        for(auto [Name, Data] : Rows)
        {
            UMyDeveloperSettings::AddRowUnlessExists(Table, Name, Data);
        }

    	// pass through all attributes in the set to initialize
		for( TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			FProperty* Property = *It;
			check(FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
			
			FString RowNameStr = FString::Printf(TEXT("%s.%s"), *Property->GetOwnerVariant().GetName(), *Property->GetName());

			const auto* InitializationData =
				Table->FindRow<FMyAttributeInitializationData>
					(*RowNameStr
					, "UMyAttributeSetBase: Initializing attribute"
					, false
					);
			if (InitializationData)
			{
				const auto* StructProperty = CastField<FStructProperty>(Property);
				check(StructProperty)
				auto* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
				check(DataPtr)
				DataPtr->SetBaseValue(InitializationData->Value);
				DataPtr->SetCurrentValue(InitializationData->Value);
			}
			else
			{
				UE_LOGFMT(LogMyGame, Error, "{0}: missing attribute initial value for {1}", GetFName(), RowNameStr);
			}
		}
    }
}
