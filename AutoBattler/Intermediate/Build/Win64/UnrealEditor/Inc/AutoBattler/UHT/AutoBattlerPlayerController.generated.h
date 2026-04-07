// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "AutoBattlerPlayerController.h"

#ifdef AUTOBATTLER_AutoBattlerPlayerController_generated_h
#error "AutoBattlerPlayerController.generated.h already included, missing '#pragma once' in AutoBattlerPlayerController.h"
#endif
#define AUTOBATTLER_AutoBattlerPlayerController_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AAutoBattlerPlayerController *********************************************
struct Z_Construct_UClass_AAutoBattlerPlayerController_Statics;
AUTOBATTLER_API UClass* Z_Construct_UClass_AAutoBattlerPlayerController_NoRegister();

#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h_19_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAAutoBattlerPlayerController(); \
	friend struct ::Z_Construct_UClass_AAutoBattlerPlayerController_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend AUTOBATTLER_API UClass* ::Z_Construct_UClass_AAutoBattlerPlayerController_NoRegister(); \
public: \
	DECLARE_CLASS2(AAutoBattlerPlayerController, APlayerController, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/AutoBattler"), Z_Construct_UClass_AAutoBattlerPlayerController_NoRegister) \
	DECLARE_SERIALIZER(AAutoBattlerPlayerController)


#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h_19_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API AAutoBattlerPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	AAutoBattlerPlayerController(AAutoBattlerPlayerController&&) = delete; \
	AAutoBattlerPlayerController(const AAutoBattlerPlayerController&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AAutoBattlerPlayerController); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AAutoBattlerPlayerController); \
	DEFINE_ABSTRACT_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(AAutoBattlerPlayerController) \
	NO_API virtual ~AAutoBattlerPlayerController();


#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h_16_PROLOG
#define FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h_19_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h_19_INCLASS_NO_PURE_DECLS \
	FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h_19_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AAutoBattlerPlayerController;

// ********** End Class AAutoBattlerPlayerController ***********************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_School_AutoBattler_FinalThesis_AutoBattler_Source_AutoBattler_AutoBattlerPlayerController_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
