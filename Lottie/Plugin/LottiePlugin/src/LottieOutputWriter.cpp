#include "OutputWriter.h"
#include "PluginConfiguration.h"

#include <vector>
#include <cstring>
#include <fstream>
#include "FlashFCMPublicIDs.h"
#include "FCMPluginInterface.h"
#include "libjson.h"
#include "Utils.h"
#include "FrameElement/ISound.h"
#include "Service/Image/IBitmapExportService.h"
#include "Service/TextLayout/ITextLinesGeneratorService.h"
#include "Service/TextLayout/ITextLine.h"
#include "Service/Sound/ISoundExportService.h"
#include "GraphicFilter/IDropShadowFilter.h"
#include "GraphicFilter/IAdjustColorFilter.h"
#include "GraphicFilter/IBevelFilter.h"
#include "GraphicFilter/IBlurFilter.h"
#include "GraphicFilter/IGlowFilter.h"
#include "GraphicFilter/IGradientBevelFilter.h"
#include "GraphicFilter/IGradientGlowFilter.h"
#include "Utils/ILinearColorGradient.h"
#include <math.h>

#ifdef _WINDOWS
#include "Windows.h"
#endif
#ifdef _MAC
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR		'/'
#endif

#else	// Win32 Version
#define PATH_SEPARATOR		'\\'
#endif

namespace LottieExporter
{
    static const std::string moveTo = "M";
    static const std::string lineTo = "L";
    static const std::string bezierCurveTo = "Q";
    static const std::string space = " ";
    static const std::string comma = ",";
    static const std::string semiColon = ";";



    static const FCM::Float GRADIENT_VECTOR_CONSTANT = 16384.0;

    static const char* htmlOutput = 
        "<!DOCTYPE html>\r\n \
        <html>\r\n \
        <head> \r\n\
            <script src=\"%s/cjs/createjs-2013.12.12.min.js\"></script> \r\n\
            <script src=\"%s/cjs/movieclip-0.7.1.min.js\"></script> \r\n\
            <script src=\"%s/cjs/easeljs-0.7.0.min.js\"></script> \r\n\
            <script src=\"%s/cjs/tweenjs-0.5.1.min.js\"></script> \r\n\
            <script src=\"%s/cjs/preloadjs-0.4.1.min.js\"></script> \r\n\
           \r\n\
            <script src=\"%s/dist/jquery-1.10.2.min.js\"></script> \r\n\
            <script src=\"%s/runtime/resourcemanager.js\"></script> \r\n \
            <script src=\"%s/runtime/utils.js\"></script>     \r\n\
            <script src=\"%s/runtime/timelineanimator.js\"></script>    \r\n\
            <script src=\"%s/runtime/player.js\"></script>     \r\n\
            \r\n\
            <script type=\"text/javascript\"> \r\n\
            \r\n\
            var loader = new createjs.LoadQueue(false); \r\n\
            loader.addEventListener(\"complete\", handleComplete); \r\n\
            loader.loadManifest(\"./%s/images/B.png\"); \r\n\
            function handleComplete() \r\n\
                { \r\n\
                $(document).ready(function() { \r\n\
                \r\n\
                \r\n\
                    var canvas = document.getElementById(\"canvas\"); \r\n\
                    var stage = new createjs.Stage(canvas);         \r\n\
                    //pass FPS and use that in the player \r\n\
                    init(stage, \"%s\", %d);             \r\n\
                }); \r\n\
                } \r\n\
            </script> \r\n\
        </head> \r\n\
        \r\n\
        <body> \r\n\
            <canvas id=\"canvas\" width=\"%d\" height=\"%d\" style=\"background-color:#%06X\"> \r\n\
                alternate content \r\n\
            </canvas> \r\n\
        </body> \r\n\
        </html>";


    /* -------------------------------------------------- JSONOutputWriter */

	FCM::Result JSONOutputWriter::InitFileName(const std::string &outputFileName)
	{
		std::string parent;
		std::string LottieFile;
		Utils::GetParent(outputFileName, parent);
		int res = Utils::CreateDir(parent, m_pCallback);
		if (!(FCM_SUCCESS_CODE(res)))
		{
			Utils::Trace(m_pCallback, "Output parent folder (%s) could not be created\n", parent.c_str());
			return res;
		}
		Utils::GetFileNameWithoutExtension(outputFileName, LottieFile);
		m_outputFolder = parent + LottieFile + PATH_SEPARATOR;
		if (!m_outputFolderCreated)
		{
			res = Utils::CreateDir(m_outputFolder, m_pCallback);
			if (!(FCM_SUCCESS_CODE(res)))
			{
				Utils::Trace(m_pCallback, "Output image folder (%s) could not be created\n", m_outputFolder.c_str());
				return res;
			}
			m_outputFolderCreated = true;
		}
		m_outputLottieFilePath = m_outputFolder + LottieFile + ".json";
		
		return FCM_SUCCESS;
	}


    FCM::Result JSONOutputWriter::StartOutput(std::string& outputFileName)
    {
        std::string parent;
        std::string jsonFile;
		
		FCM::Result res = InitFileName(outputFileName);
		if (res == FCM_SUCCESS) {
			Utils::GetParent(outputFileName, parent);
			Utils::GetFileNameWithoutExtension(outputFileName, jsonFile);
			m_outputHTMLFile = outputFileName;
			m_outputJSONFileName = jsonFile + ".json";
			m_outputJSONFilePath = parent + jsonFile + ".json";
			m_outputImageFolder = parent + IMAGE_FOLDER;
			m_outputSoundFolder = parent + SOUND_FOLDER;
		}
		m_LottieManager = new LottieExporter::LottieManager;
        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::EndOutput()
    {

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::StartDocument(
        const DOM::Utils::COLOR& background,
        FCM::U_Int32 stageHeight, 
        FCM::U_Int32 stageWidth,
        FCM::U_Int32 fps)
    {
		
		m_LottieManager->Init(m_outputLottieFilePath);
		m_LottieManager->SetFPS(fps);
        //m_LottieManager->SetIp();
		m_LottieManager->SetStageWidthHeight(stageWidth, stageHeight);
		return FCM_SUCCESS;

       
    }


    FCM::Result JSONOutputWriter::EndDocument()
    {
        std::fstream file;
             
        // Write the JSON file (overwrite file if it already exists)
        Utils::OpenFStream(m_outputJSONFilePath, file, std::ios_base::trunc|std::ios_base::out, m_pCallback);
        if(!file)
            std::cout<<"UNABLE TO OPEN"<<std::endl;
        
        JSONNode firstNode(JSON_NODE);
		AddVersion(firstNode);
		AddWidthHeight(firstNode);
		AddIp(firstNode);
		AddOp(firstNode);
		AddFr(firstNode);
        AddAssets(firstNode);
		AddLayers(firstNode);
		AddMarkers(firstNode);
        
        file << firstNode.write_formatted();
        file.close();
        
        bool removeWhiteSpaces = true;
        if(removeWhiteSpaces)
        {
            std::fstream ifs;
            Utils::OpenFStream(m_outputJSONFilePath, ifs, std::ios_base::binary | std::ios_base::in, m_pCallback);
            if (ifs) {
                std::vector<char> content((std::istreambuf_iterator<char>(ifs)),
                                          (std::istreambuf_iterator<char>()));
                ifs.close();
                content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());
                std::fstream outfile;
                Utils::OpenFStream(m_outputJSONFilePath, outfile, std::ios_base::binary | std::ios_base::out, m_pCallback);
                if (outfile) {
                    outfile.write(content.data(), content.size());
                    outfile.close();
                }
            }
        }
        
      
        delete m_inv;
        delete m_outv;
        delete m_cv;
        delete m_items;
        delete m_version;
        delete m_layers;
        return FCM_SUCCESS;
    }
	FCM::Result JSONOutputWriter::AddFr(JSONNode &firstNode)
	{//std::cout<<"ENTERED fr"<<std::endl;
		
		(firstNode).push_back(JSONNode("fr", m_LottieManager->GetFPS()));
		return FCM_SUCCESS;
	}
	FCM::Result JSONOutputWriter::AddIp(JSONNode &firstNode)
	{//std::cout<<"ENTERED ip"<<std::endl;
		
		firstNode.push_back(JSONNode("ip", m_LottieManager->GetIp()));
		return FCM_SUCCESS;
	}
    FCM::Result JSONOutputWriter::AddOp(JSONNode &firstNode)
    {
        //std::cout<<"ENTERED op"<<std::endl;
        firstNode.push_back(JSONNode("op", m_LottieManager->GetOp()));
         return FCM_SUCCESS;

        \
        
    }
                                        
                                                
    FCM::Result JSONOutputWriter::AddLayers(JSONNode &firstNode)
    {
      //std::cout<<"ENTERED layrs"<<std::endl;
        if(m_layers)
            delete m_layers;
        m_layers=new JSONNode(JSON_ARRAY);
        m_layers->set_name("layers");
        std::uint32_t size=m_LottieManager->GetNumofLayers();
        int index;
        std::map<int , Layer *>::iterator it;
        std::map<int , Layer *>objectid_Layer=m_LottieManager->GetobjIdLayer();
        //m_LottieManager->SortLayers();
            
        for(int i=0;i<size;i++)
        {
            Layer * layer =m_LottieManager->GetLayerAtIndex(i);
            JSONNode layerprop;
            layerprop.push_back(JSONNode("ddd",layer->ddd));
            layerprop.push_back(JSONNode("ind",layer->ind));
            layerprop.push_back(JSONNode("ty",layer->ty));
            layerprop.push_back(JSONNode("nm",layer->nm));
            if(layer->ty == 2)
            {image_resource * image = m_LottieManager->Getimage_resource_with_id(layer->resourceId);
                layerprop.push_back(JSONNode("cl",image->cl));
                layerprop.push_back(JSONNode("refId",image->ref_id));
                
            }
            layerprop.push_back(JSONNode("ip",layer->ip));
            layerprop.push_back(JSONNode("op",layer->op));
            layerprop.push_back(JSONNode("ao",layer->ao));
            layerprop.push_back(JSONNode("st",layer->st));
            layerprop.push_back(JSONNode("bm",layer->bm));
            
            JSONNode layer_transformprop;
            layer_transformprop.set_name("ks");
            
            //POSITION
            
            std::uint32_t p_size=layer->ks.p.size();
            
            if(p_size==1){
                JSONNode position;
            position.set_name("p");
            position.push_back(JSONNode("a",layer->ks.p[p_size-1].a));
            JSONNode value_p(JSON_ARRAY);
            value_p.set_name("k");
            value_p.push_back(JSONNode("",layer->ks.p[p_size-1].k[0]));
            value_p.push_back(JSONNode("",layer->ks.p[p_size-1].k[1]));
            value_p.push_back(JSONNode("",layer->ks.p[p_size-1].k[2]));
            
            position.push_back(value_p);
            position.push_back(JSONNode("ix",layer->ks.p[p_size-1].ix));
            layer_transformprop.push_back(position);
            }
            
            else
            {
                JSONNode position;
               position.set_name("p");
               position.push_back(JSONNode("a",layer->ks.p[p_size-1].a));
                JSONNode value_p(JSON_ARRAY);
                value_p.set_name("k");
                for(int i=0;i<p_size-1;i++)
                {
                    JSONNode pos_node;
              /*    JSONNode in_value;
                   in_value.set_name("i");
                    in_value.push_back(JSONNode("x",layer->ks.p[i].offset.i.x));
                    in_value.push_back(JSONNode("y",layer->ks.p[i].offset.i.y));
                    JSONNode out_value;
                    out_value.set_name("o");
                    out_value.push_back(JSONNode("x",layer->ks.p[i].offset.o.x));
                    out_value.push_back(JSONNode("y",layer->ks.p[i].offset.o.y));
                    pos_node.push_back(in_value);
                    pos_node.push_back(out_value);*/
                    pos_node.push_back(JSONNode("t",layer->ks.p[i].offset.time));
                    JSONNode start_value(JSON_ARRAY);
                    start_value.set_name("s");
                    start_value.push_back(JSONNode("",layer->ks.p[i].offset.start[0]));
                    start_value.push_back(JSONNode("",layer->ks.p[i].offset.start[1]));
                   // start_value.push_back(JSONNode("",layer->ks.p[i].offset.start[2]));
                    pos_node.push_back(start_value);
                   pos_node.push_back(JSONNode("h",layer->ks.p[i].offset.h));
           /*   JSONNode ti(JSON_ARRAY);
                    ti.set_name("ti");
                    ti.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.ti.x));
                    ti.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.ti.y));
                    JSONNode to(JSON_ARRAY);
                    to.set_name("to");
                    to.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.to.x));
                    to.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.to.y));
                    pos_node.push_back(to);
                    pos_node.push_back(ti);*/
                    value_p.push_back(pos_node);
                }
                JSONNode last_pos;
                last_pos.push_back(JSONNode("t",layer->ks.p[p_size-1].offset.time));
                JSONNode start_value(JSON_ARRAY);
                start_value.set_name("s");
                start_value.push_back(JSONNode("",layer->ks.p[p_size-1].k[0]));
                start_value.push_back(JSONNode("",layer->ks.p[p_size-1].k[1]));
               //start_value.push_back(JSONNode("",layer->ks.p[p_size-1].k[2]));
               last_pos.push_back(JSONNode("h",layer->ks.p[p_size-1].offset.h));
                last_pos.push_back(start_value);
                value_p.push_back(last_pos);
                position.push_back(value_p);
                position.push_back(JSONNode("ix",layer->ks.p[p_size-1].ix));
                layer_transformprop.push_back(position);
    
               
            }
            
            //ANCHORPOINT
            std::uint32_t a_size=layer->ks.a.size();
            if(a_size==1)
            {
            JSONNode anchorpoint;
            anchorpoint.set_name("a");
            anchorpoint.push_back(JSONNode("a",layer->ks.a[a_size-1].a));
            JSONNode value_a(JSON_ARRAY);
            value_a.set_name("k");
            value_a.push_back(JSONNode("",layer->ks.a[a_size-1].k[0]));
            value_a.push_back(JSONNode("",layer->ks.a[a_size-1].k[1]));
            value_a.push_back(JSONNode("",layer->ks.a[a_size-1].k[2]));
            anchorpoint.push_back(value_a);
            anchorpoint.push_back(JSONNode("ix",layer->ks.a[a_size-1].ix));
            layer_transformprop.push_back(anchorpoint);
            }
            
            //SCALE
            std::uint32_t s_size=layer->ks.s.size();
            if(s_size==1)
            {
            JSONNode scale;
            scale.set_name("s");
            scale.push_back(JSONNode("a",layer->ks.s[s_size-1].a));
            JSONNode value_s(JSON_ARRAY);
            value_s.set_name("k");
            value_s.push_back(JSONNode("",layer->ks.s[s_size-1].k[0]));
            value_s.push_back(JSONNode("",layer->ks.s[s_size-1].k[1]));
            value_s.push_back(JSONNode("",layer->ks.s[s_size-1].k[2]));
            scale.push_back(value_s);
            scale.push_back(JSONNode("ix",layer->ks.s[s_size-1].ix));
            layer_transformprop.push_back(scale);
            }
            else
            {
                JSONNode scale;
                scale.set_name("s");
                scale.push_back(JSONNode("a",layer->ks.s[s_size-1].a));
                JSONNode value_s(JSON_ARRAY);
                value_s.set_name("k");
                for(int i=0;i<s_size-1;i++)
                {
                    JSONNode scale_node;
                    JSONNode in_value;
                 /*   in_value.set_name("i");
                    in_value.push_back(JSONNode("x",layer->ks.s[i].offset.i.x));
                    in_value.push_back(JSONNode("y",layer->ks.s[i].offset.i.y));
                    JSONNode out_value;
                    out_value.set_name("o");
                    out_value.push_back(JSONNode("x",layer->ks.s[i].offset.o.x));
                    out_value.push_back(JSONNode("y",layer->ks.s[i].offset.o.y));
                    scale_node.push_back(in_value);
                    scale_node.push_back(out_value);*/
                    scale_node.push_back(JSONNode("t",layer->ks.s[i].offset.time));
                    JSONNode start_value(JSON_ARRAY);
                    start_value.set_name("s");
                    start_value.push_back(JSONNode("",layer->ks.s[i].offset.start[0]));
                    start_value.push_back(JSONNode("",layer->ks.s[i].offset.start[1]));
                    //start_value.push_back(JSONNode("",layer->ks.s[i].offset.start[2]));
                    scale_node.push_back(start_value);
                    scale_node.push_back(JSONNode("h",layer->ks.s[i].offset.h));
                    value_s.push_back(scale_node);
                    
                }
                JSONNode last_scale;
                last_scale.push_back(JSONNode("t",layer->ks.s[s_size-1].offset.time));
                JSONNode start_value(JSON_ARRAY);
                start_value.set_name("s");
                start_value.push_back(JSONNode("",layer->ks.s[s_size-1].k[0]));
                start_value.push_back(JSONNode("",layer->ks.s[s_size-1].k[1]));
               // start_value.push_back(JSONNode("",layer->ks.s[s_size-1].k[2]));
                last_scale.push_back(start_value);
                last_scale.push_back(JSONNode("h",layer->ks.s[s_size-1].offset.h));
                value_s.push_back(last_scale);
                scale.push_back(value_s);
                
                scale.push_back(JSONNode("ix",layer->ks.s[s_size-1].ix));
                layer_transformprop.push_back(scale);
                
                
            }
            
            
            //ROTATION
            std::uint32_t r_size=layer->ks.r.size();
            if(r_size==1)
            {
            JSONNode rotation;
            rotation.set_name("r");
            rotation.push_back(JSONNode("a",layer->ks.r[r_size-1].a));
            rotation.push_back(JSONNode("k",layer->ks.r[r_size-1].k));
            rotation.push_back(JSONNode("ix",layer->ks.r[r_size-1].ix));
            layer_transformprop.push_back(rotation);
            }
            
            else
            {
                JSONNode rotation;
                rotation.set_name("r");
                rotation.push_back(JSONNode("a",layer->ks.r[r_size-1].a));
                JSONNode value_r(JSON_ARRAY);
                value_r.set_name("k");
                for(int i=0;i<r_size-1;i++)
                {
                    JSONNode rot_node;
                 /*   JSONNode in_value;
                    in_value.set_name("i");
                    in_value.push_back(JSONNode("x",layer->ks.r[i].offset.i.x));
                    in_value.push_back(JSONNode("y",layer->ks.r[i].offset.i.y));
                    JSONNode out_value;
                    out_value.set_name("o");
                    out_value.push_back(JSONNode("x",layer->ks.r[i].offset.o.x));
                    out_value.push_back(JSONNode("y",layer->ks.r[i].offset.o.y));
                    rot_node.push_back(in_value);
                    rot_node.push_back(out_value);*/
                    rot_node.push_back(JSONNode("t",layer->ks.r[i].offset.time));
                    JSONNode start_value(JSON_ARRAY);
                    start_value.set_name("s");
                    start_value.push_back(JSONNode("",layer->ks.r[i].offset.start[0]));
                    rot_node.push_back(start_value);
                    rot_node.push_back(JSONNode("h",layer->ks.r[i].offset.h));
                    value_r.push_back(rot_node);
                }
                JSONNode last_pos;
                last_pos.push_back(JSONNode("t",layer->ks.r[r_size-1].offset.time));
                JSONNode start_value(JSON_ARRAY);
                start_value.set_name("s");
                start_value.push_back(JSONNode("",layer->ks.r[r_size-1].k));
                last_pos.push_back(start_value);
                last_pos.push_back(JSONNode("h",layer->ks.r[r_size-1].offset.h));
                value_r.push_back(last_pos);
                rotation.push_back(value_r);
                rotation.push_back(JSONNode("ix",layer->ks.r[r_size-1].ix));
                layer_transformprop.push_back(rotation);
            }
            
            
            //OPACITY
            std::uint32_t o_size=layer->ks.o.size();
            if(o_size==1)
            {
            JSONNode opacity;
            opacity.set_name("o");
            opacity.push_back(JSONNode("a",layer->ks.o[o_size-1].a));
            opacity.push_back(JSONNode("k",layer->ks.o[o_size-1].k));
            opacity.push_back(JSONNode("ix",layer->ks.o[o_size-1].ix));
            layer_transformprop.push_back(opacity);
            }
            
            
            layerprop.push_back(layer_transformprop);
            if(layer->parent_ind != -1)
                layerprop.push_back(JSONNode("parent",layer->parent_ind));
           // std::cout<<layer->nm<<std::endl;
            bool hasmask;int index = -1;
            AddGroup(layer->resourceId);
            layerprop.push_back(*m_group);
            
            
           // delete m_group;
            m_layers->push_back(layerprop);
           //f delete layer;
            
            
        }
        AddHoles();
        firstNode.push_back(*m_layers);
    
        
        
        return FCM_SUCCESS;
                                                             
    }
   FCM::Result JSONOutputWriter::AddHoles()
     {
     
         std::uint32_t layer_size=m_LottieManager->GetNumofLayers();
         std::uint32_t hole_ind= layer_size;
         std::map<int,std::vector<hole_layer *>>  hole_resource_map = m_LottieManager->get_hole_resource_map();
         std::map<int,std::vector<hole_layer *>>:: iterator it;
         for(it = hole_resource_map.begin();it != hole_resource_map.end();it++)
         {
             std::vector< hole_layer * > hole_layers= m_LottieManager->GetholelayersAtresourceid(it->first);
             std::map<int,std::vector<int>>  object_resource_map = m_LottieManager->get_object_resource_map();
             std::vector<int>objectids= object_resource_map[it->first];
             std::uint32_t hole_layersize = hole_layers.size();
             for(int j=0 ; j< hole_layersize;j++)
             {
             for(int i = 0; i < objectids.size(); i++)
             {   hole_ind ++;
                 Layer * layer =m_LottieManager->GetLayerAtObjectId(objectids[i]);
                 hole_layer * hole_layer = m_LottieManager->Getholelayerfrommap(it->first,j);
                 JSONNode layerprop;
                 layerprop.push_back(JSONNode("ddd",layer->ddd));
                 layerprop.push_back(JSONNode("ind",hole_ind));
                 layerprop.push_back(JSONNode("ty",layer->ty));
                 layerprop.push_back(JSONNode("nm","hole_layer"));
                 layerprop.push_back(JSONNode("ip",layer->ip));
                 layerprop.push_back(JSONNode("op",layer->op));
                 layerprop.push_back(JSONNode("ao",layer->ao));
                 layerprop.push_back(JSONNode("st",layer->st));
                 layerprop.push_back(JSONNode("bm",layer->bm));
                 layerprop.push_back(JSONNode("hasMask",true));

                 JSONNode shapes(JSON_ARRAY);
                 JSONNode groupprop;
                 shapes.set_name("shapes");
                 group * gr= hole_layer->gr;
                 if(gr!=NULL ){
                     groupprop.push_back(JSONNode("ty",gr->ty));
                     AddItems(gr);
                     groupprop.push_back(*m_items);
                     groupprop.push_back(JSONNode("nm",gr->nm));
                     groupprop.push_back(JSONNode("mn",gr->mn));
                     groupprop.push_back(JSONNode("np",gr->np));
                     groupprop.push_back(JSONNode("cix",gr->cix));
                     groupprop.push_back(JSONNode("bm",gr->bm));
                     groupprop.push_back(JSONNode("ix",gr->ix));
                     groupprop.push_back(JSONNode("hd",gr->hd));
                     shapes.push_back(groupprop);
                     layerprop.push_back(shapes);
                     JSONNode maskproperties(JSON_ARRAY);
                     maskproperties.set_name("masksProperties");
                     std::uint32_t mp_size= hole_layer->mp.size();
                     for(int i=0;i<mp_size;i++)
                     {   JSONNode mask_node;
                         mask_node.push_back(JSONNode("inv",hole_layer->mp[i]->inv));
                         mask_node.push_back(JSONNode("mode",hole_layer->mp[i]->mode));
                         JSONNode pt;
                         pt.set_name("pt");
                         pt.push_back(JSONNode("a",hole_layer->mp[i]->pt.a));
                         
                         JSONNode k;
                         k.set_name("k");
                         
                         std::uint32_t size=hole_layer->mp[i]->pt.i.size();
                         if(m_inv)
                             delete m_inv;
                         m_inv=new JSONNode(JSON_ARRAY);
                         
                         m_inv->set_name("i");
                         if(m_outv)
                             delete m_outv;
                         m_outv=new JSONNode(JSON_ARRAY);
                         m_outv->set_name("o");
                         if(m_cv)
                             delete m_cv;
                       
                         m_cv=new JSONNode(JSON_ARRAY);
                           m_cv->set_name("v");
                         
                         for(int index=0;index<size;index++)
                         {
                             JSONNode inv(JSON_ARRAY);
                             
                             inv.push_back(JSONNode("",hole_layer->mp[i]->pt.i[index].x));
                             inv.push_back(JSONNode("",hole_layer->mp[i]->pt.i[index].y));
                             JSONNode outv(JSON_ARRAY);
                             
                             outv.push_back(JSONNode("",hole_layer->mp[i]->pt.o[index].x));
                             outv.push_back(JSONNode("",hole_layer->mp[i]->pt.o[index].y));
                             JSONNode cv(JSON_ARRAY);
                             
                             cv.push_back(JSONNode("",hole_layer->mp[i]->pt.v[index].x));
                             cv.push_back(JSONNode("",hole_layer->mp[i]->pt.v[index].y));
                             
                             m_inv->push_back(inv);
                             m_outv->push_back(outv);
                             m_cv->push_back(cv);
                             
                         }
                         //m_inv->set_name("i");
                         k.push_back(*m_inv);
                         k.push_back(*m_outv);
                         k.push_back(*m_cv);
                         
                         k.push_back(JSONNode("c",hole_layer->mp[i]->pt.c));
                         
                         
                         pt.push_back(k);
                         pt.push_back(JSONNode("ix",hole_layer->mp[i]->pt.ix));
                         mask_node.push_back(pt);
                         
                         JSONNode opacity;
                         opacity.set_name("o");
                         opacity.push_back(JSONNode("a",hole_layer->mp[i]->o.a));
                         opacity.push_back(JSONNode("k",hole_layer->mp[i]->o.k));
                         opacity.push_back(JSONNode("ix",hole_layer->mp[i]->o.ix));
                         
                         mask_node.push_back(opacity);
                         JSONNode x;
                         x.set_name("x");
                         x.push_back(JSONNode("a",hole_layer->mp[i]->expansion.a));
                         x.push_back(JSONNode("k",hole_layer->mp[i]->expansion.k));
                         x.push_back(JSONNode("ix",hole_layer->mp[i]->expansion.ix));
                         
                         mask_node.push_back(x);
                         mask_node.push_back(JSONNode("nm",hole_layer->mp[i]->nm));
                         
                         maskproperties.push_back(mask_node);
                         

                     }layerprop.push_back(JSONNode("parent",layer->ind));
                     layerprop.push_back(maskproperties);
                     
                     JSONNode layer_transformprop;
                     layer_transformprop.set_name("ks");
                     
                     //POSITION
                     
                     std::uint32_t p_size=layer->ks.p.size();
                     
                     if(p_size==1){
                         JSONNode position;
                         position.set_name("p");
                         position.push_back(JSONNode("a",layer->ks.p[p_size-1].a));
                         JSONNode value_p(JSON_ARRAY);
                         value_p.set_name("k");
                         value_p.push_back(JSONNode("",layer->ks.p[p_size-1].k[0]));
                         value_p.push_back(JSONNode("",layer->ks.p[p_size-1].k[1]));
                         value_p.push_back(JSONNode("",layer->ks.p[p_size-1].k[2]));
                         
                         position.push_back(value_p);
                         position.push_back(JSONNode("ix",layer->ks.p[p_size-1].ix));
                         layer_transformprop.push_back(position);
                     }
                     
                     else
                     {
                         JSONNode position;
                         position.set_name("p");
                         position.push_back(JSONNode("a",layer->ks.p[p_size-1].a));
                         JSONNode value_p(JSON_ARRAY);
                         value_p.set_name("k");
                         for(int i=0;i<p_size-1;i++)
                         {
                             JSONNode pos_node;
                             /*    JSONNode in_value;
                              in_value.set_name("i");
                              in_value.push_back(JSONNode("x",layer->ks.p[i].offset.i.x));
                              in_value.push_back(JSONNode("y",layer->ks.p[i].offset.i.y));
                              JSONNode out_value;
                              out_value.set_name("o");
                              out_value.push_back(JSONNode("x",layer->ks.p[i].offset.o.x));
                              out_value.push_back(JSONNode("y",layer->ks.p[i].offset.o.y));
                              pos_node.push_back(in_value);
                              pos_node.push_back(out_value);*/
                             pos_node.push_back(JSONNode("t",layer->ks.p[i].offset.time));
                             JSONNode start_value(JSON_ARRAY);
                             start_value.set_name("s");
                             start_value.push_back(JSONNode("",layer->ks.p[i].offset.start[0]));
                             start_value.push_back(JSONNode("",layer->ks.p[i].offset.start[1]));
                             // start_value.push_back(JSONNode("",layer->ks.p[i].offset.start[2]));
                             pos_node.push_back(start_value);
                             pos_node.push_back(JSONNode("h",layer->ks.p[i].offset.h));
                             /*   JSONNode ti(JSON_ARRAY);
                              ti.set_name("ti");
                              ti.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.ti.x));
                              ti.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.ti.y));
                              JSONNode to(JSON_ARRAY);
                              to.set_name("to");
                              to.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.to.x));
                              to.push_back(JSONNode("",layer->ks.p[i].multi_keyframe.to.y));
                              pos_node.push_back(to);
                              pos_node.push_back(ti);*/
                             value_p.push_back(pos_node);
                         }
                         JSONNode last_pos;
                         last_pos.push_back(JSONNode("t",layer->ks.p[p_size-1].offset.time));
                         JSONNode start_value(JSON_ARRAY);
                         start_value.set_name("s");
                         start_value.push_back(JSONNode("",layer->ks.p[p_size-1].k[0]));
                         start_value.push_back(JSONNode("",layer->ks.p[p_size-1].k[1]));
                         //start_value.push_back(JSONNode("",layer->ks.p[p_size-1].k[2]));
                         last_pos.push_back(JSONNode("h",layer->ks.p[p_size-1].offset.h));
                         last_pos.push_back(start_value);
                         value_p.push_back(last_pos);
                         position.push_back(value_p);
                         position.push_back(JSONNode("ix",layer->ks.p[p_size-1].ix));
                         layer_transformprop.push_back(position);
                         
                         
                     }
                     
                     //ANCHORPOINT
                     std::uint32_t a_size=layer->ks.a.size();
                     if(a_size==1)
                     {
                         JSONNode anchorpoint;
                         anchorpoint.set_name("a");
                         anchorpoint.push_back(JSONNode("a",layer->ks.a[a_size-1].a));
                         JSONNode value_a(JSON_ARRAY);
                         value_a.set_name("k");
                         value_a.push_back(JSONNode("",layer->ks.a[a_size-1].k[0]));
                         value_a.push_back(JSONNode("",layer->ks.a[a_size-1].k[1]));
                         value_a.push_back(JSONNode("",layer->ks.a[a_size-1].k[2]));
                         anchorpoint.push_back(value_a);
                         anchorpoint.push_back(JSONNode("ix",layer->ks.a[a_size-1].ix));
                         layer_transformprop.push_back(anchorpoint);
                     }
                     
                     //SCALE
                     std::uint32_t s_size=layer->ks.s.size();
                     if(s_size==1)
                     {
                         JSONNode scale;
                         scale.set_name("s");
                         scale.push_back(JSONNode("a",layer->ks.s[s_size-1].a));
                         JSONNode value_s(JSON_ARRAY);
                         value_s.set_name("k");
                         value_s.push_back(JSONNode("",layer->ks.s[s_size-1].k[0]));
                         value_s.push_back(JSONNode("",layer->ks.s[s_size-1].k[1]));
                         value_s.push_back(JSONNode("",layer->ks.s[s_size-1].k[2]));
                         scale.push_back(value_s);
                         scale.push_back(JSONNode("ix",layer->ks.s[s_size-1].ix));
                         layer_transformprop.push_back(scale);
                     }
                     else
                     {
                         JSONNode scale;
                         scale.set_name("s");
                         scale.push_back(JSONNode("a",layer->ks.s[s_size-1].a));
                         JSONNode value_s(JSON_ARRAY);
                         value_s.set_name("k");
                         for(int i=0;i<s_size-1;i++)
                         {
                             JSONNode scale_node;
                             JSONNode in_value;
                             /*   in_value.set_name("i");
                              in_value.push_back(JSONNode("x",layer->ks.s[i].offset.i.x));
                              in_value.push_back(JSONNode("y",layer->ks.s[i].offset.i.y));
                              JSONNode out_value;
                              out_value.set_name("o");
                              out_value.push_back(JSONNode("x",layer->ks.s[i].offset.o.x));
                              out_value.push_back(JSONNode("y",layer->ks.s[i].offset.o.y));
                              scale_node.push_back(in_value);
                              scale_node.push_back(out_value);*/
                             scale_node.push_back(JSONNode("t",layer->ks.s[i].offset.time));
                             JSONNode start_value(JSON_ARRAY);
                             start_value.set_name("s");
                             start_value.push_back(JSONNode("",layer->ks.s[i].offset.start[0]));
                             start_value.push_back(JSONNode("",layer->ks.s[i].offset.start[1]));
                             //start_value.push_back(JSONNode("",layer->ks.s[i].offset.start[2]));
                             scale_node.push_back(start_value);
                             scale_node.push_back(JSONNode("h",layer->ks.s[i].offset.h));
                             value_s.push_back(scale_node);
                             
                         }
                         JSONNode last_scale;
                         last_scale.push_back(JSONNode("t",layer->ks.s[s_size-1].offset.time));
                         JSONNode start_value(JSON_ARRAY);
                         start_value.set_name("s");
                         start_value.push_back(JSONNode("",layer->ks.s[s_size-1].k[0]));
                         start_value.push_back(JSONNode("",layer->ks.s[s_size-1].k[1]));
                         // start_value.push_back(JSONNode("",layer->ks.s[s_size-1].k[2]));
                         last_scale.push_back(start_value);
                         last_scale.push_back(JSONNode("h",layer->ks.s[s_size-1].offset.h));
                         value_s.push_back(last_scale);
                         scale.push_back(value_s);
                         
                         scale.push_back(JSONNode("ix",layer->ks.s[s_size-1].ix));
                         layer_transformprop.push_back(scale);
                         
                         
                     }
                     
                     
                     //ROTATION
                     std::uint32_t r_size=layer->ks.r.size();
                     if(r_size==1)
                     {
                         JSONNode rotation;
                         rotation.set_name("r");
                         rotation.push_back(JSONNode("a",layer->ks.r[r_size-1].a));
                         rotation.push_back(JSONNode("k",layer->ks.r[r_size-1].k));
                         rotation.push_back(JSONNode("ix",layer->ks.r[r_size-1].ix));
                         layer_transformprop.push_back(rotation);
                     }
                     
                     else
                     {
                         JSONNode rotation;
                         rotation.set_name("r");
                         rotation.push_back(JSONNode("a",layer->ks.r[r_size-1].a));
                         JSONNode value_r(JSON_ARRAY);
                         value_r.set_name("k");
                         for(int i=0;i<r_size-1;i++)
                         {
                             JSONNode rot_node;
                             /*   JSONNode in_value;
                              in_value.set_name("i");
                              in_value.push_back(JSONNode("x",layer->ks.r[i].offset.i.x));
                              in_value.push_back(JSONNode("y",layer->ks.r[i].offset.i.y));
                              JSONNode out_value;
                              out_value.set_name("o");
                              out_value.push_back(JSONNode("x",layer->ks.r[i].offset.o.x));
                              out_value.push_back(JSONNode("y",layer->ks.r[i].offset.o.y));
                              rot_node.push_back(in_value);
                              rot_node.push_back(out_value);*/
                             rot_node.push_back(JSONNode("t",layer->ks.r[i].offset.time));
                             JSONNode start_value(JSON_ARRAY);
                             start_value.set_name("s");
                             start_value.push_back(JSONNode("",layer->ks.r[i].offset.start[0]));
                             rot_node.push_back(start_value);
                             rot_node.push_back(JSONNode("h",layer->ks.r[i].offset.h));
                             value_r.push_back(rot_node);
                         }
                         JSONNode last_pos;
                         last_pos.push_back(JSONNode("t",layer->ks.r[r_size-1].offset.time));
                         JSONNode start_value(JSON_ARRAY);
                         start_value.set_name("s");
                         start_value.push_back(JSONNode("",layer->ks.r[r_size-1].k));
                         last_pos.push_back(start_value);
                         last_pos.push_back(JSONNode("h",layer->ks.r[r_size-1].offset.h));
                         value_r.push_back(last_pos);
                         rotation.push_back(value_r);
                         rotation.push_back(JSONNode("ix",layer->ks.r[r_size-1].ix));
                         layer_transformprop.push_back(rotation);
                     }
                     
                     
                     //OPACITY
                     std::uint32_t o_size=layer->ks.o.size();
                     if(o_size==1)
                     {
                         JSONNode opacity;
                         opacity.set_name("o");
                         opacity.push_back(JSONNode("a",layer->ks.o[o_size-1].a));
                         opacity.push_back(JSONNode("k",layer->ks.o[o_size-1].k));
                         opacity.push_back(JSONNode("ix",layer->ks.o[o_size-1].ix));
                         layer_transformprop.push_back(opacity);
                     }
                     layerprop.push_back(layer_transformprop);
                    
             
                 } m_layers->push_back(layerprop);}
  
        }
             
         }
         return FCM_SUCCESS;
     }

         
         
         
     
                                                         
                                                         
        FCM::Result JSONOutputWriter::AddGroup(int resourceid)
     {
         //std::cout<<"ENTERED group"<<std::endl;
         if(m_group)
             delete m_group;
         m_group=new JSONNode(JSON_ARRAY);
         
         m_group->set_name("shapes");
        // std::cout<<"layer number"<<index+1<<std::endl;
         std::uint32_t size=m_LottieManager->GetNumofGroups(resourceid);
         //std:: cout<<size<<std::endl;
         for(int ind=size-1;ind>=0;ind--)
         {
             group * gr = m_LottieManager->GetGroupAtIndex(ind,resourceid);
             JSONNode groupprop;
             /*std::  cout<<gr->fl.isfilled<<std::endl;*/
           
             if(gr!=NULL ){
                groupprop.push_back(JSONNode("ty",gr->ty));
                AddItems(gr);
                 groupprop.push_back(*m_items);
                 groupprop.push_back(JSONNode("nm",gr->nm));
                 groupprop.push_back(JSONNode("mn",gr->mn));
                 groupprop.push_back(JSONNode("np",gr->np));
                 groupprop.push_back(JSONNode("cix",gr->cix));
                 groupprop.push_back(JSONNode("bm",gr->bm));
                 groupprop.push_back(JSONNode("ix",gr->ix));
                 groupprop.push_back(JSONNode("hd",gr->hd));
                 
                 m_group->push_back(groupprop);
              //   delete gr;
                 
         }
           
             
        
             
         }
         
           return FCM_SUCCESS;
                                                             
    }
                                                         
    FCM::Result JSONOutputWriter::AddItems(struct group * gr)
        {//Adding sh
            //std::cout<<"ENTERED items"<<std::endl;
            m_items=new JSONNode(JSON_ARRAY);
            m_items->set_name("it");
           if(m_inv)
                delete m_inv;
            m_inv=new JSONNode(JSON_ARRAY);
            
            m_inv->set_name("i");
            if(m_outv)
                delete m_outv;
            m_outv=new JSONNode(JSON_ARRAY);
            m_outv->set_name("o");
            if(m_cv)
                delete m_cv;
            m_cv=new JSONNode(JSON_ARRAY);
         /*   if(m_cv2)
                delete m_cv2;
            m_cv2=new JSONNode(JSON_ARRAY);*/
            m_cv->set_name("v");
           // m_cv2->set_name("v2");
            if(gr->r.isrect == true)
            {
                JSONNode itemproprect;
                itemproprect.push_back(JSONNode("ty",gr->r.ty));
                itemproprect.push_back(JSONNode("nm",gr->r.nm));
                itemproprect.push_back(JSONNode("mn",gr->r.mn));
                itemproprect.push_back(JSONNode("hd",gr->r.hd));
                itemproprect.push_back(JSONNode("d",gr->r.d));
                JSONNode size;
                size.set_name("s");
                size.push_back(JSONNode("a",gr->r.s.a));
                JSONNode size_k(JSON_ARRAY);
                size_k.set_name("k");
                size_k.push_back(JSONNode("",gr->r.s.k[0]));
                size_k.push_back(JSONNode("",gr->r.s.k[1]));
     
                size.push_back(size_k);
                size.push_back(JSONNode("ix",gr->r.s.ix));
                itemproprect.push_back(size);
                
                JSONNode position;
                position.set_name("p");
                position.push_back(JSONNode("a",gr->r.rc.a));
                JSONNode position_k(JSON_ARRAY);
                position_k.set_name("k");
                position_k.push_back(JSONNode("",gr->r.rc.k[0]));
                position_k.push_back(JSONNode("",gr->r.rc.k[1]));
                
                position.push_back(position_k);
                position.push_back(JSONNode("ix",gr->r.rc.ix));
                itemproprect.push_back(position);
                
                JSONNode rounded_corners;
                rounded_corners.set_name("r");
                rounded_corners.push_back(JSONNode("a",gr->r.r.a));
            
                rounded_corners.push_back(JSONNode("k",gr->r.r.k));
                rounded_corners.push_back(JSONNode("ix",gr->r.r.ix));
                itemproprect.push_back(rounded_corners);
                
            
                m_items->push_back(itemproprect);
                
            }
            else
            {
            JSONNode itempropsh;
            itempropsh.push_back(JSONNode("ty",gr->sh.ty));
            itempropsh.push_back(JSONNode("nm",gr->sh.nm));
            itempropsh.push_back(JSONNode("mn",gr->sh.mn));
            itempropsh.push_back(JSONNode("hd",gr->sh.hd));
            itempropsh.push_back(JSONNode("ind","0"));
            itempropsh.push_back(JSONNode("ix",gr->sh.ix));
            
            JSONNode ks;
            ks.set_name("ks");
            ks.push_back(JSONNode("a",gr->sh.shp.a));
            
            JSONNode k;
            k.set_name("k");
            
            std::uint32_t size=gr->sh.shp.i.size();
            
            for(int index=0;index<size;index++)
            {
                JSONNode inv(JSON_ARRAY);
             
                inv.push_back(JSONNode("",gr->sh.shp.i[index].x));
                inv.push_back(JSONNode("",gr->sh.shp.i[index].y));
                JSONNode outv(JSON_ARRAY);
           
                outv.push_back(JSONNode("",gr->sh.shp.o[index].x));
                outv.push_back(JSONNode("",gr->sh.shp.o[index].y));
                JSONNode cv(JSON_ARRAY);
        
                cv.push_back(JSONNode("",gr->sh.shp.v[index].x));
                cv.push_back(JSONNode("",gr->sh.shp.v[index].y));

                m_inv->push_back(inv);
                m_outv->push_back(outv);
                m_cv->push_back(cv);
                
            }
             m_inv->set_name("i");
             k.push_back(*m_inv);
             k.push_back(*m_outv);
             k.push_back(*m_cv);
            
            k.push_back(JSONNode("c",gr->sh.shp.c));
            ks.push_back(k);
            ks.push_back(JSONNode("ix",gr->sh.shp.ix));
            itempropsh.push_back(ks);
            m_items->push_back(itempropsh);
            }
        
            //STROKE FILLING
            
            if(gr->st.hasstroke){
                JSONNode itempropst;
                
                if(gr->st.issolid){
                    itempropst.push_back(JSONNode("ty",gr->st.solid.ty));
                    JSONNode opacity;
                    opacity.set_name("o");
                    opacity.push_back(JSONNode("a",gr->st.solid.o.a));
                    opacity.push_back(JSONNode("k",gr->st.solid.o.k));
                    opacity.push_back(JSONNode("ix",gr->st.solid.o.ix));
                    itempropst.push_back(opacity);
                    
                    JSONNode width;
                    width.set_name("w");
                    width.push_back(JSONNode("a",gr->st.solid.w.a));
                    width.push_back(JSONNode("k",gr->st.solid.w.k));
                    width.push_back(JSONNode("ix",gr->st.solid.w.ix));
                    itempropst.push_back(width);
                    
                    
                    itempropst.push_back(JSONNode("lc",gr->st.solid.lc));
                    itempropst.push_back(JSONNode("lj",gr->st.solid.lj));
                    if(gr->st.solid.lj==1)
                        itempropst.push_back(JSONNode("ml",gr->st.solid.ml));
                    itempropst.push_back(JSONNode("bm",gr->st.solid.bm));
                    itempropst.push_back(JSONNode("nm",gr->st.solid.nm));
                    itempropst.push_back(JSONNode("mn",gr->st.solid.mn));
                    itempropst.push_back(JSONNode("hd",gr->st.solid.hd));
                    
                    
                    JSONNode colornode;
                    colornode.set_name("c");
                    colornode.push_back(JSONNode("a",gr->st.solid.color1.a));
                    JSONNode color(JSON_ARRAY);
                    color.set_name("k");
                    color.push_back(JSONNode("",gr->st.solid.color1.r));
                    color.push_back(JSONNode("",gr->st.solid.color1.g));
                    color.push_back(JSONNode("",gr->st.solid.color1.b));
                    color.push_back(JSONNode("",gr->st.solid.color1.alpha));
                    colornode.push_back(color);
                    colornode.push_back(JSONNode("ix",gr->st.solid.color1.ix));
                    itempropst.push_back(colornode);
                    
                }
                m_items->push_back(itempropst);
            }
            
            
            //Adding fl
            if(gr->fl.isfilled){
                if(gr->fl.issolid){
            JSONNode itempropfl;
            itempropfl.push_back(JSONNode("ty",gr->fl.solid.ty));
            JSONNode colornode;
                colornode.set_name("c");
            colornode.push_back(JSONNode("a",gr->fl.solid.color1.a));
            JSONNode color(JSON_ARRAY);
            color.set_name("k");
            color.push_back(JSONNode("",gr->fl.solid.color1.r));
            color.push_back(JSONNode("",gr->fl.solid.color1.g));
            color.push_back(JSONNode("",gr->fl.solid.color1.b));
            color.push_back(JSONNode("",gr->fl.solid.color1.alpha));
            colornode.push_back(color);
            colornode.push_back(JSONNode("ix",gr->fl.solid.color1.ix));
            itempropfl.push_back(colornode);
            
                JSONNode opacity;
                opacity.set_name("o");
                opacity.push_back(JSONNode("a",gr->fl.solid.o.a));
                opacity.push_back(JSONNode("k",gr->fl.solid.o.k));
                opacity.push_back(JSONNode("ix",gr->fl.solid.o.ix));
                itempropfl.push_back(opacity);
            itempropfl.push_back(JSONNode("r",gr->fl.solid.r));
            itempropfl.push_back(JSONNode("bm",gr->fl.solid.bm));
            itempropfl.push_back(JSONNode("nm",gr->fl.solid.nm));
            itempropfl.push_back(JSONNode("mn",gr->fl.solid.mn));
            itempropfl.push_back(JSONNode("hd",gr->fl.solid.hd));
                m_items->push_back(itempropfl);
                }
                
                //linear gradient
                
        else if(gr->fl.islinear_gradient){
                    JSONNode itempropfl;
                    itempropfl.push_back(JSONNode("ty",gr->fl.linear.ty));
                    
                    JSONNode opacity;
                    opacity.set_name("o");
                    opacity.push_back(JSONNode("a",gr->fl.linear.o.a));
                    opacity.push_back(JSONNode("k",gr->fl.linear.o.k));
                    opacity.push_back(JSONNode("ix",gr->fl.linear.o.ix));
                    itempropfl.push_back(opacity);
                    
                    itempropfl.push_back(JSONNode("r",gr->fl.linear.r));
            
                    itempropfl.push_back(JSONNode("nm",gr->fl.linear.nm));
            
            JSONNode start_point;
            start_point.set_name("s");
            start_point.push_back(JSONNode("a",gr->fl.linear.s.a));
            
            JSONNode start_point_value(JSON_ARRAY);
            start_point_value.set_name("k");

            start_point_value.push_back(JSONNode("",gr->fl.linear.s.k[0]));
            start_point_value.push_back(JSONNode("",gr->fl.linear.s.k[1]));
            
            start_point.push_back(start_point_value);
            start_point.push_back(JSONNode("ix",gr->fl.linear.s.ix));
            itempropfl.push_back(JSONNode("t",gr->fl.linear.type));
            itempropfl.push_back(start_point);
            itempropfl.push_back(JSONNode("bm",gr->fl.linear.bm));
            
            JSONNode gradient_color;
            gradient_color.set_name("g");
            gradient_color.push_back(JSONNode("p",gr->fl.linear.g.p));
            
            JSONNode gradient_color_values;
            gradient_color_values.set_name("k");
            gradient_color_values.push_back(JSONNode("a",gr->fl.linear.g.k.a));
            
            JSONNode g_color(JSON_ARRAY);
            g_color.set_name("k");
            for(int i=0;i<(4*(gr->fl.linear.g.p));i++)
            {
                g_color.push_back(JSONNode("",gr->fl.linear.g.k.color[i]));
            }
            gradient_color_values.push_back(g_color);
            gradient_color_values.push_back(JSONNode("ix",gr->fl.linear.g.k.ix));
            gradient_color.push_back(gradient_color_values);
            itempropfl.push_back(gradient_color);
            
            
            JSONNode end_point;
            end_point.set_name("e");
            end_point.push_back(JSONNode("a",gr->fl.linear.e.a));
            
            JSONNode end_point_value(JSON_ARRAY);
            end_point_value.set_name("k");
            end_point_value.push_back(JSONNode("",gr->fl.linear.e.k[0]));
            end_point_value.push_back(JSONNode("",gr->fl.linear.e.k[1]));
            
            end_point.push_back(end_point_value);
            end_point.push_back(JSONNode("ix",gr->fl.linear.e.ix));
            
            itempropfl.push_back(end_point);
            itempropfl.push_back(JSONNode("mn",gr->fl.linear.mn));
            itempropfl.push_back(JSONNode("hd",gr->fl.linear.hd));
            m_items->push_back(itempropfl);
                }
                
                
              //radial gradient
                
                else if(gr->fl.isradial_gradient){
                    JSONNode itempropfl;
                    itempropfl.push_back(JSONNode("ty",gr->fl.radial.radial_fill.ty));
                    
                    JSONNode opacity;
                    opacity.set_name("o");
                    opacity.push_back(JSONNode("a",gr->fl.radial.radial_fill.o.a));
                    opacity.push_back(JSONNode("k",gr->fl.radial.radial_fill.o.k));
                    opacity.push_back(JSONNode("ix",gr->fl.radial.radial_fill.o.ix));
                    itempropfl.push_back(opacity);
                    
                    itempropfl.push_back(JSONNode("r",gr->fl.radial.radial_fill.r));
                    
                    itempropfl.push_back(JSONNode("nm",gr->fl.radial.radial_fill.nm));
                    
                    JSONNode start_point;
                    start_point.set_name("s");
                    start_point.push_back(JSONNode("a",gr->fl.radial.radial_fill.s.a));
                    
                    JSONNode start_point_value(JSON_ARRAY);
                    start_point_value.set_name("k");
                    
                    start_point_value.push_back(JSONNode("",gr->fl.radial.radial_fill.s.k[0]));
                    start_point_value.push_back(JSONNode("",gr->fl.radial.radial_fill.s.k[1]));
                    
                    start_point.push_back(start_point_value);
                    start_point.push_back(JSONNode("ix",gr->fl.radial.radial_fill.s.ix));
                    itempropfl.push_back(JSONNode("t",gr->fl.radial.radial_fill.type));
                    itempropfl.push_back(start_point);
                    itempropfl.push_back(JSONNode("bm",gr->fl.radial.radial_fill.bm));
                    
                    JSONNode gradient_color;
                    gradient_color.set_name("g");
                    gradient_color.push_back(JSONNode("p",gr->fl.radial.radial_fill.g.p));
                    
                    JSONNode gradient_color_values;
                    gradient_color_values.set_name("k");
         gradient_color_values.push_back(JSONNode("a",gr->fl.radial.radial_fill.g.k.a));
                    
                    JSONNode g_color(JSON_ARRAY);
                    g_color.set_name("k");
                    for(int i=0;i<(4*(gr->fl.radial.radial_fill.g.p));i++)
                    {
                        g_color.push_back(JSONNode("",gr->fl.radial.radial_fill.g.k.color[i]));
                    }
                    gradient_color_values.push_back(g_color);
                    gradient_color_values.push_back(JSONNode("ix",gr->fl.radial.radial_fill.g.k.ix));
                    gradient_color.push_back(gradient_color_values);
                    itempropfl.push_back(gradient_color);
                    
                    
                    JSONNode end_point;
                    end_point.set_name("e");
                    end_point.push_back(JSONNode("a",gr->fl.radial.radial_fill.e.a));
                    
                    JSONNode end_point_value(JSON_ARRAY);
                    end_point_value.set_name("k");
                    end_point_value.push_back(JSONNode("",gr->fl.radial.radial_fill.e.k[0]));
                    end_point_value.push_back(JSONNode("",gr->fl.radial.radial_fill.e.k[1]));
                    
                    end_point.push_back(end_point_value);
                    end_point.push_back(JSONNode("ix",gr->fl.radial.radial_fill.e.ix));
                    
                    itempropfl.push_back(end_point);
                    
                    JSONNode highlight_length;
                    highlight_length.set_name("h");
                    highlight_length.push_back(JSONNode("a",gr->fl.radial.h.a));
                    highlight_length.push_back(JSONNode("k",gr->fl.radial.h.k));
                   highlight_length.push_back(JSONNode("ix",gr->fl.radial.h.ix));
                    itempropfl.push_back(highlight_length);
                    
                    
                    
                    JSONNode highlight_angle;
                    highlight_angle.set_name("a");
                    highlight_angle.push_back(JSONNode("a",gr->fl.radial.a.a));
                    highlight_angle.push_back(JSONNode("k",gr->fl.radial.a.k));
                    highlight_angle.push_back(JSONNode("ix",gr->fl.radial.a.ix));
                    itempropfl.push_back(highlight_angle);
                    
                    
                    itempropfl.push_back(JSONNode("mn",gr->fl.radial.radial_fill.mn));
                    itempropfl.push_back(JSONNode("hd",gr->fl.radial.radial_fill.hd));
                    m_items->push_back(itempropfl);
                }
            
            }
            
     
            
            JSONNode itemproptr;
            itemproptr.push_back(JSONNode("ty","tr"));
            
            //POSITION
            
            std::uint32_t p_size=gr->ks.p.size();
            if(p_size==1){
            JSONNode position;
            position.set_name("p");
            position.push_back(JSONNode("a",gr->ks.p[p_size-1].a));
            JSONNode value_p(JSON_ARRAY);
            value_p.set_name("k");
            value_p.push_back(JSONNode("",gr->ks.p[p_size-1].k[0]));
            value_p.push_back(JSONNode("",gr->ks.p[p_size-1].k[1]));
            position.push_back(value_p);
            position.push_back(JSONNode("ix",gr->ks.p[p_size-1].ix));
            itemproptr.push_back(position);
            }
            
            //ANCHOR POINT
            
            std::uint32_t a_size=gr->ks.a.size();
            if(a_size==1){
            JSONNode anchorpoint;
            anchorpoint.set_name("a");
            anchorpoint.push_back(JSONNode("a",gr->ks.a[a_size-1].a));
            JSONNode value_a(JSON_ARRAY);
            value_a.set_name("k");
            value_a.push_back(JSONNode("",gr->ks.a[a_size-1].k[0]));
            value_a.push_back(JSONNode("",gr->ks.a[a_size-1].k[1]));
            anchorpoint.push_back(value_a);
            anchorpoint.push_back(JSONNode("ix",gr->ks.a[a_size-1].ix));
            itemproptr.push_back(anchorpoint);
            }
            
            //SCALE
            
            std::uint32_t s_size=gr->ks.s.size();
            if(s_size==1)
            {
            JSONNode scale;
            scale.set_name("s");
            scale.push_back(JSONNode("a",gr->ks.s[s_size-1].a));
            JSONNode value_s(JSON_ARRAY);
            value_s.set_name("k");
            value_s.push_back(JSONNode("",gr->ks.s[s_size-1].k[0]));
            value_s.push_back(JSONNode("",gr->ks.s[s_size-1].k[1]));
            scale.push_back(value_s);
            scale.push_back(JSONNode("ix",gr->ks.s[s_size-1].ix));
            itemproptr.push_back(scale);
            }
            
            //ROTATION
            
            std::uint32_t r_size=gr->ks.r.size();
            if(r_size==1)
            {
            JSONNode rotation;
            rotation.set_name("r");
            rotation.push_back(JSONNode("a",gr->ks.r[r_size-1].a));
            rotation.push_back(JSONNode("k",gr->ks.r[r_size-1].k));
            rotation.push_back(JSONNode("ix",gr->ks.r[r_size-1].ix));
            itemproptr.push_back(rotation);
            }
            
            //OPACITY
            
            std::uint32_t o_size=gr->ks.o.size();
            if(o_size==1)
            {
            JSONNode opacity;
            opacity.set_name("o");
            opacity.push_back(JSONNode("a",gr->ks.o[o_size-1].a));
            opacity.push_back(JSONNode("k",gr->ks.o[o_size-1].k));
            opacity.push_back(JSONNode("ix",gr->ks.o[o_size-1].ix));
            itemproptr.push_back(opacity);
            }
            
            //SKEW
            
            std::uint32_t sk_size=gr->ks.sk.size();
            if(sk_size==1)
            {
            JSONNode skew;
            skew.set_name("sk");
            skew.push_back(JSONNode("a",gr->ks.sk[sk_size-1].a));
            skew.push_back(JSONNode("k",gr->ks.sk[sk_size-1].k));
            skew.push_back(JSONNode("ix",gr->ks.sk[sk_size-1].ix));
            itemproptr.push_back(skew);
            }
            
            
            //SKEW AXIS
            
            std::uint32_t sa_size=gr->ks.sa.size();
            if(sa_size==1)
            {
            JSONNode skewaxis;
            skewaxis.set_name("sa");
            skewaxis.push_back(JSONNode("a",gr->ks.sa[sa_size-1].a));
            skewaxis.push_back(JSONNode("k",gr->ks.sa[sa_size-1].k));
            skewaxis.push_back(JSONNode("ix",gr->ks.sa[sa_size-1].ix));
            itemproptr.push_back(skewaxis);
            }
            
            itemproptr.push_back(JSONNode("nm","Transform"));
            m_items->push_back(itemproptr);
            return FCM_SUCCESS;
                                                             
        }
       
                                                    
	FCM::Result JSONOutputWriter::AddVersion(JSONNode &firstNode)
                                                         {// std::cout<<"ENTERED VERSION"<<std::endl;
		firstNode.push_back(JSONNode("v", m_LottieManager->GetVersion()));
		return FCM_SUCCESS;
	}
	FCM::Result JSONOutputWriter::AddAssets(JSONNode &firstNode)
	{
      //  std::cout<<"ENTERED assets"<<std::endl;
        if(m_assets)
        firstNode.push_back(*m_assets);
		return FCM_SUCCESS;
        
	}
	FCM::Result JSONOutputWriter::AddMarkers(JSONNode &firstNode)
	{
       // std::cout<<"ENTERED markers"<<std::endl;
		JSONNode c = JSONNode(JSON_ARRAY);
		c.set_name("markers");
		firstNode.push_back(c);
		return FCM_SUCCESS;
	}
    
	/*FCM::Result JSONOutputWriter::AddOp()
	{

	}*/
	
	FCM::Result JSONOutputWriter::AddWidthHeight(JSONNode &firstNode)
	{
		int width; int height;
        //std::cout<<"ENTERED w and h"<<std::endl;
		m_LottieManager-> GetStageWidthHeight(  width, height);
		firstNode.push_back(JSONNode("w", width));
		firstNode.push_back(JSONNode("h", height));
		return FCM_SUCCESS;
	}
	

    FCM::Result JSONOutputWriter::StartDefineTimeline()
    {
        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::EndDefineTimeline(
        FCM::U_Int32 resId, 
        FCM::StringRep16 pName,
        ITimelineWriter* pTimelineWriter)
    {
        JSONTimelineWriter* pWriter = static_cast<JSONTimelineWriter*> (pTimelineWriter);

        pWriter->Finish(resId, pName);

        m_pTimelineArray->push_back(*(pWriter->GetRoot()));

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::StartDefineShape()
    {
        m_shapeElem = new JSONNode(JSON_NODE);
        ASSERT(m_shapeElem);

        m_pathArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pathArray);
        m_pathArray->set_name("path");

        return FCM_SUCCESS;
    }


    // Marks the end of a shape
    FCM::Result JSONOutputWriter::EndDefineShape(FCM::U_Int32 resId)
    {
        m_shapeElem->push_back(JSONNode(("charid"), LottieExporter::Utils::ToString(resId)));
        m_shapeElem->push_back(*m_pathArray);

        m_pShapeArray->push_back(*m_shapeElem);

        delete m_pathArray;
        delete m_shapeElem;

        return FCM_SUCCESS;
    }


    // Start of fill region definition
    FCM::Result JSONOutputWriter::StartDefineFill()
    {
        m_pathElem = new JSONNode(JSON_NODE);
        ASSERT(m_pathElem);

        m_pathCmdStr.clear();

        return FCM_SUCCESS;
    }


    // Solid fill style definition
    FCM::Result JSONOutputWriter::DefineSolidFillStyle(const DOM::Utils::COLOR& color)
    {
        std::string colorStr = Utils::ToString(color);
        std::string colorOpacityStr = LottieExporter::Utils::ToString((double)(color.alpha / 255.0));

        m_pathElem->push_back(JSONNode("color", colorStr.c_str()));
        m_pathElem->push_back(JSONNode("colorOpacity", colorOpacityStr.c_str()));

        return FCM_SUCCESS;
    }


    // Bitmap fill style definition
    FCM::Result JSONOutputWriter::DefineBitmapFillStyle(
        FCM::Boolean clipped,
        const DOM::Utils::MATRIX2D& matrix,
        FCM::S_Int32 height, 
        FCM::S_Int32 width,
        const std::string& libPathName,
        DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        FCM::Result res;
        std::string name;
        JSONNode bitmapElem(JSON_NODE);
        std::string bitmapPath;
        std::string bitmapName;

        bitmapElem.set_name("image");
        
        bitmapElem.push_back(JSONNode(("height"), LottieExporter::Utils::ToString(height)));
        bitmapElem.push_back(JSONNode(("width"), LottieExporter::Utils::ToString(width)));

        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        std::string bitmapRelPath;
        std::string bitmapExportPath = m_outputImageFolder + "/";
            
        FCM::Boolean alreadyExported = GetImageExportFileName(libPathName, name);
        if (!alreadyExported)
        {
            if (!m_imageFolderCreated)
            {
                res = Utils::CreateDir(m_outputImageFolder, m_pCallback);
                if (!(FCM_SUCCESS_CODE(res)))
                {
                    Utils::Trace(m_pCallback, "Output image folder (%s) could not be created\n", m_outputImageFolder.c_str());
                    return res;
                }
                m_imageFolderCreated = true;
            }
            CreateImageFileName(libPathName, name);
            SetImageExportFileName(libPathName, name);
        }

        bitmapExportPath += name;
            
        bitmapRelPath = "./";
        bitmapRelPath += IMAGE_FOLDER;
        bitmapRelPath += "/";
        bitmapRelPath += name;

        res = m_pCallback->GetService(DOM::FLA_BITMAP_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::AutoPtr<DOM::Service::Image::IBitmapExportService> bitmapExportService = pUnk;
        if (bitmapExportService)
        {
            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
            FCM::StringRep16 pFilePath = Utils::ToString16(bitmapExportPath, m_pCallback);
            res = bitmapExportService->ExportToFile(pMediaItem, pFilePath, 100);
            ASSERT(FCM_SUCCESS_CODE(res));

            pCalloc =LottieExporter::Utils::GetCallocService(m_pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);

            pCalloc->Free(pFilePath);
        }

        bitmapElem.push_back(JSONNode(("bitmapPath"), bitmapRelPath)); 

        DOM::Utils::MATRIX2D matrix1 = matrix;
        matrix1.a /= 20.0;
        matrix1.b /= 20.0;
        matrix1.c /= 20.0;
        matrix1.d /= 20.0;

        bitmapElem.push_back(JSONNode(("patternUnits"), "userSpaceOnUse"));
        bitmapElem.push_back(JSONNode(("patternTransform"), Utils::ToString(matrix1).c_str()));

        m_pathElem->push_back(bitmapElem);

        return FCM_SUCCESS;
    }


    // Start Linear Gradient fill style definition
    FCM::Result JSONOutputWriter::StartDefineLinearGradientFillStyle(
        DOM::FillStyle::GradientSpread spread,
        const DOM::Utils::MATRIX2D& matrix)
    {
        DOM::Utils::POINT2D point;

        m_gradientColor = new JSONNode(JSON_NODE);
        ASSERT(m_gradientColor);
        m_gradientColor->set_name("linearGradient");

        point.x = -GRADIENT_VECTOR_CONSTANT / 20;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point);

        m_gradientColor->push_back(JSONNode("x1", Utils::ToString((double) (point.x))));
        m_gradientColor->push_back(JSONNode("y1", Utils::ToString((double) (point.y))));

        point.x = GRADIENT_VECTOR_CONSTANT / 20;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point);

        m_gradientColor->push_back(JSONNode("x2", Utils::ToString((double) (point.x))));
        m_gradientColor->push_back(JSONNode("y2", Utils::ToString((double) (point.y))));

        m_gradientColor->push_back(JSONNode("spreadMethod", Utils::ToString(spread)));

        m_stopPointArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_stopPointArray);
        m_stopPointArray->set_name("stop");

        return FCM_SUCCESS;
    }


    // Sets a specific key point in a color ramp (for both radial and linear gradient)
    FCM::Result JSONOutputWriter::SetKeyColorPoint(
        const DOM::Utils::GRADIENT_COLOR_POINT& colorPoint)
    {
        JSONNode stopEntry(JSON_NODE);
        FCM::Float offset;
        
        offset = (float)((colorPoint.pos * 100) / 255.0);

        stopEntry.push_back(JSONNode("offset", Utils::ToString((double) offset)));
        stopEntry.push_back(JSONNode("stopColor", Utils::ToString(colorPoint.color)));
        stopEntry.push_back(JSONNode("stopOpacity", Utils::ToString((double)(colorPoint.color.alpha / 255.0))));

        m_stopPointArray->push_back(stopEntry);

        return FCM_SUCCESS;
    }


    // End Linear Gradient fill style definition
    FCM::Result JSONOutputWriter::EndDefineLinearGradientFillStyle()
    {
        m_gradientColor->push_back(*m_stopPointArray);
        m_pathElem->push_back(*m_gradientColor);

        delete m_stopPointArray;
        delete m_gradientColor;

        return FCM_SUCCESS;
    }


    // Start Radial Gradient fill style definition
    FCM::Result JSONOutputWriter::StartDefineRadialGradientFillStyle(
        DOM::FillStyle::GradientSpread spread,
        const DOM::Utils::MATRIX2D& matrix,
        FCM::S_Int32 focalPoint)
    {
        DOM::Utils::POINT2D point;
        DOM::Utils::POINT2D point1;
        DOM::Utils::POINT2D point2;

        m_gradientColor = new JSONNode(JSON_NODE);
        ASSERT(m_gradientColor);
        m_gradientColor->set_name("radialGradient");

        point.x = 0;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point1);

        point.x = GRADIENT_VECTOR_CONSTANT / 20;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point2);

        FCM::Float xd = point1.x - point2.x;
        FCM::Float yd = point1.y - point2.y;
        FCM::Float r = sqrt(xd * xd + yd * yd);

        FCM::Float angle = atan2(yd, xd);
        double focusPointRatio = focalPoint / 255.0;
        double fx = -r * focusPointRatio * cos(angle);
        double fy = -r * focusPointRatio * sin(angle);

        m_gradientColor->push_back(JSONNode("cx", "0"));
        m_gradientColor->push_back(JSONNode("cy", "0"));
        m_gradientColor->push_back(JSONNode("r", Utils::ToString((double) r)));
        m_gradientColor->push_back(JSONNode("fx", Utils::ToString((double) fx)));
        m_gradientColor->push_back(JSONNode("fy", Utils::ToString((double) fy)));

        FCM::Float scaleFactor = (GRADIENT_VECTOR_CONSTANT / 20) / r;
        DOM::Utils::MATRIX2D matrix1 = {};
        matrix1.a = matrix.a * scaleFactor;
        matrix1.b = matrix.b * scaleFactor;
        matrix1.c = matrix.c * scaleFactor;
        matrix1.d = matrix.d * scaleFactor;
        matrix1.tx = matrix.tx;
        matrix1.ty = matrix.ty;

        m_gradientColor->push_back(JSONNode("gradientTransform", Utils::ToString(matrix1)));
        m_gradientColor->push_back(JSONNode("spreadMethod", Utils::ToString(spread)));

        m_stopPointArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_stopPointArray);
        m_stopPointArray->set_name("stop");

        return FCM_SUCCESS;
    }


    // End Radial Gradient fill style definition
    FCM::Result JSONOutputWriter::EndDefineRadialGradientFillStyle()
    {
        m_gradientColor->push_back(*m_stopPointArray);
        m_pathElem->push_back(*m_gradientColor);

        delete m_stopPointArray;
        delete m_gradientColor;

        return FCM_SUCCESS;
    }


    // Start of fill region boundary
    FCM::Result JSONOutputWriter::StartDefineBoundary()
    {
        return StartDefinePath();
    }


    // Sets a segment of a path (Used for boundary, holes)
    FCM::Result JSONOutputWriter::SetSegment(const DOM::Utils::SEGMENT& segment)
    {
        if (m_firstSegment)
        {
            if (segment.segmentType == DOM::Utils::LINE_SEGMENT)
            {
                m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.line.endPoint1.x)));
                m_pathCmdStr.append(space);
                m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.line.endPoint1.y)));
                m_pathCmdStr.append(space);
            }
            else
            {
                m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.quadBezierCurve.anchor1.x)));
                m_pathCmdStr.append(space);
                m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.quadBezierCurve.anchor1.y)));
                m_pathCmdStr.append(space);
            }
            m_firstSegment = false;
        }

        if (segment.segmentType == DOM::Utils::LINE_SEGMENT)
        {
            m_pathCmdStr.append(lineTo);
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.line.endPoint2.x)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.line.endPoint2.y)));
            m_pathCmdStr.append(space);
        }
        else
        {
            m_pathCmdStr.append(bezierCurveTo);
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.quadBezierCurve.control.x)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.quadBezierCurve.control.y)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.quadBezierCurve.anchor2.x)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(LottieExporter::Utils::ToString((double)(segment.quadBezierCurve.anchor2.y)));
            m_pathCmdStr.append(space);
        }

        return FCM_SUCCESS;
    }


    // End of fill region boundary
    FCM::Result JSONOutputWriter::EndDefineBoundary()
    {
        return EndDefinePath();
    }


    // Start of fill region hole
    FCM::Result JSONOutputWriter::StartDefineHole()
    {
        return StartDefinePath();
    }


    // End of fill region hole
    FCM::Result JSONOutputWriter::EndDefineHole()
    {
        return EndDefinePath();
    }


    // Start of stroke group
    FCM::Result JSONOutputWriter::StartDefineStrokeGroup()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }


    // Start solid stroke style definition
    FCM::Result JSONOutputWriter::StartDefineSolidStrokeStyle(
        FCM::Double thickness,
        const DOM::StrokeStyle::JOIN_STYLE& joinStyle,
        const DOM::StrokeStyle::CAP_STYLE& capStyle,
        DOM::Utils::ScaleType scaleType,
        FCM::Boolean strokeHinting)
    {
        m_strokeStyle.type = SOLID_STROKE_STYLE_TYPE;
        m_strokeStyle.solidStrokeStyle.capStyle = capStyle;
        m_strokeStyle.solidStrokeStyle.joinStyle = joinStyle;
        m_strokeStyle.solidStrokeStyle.thickness = thickness;
        m_strokeStyle.solidStrokeStyle.scaleType = scaleType;
        m_strokeStyle.solidStrokeStyle.strokeHinting = strokeHinting;

        return FCM_SUCCESS;
    }


    // End of solid stroke style
    FCM::Result JSONOutputWriter::EndDefineSolidStrokeStyle()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }


    // Start of stroke 
    FCM::Result JSONOutputWriter::StartDefineStroke()
    {
        m_pathElem = new JSONNode(JSON_NODE);
        ASSERT(m_pathElem);

        m_pathCmdStr.clear();
        StartDefinePath();

        return FCM_SUCCESS;
    }


    // End of a stroke 
    FCM::Result JSONOutputWriter::EndDefineStroke()
    {
        m_pathElem->push_back(JSONNode("d", m_pathCmdStr));

        if (m_strokeStyle.type == SOLID_STROKE_STYLE_TYPE)
        {
            m_pathElem->push_back(JSONNode("strokeWidth", LottieExporter::Utils::ToString((double)m_strokeStyle.solidStrokeStyle.thickness).c_str()));
            m_pathElem->push_back(JSONNode("fill", "none"));
            m_pathElem->push_back(JSONNode("strokeLinecap", Utils::ToString(m_strokeStyle.solidStrokeStyle.capStyle.type).c_str()));
            m_pathElem->push_back(JSONNode("strokeLinejoin", Utils::ToString(m_strokeStyle.solidStrokeStyle.joinStyle.type).c_str()));

            if (m_strokeStyle.solidStrokeStyle.joinStyle.type == DOM::Utils::MITER_JOIN)
            {
                m_pathElem->push_back(JSONNode(
                    "stroke-miterlimit", 
                    LottieExporter::Utils::ToString((double)m_strokeStyle.solidStrokeStyle.joinStyle.miterJoinProp.miterLimit).c_str()));
            }
            m_pathElem->push_back(JSONNode("pathType", "Stroke"));
        }
        m_pathArray->push_back(*m_pathElem);

        delete m_pathElem;

        m_pathElem = NULL;

        return FCM_SUCCESS;
    }


    // End of stroke group
    FCM::Result JSONOutputWriter::EndDefineStrokeGroup()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }


    // End of fill style definition
    FCM::Result JSONOutputWriter::EndDefineFill()
    {
        m_pathElem->push_back(JSONNode("d", m_pathCmdStr));
        m_pathElem->push_back(JSONNode("pathType", JSON_TEXT("Fill")));
        m_pathElem->push_back(JSONNode("stroke", JSON_TEXT("none")));

        m_pathArray->push_back(*m_pathElem);

        delete m_pathElem;

        m_pathElem = NULL;
        
        return FCM_SUCCESS;
    }


    // Define a bitmap
    FCM::Result JSONOutputWriter::DefineBitmap(
        FCM::U_Int32 resId,
        FCM::S_Int32 height, 
        FCM::S_Int32 width,
        const std::string& libPathName,
        DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        if(!m_assets)
        {m_assets =new JSONNode(JSON_ARRAY);m_assets->set_name("assets");}
        
        std::map<int , image_resource *>::iterator it;
        std::map<int , image_resource *>image_resource_id=m_LottieManager->Getimageresource_id();
       
            image_resource * image =m_LottieManager->Getimage_resource_with_id(resId);
            JSONNode imagenode;

        
            imagenode.push_back(JSONNode("id",image->ref_id));
            imagenode.push_back(JSONNode("w",width));
            imagenode.push_back(JSONNode("h",height));
            imagenode.push_back(JSONNode("e",0));
        
     FCM::Result res;
        JSONNode bitmapElem(JSON_NODE);
        std::string bitmapPath;
        std::string bitmapName;
        std::string name;

      /*  bitmapElem.set_name("image");
        bitmapElem.push_back(JSONNode(("charid"), LottieExporter::Utils::ToString(resId)));
        bitmapElem.push_back(JSONNode(("height"), LottieExporter::Utils::ToString(height)));
        bitmapElem.push_back(JSONNode(("width"), LottieExporter::Utils::ToString(width)));*/

        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        std::string bitmapRelPath;
        std::string bitmapExportPath = m_outputImageFolder + "/";
            
        FCM::Boolean alreadyExported = GetImageExportFileName(libPathName, name);
        if (!alreadyExported)
        {
            if (!m_imageFolderCreated)
            {
                res = Utils::CreateDir(m_outputImageFolder, m_pCallback);
                if (!(FCM_SUCCESS_CODE(res)))
                {
                    Utils::Trace(m_pCallback, "Output image folder (%s) could not be created\n", m_outputImageFolder.c_str());
                    return res;
                }
                m_imageFolderCreated = true;
            }
            CreateImageFileName(libPathName, name);
            SetImageExportFileName(libPathName, name);
        }
        
        bitmapExportPath += name;
            
        bitmapRelPath = "./";
        bitmapRelPath += IMAGE_FOLDER;
        bitmapRelPath += "/";
        imagenode.push_back(JSONNode("u",bitmapRelPath));
        bitmapRelPath += name;
        imagenode.push_back(JSONNode("p",name));
        

        res = m_pCallback->GetService(DOM::FLA_BITMAP_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::AutoPtr<DOM::Service::Image::IBitmapExportService> bitmapExportService = pUnk;
        if (bitmapExportService)
        {
            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
            FCM::StringRep16 pFilePath = Utils::ToString16(bitmapExportPath, m_pCallback);
            res = bitmapExportService->ExportToFile(pMediaItem, pFilePath, 100);
            ASSERT(FCM_SUCCESS_CODE(res));

            pCalloc = LottieExporter::Utils::GetCallocService(m_pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);

            pCalloc->Free(pFilePath);
        }

        bitmapElem.push_back(JSONNode(("bitmapPath"), bitmapRelPath));
        
        m_assets->push_back(imagenode);


        m_pBitmapArray->push_back(bitmapElem);

        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::DefineText(
            FCM::U_Int32 resId, 
            const std::string& name, 
            const DOM::Utils::COLOR& color, 
            const std::string& displayText, 
            DOM::FrameElement::PIClassicText pTextItem)
    {
        std::string txt = displayText;
        std::string colorStr = Utils::ToString(color);
        std::string find = "\r";
        std::string replace = "\\r";
        std::string::size_type i =0;
        JSONNode textElem(JSON_NODE);

        while (true) {
            /* Locate the substring to replace. */
            i = txt.find(find, i);
           
            if (i == std::string::npos) break;
            /* Make the replacement. */
            txt.replace(i, find.length(), replace);

            /* Advance index forward so the next iteration doesn't pick it up as well. */
            i += replace.length();
        }

        
        textElem.push_back(JSONNode(("charid"), LottieExporter::Utils::ToString(resId)));
        textElem.push_back(JSONNode(("displayText"),txt ));
        textElem.push_back(JSONNode(("font"),name));
        textElem.push_back(JSONNode("color", colorStr.c_str()));

        m_pTextArray->push_back(textElem);

        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::DefineSound(
            FCM::U_Int32 resId, 
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        FCM::Result res;
        JSONNode soundElem(JSON_NODE);
        std::string soundPath;
        std::string soundName;
        std::string name;

        soundElem.set_name("sound");
        soundElem.push_back(JSONNode(("charid"),LottieExporter::Utils::ToString(resId)));
        
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        std::string soundRelPath;
        std::string soundExportPath = m_outputSoundFolder + "/";

        if (!m_soundFolderCreated)
        {
            res = Utils::CreateDir(m_outputSoundFolder, m_pCallback);
            if (!(FCM_SUCCESS_CODE(res)))
            {
                Utils::Trace(m_pCallback, "Output sound folder (%s) could not be created\n", m_outputSoundFolder.c_str());
                return res;
            }
            m_soundFolderCreated = true;
        }
        
        CreateSoundFileName(libPathName, name);
        soundExportPath += name;

        soundRelPath = "./";
        soundRelPath += SOUND_FOLDER;
        soundRelPath += "/";
        soundRelPath += name;

        res = m_pCallback->GetService(DOM::FLA_SOUND_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));
        FCM::AutoPtr<DOM::Service::Sound::ISoundExportService> soundExportService = pUnk;
        if (soundExportService)
        {
            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
            FCM::StringRep16 pFilePath = Utils::ToString16(soundExportPath, m_pCallback);
            res = soundExportService->ExportToFile(pMediaItem, pFilePath);
            ASSERT(FCM_SUCCESS_CODE(res));
            pCalloc = LottieExporter::Utils::GetCallocService(m_pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);
            pCalloc->Free(pFilePath);
        }
        
        soundElem.push_back(JSONNode(("soundPath"), soundRelPath)); 
        m_pSoundArray->push_back(soundElem);
        
        return FCM_SUCCESS;
    }

    JSONOutputWriter::JSONOutputWriter(FCM::PIFCMCallback pCallback)
        : m_pCallback(pCallback),
          m_shapeElem(NULL),
          m_pathArray(NULL),
          m_pathElem(NULL),
          m_firstSegment(false),
          m_HTMLOutput(NULL),
          m_imageFileNameLabel(0),
          m_soundFileNameLabel(0),
          m_imageFolderCreated(false),
          m_soundFolderCreated(false)
    {
        m_pRootNode = new JSONNode(JSON_NODE);
        ASSERT(m_pRootNode);
        m_pRootNode->set_name("DOMDocument");

        m_pShapeArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pShapeArray);
        m_pShapeArray->set_name("Shape");

        m_pTimelineArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pTimelineArray);
        m_pTimelineArray->set_name("Timeline");

        m_pBitmapArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pBitmapArray);
        m_pBitmapArray->set_name("Bitmaps");

        m_pTextArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pTextArray);
        m_pTextArray->set_name("Text");

        m_pSoundArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pSoundArray);
        m_pSoundArray->set_name("Sounds");
        m_strokeStyle.type = INVALID_STROKE_STYLE_TYPE;
    }


    JSONOutputWriter::~JSONOutputWriter()
    {
        delete m_pBitmapArray;
        delete m_pSoundArray;

        delete m_pTimelineArray;

        delete m_pShapeArray;

        delete m_pTextArray;

        delete m_pRootNode;
    }


    FCM::Result JSONOutputWriter::StartDefinePath()
    {
        m_pathCmdStr.append(moveTo);
        m_pathCmdStr.append(space);
        m_firstSegment = true;

        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::EndDefinePath()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::CreateImageFileName(const std::string& libPathName, std::string& name)
    {
        std::string str;
        size_t pos;
        std::string fileLabel;

        fileLabel = Utils::ToString(m_imageFileNameLabel);
        name = "Image" + fileLabel;
        m_imageFileNameLabel++;

        str = libPathName;

        // DOM APIs do not provide a way to get the compression of the image.
        // For time being, we will use the extension of the library item name.
        pos = str.rfind(".");
        if (pos != std::string::npos)
        {
            if (str.substr(pos + 1) == "jpg")
            {
                name += ".jpg";
            }
            else if (str.substr(pos + 1) == "png")
            {
                name += ".png";
            }
            else
            {
                name += ".png";
            }
        }
        else
        {
            name += ".png";
        }

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::CreateSoundFileName(const std::string& libPathName, std::string& name)
    {
        std::string str;
        size_t pos;
        std::string fileLabel;

        fileLabel = Utils::ToString(m_soundFileNameLabel);
        name = "Sound" + fileLabel;
        m_soundFileNameLabel++;

        str = libPathName;

        // DOM APIs do not provide a way to get the compression of the sound.
        // For time being, we will use the extension of the library item name.
        pos = str.rfind(".");
        if (pos != std::string::npos)
        {
            if (str.substr(pos + 1) == "wav")
            {
                name += ".WAV";
            }
            else if (str.substr(pos + 1) == "mp3")
            {
                name += ".MP3";
            }
            else
            {
                name += ".MP3";
            }
        }
        else
        {
            name += ".MP3";
        }

        return FCM_SUCCESS;
    }


    FCM::Boolean JSONOutputWriter::GetImageExportFileName(const std::string& libPathName, std::string& name)
    {
        std::map<std::string, std::string>::iterator it = m_imageMap.find(libPathName);

        name = "";

        if (it != m_imageMap.end())
        {
            // Image already exported
            name = it->second;
            return true;
        }

        return false;
    }


    void JSONOutputWriter::SetImageExportFileName(const std::string& libPathName, const std::string& name)
    {
        // Assumption: Name is not already present in the map
        ASSERT(m_imageMap.find(libPathName) == m_imageMap.end());

        m_imageMap.insert(std::pair<std::string, std::string>(libPathName, name));
    }
    /* -------------------------------------------------- JSONTimelineWriter */

    FCM::Result JSONTimelineWriter::PlaceObject(
        FCM::U_Int32 resId,
        FCM::U_Int32 objectId,
        FCM::U_Int32 placeAfterObjectId,
        const DOM::Utils::MATRIX2D* pMatrix,
        FCM::PIFCMUnknown pUnknown /* = NULL*/)
    {
        JSONNode commandElement(JSON_NODE);

        commandElement.push_back(JSONNode("cmdType", "Place"));
        commandElement.push_back(JSONNode("charid", LottieExporter::Utils::ToString(resId)));
        commandElement.push_back(JSONNode("objectId",LottieExporter::Utils::ToString(objectId)));
        commandElement.push_back(JSONNode("placeAfter", LottieExporter::Utils::ToString(placeAfterObjectId)));

        if (pMatrix)
        {
            commandElement.push_back(JSONNode("transformMatrix", Utils::ToString(*pMatrix).c_str()));
        }

        m_pCommandArray->push_back(commandElement);

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::PlaceObject(
        FCM::U_Int32 resId,
        FCM::U_Int32 objectId,
        FCM::PIFCMUnknown pUnknown /* = NULL*/)
    {
        FCM::Result res;

        JSONNode commandElement(JSON_NODE);
        FCM::AutoPtr<DOM::FrameElement::ISound> pSound;

        commandElement.push_back(JSONNode("cmdType", "Place"));
        commandElement.push_back(JSONNode("charid", LottieExporter::Utils::ToString(resId)));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));

        pSound = pUnknown;
        if (pSound)
        {
            DOM::FrameElement::SOUND_LOOP_MODE lMode;
            DOM::FrameElement::SOUND_LIMIT soundLimit;
            DOM::FrameElement::SoundSyncMode syncMode;

            soundLimit.structSize = sizeof(DOM::FrameElement::SOUND_LIMIT);
            lMode.structSize = sizeof(DOM::FrameElement::SOUND_LOOP_MODE);

            res = pSound->GetLoopMode(lMode);
            ASSERT(FCM_SUCCESS_CODE(res));

            commandElement.push_back(JSONNode("loopMode", 
				LottieExporter::Utils::ToString(lMode.loopMode)));
            commandElement.push_back(JSONNode("repeatCount", 
				LottieExporter::Utils::ToString(lMode.repeatCount)));

            res = pSound->GetSyncMode(syncMode);
            ASSERT(FCM_SUCCESS_CODE(res));

            commandElement.push_back(JSONNode("syncMode", 
				LottieExporter::Utils::ToString(syncMode)));

            // We should not get SOUND_SYNC_STOP as for stop, "RemoveObject" command will
            // be generated by Exporter Service.
            ASSERT(syncMode != DOM::FrameElement::SOUND_SYNC_STOP); 

            res = pSound->GetSoundLimit(soundLimit);
            ASSERT(FCM_SUCCESS_CODE(res));

            commandElement.push_back(JSONNode("LimitInPos44", 
				LottieExporter::Utils::ToString(soundLimit.inPos44)));
            commandElement.push_back(JSONNode("LimitOutPos44", 
				LottieExporter::Utils::ToString(soundLimit.outPos44)));
        }

        m_pCommandArray->push_back(commandElement);

        return res;
    }


    FCM::Result JSONTimelineWriter::RemoveObject(
        FCM::U_Int32 objectId)
    {
        JSONNode commandElement(JSON_NODE);

        commandElement.push_back(JSONNode("cmdType", "Remove"));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));

        m_pCommandArray->push_back(commandElement);

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::UpdateZOrder(
        FCM::U_Int32 objectId,
        FCM::U_Int32 placeAfterObjectId)
    {
        JSONNode commandElement(JSON_NODE);

        commandElement.push_back(JSONNode("cmdType", "UpdateZOrder"));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));
        commandElement.push_back(JSONNode("placeAfter", LottieExporter::Utils::ToString(placeAfterObjectId)));

        m_pCommandArray->push_back(commandElement);

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::UpdateMask(
        FCM::U_Int32 objectId,
        FCM::U_Int32 maskTillObjectId)
    {
        // Commenting out the function since the runtime
        // does not support masking
        /*
        JSONNode commandElement(JSON_NODE);

        commandElement.push_back(JSONNode("cmdType", "UpdateMask"));
        commandElement.push_back(JSONNode("objectId", CreateJS::Utils::ToString(objectId)));
        commandElement.push_back(JSONNode("maskTill", CreateJS::Utils::ToString(maskTillObjectId)));

        m_pCommandArray->push_back(commandElement);
        */
        
        return FCM_SUCCESS;
    }

    FCM::Result JSONTimelineWriter::UpdateBlendMode(
        FCM::U_Int32 objectId,
        DOM::FrameElement::BlendMode blendMode)
    {
        JSONNode commandElement(JSON_NODE);

        commandElement.push_back(JSONNode("cmdType", "UpdateBlendMode"));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));
        if(blendMode == 0)
            commandElement.push_back(JSONNode("blendMode","Normal"));
        else if(blendMode == 1)
            commandElement.push_back(JSONNode("blendMode","Layer"));
        else if(blendMode == 2)
            commandElement.push_back(JSONNode("blendMode","Darken"));
        else if(blendMode == 3)
            commandElement.push_back(JSONNode("blendMode","Multiply"));
        else if(blendMode == 4)
            commandElement.push_back(JSONNode("blendMode","Lighten"));
        else if(blendMode == 5)
            commandElement.push_back(JSONNode("blendMode","Screen"));
        else if(blendMode == 6)
            commandElement.push_back(JSONNode("blendMode","Overlay"));
        else if(blendMode == 7)
            commandElement.push_back(JSONNode("blendMode","Hardlight"));
        else if(blendMode == 8)
            commandElement.push_back(JSONNode("blendMode","Add"));
        else if(blendMode == 9)
            commandElement.push_back(JSONNode("blendMode","Substract"));
        else if(blendMode == 10)
            commandElement.push_back(JSONNode("blendMode","Difference"));
        else if(blendMode == 11)
            commandElement.push_back(JSONNode("blendMode","Invert"));
        else if(blendMode == 12)
            commandElement.push_back(JSONNode("blendMode","Alpha"));
        else if(blendMode == 13)
            commandElement.push_back(JSONNode("blendMode","Erase"));

         m_pCommandArray->push_back(commandElement);
        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::UpdateVisibility(
        FCM::U_Int32 objectId,
        FCM::Boolean visible)
    {
        JSONNode commandElement(JSON_NODE);

        commandElement.push_back(JSONNode("cmdType", "UpdateVisibility"));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));

        if (visible)
        {
            commandElement.push_back(JSONNode("visibility", "true"));
        }
        else
        {
            commandElement.push_back(JSONNode("visibility", "false"));
        }

        m_pCommandArray->push_back(commandElement);

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::AddGraphicFilter(
        FCM::U_Int32 objectId,
        FCM::PIFCMUnknown pFilter)
    {
        FCM::Result res;
        JSONNode commandElement(JSON_NODE);
        commandElement.push_back(JSONNode("cmdType", "UpdateFilter"));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));
        FCM::AutoPtr<DOM::GraphicFilter::IDropShadowFilter> pDropShadowFilter = pFilter;
        FCM::AutoPtr<DOM::GraphicFilter::IBlurFilter> pBlurFilter = pFilter;
        FCM::AutoPtr<DOM::GraphicFilter::IGlowFilter> pGlowFilter = pFilter;
        FCM::AutoPtr<DOM::GraphicFilter::IBevelFilter> pBevelFilter = pFilter;
        FCM::AutoPtr<DOM::GraphicFilter::IGradientGlowFilter> pGradientGlowFilter = pFilter;
        FCM::AutoPtr<DOM::GraphicFilter::IGradientBevelFilter> pGradientBevelFilter = pFilter;
        FCM::AutoPtr<DOM::GraphicFilter::IAdjustColorFilter> pAdjustColorFilter = pFilter;

        if (pDropShadowFilter)
        {
            FCM::Boolean enabled;
            FCM::Double  angle;
            FCM::Double  blurX;
            FCM::Double  blurY;
            FCM::Double  distance;
            FCM::Boolean hideObject;
            FCM::Boolean innerShadow;
            FCM::Boolean knockOut;
            DOM::Utils::FilterQualityType qualityType;
            DOM::Utils::COLOR color;
            FCM::S_Int32 strength;
            std::string colorStr;

            commandElement.push_back(JSONNode("filterType", "DropShadowFilter"));

            pDropShadowFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pDropShadowFilter->GetAngle(angle);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("angle", LottieExporter::Utils::ToString((double)angle)));

            res = pDropShadowFilter->GetBlurX(blurX);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurX", LottieExporter::Utils::ToString((double)blurX)));

            res = pDropShadowFilter->GetBlurY(blurY);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurY", LottieExporter::Utils::ToString((double)blurY)));

            res = pDropShadowFilter->GetDistance(distance);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("distance", LottieExporter::Utils::ToString((double)distance)));

            res = pDropShadowFilter->GetHideObject(hideObject);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(hideObject)
            {
                commandElement.push_back(JSONNode("hideObject", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("hideObject", "false"));
            }

            res = pDropShadowFilter->GetInnerShadow(innerShadow);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(innerShadow)
            {
                commandElement.push_back(JSONNode("innerShadow", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("innerShadow", "false"));
            }

            res = pDropShadowFilter->GetKnockout(knockOut);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(knockOut)
            {
                commandElement.push_back(JSONNode("knockOut", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("knockOut", "false"));
            }

            res = pDropShadowFilter->GetQuality(qualityType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (qualityType == 0)
                commandElement.push_back(JSONNode("qualityType", "low"));
            else if (qualityType == 1)
                commandElement.push_back(JSONNode("qualityType", "medium"));
            else if (qualityType == 2)
                commandElement.push_back(JSONNode("qualityType", "high"));

            res = pDropShadowFilter->GetStrength(strength);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("strength", LottieExporter::Utils::ToString(strength)));

            res = pDropShadowFilter->GetShadowColor(color);
            ASSERT(FCM_SUCCESS_CODE(res));
            colorStr = Utils::ToString(color);
            commandElement.push_back(JSONNode("shadowColor", colorStr.c_str()));

        }
        if(pBlurFilter)
        {
            FCM::Boolean enabled;
            FCM::Double  blurX;
            FCM::Double  blurY;
            DOM::Utils::FilterQualityType qualityType;


            commandElement.push_back(JSONNode("filterType", "BlurFilter"));

            res = pBlurFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pBlurFilter->GetBlurX(blurX);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurX", LottieExporter::Utils::ToString((double)blurX)));

            res = pBlurFilter->GetBlurY(blurY);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurY", LottieExporter::Utils::ToString((double)blurY)));

            res = pBlurFilter->GetQuality(qualityType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (qualityType == 0)
                commandElement.push_back(JSONNode("qualityType", "low"));
            else if (qualityType == 1)
                commandElement.push_back(JSONNode("qualityType", "medium"));
            else if (qualityType == 2)
                commandElement.push_back(JSONNode("qualityType", "high"));
        }

        if(pGlowFilter)
        {
            FCM::Boolean enabled;
            FCM::Double  blurX;
            FCM::Double  blurY;
            FCM::Boolean innerShadow;
            FCM::Boolean knockOut;
            DOM::Utils::FilterQualityType qualityType;
            DOM::Utils::COLOR color;
            FCM::S_Int32 strength;
            std::string colorStr;

            commandElement.push_back(JSONNode("filterType", "GlowFilter"));

            res = pGlowFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pGlowFilter->GetBlurX(blurX);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurX", LottieExporter::Utils::ToString((double)blurX)));

            res = pGlowFilter->GetBlurY(blurY);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurY", LottieExporter::Utils::ToString((double)blurY)));

            res = pGlowFilter->GetInnerShadow(innerShadow);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(innerShadow)
            {
                commandElement.push_back(JSONNode("innerShadow", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("innerShadow", "false"));
            }

            res = pGlowFilter->GetKnockout(knockOut);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(knockOut)
            {
                commandElement.push_back(JSONNode("knockOut", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("knockOut", "false"));
            }

            res = pGlowFilter->GetQuality(qualityType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (qualityType == 0)
                commandElement.push_back(JSONNode("qualityType", "low"));
            else if (qualityType == 1)
                commandElement.push_back(JSONNode("qualityType", "medium"));
            else if (qualityType == 2)
                commandElement.push_back(JSONNode("qualityType", "high"));

            res = pGlowFilter->GetStrength(strength);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("strength", LottieExporter::Utils::ToString(strength)));

            res = pGlowFilter->GetShadowColor(color);
            ASSERT(FCM_SUCCESS_CODE(res));
            colorStr = Utils::ToString(color);
            commandElement.push_back(JSONNode("shadowColor", colorStr.c_str()));
        }

        if(pBevelFilter)
        {
            FCM::Boolean enabled;
            FCM::Double  angle;
            FCM::Double  blurX;
            FCM::Double  blurY;
            FCM::Double  distance;
            DOM::Utils::COLOR highlightColor;
            FCM::Boolean knockOut;
            DOM::Utils::FilterQualityType qualityType;
            DOM::Utils::COLOR color;
            FCM::S_Int32 strength;
            DOM::Utils::FilterType filterType;
            std::string colorStr;
            std::string colorString;

            commandElement.push_back(JSONNode("filterType", "BevelFilter"));

            res = pBevelFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pBevelFilter->GetAngle(angle);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("angle", LottieExporter::Utils::ToString((double)angle)));

            res = pBevelFilter->GetBlurX(blurX);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurX", LottieExporter::Utils::ToString((double)blurX)));

            res = pBevelFilter->GetBlurY(blurY);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurY", LottieExporter::Utils::ToString((double)blurY)));

            res = pBevelFilter->GetDistance(distance);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("distance", LottieExporter::Utils::ToString((double)distance)));

            res = pBevelFilter->GetHighlightColor(highlightColor);
            ASSERT(FCM_SUCCESS_CODE(res));
            colorString = Utils::ToString(highlightColor);
            commandElement.push_back(JSONNode("highlightColor",colorString.c_str()));

            res = pBevelFilter->GetKnockout(knockOut);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(knockOut)
            {
                commandElement.push_back(JSONNode("knockOut", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("knockOut", "false"));
            }

            res = pBevelFilter->GetQuality(qualityType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (qualityType == 0)
                commandElement.push_back(JSONNode("qualityType", "low"));
            else if (qualityType == 1)
                commandElement.push_back(JSONNode("qualityType", "medium"));
            else if (qualityType == 2)
                commandElement.push_back(JSONNode("qualityType", "high"));

            res = pBevelFilter->GetStrength(strength);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("strength", LottieExporter::Utils::ToString(strength)));

            res = pBevelFilter->GetShadowColor(color);
            ASSERT(FCM_SUCCESS_CODE(res));
            colorStr = Utils::ToString(color);
            commandElement.push_back(JSONNode("shadowColor", colorStr.c_str()));

            res = pBevelFilter->GetFilterType(filterType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (filterType == 0)
                commandElement.push_back(JSONNode("filterType", "inner"));
            else if (filterType == 1)
                commandElement.push_back(JSONNode("filterType", "outer"));
            else if (filterType == 2)
                commandElement.push_back(JSONNode("filterType", "full"));

        }

        if(pGradientGlowFilter)
        {
            FCM::Boolean enabled;
            FCM::Double  angle;
            FCM::Double  blurX;
            FCM::Double  blurY;
            FCM::Double  distance;
            FCM::Boolean knockOut;
            DOM::Utils::FilterQualityType qualityType;
            FCM::S_Int32 strength;
            DOM::Utils::FilterType filterType;

            commandElement.push_back(JSONNode("filterType", "GradientGlowFilter"));

            pGradientGlowFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pGradientGlowFilter->GetAngle(angle);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("angle", LottieExporter::Utils::ToString((double)angle)));

            res = pGradientGlowFilter->GetBlurX(blurX);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurX", LottieExporter::Utils::ToString((double)blurX)));

            res = pGradientGlowFilter->GetBlurY(blurY);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurY", LottieExporter::Utils::ToString((double)blurY)));

            res = pGradientGlowFilter->GetDistance(distance);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("distance", LottieExporter::Utils::ToString((double)distance)));

            res = pGradientGlowFilter->GetKnockout(knockOut);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(knockOut)
            {
                commandElement.push_back(JSONNode("knockOut", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("knockOut", "false"));
            }

            res = pGradientGlowFilter->GetQuality(qualityType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (qualityType == 0)
                commandElement.push_back(JSONNode("qualityType", "low"));
            else if (qualityType == 1)
                commandElement.push_back(JSONNode("qualityType", "medium"));
            else if (qualityType == 2)
                commandElement.push_back(JSONNode("qualityType", "high"));

            res = pGradientGlowFilter->GetStrength(strength);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("strength", LottieExporter::Utils::ToString(strength)));

            res = pGradientGlowFilter->GetFilterType(filterType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (filterType == 0)
                commandElement.push_back(JSONNode("filterType", "inner"));
            else if (filterType == 1)
                commandElement.push_back(JSONNode("filterType", "outer"));
            else if (filterType == 2)
                commandElement.push_back(JSONNode("filterType", "full"));

            FCM::AutoPtr<FCM::IFCMUnknown> pColorGradient;
            res = pGradientGlowFilter->GetGradient(pColorGradient.m_Ptr);
            ASSERT(FCM_SUCCESS_CODE(res));

            FCM::AutoPtr<DOM::Utils::ILinearColorGradient> pLinearGradient = pColorGradient;
            if (pLinearGradient)
            {

                FCM::U_Int8 colorCount;
                //DOM::Utils::GRADIENT_COLOR_POINT colorPoint;

                res = pLinearGradient->GetKeyColorCount(colorCount);
                ASSERT(FCM_SUCCESS_CODE(res));

                std::string colorArray ;
                std::string posArray ;
                JSONNode*   stopPointArray = new JSONNode(JSON_ARRAY);

                for (FCM::U_Int32 l = 0; l < colorCount; l++)
                {
                    DOM::Utils::GRADIENT_COLOR_POINT colorPoint;
                    pLinearGradient->GetKeyColorAtIndex(l, colorPoint);
                    JSONNode stopEntry(JSON_NODE);
                    FCM::Float offset;

                    offset = (float)((colorPoint.pos * 100) / 255.0);

                    stopEntry.push_back(JSONNode("offset", Utils::ToString((double) offset)));
                    stopEntry.push_back(JSONNode("stopColor", Utils::ToString(colorPoint.color)));
                    stopEntry.push_back(JSONNode("stopOpacity", Utils::ToString((double)(colorPoint.color.alpha / 255.0))));
                    stopPointArray->set_name("GradientStops");
                    stopPointArray->push_back(stopEntry);
                }

                commandElement.push_back(*stopPointArray);

            }//lineargradient
        }

        if(pGradientBevelFilter)
        {
            FCM::Boolean enabled;
            FCM::Double  angle;
            FCM::Double  blurX;
            FCM::Double  blurY;
            FCM::Double  distance;
            FCM::Boolean knockOut;
            DOM::Utils::FilterQualityType qualityType;
            FCM::S_Int32 strength;
            DOM::Utils::FilterType filterType;

            commandElement.push_back(JSONNode("filterType", "GradientBevelFilter"));

            pGradientBevelFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pGradientBevelFilter->GetAngle(angle);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("angle", LottieExporter::Utils::ToString((double)angle)));

            res = pGradientBevelFilter->GetBlurX(blurX);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurX", LottieExporter::Utils::ToString((double)blurX)));

            res = pGradientBevelFilter->GetBlurY(blurY);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("blurY", LottieExporter::Utils::ToString((double)blurY)));

            res = pGradientBevelFilter->GetDistance(distance);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("distance", LottieExporter::Utils::ToString((double)distance)));

            res = pGradientBevelFilter->GetKnockout(knockOut);
            ASSERT(FCM_SUCCESS_CODE(res));
            if(knockOut)
            {
                commandElement.push_back(JSONNode("knockOut", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("knockOut", "false"));
            }

            res = pGradientBevelFilter->GetQuality(qualityType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (qualityType == 0)
                commandElement.push_back(JSONNode("qualityType", "low"));
            else if (qualityType == 1)
                commandElement.push_back(JSONNode("qualityType", "medium"));
            else if (qualityType == 2)
                commandElement.push_back(JSONNode("qualityType", "high"));

            res = pGradientBevelFilter->GetStrength(strength);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("strength", LottieExporter::Utils::ToString(strength)));

            res = pGradientBevelFilter->GetFilterType(filterType);
            ASSERT(FCM_SUCCESS_CODE(res));
            if (filterType == 0)
                commandElement.push_back(JSONNode("filterType", "inner"));
            else if (filterType == 1)
                commandElement.push_back(JSONNode("filterType", "outer"));
            else if (filterType == 2)
                commandElement.push_back(JSONNode("filterType", "full"));

            FCM::AutoPtr<FCM::IFCMUnknown> pColorGradient;
            res = pGradientBevelFilter->GetGradient(pColorGradient.m_Ptr);
            ASSERT(FCM_SUCCESS_CODE(res));

            FCM::AutoPtr<DOM::Utils::ILinearColorGradient> pLinearGradient = pColorGradient;
            if (pLinearGradient)
            {

                FCM::U_Int8 colorCount;
                //DOM::Utils::GRADIENT_COLOR_POINT colorPoint;

                res = pLinearGradient->GetKeyColorCount(colorCount);
                ASSERT(FCM_SUCCESS_CODE(res));

                std::string colorArray ;
                std::string posArray ;
                JSONNode*   stopPointsArray = new JSONNode(JSON_ARRAY);

                for (FCM::U_Int32 l = 0; l < colorCount; l++)
                {
                    DOM::Utils::GRADIENT_COLOR_POINT colorPoint;
                    pLinearGradient->GetKeyColorAtIndex(l, colorPoint);
                    JSONNode stopEntry(JSON_NODE);
                    FCM::Float offset;

                    offset = (float)((colorPoint.pos * 100) / 255.0);

                    stopEntry.push_back(JSONNode("offset", Utils::ToString((double) offset)));
                    stopEntry.push_back(JSONNode("stopColor", Utils::ToString(colorPoint.color)));
                    stopEntry.push_back(JSONNode("stopOpacity", Utils::ToString((double)(colorPoint.color.alpha / 255.0))));
                    stopPointsArray->set_name("GradientStops");
                    stopPointsArray->push_back(stopEntry);
                }

                commandElement.push_back(*stopPointsArray);

            }//lineargradient
        }

        if(pAdjustColorFilter)
        {
            FCM::Double brightness;
            FCM::Double contrast;
            FCM::Double saturation;
            FCM::Double hue;
            FCM::Boolean enabled;

            commandElement.push_back(JSONNode("filterType", "AdjustColorFilter"));

            pAdjustColorFilter->IsEnabled(enabled);
            if(enabled)
            {
                commandElement.push_back(JSONNode("enabled", "true"));
            }
            else
            {
                commandElement.push_back(JSONNode("enabled", "false"));
            }

            res = pAdjustColorFilter->GetBrightness(brightness);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("brightness", LottieExporter::Utils::ToString((double)brightness)));

            res = pAdjustColorFilter->GetContrast(contrast);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("contrast", LottieExporter::Utils::ToString((double)contrast)));

            res = pAdjustColorFilter->GetSaturation(saturation);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("saturation", LottieExporter::Utils::ToString((double)saturation)));

            res = pAdjustColorFilter->GetHue(hue);
            ASSERT(FCM_SUCCESS_CODE(res));
            commandElement.push_back(JSONNode("hue", LottieExporter::Utils::ToString((double)hue)));
        }

        m_pCommandArray->push_back(commandElement);

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::UpdateDisplayTransform(
        FCM::U_Int32 objectId,
        const DOM::Utils::MATRIX2D& matrix)
    {
        JSONNode commandElement(JSON_NODE);
        std::string transformMat;

        commandElement.push_back(JSONNode("cmdType", "Move"));
        commandElement.push_back(JSONNode("objectId", LottieExporter::Utils::ToString(objectId)));
        transformMat = LottieExporter::Utils::ToString(matrix);
        commandElement.push_back(JSONNode("transformMatrix", transformMat.c_str()));

        m_pCommandArray->push_back(commandElement);

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::UpdateColorTransform(
        FCM::U_Int32 objectId,
        const DOM::Utils::COLOR_MATRIX& colorMatrix)
    {
        // add code to write the color transform 
        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::ShowFrame(FCM::U_Int32 frameNum)
    {
        m_pFrameElement->push_back(JSONNode(("num"), LottieExporter::Utils::ToString(frameNum)));
        m_pFrameElement->push_back(*m_pCommandArray);
        m_pFrameArray->push_back(*m_pFrameElement);

        delete m_pCommandArray;
        delete m_pFrameElement;

        m_pCommandArray = new JSONNode(JSON_ARRAY);
        m_pCommandArray->set_name("Command");

        m_pFrameElement = new JSONNode(JSON_NODE);
        ASSERT(m_pFrameElement);

        
        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::AddFrameScript(FCM::CStringRep16 pScript, FCM::U_Int32 layerNum)
    {
        std::string script = Utils::ToString(pScript, m_pCallback);

        std::string scriptWithLayerNumber = "script Layer" +  Utils::ToString(layerNum);

        std::string find = "\n";
        std::string replace = "\\n";
        std::string::size_type i =0;
        JSONNode textElem(JSON_NODE);

        while (true) {
            /* Locate the substring to replace. */
            i = script.find(find, i);
           
            if (i == std::string::npos) break;
            /* Make the replacement. */
            script.replace(i, find.length(), replace);

            /* Advance index forward so the next iteration doesn't pick it up as well. */
            i += replace.length();
        }

        
        Utils::Trace(m_pCallback, "[AddFrameScript] (Layer: %d): %s\n", layerNum, script.c_str());

        m_pFrameElement->push_back(JSONNode(scriptWithLayerNumber,script));

        return FCM_SUCCESS;
    }


    FCM::Result JSONTimelineWriter::RemoveFrameScript(FCM::U_Int32 layerNum)
    {
        Utils::Trace(m_pCallback, "[RemoveFrameScript] (Layer: %d)\n", layerNum);

        return FCM_SUCCESS;
    }

    FCM::Result JSONTimelineWriter::SetFrameLabel(FCM::StringRep16 pLabel, DOM::KeyFrameLabelType labelType)
    {
        std::string label = Utils::ToString(pLabel, m_pCallback);
        Utils::Trace(m_pCallback, "[SetFrameLabel] (Type: %d): %s\n", labelType, label.c_str());

        if(labelType == 1)
             m_pFrameElement->push_back(JSONNode("LabelType:Name",label));
        else if(labelType == 2)
             m_pFrameElement->push_back(JSONNode("labelType:Comment",label));
        else if(labelType == 3)
             m_pFrameElement->push_back(JSONNode("labelType:Ancor",label));
        else if(labelType == 0)
             m_pFrameElement->push_back(JSONNode("labelType","None"));

        return FCM_SUCCESS;
    }


    JSONTimelineWriter::JSONTimelineWriter(FCM::PIFCMCallback pCallback) :
        m_pCallback(pCallback)
    {
        m_pCommandArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pCommandArray);
        m_pCommandArray->set_name("Command");

        m_pFrameArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pFrameArray);
        m_pFrameArray->set_name("Frame");

        m_pTimelineElement = new JSONNode(JSON_NODE);
        ASSERT(m_pTimelineElement);
        m_pTimelineElement->set_name("Timeline");

        m_pFrameElement = new JSONNode(JSON_NODE);
        ASSERT(m_pFrameElement);
        //m_pCommandArray->set_name("Command");
    }


    JSONTimelineWriter::~JSONTimelineWriter()
    {
        delete m_pCommandArray;

        delete m_pFrameArray;

        delete m_pTimelineElement;
        
        delete m_pFrameElement;
    }


    const JSONNode* JSONTimelineWriter::GetRoot()
    {
        return m_pTimelineElement;
    }


    void JSONTimelineWriter::Finish(FCM::U_Int32 resId, FCM::StringRep16 pName)
    {
        if (resId != 0)
        {
            m_pTimelineElement->push_back(
                JSONNode(("charid"), 
					LottieExporter::Utils::ToString(resId)));
        }

        m_pTimelineElement->push_back(*m_pFrameArray);
    }

};
