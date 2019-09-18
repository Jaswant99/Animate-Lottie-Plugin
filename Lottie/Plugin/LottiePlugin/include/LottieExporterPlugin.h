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

#ifndef _LottieExporterPlugin_Header_
#define _LottieExporterPlugin_Header_

#include "FCMPluginInterface.h"

namespace LottieExporter
{
	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassInfo(FCM::PIFCMCalloc pCalloc, FCM::PFCMClassInterfaceInfo* ppClassInfo);
	extern "C" FCMPLUGIN_IMP_EXP FCM::Result  PluginGetClassObject(FCM::PIFCMUnknown pUnkOuter, FCM::ConstRefFCMCLSID clsid, FCM::ConstRefFCMIID iid, FCM::PPVoid pAny);
	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginBoot(FCM::PIFCMCallback pCallback);
	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginRegister(FCM::PIFCMPluginDictionary pPluginDict);
	extern "C" FCMPLUGIN_IMP_EXP FCM::U_Int32	PluginCanUnloadNow(void);
	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginShutdown();

};
#endif //_LottieExporterPlugin_Header_
#pragma once
