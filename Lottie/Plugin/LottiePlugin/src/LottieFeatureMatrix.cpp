#include "LottieFeatureMatrix.h"
#include "LottieDocType.h"
#include "Utils.h"
#include <fstream>
#include <sstream>

#include "Application/Service/IOutputConsoleService.h"
#include "PluginConfiguration.h"
#include "FlashFCMPublicIDs.h"

namespace LottieExporter
{

    /* -------------------------------------------------- Constants */

    static const std::string kElement_Features("Features");
    static const std::string kElement_Feature("Feature");
    static const std::string kElement_Property("Property");
    static const std::string kElement_Value("Value");
    static const std::string kAttribute_name("name");
    static const std::string kAttribute_supported("supported");
    static const std::string kAttribute_default("default");
    static const std::string kValue_true("true");
    static const std::string kValue_false("false");


    /* -------------------------------------------------- FeatureMatrix */

    FeatureMatrix::FeatureMatrix()
    {
        m_bInited = false;
    }

    FeatureMatrix::~FeatureMatrix()
    {
    }

    void FeatureMatrix::Init(FCM::PIFCMCallback pCallback)
    {
        std::string featureXMLPath;

        if (m_bInited)
        {
            return;
        }

        Utils::GetModuleFilePath(featureXMLPath, pCallback);
        featureXMLPath = featureXMLPath.substr(0, featureXMLPath.length() - 1 );
		std::string commonCfgPath = "/Common/Configuration/Document Types/Lottie/";
       
            commonCfgPath += "Features.xml";
        
#ifdef _WINDOWS
		featureXMLPath += commonCfgPath;
#else
        std::string parentDirName;
        Utils::GetFileName(featureXMLPath, parentDirName);
        if(parentDirName == "debug" || parentDirName == "release")
        {
           
                featureXMLPath += "/../../../res/Features.xml";
           
        }
        else if(parentDirName == "Frameworks")
        {
            featureXMLPath += "/.." + commonCfgPath;
        }
#endif

        // trace
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::Result res = pCallback->GetService(Application::Service::FLASHAPP_OUTPUT_CONSOLE_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));
        
		std::fstream xmlFile;
		Utils::OpenFStream(featureXMLPath, xmlFile, std::ios_base::binary | std::ios_base::in, pCallback);

        char *buffer = NULL;
        long length = 0;

        if (xmlFile) 
        {
            xmlFile.seekg (0, xmlFile.end);
            length = (long)xmlFile.tellg();
            xmlFile.seekg (0, xmlFile.beg);
            buffer = new char [length + 1];
            xmlFile.read (buffer, length);
            buffer[length] = 0;
        }
        xmlFile.close();        
       
        try {
            m_bInited = true;
            UpdateFeatureMatrix();
        }
        catch (...) {
            ASSERT(0);
        }
        delete[] buffer;
    }

    void FeatureMatrix::UpdateFeatureMatrix()
    {
        //Explicitly disable VR feature and VR_Pano, VR_360 attributes
        const std::string kFeature_VR("VR");
        const std::string kProperty_Panoramic("Panoramic");
        const std::string kProperty_Spherical("Spherical");

        std::map<std::string, std::string> featureMap;
        featureMap[kAttribute_name] = kFeature_VR;
        featureMap[kAttribute_supported] = kValue_false;
        Feature* feature_VR = UpdateFeature(featureMap);
        if (feature_VR) {
            featureMap.clear();
            featureMap[kAttribute_name] = kProperty_Panoramic;
            featureMap[kAttribute_supported] = kValue_false;
            UpdateProperty(feature_VR, featureMap);
            featureMap.clear();
            featureMap[kAttribute_name] = kProperty_Spherical;
            featureMap[kAttribute_supported] = kValue_false;
            UpdateProperty(feature_VR, featureMap);
        }
    }

    FCM::Result FeatureMatrix::IsSupported(CStringRep16 inFeatureName, FCM::Boolean& isSupported)
    {
        std::string featureLC = Utils::ToString(inFeatureName, GetCallback());
        
        Feature* pFeature = FindFeature(featureLC);
        if (pFeature == NULL)
        {
            /* If a feature is not found, it is supported */
            isSupported = true;
        }
        else
        {
            isSupported =  pFeature->IsSupported();
        }
        return FCM_SUCCESS;
    }

    FCM::Result FeatureMatrix::IsSupported(
        CStringRep16 inFeatureName, 
        CStringRep16 inPropName, 
        FCM::Boolean& isSupported)
    {     
        std::string featureLC = Utils::ToString(inFeatureName, GetCallback());

        Feature* pFeature = FindFeature(featureLC);
        if (pFeature == NULL)
        {
            /* If a feature is not found, it is supported */
            isSupported =  true;
        }
        else
        {
            if (!pFeature->IsSupported())
            {
                /* If a feature is not supported, sub-features are not supported */
                isSupported = false;
            }
            else
            {
                // Look if sub-features are supported.
                std::string propertyLC = Utils::ToString(inPropName, GetCallback());
                
                Property* pProperty = pFeature->FindProperty(propertyLC);
                if (pProperty == NULL)
                {
                    /* If a property is not found, it is supported */
                    isSupported =  true;
                }
                else
                {
                    isSupported = pProperty->IsSupported();
                }
            }
        }
        return FCM_SUCCESS;
    }


    FCM::Result FeatureMatrix::IsSupported(
        CStringRep16 inFeatureName, 
        CStringRep16 inPropName, 
        CStringRep16 inValName, 
        FCM::Boolean& isSupported)
    {
        std::string featureLC = Utils::ToString(inFeatureName, GetCallback());

        Feature* pFeature = FindFeature(featureLC);
        if (pFeature == NULL)
        {
            /* If a feature is not found, it is supported */
            isSupported = true;
        }
        else
        {
            if (!pFeature->IsSupported())
            {
                /* If a feature is not supported, sub-features are not supported */
                isSupported = false;
            }
            else
            {
                std::string propertyLC(Utils::ToString(inPropName, GetCallback()));

                Property* pProperty = pFeature->FindProperty(propertyLC);
                if (pProperty == NULL)
                {
                    /* If a property is not found, it is supported */
                    isSupported = true;
                }
                else
                {
                    if (!pProperty->IsSupported())
                    {
                        /* If a property is not supported, all values are not supported */
                        isSupported = false;
                    }
                    else
                    {
                        std::string valueLC(Utils::ToString(inValName, GetCallback()));

                        Value* pValue = pProperty->FindValue(valueLC);
                        if (pValue == NULL)
                        {
                            /* If a value is not found, it is supported */
                            isSupported = true;
                        }
                        else
                        {
                            isSupported = pValue->IsSupported();
                        }
                    }

                }
            }

        }
        return FCM_SUCCESS;
    }


    FCM::Result FeatureMatrix::GetDefaultValue(CStringRep16 inFeatureName, 
            CStringRep16 inPropName,
            FCM::VARIANT& outDefVal)
    {
        // Any boolean value retuened as string should be "true" or "false"
        FCM::Result res = FCM_INVALID_PARAM;
        std::string featureName = Utils::ToString(inFeatureName, GetCallback());
        std::string propName = Utils::ToString(inPropName, GetCallback());
        
        Property* pProperty = NULL;
        Feature* pFeature = FindFeature(featureName);
        if (pFeature != NULL && pFeature->IsSupported())
        {
            pProperty = pFeature->FindProperty(propName);
            if (pProperty != NULL /*&& pProperty->IsSupported()*/)
            {
                std::string strVal = pProperty->GetDefault();
                std::istringstream iss(strVal);
                res = FCM_SUCCESS;
                switch (outDefVal.m_type) {
                    case kFCMVarype_UInt32: iss>>outDefVal.m_value.uVal;break;
                    case kFCMVarype_Float: iss>>outDefVal.m_value.fVal;break;
                    case kFCMVarype_Bool: outDefVal.m_value.bVal = (kValue_true == strVal); break;
                    case kFCMVarype_CString: outDefVal.m_value.strVal = Utils::ToString16(strVal, GetCallback()); break;
                    case kFCMVarype_Double: iss>>outDefVal.m_value.dVal;break;
                    default: 
                    ASSERT(0);
                    res = FCM_INVALID_PARAM;
                    break;
                }
            }
        }

        return res;
    }


    FCM::Result FeatureMatrix::StartElement(
        const std::string name,
        const std::map<std::string, std::string>& attrs)
    {
        std::string name8(name);

        if (kElement_Feature.compare(name8) == 0)
        {
            // Start of a feature tag
            mCurrentFeature = UpdateFeature(attrs);
            mCurrentProperty = NULL;
        }
        else if (kElement_Property.compare(name8) == 0)
        {
            // Start of a property tag
            mCurrentProperty = UpdateProperty(mCurrentFeature, attrs);
        }
        else if (kElement_Value.compare(name8) == 0)
        {
            // Start of a value tag
            UpdateValue(mCurrentProperty, attrs);
        }

        return FCM_SUCCESS;
    }

    FCM::Result FeatureMatrix::EndElement(const std::string name)
    {
        std::string name8(name);

        if (kElement_Feature.compare(name8) == 0)
        {
            // End of a feature tag
            mCurrentFeature = NULL;
            mCurrentProperty = NULL;
        }
        else if (kElement_Property.compare(name8) == 0)
        {
            // End of a property tag
            mCurrentProperty = NULL;
        }
        return FCM_SUCCESS;
    }

    Feature* FeatureMatrix::FindFeature(const std::string& inFeatureName)
    {
        StrFeatureMap::iterator itr = mFeatures.find(inFeatureName);
        if (itr != mFeatures.end())
        {
            return itr->second;
        }
        return NULL;
    }

    Feature* FeatureMatrix::UpdateFeature(const std::map<std::string, std::string>& inAttrs)
    {
        // name: mandatory attribute
        std::string name;
        std::map<std::string, std::string>::const_iterator itr = inAttrs.find(kAttribute_name);
        if ((itr == inAttrs.end()) || (itr->second.empty()))
        {
            return NULL;
        }
        else
        {
            name = itr->second;
        }

        // supported: optional attribute
        bool supported = true;
        itr = inAttrs.find(kAttribute_supported);
        if (itr != inAttrs.end())
        {
            supported = (itr->second == kValue_true);
        }
        
        // Find or Create new Feature

        Feature * pFeature = FindFeature(name);   
        if (pFeature == NULL)
        {
            pFeature = new Feature(supported);
            mFeatures.insert(std::pair<std::string, Feature*>(name, pFeature));
        }

        return pFeature;
    }


    Property* FeatureMatrix::UpdateProperty(
        Feature* inFeature, 
        const std::map<std::string,std::string>& inAttrs)
    {
        if (inFeature == NULL)
        {
            return NULL;
        }
        
        std::string name;
        
        // name: mandatory attribute
        std::map<std::string, std::string>::const_iterator itr = inAttrs.find(kAttribute_name);
        if ((itr == inAttrs.end()) || (itr->second.empty()))
        {
            return NULL;
        }
        else
        {
            name = itr->second;
        }

        // supported: optional attribute
        bool supported = true;
        itr = inAttrs.find(kAttribute_supported);
        if (itr != inAttrs.end())
        {
            supported = itr->second == kValue_true;
        }

        // default: optional attribute
        std::string def;
        itr = inAttrs.find(kAttribute_default);
        if ((itr != inAttrs.end()) && (itr->second.empty() == false))
        {
            def = itr->second;
        }

        // Find or Create new Property
        Property* pProperty = NULL;
        pProperty = inFeature->FindProperty(name);
        if (pProperty == NULL)
        {
            pProperty = new Property(def, supported);
            if (pProperty != NULL)
            {
                inFeature->AddProperty(name, pProperty);
            }
        }

        return pProperty;
    }


    Value* FeatureMatrix::UpdateValue(Property* inProperty, const std::map<std::string, std::string>& inAttrs)
    {
        if (inProperty == NULL)
        {
            return NULL;
        }

        // name: mandatory attribute
        std::string name;
        std::map<std::string, std::string>::const_iterator itr = inAttrs.find(kAttribute_name);
        if ((itr == inAttrs.end()) || (itr->second.empty()))
        {
            return NULL;
        }else
        {
            name = itr->second;
        }

        // supported: optional attribute
        bool supported = true;
        itr = inAttrs.find(kAttribute_supported);
        if (itr != inAttrs.end())
        {
            supported = (itr->second == kValue_true);
        }

        // Find or Create new Value
        Value * pValue = inProperty->FindValue(name);
        if (pValue == NULL)
        {
            pValue = new Value(supported);
            if (pValue != NULL)
            {
                inProperty->AddValue(name, pValue);
            }
        }

        return pValue;
    }

   

    /* -------------------------------------------------- Value */

    Value::Value(bool supported) 
    { 
        mbSupported = supported;
    }

    Value::~Value()
    {
    }

    bool Value::IsSupported()
    {
        return mbSupported;
    }


    /* -------------------------------------------------- Property */

    Property::Property(const std::string& def, bool supported)
    {
        mbSupported = supported;
        mDefault = def;
    }

    Property::~Property()
    {
        StrValueMap::iterator itr = mValues.begin();
        for(; itr != mValues.end(); itr++)
        {
            if (itr->second) delete itr->second;
        }
        mValues.clear();        
    }

    Value* Property::FindValue(const std::string& inValueName)
    {
        StrValueMap::iterator itr = mValues.find(inValueName);
        if (itr != mValues.end())
            return itr->second;
        return NULL;
    }

    bool Property::AddValue(const std::string& valueName, Value* pValue)
    {
        mValues.insert(std::pair<std::string, Value*>(valueName, pValue));

        return true;
    }

    bool Property::IsSupported()
    {
        return mbSupported;
    }


    std::string Property::GetDefault()
    {
        return mDefault;
    }


    /* -------------------------------------------------- Feature */

    Feature::Feature(bool supported)
    {
        mbSupported = supported;
    }

    Feature::~Feature()
    {
        StrPropertyMap::iterator itr = mProperties.begin();
        for(; itr != mProperties.end(); itr++)
        {
            if (itr->second) delete itr->second;
        }
        mProperties.clear();
    }

    Property* Feature::FindProperty(const std::string& inPropertyName)
    {
        StrPropertyMap::iterator itr = mProperties.find(inPropertyName);
        if (itr != mProperties.end())
        {
            return itr->second;
        }
        return NULL;
    }

    bool Feature::AddProperty(const std::string& name, Property* pProperty)
    {
        mProperties.insert(std::pair<std::string, Property*>(name, pProperty));

        return true;
    }

    bool Feature::IsSupported()
    {
        return mbSupported;
    }

};
