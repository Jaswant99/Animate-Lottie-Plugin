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
 * @file  LottieFeatureMatrix.h
 *
 * @brief This file contains declarations for a DocType plugin.
 */


#ifndef Lottie_FEATURE_MATRIX_H_
#define Lottie_FEATURE_MATRIX_H_

#include <map>

#include "Version.h"
#include "FCMTypes.h"
#include "FCMPluginInterface.h"
//#include "ApplicationFCMPublicIDs.h"
#include "DocType/IDocType.h"
#include "DocType/IFeatureMatrix.h"
#include <string>
#include "PluginConfiguration.h"

/* -------------------------------------------------- Forward Decl */

using namespace FCM;
using namespace DocType;


/* -------------------------------------------------- Class Decl */

namespace LottieExporter
{
    
	// {3A317E31-5078-4B7A-BD68-360EC1146BC6}
    const FCM::FCMCLSID CLSID_FeatureMatrix =
		
		
	{ 0x3a317e31, 0x5078, 0x4b7a,{ 0xbd, 0x68, 0x36, 0xe, 0xc1, 0x14, 0x6b, 0xc6 } };

    
  
    
    class Value
    {
    public:
        
        Value(bool supported);
        
        ~Value();
        
        bool IsSupported();
        
    private:
        bool mbSupported;
    };
    
    typedef std::map<std::string, Value*> StrValueMap;
    
    class Property
    {
    public:
        Property(const std::string& def, bool supported);
        
        ~Property();
        
        Value* FindValue(const std::string& inValueName);
        
        bool AddValue(const std::string& valueName, Value* pValue);
        
        bool IsSupported();
        
        std::string GetDefault();
        
    private:
        std::string mDefault;
        bool mbSupported;
        StrValueMap mValues;
    };
    
    typedef std::map<std::string, Property*> StrPropertyMap;
    
    class Feature
    {
        
    public:
        
        Feature(bool supported);
        
        ~Feature();
        
        Property* FindProperty(const std::string& inPropertyName);
        
        bool AddProperty(const std::string& name, Property* pProperty);
        
        bool IsSupported();
        
    private:
        
        bool mbSupported;
        
        StrPropertyMap mProperties;
    };
    
    typedef std::map<std::string, Feature*> StrFeatureMap;
    
    class FeatureMatrix : public DocType::IFeatureMatrix, public FCM::FCMObjectBase
    {
        BEGIN_MULTI_INTERFACE_MAP(FeatureMatrix,Lottie_PLUGIN_VERSION)
        INTERFACE_ENTRY(IFeatureMatrix)
        END_INTERFACE_MAP
        
    public:
        
        virtual FCM::Result _FCMCALL IsSupported(
                                                 CStringRep16 inFeatureName,
                                                 FCM::Boolean& isSupported);
        
        virtual FCM::Result _FCMCALL IsSupported(
                                                 CStringRep16 inFeatureName,
                                                 CStringRep16 inPropName,
                                                 FCM::Boolean& isSupported);
        
        virtual FCM::Result _FCMCALL IsSupported(
                                                 CStringRep16 inFeatureName,
                                                 CStringRep16 inPropName,
                                                 CStringRep16 inValName,
                                                 FCM::Boolean& isSupported);
        
        virtual FCM::Result _FCMCALL GetDefaultValue(
                                                     CStringRep16 inFeatureName,
                                                     CStringRep16 inPropName,
                                                     FCM::VARIANT& outDefVal);
        
        FeatureMatrix();
        
        ~FeatureMatrix();
        
        void Init(FCM::PIFCMCallback pCallback);
       
        
    private:
        
        FCM::Result StartElement(
                                 const std::string name,
                                 const std::map<std::string, std::string>& attrs);
        
        FCM::Result EndElement(const std::string name);
        
        Feature* FindFeature(const std::string& inFeatureName);
        
        Feature* UpdateFeature(const std::map<std::string, std::string>& inAttrs);
        
        Property* UpdateProperty(Feature* inFeature, const std::map<std::string,std::string>& inAttrs);
        
        Value* UpdateValue(Property* inProperty, const std::map<std::string, std::string>& inAttrs);
        
        void UpdateFeatureMatrix();
        
    private:
        
        StrFeatureMap mFeatures;
        
        Feature* mCurrentFeature;
        
        Property* mCurrentProperty;
        
        bool m_bInited;
        
     
        
        friend class FeatureDocumentHandler;
    };
};


#endif // Lottie_FEATURE_MATRIX_H_

