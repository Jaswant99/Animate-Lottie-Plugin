/*************************************************************************
* ADOBE SYSTEMS INCORPORATED
* Copyright 2014 Adobe Systems Incorporated
* All Rights Reserved.

* NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the
* terms of the Adobe license agreement accompanying it.  If you have received this file from a
* source other than Adobe, then your use, modification, or distribution of it requires the prior
* written permission of Adobe.
**************************************************************************/


/*
 * PLUGIN DEVELOPERS MUST CHANGE VALUES OF ALL THE MACROS AND CONSTANTS IN THIS FILE 
 * IN ORDER TO AVOID ANY CLASH WITH OTHER PLUGINS.
 */


#ifndef _PLUGIN_CONFIGURATION_H_
#define _PLUGIN_CONFIGURATION_H_

#define PUBLISHER_NAME						"LottieExporterPlugin2"
#define PUBLISHER_UNIVERSAL_NAME			"com.adobe.LottieExporterPluginPublisher2"

/* The value of the PUBLISH_SETTINGS_UI_ID has to be the HTML extension ID used for Publish settings dialog*/
#define PUBLISH_SETTINGS_UI_ID				"com.adobe.LottieExporterPlugin.PublishSettings"

/* The value of RUNTIME_FOLDER_NAME must be the name of the runtime folder present in EclipseProject/ExtensionContent. */
#define RUNTIME_FOLDER_NAME                 "SampleRuntime"

namespace LottieExporter
{
	// {8CE65340-9FC7-4239-99B5-1AB3913D2057}
    const FCM::FCMCLSID CLSID_Publisher =
		{ 0x8ce65340, 0x9fc7, 0x4239,{ 0x99, 0xb5, 0x1a, 0xb3, 0x91, 0x3d, 0x20, 0x57 } };


	// {94F36F25-CB5C-44E6-BA30-350BD3678EA9}
    const FCM::FCMCLSID CLSID_ResourcePalette =
		{ 0x94f36f25, 0xcb5c, 0x44e6,{ 0xba, 0x30, 0x35, 0xb, 0xd3, 0x67, 0x8e, 0xa9 } };


	
	// {52DCF475-5763-43F8-B0EE-AD91F1B80144}
    const FCM::FCMCLSID CLSID_TimelineBuilder =
		{ 0x52dcf475, 0x5763, 0x43f8,{ 0xb0, 0xee, 0xad, 0x91, 0xf1, 0xb8, 0x1, 0x44 } };

	// {485463A1-7A67-4AEC-B5DB-7079CC90BE76}
    const FCM::FCMCLSID CLSID_TimelineBuilderFactory =
		{ 0x485463a1, 0x7a67, 0x4aec,{ 0xb5, 0xdb, 0x70, 0x79, 0xcc, 0x90, 0xbe, 0x76 } };

}


#endif // _PLUGIN_CONFIGURATION_H_
