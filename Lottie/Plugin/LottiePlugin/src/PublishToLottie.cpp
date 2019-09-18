#include "PublishToLottie.h"
#include "IFLADocument.h"
#include "ITimeline.h"
#include "ILayer.h"
#include "IFrame.h"
#include "FrameElement/IShape.h"
#include "fstream"
#include "Utils.h"
#include <algorithm>

#define _USE_MATH_DEFINES // for C++  
#include <math.h>

namespace LottieExporter {


	void LottieManager::Init(std::string outputFilePath)
	{
		mOutputFilePath = outputFilePath;
		
	}




	void LottieManager::SetFPS(int fps)
	{
		m_fps = fps;
	}


	


	


	
	


	
	}

