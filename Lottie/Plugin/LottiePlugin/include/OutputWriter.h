/*************************************************************************
* ADOBE SYSTEMS INCORPORATED
* Copyright 2013 Adobe Systems Incorporated
* All Rights Reserved.

* NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the
* terms of the Adobe license agreement accompanying it.  If you have received this file from a
* source other than Adobe, then your use, modification, or distribution of it requires the prior
* written permission of Adobe.
**************************************************************************/

/**
 * @file  OutputWriter.h
 *
 * @brief This file contains declarations for a output writer.
 */

#ifndef OUTPUT_WRITER_H_
#define OUTPUT_WRITER_H_
#include "PublishToLottie.h"
#include "IOutputWriter.h"
#include <string>
#include <map>

/* -------------------------------------------------- Forward Decl */

class JSONNode;

namespace LottieExporter
{
    class ITimelineWriter;
}

/* -------------------------------------------------- Enums */

namespace LottieExporter
{
    enum StrokeStyleType
    {
        INVALID_STROKE_STYLE_TYPE,
        SOLID_STROKE_STYLE_TYPE
    };
}


/* -------------------------------------------------- Macros / Constants */

#define IMAGE_FOLDER "images"
#define SOUND_FOLDER "sounds"


/* -------------------------------------------------- Structs / Unions */

namespace LottieExporter
{
    struct SOLID_STROKE_STYLE
    {
        FCM::Double thickness;
        DOM::StrokeStyle::JOIN_STYLE joinStyle;
        DOM::StrokeStyle::CAP_STYLE capStyle;
        DOM::Utils::ScaleType scaleType;
        FCM::Boolean strokeHinting;
    };

    struct STROKE_STYLE
    {
        StrokeStyleType type;

        union
        {
            SOLID_STROKE_STYLE solidStrokeStyle;
        };
    };


struct FRAME_SCRIPT
{
	FCM::U_Int32 frameNumber;
	FCM::U_Int32 layerNumber;
	std::string script;
};
}
/* -------------------------------------------------- Class Decl */

namespace LottieExporter
{
    class JSONOutputWriter : public IOutputWriter
    {
    public:

        // Marks the begining of the output
        virtual FCM::Result StartOutput(std::string& outputFileName);

        // Marks the end of the output
        virtual FCM::Result EndOutput();

        // Marks the begining of the Document
        virtual FCM::Result StartDocument(
            const DOM::Utils::COLOR& background,
            FCM::U_Int32 stageHeight, 
            FCM::U_Int32 stageWidth,
            FCM::U_Int32 fps);

        // Marks the end of the Document
        virtual FCM::Result EndDocument();

        // Marks the start of a timeline
        virtual FCM::Result StartDefineTimeline();

        // Marks the end of a timeline
        virtual FCM::Result EndDefineTimeline(
            FCM::U_Int32 resId, 
            FCM::StringRep16 pName, 
            ITimelineWriter* pTimelineWriter);

        // Marks the start of a shape
        virtual FCM::Result StartDefineShape();

        // Start of fill region definition
        virtual FCM::Result StartDefineFill();

        // Solid fill style definition
        virtual FCM::Result DefineSolidFillStyle(const DOM::Utils::COLOR& color);

        // Bitmap fill style definition
        virtual FCM::Result DefineBitmapFillStyle(
            FCM::Boolean clipped,
            const DOM::Utils::MATRIX2D& matrix,
            FCM::S_Int32 height, 
            FCM::S_Int32 width,
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem);

        // Start Linear Gradient fill style definition
        virtual FCM::Result StartDefineLinearGradientFillStyle(
            DOM::FillStyle::GradientSpread spread,
            const DOM::Utils::MATRIX2D& matrix);

        // Sets a specific key point in a color ramp (for both radial and linear gradient)
        virtual FCM::Result SetKeyColorPoint(
            const DOM::Utils::GRADIENT_COLOR_POINT& colorPoint);

        // End Linear Gradient fill style definition
        virtual FCM::Result EndDefineLinearGradientFillStyle();

        // Start Radial Gradient fill style definition
        virtual FCM::Result StartDefineRadialGradientFillStyle(
            DOM::FillStyle::GradientSpread spread,
            const DOM::Utils::MATRIX2D& matrix,
            FCM::S_Int32 focalPoint);

        // End Radial Gradient fill style definition
        virtual FCM::Result EndDefineRadialGradientFillStyle();

        // Start of fill region boundary
        virtual FCM::Result StartDefineBoundary();

        // Sets a segment of a path (Used for boundary, holes)
        virtual FCM::Result SetSegment(const DOM::Utils::SEGMENT& segment);

        // End of fill region boundary
        virtual FCM::Result EndDefineBoundary();

        // Start of fill region hole
        virtual FCM::Result StartDefineHole();

        // End of fill region hole
        virtual FCM::Result EndDefineHole();

        // Start of stroke group
        virtual FCM::Result StartDefineStrokeGroup();

        // Start solid stroke style definition
        virtual FCM::Result StartDefineSolidStrokeStyle(
            FCM::Double thickness,
            const DOM::StrokeStyle::JOIN_STYLE& joinStyle,
            const DOM::StrokeStyle::CAP_STYLE& capStyle,
            DOM::Utils::ScaleType scaleType,
            FCM::Boolean strokeHinting);

        // End of solid stroke style
        virtual FCM::Result EndDefineSolidStrokeStyle();

        // Start of stroke 
        virtual FCM::Result StartDefineStroke();

        // End of a stroke 
        virtual FCM::Result EndDefineStroke();

        // End of stroke group
        virtual FCM::Result EndDefineStrokeGroup();

        // End of fill style definition
        virtual FCM::Result EndDefineFill();

        // Marks the end of a shape
        virtual FCM::Result EndDefineShape(FCM::U_Int32 resId);

        // Define a bitmap
        virtual FCM::Result DefineBitmap(
            FCM::U_Int32 resId,
            FCM::S_Int32 height, 
            FCM::S_Int32 width,
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem);

        // Define text
        virtual FCM::Result DefineText(
            FCM::U_Int32 resId, 
            const std::string& name, 
            const DOM::Utils::COLOR& color,
            const std::string& displayText, 
            DOM::FrameElement::PIClassicText pTextItem);

        virtual FCM::Result DefineSound(
            FCM::U_Int32 resId, 
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem);
        JSONOutputWriter(FCM::PIFCMCallback pCallback);

        virtual ~JSONOutputWriter();

        // Start of a path
        virtual FCM::Result StartDefinePath();
        
        // End of a path 
		virtual FCM::Result EndDefinePath();
		FCM::Result AddIp(JSONNode &firstNode);
		FCM::Result AddOp(JSONNode &firstNode);
		FCM::Result AddVersion(JSONNode &firstNode);
		FCM::Result AddFr(JSONNode &firstNode);
        FCM::Result AddLayers(JSONNode &firstNode);
		FCM::Result AddWidthHeight(JSONNode &firstNode);
		
		FCM::Result AddAssets(JSONNode &firstNode);
		FCM::Result AddMarkers(JSONNode &firstNode);
        FCM::Result AddGroup(int resourceId);
        FCM::Result AddHoles();
        FCM::Result AddItems(struct group *gr);
		FCM::Result InitFileName(const std::string& outputFileName);
		LottieExporter::LottieManager* GetLottieManager() { return m_LottieManager; }
		

    private:
        
        FCM::Result CreateImageFileName(const std::string& libPathName, std::string& name);

        FCM::Result CreateSoundFileName(const std::string& libPathName, std::string& name);

        FCM::Boolean GetImageExportFileName(const std::string& libPathName, std::string& name);

        void SetImageExportFileName(const std::string& libPathName, const std::string& name);

    private:

		std::string m_outputFolder;
		FCM::Boolean m_outputFolderCreated;
		std::string m_outputLottieFilePath;

		//JSONNode* version;

        JSONNode* m_pRootNode;

        JSONNode* m_pShapeArray;

        JSONNode* m_pTimelineArray;

        JSONNode* m_pBitmapArray;

        JSONNode* m_pSoundArray;

        JSONNode* m_pTextArray;

        JSONNode*   m_shapeElem;

        JSONNode*   m_pathArray;

        JSONNode*   m_pathElem;

        JSONNode*   m_gradientColor;

        JSONNode*   m_stopPointArray;


		//JSONNode * m_version;

        std::string m_pathCmdStr;

        bool        m_firstSegment;

        STROKE_STYLE m_strokeStyle;

        std::string m_outputHTMLFile;

        std::string m_outputJSONFilePath;

        std::string m_outputJSONFileName;

        std::string m_outputImageFolder;

        std::string m_outputSoundFolder;

        char* m_HTMLOutput;

        FCM::PIFCMCallback m_pCallback;

        FCM::U_Int32 m_imageFileNameLabel;

        FCM::U_Int32 m_soundFileNameLabel;

        std::map<std::string, std::string> m_imageMap;
        
        FCM::Boolean m_imageFolderCreated;
        
        FCM::Boolean m_soundFolderCreated;

		JSONNode* m_ip = nullptr;
		JSONNode* m_op = nullptr;
		JSONNode* m_version = nullptr;
		JSONNode* m_fr = nullptr;
		JSONNode* m_width = nullptr;
		JSONNode* m_height = nullptr;
        JSONNode * m_layers =nullptr;
        JSONNode *m_shapes= nullptr;
        JSONNode *m_items= nullptr;
        JSONNode *m_group= nullptr;
        JSONNode *m_assets= nullptr;
        JSONNode *m_inv= nullptr;
        JSONNode *m_outv= nullptr;
        JSONNode *m_cv= nullptr;
        JSONNode *m_cv2= nullptr;
		LottieExporter::LottieManager *m_LottieManager = nullptr;
       

    };


    class JSONTimelineWriter : public ITimelineWriter
    {
    public:

        virtual FCM::Result PlaceObject(
            FCM::U_Int32 resId,
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId,
            const DOM::Utils::MATRIX2D* pMatrix,
            FCM::PIFCMUnknown pUnknown = NULL);
        virtual FCM::Result PlaceObject(
            FCM::U_Int32 resId,
            FCM::U_Int32 objectId,
            FCM::PIFCMUnknown pUnknown = NULL);

        virtual FCM::Result RemoveObject(
            FCM::U_Int32 objectId);

        virtual FCM::Result UpdateZOrder(
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId);

        virtual FCM::Result UpdateMask(
            FCM::U_Int32 objectId,
            FCM::U_Int32 maskTillObjectId);

        virtual FCM::Result UpdateBlendMode(
            FCM::U_Int32 objectId,
            DOM::FrameElement::BlendMode blendMode);

        virtual FCM::Result UpdateVisibility(
            FCM::U_Int32 objectId,
            FCM::Boolean visible);

        virtual FCM::Result AddGraphicFilter(
            FCM::U_Int32 objectId,
            FCM::PIFCMUnknown pFilter);

        virtual FCM::Result UpdateDisplayTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::MATRIX2D& matrix);

        virtual FCM::Result UpdateColorTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::COLOR_MATRIX& colorMatrix);

        virtual FCM::Result ShowFrame(FCM::U_Int32 frameNum);

        virtual FCM::Result AddFrameScript(FCM::CStringRep16 pScript, FCM::U_Int32 layerNum);

        virtual FCM::Result RemoveFrameScript(FCM::U_Int32 layerNum);

        virtual FCM::Result SetFrameLabel(FCM::StringRep16 pLabel, DOM::KeyFrameLabelType labelType);

        JSONTimelineWriter(FCM::PIFCMCallback pCallback);

        virtual ~JSONTimelineWriter();

        const JSONNode* GetRoot();

        void Finish(FCM::U_Int32 resId, FCM::StringRep16 pName);

    private:

        JSONNode* m_pCommandArray;

        JSONNode* m_pFrameArray;

        JSONNode* m_pTimelineElement;

        JSONNode* m_pFrameElement;

        FCM::PIFCMCallback m_pCallback;
		FCM::S_Int32 m_frameIndex = -1;
		std::vector<LottieExporter::FRAME_SCRIPT> m_TimelineScripts;
		JSONNode* m_TimelineSounds = nullptr;
		
	
    };
};

#endif // OUTPUT_WRITER_H_

