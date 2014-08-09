#pragma once
#include "opencv2/opencv.hpp"

#include <stdio.h>
#include <Windows.h>

using namespace cv;
using namespace std;

#define BUFFER_SIZE 256

static void get_dir_contents( const string & directory, const string & extension, vector<string> & contents )
{
	char search_path[BUFFER_SIZE];
	sprintf(search_path, "%s*.*", directory.c_str());
	WIN32_FIND_DATAA fd; 
	HANDLE hFind = ::FindFirstFileA(search_path, &fd); 
	if(hFind != INVALID_HANDLE_VALUE) 
	{ 
		do 
		{ 
			if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
			{
				string _tmpContent = fd.cFileName;
				if(_tmpContent.substr(_tmpContent.find_last_of(".") + 1) == extension) 
				{
					contents.push_back(_tmpContent);
				}
			}
		}
		while(::FindNextFileA(hFind, &fd)); 
		::FindClose(hFind); 
	}
}