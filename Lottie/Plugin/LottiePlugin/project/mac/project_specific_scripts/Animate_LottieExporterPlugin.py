
import os, sys, glob, string

def get_source_and_dest_copy_paths(config):
	path_list = {}
	
	DEBUG_path_list = []
	RELEASE_path_list = []
	DEBUG_path_list.append([r'../../lib/mac/debug/LottieExporterPlugin.framework', r'/Users/ranawat/Perforce/ranawat_animate_newMBP/main/flashpro/src/Flash/lib/mac/debug/Adobe Animate Prerelease.app/Contents/Frameworks'])
	RELEASE_path_list.append([r'../../lib/mac/release/LottieExporterPlugin.framework', r'/Users/ranawat/Perforce/ranawat_animate_newMBP/main/flashpro/src/Flash/lib/mac/release/Adobe Animate Prerelease.app/Contents/Frameworks'])
	path_list["DEBUG"] = DEBUG_path_list
	path_list["RELEASE"] = RELEASE_path_list

	return path_list.get(config)