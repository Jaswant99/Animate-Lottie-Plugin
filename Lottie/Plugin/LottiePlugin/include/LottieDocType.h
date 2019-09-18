/*************************************************************************
* ADOBE CONFIDENTIAL
* ___________________
*
*  Copyright 2018 Adobe Systems Incorporated
*  All Rights Reserved.
*
* NOTICE:  All information contained herein is, and remains
* the property of Adobe Systems Incorporated and its suppliers,
* if any.  The intellectual and technical concepts contained
* herein are proprietary to Adobe Systems Incorporated and its
* suppliers and are protected by all applicable intellectual property
* laws, including trade secret and copyright laws.
* Dissemination of this information or reproduction of this material
* is strictly forbidden unless prior written permission is obtained
* from Adobe Systems Incorporated.
**************************************************************************/

/*
* @file  LottieDocType.h
*
* @brief This file contains declarations for a DocType plugin.
*/


#ifndef Lottie_DOC_TYPE_H_
#define Lottie_DOC_TYPE_H_

#include <map>


#include "Version.h"
#include "FCMTypes.h"
#include "FCMPluginInterface.h"
//#include "ApplicationFCMPublicIDs.h"
#include "DocType/IDocType.h"
#include "DocType/IFeatureMatrix.h"
#include <string>
#include "PluginConfiguration.h"
#include "LottieFeatureMatrix.h"


/* -------------------------------------------------- Forward Decl */

using namespace FCM;
using namespace DocType;

#define Lottie_DOCTYPE_NAME                          "Lottie(Beta)"
#define Lottie_DOCTYPE_UNIVERSAL_NAME				"com.adobe.LottieDocType"
#define Lottie_DOCTYPE_DESCRIPTION					"This document can be used to author content"

namespace LottieExporter
{
	class CDocType;
	class FeatureMatrix;
	class Value;
	class Property;
	class Feature;
	class FeatureDocumentHandler;
}

/* -------------------------------------------------- Class Decl */

namespace LottieExporter
{
	// {208A6287-27F1-4384-B651-039C96589BD8}
	const FCM::FCMCLSID CLSID_LottieDocType =
		
	{ 0x208a6287, 0x27f1, 0x4384,{ 0xb6, 0x51, 0x3, 0x9c, 0x96, 0x58, 0x9b, 0xd8 } };



	class CLottieDocType : public DocType::IDocType, public FCM::FCMObjectBase
	{
		BEGIN_INTERFACE_MAP(CLottieDocType, Lottie_PLUGIN_VERSION)
			INTERFACE_ENTRY(IDocType)
		END_INTERFACE_MAP

	public:

		virtual FCM::Result _FCMCALL GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

		CLottieDocType();

		~CLottieDocType();

	private:

		DocType::PIFeatureMatrix m_fm;
	};

	FCM::Result RegisterLottieDocType(FCM::PIFCMDictionary pPlugins, const std::string& resPath);
};


#endif // Lottie_DOC_TYPEH_

#pragma once
