// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAutoBattler_init() {}
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");	AUTOBATTLER_API UFunction* Z_Construct_UDelegateFunction_AutoBattler_OnEnemyDied__DelegateSignature();
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_AutoBattler;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_AutoBattler()
	{
		if (!Z_Registration_Info_UPackage__Script_AutoBattler.OuterSingleton)
		{
		static UObject* (*const SingletonFuncArray[])() = {
			(UObject* (*)())Z_Construct_UDelegateFunction_AutoBattler_OnEnemyDied__DelegateSignature,
		};
		static const UECodeGen_Private::FPackageParams PackageParams = {
			"/Script/AutoBattler",
			SingletonFuncArray,
			UE_ARRAY_COUNT(SingletonFuncArray),
			PKG_CompiledIn | 0x00000000,
			0xBAA9DA15,
			0x5D7D4C79,
			METADATA_PARAMS(0, nullptr)
		};
		UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_AutoBattler.OuterSingleton, PackageParams);
	}
	return Z_Registration_Info_UPackage__Script_AutoBattler.OuterSingleton;
}
static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_AutoBattler(Z_Construct_UPackage__Script_AutoBattler, TEXT("/Script/AutoBattler"), Z_Registration_Info_UPackage__Script_AutoBattler, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xBAA9DA15, 0x5D7D4C79));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
