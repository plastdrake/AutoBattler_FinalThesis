// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "AutoBattlerGameMode.h"

#ifdef AUTOBATTLER_AutoBattlerGameMode_generated_h
#error "AutoBattlerGameMode.generated.h already included, missing '#pragma once' in AutoBattlerGameMode.h"
#endif
#define AUTOBATTLER_AutoBattlerGameMode_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AAutoBattlerGameMode *****************************************************
struct Z_Construct_UClass_AAutoBattlerGameMode_Statics;
AUTOBATTLER_API UClass* Z_Construct_UClass_AAutoBattlerGameMode_NoRegister();

#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h_15_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAAutoBattlerGameMode(); \
	friend struct ::Z_Construct_UClass_AAutoBattlerGameMode_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend AUTOBATTLER_API UClass* ::Z_Construct_UClass_AAutoBattlerGameMode_NoRegister(); \
public: \
	DECLARE_CLASS2(AAutoBattlerGameMode, AGameModeBase, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/AutoBattler"), Z_Construct_UClass_AAutoBattlerGameMode_NoRegister) \
	DECLARE_SERIALIZER(AAutoBattlerGameMode)


#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h_15_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AAutoBattlerGameMode(AAutoBattlerGameMode&&) = delete; \
	AAutoBattlerGameMode(const AAutoBattlerGameMode&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AAutoBattlerGameMode); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AAutoBattlerGameMode); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(AAutoBattlerGameMode) \
	NO_API virtual ~AAutoBattlerGameMode();


#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h_12_PROLOG
#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h_15_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h_15_INCLASS_NO_PURE_DECLS \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h_15_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AAutoBattlerGameMode;

// ********** End Class AAutoBattlerGameMode *******************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerGameMode_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
