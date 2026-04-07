// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "AutoBattlerGameMode.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeAutoBattlerGameMode() {}

// ********** Begin Cross Module References ********************************************************
AUTOBATTLER_API UClass* Z_Construct_UClass_AAutoBattlerGameMode();
AUTOBATTLER_API UClass* Z_Construct_UClass_AAutoBattlerGameMode_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AGameModeBase();
UPackage* Z_Construct_UPackage__Script_AutoBattler();
// ********** End Cross Module References **********************************************************

// ********** Begin Class AAutoBattlerGameMode *****************************************************
FClassRegistrationInfo Z_Registration_Info_UClass_AAutoBattlerGameMode;
UClass* AAutoBattlerGameMode::GetPrivateStaticClass()
{
	using TClass = AAutoBattlerGameMode;
	if (!Z_Registration_Info_UClass_AAutoBattlerGameMode.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("AutoBattlerGameMode"),
			Z_Registration_Info_UClass_AAutoBattlerGameMode.InnerSingleton,
			StaticRegisterNativesAAutoBattlerGameMode,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_AAutoBattlerGameMode.InnerSingleton;
}
UClass* Z_Construct_UClass_AAutoBattlerGameMode_NoRegister()
{
	return AAutoBattlerGameMode::GetPrivateStaticClass();
}
struct Z_Construct_UClass_AAutoBattlerGameMode_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n *  Simple GameMode for a third person game\n */" },
#endif
		{ "HideCategories", "Info Rendering MovementReplication Replication Actor Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "AutoBattlerGameMode.h" },
		{ "ModuleRelativePath", "AutoBattlerGameMode.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Simple GameMode for a third person game" },
#endif
	};
#endif // WITH_METADATA

// ********** Begin Class AAutoBattlerGameMode constinit property declarations *********************
// ********** End Class AAutoBattlerGameMode constinit property declarations ***********************
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AAutoBattlerGameMode>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_AAutoBattlerGameMode_Statics
UObject* (*const Z_Construct_UClass_AAutoBattlerGameMode_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AGameModeBase,
	(UObject* (*)())Z_Construct_UPackage__Script_AutoBattler,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAutoBattlerGameMode_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AAutoBattlerGameMode_Statics::ClassParams = {
	&AAutoBattlerGameMode::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x008002ADu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AAutoBattlerGameMode_Statics::Class_MetaDataParams), Z_Construct_UClass_AAutoBattlerGameMode_Statics::Class_MetaDataParams)
};
void AAutoBattlerGameMode::StaticRegisterNativesAAutoBattlerGameMode()
{
}
UClass* Z_Construct_UClass_AAutoBattlerGameMode()
{
	if (!Z_Registration_Info_UClass_AAutoBattlerGameMode.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AAutoBattlerGameMode.OuterSingleton, Z_Construct_UClass_AAutoBattlerGameMode_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AAutoBattlerGameMode.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, AAutoBattlerGameMode);
AAutoBattlerGameMode::~AAutoBattlerGameMode() {}
// ********** End Class AAutoBattlerGameMode *******************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h__Script_AutoBattler_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AAutoBattlerGameMode, AAutoBattlerGameMode::StaticClass, TEXT("AAutoBattlerGameMode"), &Z_Registration_Info_UClass_AAutoBattlerGameMode, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AAutoBattlerGameMode), 1688546729U) },
	};
}; // Z_CompiledInDeferFile_FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h__Script_AutoBattler_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h__Script_AutoBattler_2571325226{
	TEXT("/Script/AutoBattler"),
	Z_CompiledInDeferFile_FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h__Script_AutoBattler_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h__Script_AutoBattler_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
