#include "LottieDocType.h"
#include "LottiePublisher.h"
#include "Utils.h"
#include "LottieExporterPlugin.h"
#include "FCMTypes.h"
#include<vector>

namespace LottieExporter
{
	BEGIN_MODULE(LottieExporterModule)

		BEGIN_CLASS_ENTRY

		CLASS_ENTRY(CLSID_LottieDocType, CLottieDocType)
		
		CLASS_ENTRY(CLSID_FeatureMatrix, FeatureMatrix)
		CLASS_ENTRY(CLSID_Publisher, CPublisher)
		CLASS_ENTRY(CLSID_ResourcePalette, ResourcePalette)
		CLASS_ENTRY(CLSID_TimelineBuilder, TimelineBuilder)
		CLASS_ENTRY(CLSID_TimelineBuilderFactory, TimelineBuilderFactory)

		END_CLASS_ENTRY

public:
	void SetResPath(const std::string& resPath) { m_resPath = resPath; }
	const std::string& GetResPath() { return m_resPath; }

private:
	std::string m_resPath;

	END_MODULE


		LottieExporterModule g_LottieExporterModule;

	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginBoot(FCM::PIFCMCallback pCallback)
	{
		FCM::Result res;
		std::string langCode;
		std::string modulePath;

		res = g_LottieExporterModule.init(pCallback);

		Utils::GetModuleFilePath(modulePath, pCallback);
		Utils::GetLanguageCode(pCallback, langCode);

		g_LottieExporterModule.SetResPath(modulePath + "../res/" + langCode + "/");
		return res;
	}

	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassInfo(
		FCM::PIFCMCalloc pCalloc,
		FCM::PFCMClassInterfaceInfo* ppClassInfo)
	{
		return g_LottieExporterModule.getClassInfo(pCalloc, ppClassInfo);
	}

	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassObject(
		FCM::PIFCMUnknown pUnkOuter,
		FCM::ConstRefFCMCLSID clsid,
		FCM::ConstRefFCMIID iid,
		FCM::PPVoid pAny)
	{
		return g_LottieExporterModule.getClassObject(pUnkOuter, clsid, iid, pAny);
	}

	// Register the plugin - Register plugin as both DocType and Publisher
	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginRegister(FCM::PIFCMPluginDictionary pPluginDict)
	{
		FCM::Result res = FCM_SUCCESS;

		AutoPtr<IFCMDictionary> pDictionary = pPluginDict;

		AutoPtr<IFCMDictionary> pPlugins;
		pDictionary->AddLevel((const FCM::StringRep8)kFCMComponent, pPlugins.m_Ptr);

		res = RegisterLottieDocType(pPlugins, g_LottieExporterModule.GetResPath());
		if (FCM_FAILURE_CODE(res))
		{
			return res;
		}

		
		//std::vector<FCM::FCMCLSID> docIds;
		//docIds.push_back(CLSID_LottieDocType);
		
		res = RegisterPublisher(pPlugins, CLSID_LottieDocType);

		return res;
	}

	extern "C" FCMPLUGIN_IMP_EXP FCM::U_Int32 PluginCanUnloadNow(void)
	{
		return g_LottieExporterModule.canUnloadNow();
	}

	extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginShutdown()
	{
		g_LottieExporterModule.finalize();

		return FCM_SUCCESS;
	}

};
