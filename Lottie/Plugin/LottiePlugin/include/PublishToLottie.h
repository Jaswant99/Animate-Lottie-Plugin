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

#ifndef PublishToLottie_hpp
#define PublishToLottie_hpp

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <cmath>
#include<iostream>
#define COORD_ERR 0.0001
#include <memory>
#include <string>


#define ENUM_TO_STR(ENUM) std::string(#ENUM)
#define INVALID_LAYER_INDEX -1
struct coordinates
{
    double x;
    double y;
};
struct offsetkeyframe
{
    float start[3]={0};
    float time;
    coordinates i;
    coordinates o;
    int h=1; //for hold interpolation
    
};
struct anchor_point{
    int a = 0;
    float k[3]={0,0,0};
    int ix=1;
    int frame_number;
};
struct multidimensional_keyframed
{
   coordinates ti;
   coordinates to;
};
struct position{
    int a = 0;
    float k[3]={0,0,0};
    int ix=2;
    int frame_number;
    offsetkeyframe offset; ///for in and out values
    multidimensional_keyframed multi_keyframe; //for ti and to
    
};
struct scale{
    int a = 0;
    float k[3] = { 100,100,100 };
    int ix=6;
    float frame_number;
    offsetkeyframe offset;
};
struct rotation{
    int a = 0;
    float k = 0;
    int ix=10;
    int frame_number;
    offsetkeyframe offset;
    
};
struct opacity{
    int a = 0;
    float k = 100;
    int ix=11;
    int frame_number;
};
struct skew{
    int a = 0;
    float k = 0;
    int ix;
    int frame_number;
    
};
struct skew_axis{
    int a = 0;
    float k = 0;
    int ix;
    int frame_number;
    
};

struct layer_prop {
    std::vector<opacity>o;
   // opacity o ;
    std::vector<rotation>r;
   // rotation r;
    std::vector<scale>s;
    //scale s;
    std::vector<position>p;
    //position p;
    std::vector<anchor_point>a;
    //anchor_point a;
    std::vector<skew>sk;
    //skew sk;
    std::vector<skew_axis>sa;
    //skew_axis sa;
};
enum Layer_type
{
    Precomp,Solid,Image,Null,Shape,Text
};
struct color
{
    int a=0;
    double r=1;
    double g=1;
    double b=1;
    double alpha=1;
    int ix;
    int frame_number;
};
struct gradient_color{
    int a=0;
    std::vector<double>color;
    int ix=9;
    int frame_number;
};
struct gradient{
int p;
gradient_color k;
};
struct start_point{
    int a=0;
    double k[2]={0};
    int ix=5;
    int frame_number;
};
struct end_point{
    int a=0;
    double k[2]={0};
    int ix=6;
    int frame_number;
};
struct gradient_fill{
    std::string ty="gf";
    opacity o;
    int bm=0;
    std::string nm="Gradient Fill";
    std::string mn="ADBE Vector Graphic - G-Fill";
    bool hd=false;
    gradient g;
    start_point s;
    end_point e;
    int type=1;   //type 1 -linear ( by default linear gradient )
    int r=1;
};
struct highlight_length{
    int a=0;
    float k=0;
    int ix=9;
    int frame_number;
};
struct highlight_angle{
    int a=0;
    float k=0;
    int ix=8;
    int frame_number;
};
struct radial_gradient_fill{
    gradient_fill radial_fill;
    highlight_length h;
    highlight_angle a;   //while defining radial gradient set type to 2
};

struct width{
    int a=0;
    float k;
    int ix=5;
    int frame_number;
};

struct ks{
    int a=0;
    std::vector <coordinates> i;
    std::vector <coordinates> o;
    std::vector <coordinates> v;
    std::vector <coordinates> v2;
    bool c=false;
    int ix=2;
    int frame_number;
   
};

struct shape
{
    std::string ty="sh";
    std::string nm="Path 1";
    std::string mn = "ADBE Vector Shape - Group";
    ks shp;
    bool hd=false;
    int ix=1;
    
};

struct solid_fill
{   std::string ty= "fl";
    color color1;
    opacity o;
    int r=1;
    int bm=0;
    std::string nm="Fill";
    std::string mn="ADBE Vector Graphic - Fill";
    bool hd=false;
};

struct fill{
    bool isfilled=false;
    bool issolid=false;
    bool islinear_gradient=false;
    bool isradial_gradient=false;
    solid_fill solid;
    gradient_fill linear;
    radial_gradient_fill radial;
    
};
struct solid_stroke{
    std::string ty= "st";
    color color1;
    opacity o;
    width w;
    bool hd=false;
    std:: string nm="Stroke ";
    std::string mn="ADBE Vector Graphic - G - Stroke";
    int lc;
    int lj;
    double ml;
    int bm=0;
};
struct gradient_stroke{
    std::string ty="gs";
    opacity o;
    int bm=0;
    std::string nm="Gradient Stroke";
    std::string mn="ADBE Vector Graphic - G-Stroke";
    bool hd=false;
    gradient g;
    start_point s;
    end_point e;
    int type=1;   //type 1 -linear ( by default linear gradient )
    int r=1;
};
struct radial_gradient_stroke{
    gradient_stroke radial_stroke;
    highlight_length h;
    highlight_angle a;   //while defining radial gradient set type to 2
};


struct stroke{
    bool hasstroke=false;
    bool issolid = false;
    bool islinear_gradient=false;
    bool isradial_gradient=false;
    solid_stroke solid;
    gradient_stroke linear;
    radial_gradient_stroke radial;
    
};
struct size{
    int a=0;
    int k[2];
    int ix=2;
    int frame_number;
};
struct position_for_rect
{
    int a=0;
    int k[2]={0};
    int ix=3;
    
};
struct rect_rounded_corners
{
    int a=0;
    int k=0;
    int ix=4;
};
struct rect
{
    std::string ty="rc";
    bool isrect=false;
    int d=1;
    size s;
    position_for_rect rc;
    std::string nm="Rectangle Path 1";
    std::string mn="ADBE Vector Shape - Rect";
    bool hd=false;
    rect_rounded_corners r;
    
};
struct x{
    int a=0;
    int k=0;
    int ix=4;
};

struct maskproperties
{
    bool inv=false;
    std::string nm="Mask ";
    ks pt;
    opacity o;
    std::string mode;
    x expansion;
};

struct group{
    std::string ty="gr";
    rect r; //only for null layer
    shape sh;
    fill fl;
    stroke st;
    layer_prop ks;
    int parent_layer;
    bool hd=false;
    std::string nm="shape";
    int np=3;
    int cix=2;
    int bm=0;
    int ix=1;
    std:: string mn="ADBE Vector Group";
   // bool hasmask;
   
    
};
struct image_resource
{
    int height;
    int width;
    std::string libitemname;
    int resourceid;
    std::string cl;
    std::string ref_id;
    
};
struct hole_layer
{
     group * gr;
     std::vector<maskproperties *>mp;
    bool hasmask= true;
};

struct Layer
{
    std::uint32_t ddd = 0;
    std::uint32_t ind = 0;
    enum Layer_type ty;
     int objectId;
    int resourceId;
    //std::uint32_t ty = 4;
    std::string nm = "Layer ";
    std::uint32_t sr = 1;
    std::uint32_t ao = 0;
    std::uint32_t ip = 0;
    std::uint32_t op = 0;
    std::uint32_t st = 0;
    std::uint32_t bm = 0;
    std::uint32_t parent_ind=ind;
    std::uint32_t placeafterobjectId;
    
    //int call=0;
    layer_prop ks;

   
    
};



namespace  LottieExporter {


#ifdef _MAC
	const std::string PATH_SEPARATOR = "/";
#else    // Win32 Version
	const std::string PATH_SEPARATOR = "\\";
#endif

	



	class LottieManager
	{
	public:
		~LottieManager();
		void                                Init(std::string outputFilePath);
		void                                SetFPS(int fps);
        //void                                SetIp(int frameindex);
		int                                 GetFPS() { return m_fps; }
        int                                 GetIp(){return m_ip;}
        int                                 GetOp(){return m_op;}
        //vector<Layer>                       GetLayers(){return layers;}
        int                                 GetNumofLayers(){return layers.size();}
        int                                 GetNumofGroups(int resourceid){return resourceId_group[resourceid].size();}
		std::string                         GetVersion() { return m_version; }
		void                                SetVersion(std::string val) { m_version = val; }
		void                                SetStageWidthHeight(int width, int height) { mStageWidth = width, mStageHeight = height; }
		void                                GetStageWidthHeight(int &width, int &height) { width = mStageWidth, height = mStageHeight; }
        void                                 SetOp(int frameindex){if(m_op<frameindex) m_op=frameindex;}
        void                                CreateLayer(enum Layer_type ty,int parent_ind , int objectid,int resourceId,int placeafterobjectid)
        {
            layers.push_back(new Layer);
            std::uint32_t size=layers.size();
            layers[size-1]->op=GetOp();
            layers[size-1]->ty=ty;
            layers[size-1]->ind=size;
            layers[size-1]->parent_ind=parent_ind;
            position pos;
            pos.k[0]=0;
            pos.k[1]=0;
            pos.frame_number=0;
            layers[size-1]->ks.p.push_back(pos);
            scale s;
            layers[size-1]->ks.s.push_back(s);
            anchor_point a;
            layers[size-1]->ks.a.push_back(a);
            opacity o;
            layers[size-1]->ks.o.push_back(o);
            rotation r;
            layers[size-1]->ks.r.push_back(r);
            skew sk;
            layers[size-1]->ks.sk.push_back(sk);
            skew_axis sa;
            layers[size-1]->ks.sa.push_back(sa);
            layers[size-1]->objectId = objectid;
            layers[size-1]->resourceId = resourceId;
            layers[size-1]->placeafterobjectId = placeafterobjectid;
            //layers[size-1]->ks.p.k[0]=0;
            //layers[size-1]->ks.p.k[1]=0;
            if(layers[size-1]->ty == 2)
            {
                image_resource * img =Getimage_resource_with_id(resourceId);
                layers[size-1]->nm=img->libitemname;
                
            }
            else
            {
            std::string object_id = std::to_string(objectid);
            layers[size-1]->nm.append(object_id);
            }
            

        }
        
        void shape_layer_map(int objectId, Layer * layer)
        {
            objectId_layer[objectId] = layer;
            //std:: cout<<"bm"<<shapeid_layer[objectId]->bm<<std::endl;
        }
        
        void resource_group_map(int resourceId, group * gr)
        {
            resourceId_group[resourceId].push_back(gr);
            //std:: cout<<"bm"<<shapeid_layer[objectId]->bm<<std::endl;
        }
        bool comparePlaceAfterObject(const Layer& a, const Layer& b)
        {
            

                return a.placeafterobjectId < b.placeafterobjectId;
                //return b.placeafterobjectId < a.placeafterobjectId;
                
        }
        void SortLayers()
        {
            std::sort(layers.begin(),layers.end(),[this](Layer* a, Layer* b) { return this->LottieManager::comparePlaceAfterObject(*a, *b); });
        }
        
        
        
        Layer *                                 GetLayer()
        {
            std::uint32_t size=layers.size();
            if(size!=0)
                return layers[size-1];
            else
                return NULL;
        }
        Layer *                                 GetLayerAtIndex(int index)
        {
            std::uint32_t size=layers.size();
            if(index >= 0 && index < size)
            {return layers[index];}
            else
                return NULL;
        }
        Layer *                                 GetLayerAtObjectId(int objectId)
        {
            //std::uint32_t size=layers.size();
            return objectId_layer[objectId];
            
        }
        std::vector<group *>                                GetGroupAtResourceId(int resourceId)
        {
            //std::uint32_t size=layers.size();
            return resourceId_group[resourceId];
            
        }
        std::map<int, Layer*> GetobjIdLayer()
        {
            return objectId_layer;
        }
        std::map<int, image_resource*> Getimageresource_id()
        {
            return image_resource_id;
        }
        
      /*  void CreateNewMaskProperty(std::string mode ,bool inv)
        {
            gr.push_back(new group);
             std::uint32_t size=gr.size();
     
            gr[size-1]->hasmask=true;
 
        }*/
       void CreateHoleLayer(group * gr)
        {
            hole_layers.push_back(new hole_layer);
            std::uint32_t size = hole_layers.size();
            hole_layers[size-1]->gr = gr;
            
            
            
        }
        
        
        void                                CreateGroup()
        {
            gr.push_back(new group);
            std::uint32_t size=gr.size();
            //gr[size-1]->parent_layer=layers.size();
            position pos;
            gr[size-1]->ks.p.push_back(pos);
            scale s;
            gr[size-1]->ks.s.push_back(s);
            anchor_point a;
            gr[size-1]->ks.a.push_back(a);
            opacity o;
            gr[size-1]->ks.o.push_back(o);
            rotation r;
            gr[size-1]->ks.r.push_back(r);
            skew sk;
            gr[size-1]->ks.sk.push_back(sk);
            skew_axis sa;
            gr[size-1]->ks.sa.push_back(sa);
            std::uint32_t a_size=gr[size-1]->ks.a.size();
            gr[size-1]->ks.a[a_size-1].ix=1;
            std::uint32_t p_size=gr[size-1]->ks.p.size();
            gr[size-1]->ks.p[p_size-1].ix=2;
            std::uint32_t s_size=gr[size-1]->ks.s.size();
            gr[size-1]->ks.s[s_size-1].ix=3;
            std::uint32_t sk_size=gr[size-1]->ks.sk.size();
            gr[size-1]->ks.sk[sk_size-1].ix=4;
            std::uint32_t sa_size=gr[size-1]->ks.sa.size();
            gr[size-1]->ks.sa[sa_size-1].ix=5;
            std::uint32_t r_size=gr[size-1]->ks.r.size();
            gr[size-1]->ks.r[r_size-1].ix=6;
            std::uint32_t o_size=gr[size-1]->ks.o.size();
            gr[size-1]->ks.o[o_size-1].ix=7;
             //gr[size-1]->hasmask=false;
            
            
        }
     
        group *                               Getgroup()
        {   std::uint32_t size=gr.size();
            if(size!=0)
            return gr[size-1];
            else
                return NULL;
        }
        void DeleteLastGroup(int resourceId)
        {
            gr.pop_back();
            resourceId_group[resourceId].pop_back();
        }
        hole_layer *                               Getholelayer()
        {   std::uint32_t size=hole_layers.size();
            if(size!=0)
                return hole_layers[size-1];
            else
                return NULL;
        }
        std::vector<hole_layer * >                              GetholelayersAtresourceid( int resourceid)
        {
                return resource_hole[resourceid];
          
        }
        hole_layer *                             Getholelayerfrommap( int resourceid,int j)
        {
            return resource_hole[resourceid][j];
            
        }
        group *                                 GetGroupAtIndex(int index,int resource_id)
        {
    
            return resourceId_group[resource_id][index];
          
                
            }
        void GetFileExtension(const std::string& path, std::string& extension)
        {
            size_t index = path.find_last_of(".");
            extension = "";
            if (index != std::string::npos)
            {
                extension = path.substr(index + 1, path.length());
            }
        }
        
        void createimage_resourceid(int width,int height,std::string libitemname,int resourceid){
            image_resources.push_back(new image_resource);
            std::uint32_t size=image_resources.size();
            image_resources[size-1]->width = width;
            image_resources[size-1]->height = height;
            image_resources[size-1]->libitemname = libitemname;
            image_resources[size-1]->resourceid = resourceid;
            GetFileExtension(libitemname, image_resources[size-1]->cl);
            image_resources[size-1]->ref_id="";
            image_resources[size-1]->ref_id.append("image_");
            std::string reference_id = std::to_string(resourceid);
            image_resources[size-1]->ref_id.append(reference_id);
            

            
        }
        
        
        image_resource *                               Getimage_resource()
        {   std::uint32_t size=image_resources.size();
            if(size!=0)
                return image_resources[size-1];
            else
                return NULL;
        }
        image_resource *                               Getimage_resource_with_id(int resourceid)
        {
                return image_resource_id[resourceid];
          
        }
        
        void image_resource_id_map(int resourceId, image_resource * image_resource)
        {
            image_resource_id[resourceId] = image_resource;
           
        }
        void hole_layer_resource_id_map(int resourceId, hole_layer * hole_layer)
        {
            resource_hole[resourceId] .push_back( hole_layer);
            
        }
          void object_resource_map(int resourceId,int objectid){
            object_resource[resourceId].push_back(objectid);
        }
        std::map<int,std::vector<hole_layer *>>  get_hole_resource_map()
        {
            return resource_hole;
        }
         std::map<int,std::vector<int>>  get_object_resource_map()
        {
            return object_resource;
        }
        
		
		
	

	private:
		std::string									m_version="5.5.4";
		int                                 m_fps = 24;
        int                                 m_ip=0;
        int                                 m_op=0;
		float                               mStageHeight = 550;
		float                               mStageWidth = 400;
		std::string                         mOutputFilePath;
        std::vector<Layer *>layers;
        std::vector<group *>gr;
        std::vector<image_resource *>image_resources;
        std::map<int , Layer *>objectId_layer;
        std::map<int , std::vector<group *> >resourceId_group;
        std::map<int,image_resource *> image_resource_id;
        std::map<int,std::vector<hole_layer * > > resource_hole;
        std::vector<hole_layer *>hole_layers;
        std::map<int,std::vector<int>> object_resource;
       
	
		
	};
}

#endif /* PublishToLottie_hpp */

