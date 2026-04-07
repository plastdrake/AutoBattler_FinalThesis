// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "AutoBattlerCharacter.h"

#ifdef AUTOBATTLER_AutoBattlerCharacter_generated_h
#error "AutoBattlerCharacter.generated.h already included, missing '#pragma once' in AutoBattlerCharacter.h"
#endif
#define AUTOBATTLER_AutoBattlerCharacter_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AAutoBattlerCharacter ****************************************************
#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execDoJumpEnd); \
	DECLARE_FUNCTION(execDoJumpStart); \
	DECLARE_FUNCTION(execDoLook); \
	DECLARE_FUNCTION(execDoMove);


struct Z_Construct_UClass_AAutoBattlerCharacter_Statics;
AUTOBATTLER_API UClass* Z_Construct_UClass_AAutoBattlerCharacter_NoRegister();

#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAAutoBattlerCharacter(); \
	friend struct ::Z_Construct_UClass_AAutoBattlerCharacter_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend AUTOBATTLER_API UClass* ::Z_Construct_UClass_AAutoBattlerCharacter_NoRegister(); \
public: \
	DECLARE_CLASS2(AAutoBattlerCharacter, ACharacter, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/AutoBattler"), Z_Construct_UClass_AAutoBattlerCharacter_NoRegister) \
	DECLARE_SERIALIZER(AAutoBattlerCharacter)


#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AAutoBattlerCharacter(AAutoBattlerCharacter&&) = delete; \
	AAutoBattlerCharacter(const AAutoBattlerCharacter&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AAutoBattlerCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AAutoBattlerCharacter); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(AAutoBattlerCharacter) \
	NO_API virtual ~AAutoBattlerCharacter();


#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_21_PROLOG
#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_INCLASS_NO_PURE_DECLS \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h_24_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AAutoBattlerCharacter;

// ********** End Class AAutoBattlerCharacter ******************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerCharacter_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
