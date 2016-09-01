This document contains instructions on how to run the method proposed in [1]. If you use this code for research purposes, 
please cite our paper: 
R. G. Mesquita, C. A. B. Mello and P. L. Castilho, "Visual search guided by an efficient top-down attention approach",
in  IEEE International Conference on Image Processing (ICIP), 2016[1]:

Image representing matching examples using the proposed method can be downloaded from https://www.dropbox.com/s/d4uf5xs8oli978b/img_matches%20-%20resized.rar?dl=0

1. Environment configuration

1.1 First, you need to download and install OpenCV. In our case, OpenCV 3.1.0 was used; it can be downloaded from http://opencv.org/downloads.html
	To configure OpenCV on Windows, create a new environment variable named OPENCV_DIR and set its value to the directory where OpenCV was instaled (e.g. 'C:\opencv3.1.0');
	Then, add OpenCV bin path to the system variables Path. For example, if you are using Visual Studio 2013 and a 64-bit O.S.
	you should add the following value to the Path variable '%OPENCV_DIR%\opencv\build\x64\vc12\bin;' (you should restart your computer after this operation)

1.2 Create a new empty project on Visual Studio (or any other IDE of your preference)
	
	1.2.1 Right-click on your project -> Properties -> C/C++ -> Preprocessor -> Preprocessor Definitions and add the following:
		WIN32
		_WINDOWS
		NDEBUG
		_CONSOLE
		_LIB
		_CRT_SECURE_NO_DEPRECATE
		_CRT_NONSTDC_NO_DEPRECATE
	
	1.2.1 Then go to -> Precompiled Headers and set the Precompiled Header field to 'Not Using Precompiled Headers'
	
	1.2.2 configure it with OpenCV: 
		1.2.2.1 Right-click on your project -> Properties -> C/C++ -> General -> Additional Include Directories
			add the following directory: $(OPENCV_DIR)\opencv\build\include\
		1.2.2.2 Then go to -> Linker -> General -> Additional Library Directories
			add the following directory: $(OPENCV_DIR)\opencv\build\x64\vc12\lib (In case you are using a 64-bit opencv configuration; otherwise, change x64 to x86)			
		1.2.2.3 Then go to -> Linker -> Input -> Additional Dependencies
			add the following entry: opencv_world310.lib 
			
		*ps: To run our source code in Debug mode, you should repeat the same steps 1.2.2.1 - 1.2.2.3 using the Configuration dropdown button set to Debug
		 **ps: For step 1.2.2.3, the name of the .lib file used is 'opencv_world310d.lib'
		 
		*ps2: For more information on how to install and configure OpenCV you can see the following links:		
		http://docs.opencv.org/2.4/doc/tutorials/introduction/windows_install/windows_install.html#windows-installation) 
		http://docs.opencv.org/2.4/doc/tutorials/introduction/windows_visual_studio_Opencv/windows_visual_studio_Opencv.html

	1.2.3 Download our source code from https://github.com/rgalvaomesquita/VATopDownSearch and extract it to <your_Solution>\VATopDownSearch folder
	
	1.2.4 Download the Bloom Filter implementation developed by Arash Partow (and improved by us) from
	https://bloom.codeplex.com/SourceControl/network/forks/rgalvaomesquita/serializableBloom; create a folder named BloomFilter
    on your directory Solution and extract the file bloom_filter.hpp to it	
	
	1.2.5 Similarly to what was done in step 1.2.2.1, add the following directories to 'Additional Include Directories':
			$(SolutionDir)\VATopDownSearch\BStringTree\
			$(SolutionDir)\BloomFilter\
			$(SolutionDir)\VATopDownSearch\Homography_Match\
			$(SolutionDir)\VATopDownSearch\Input_Output\
			$(SolutionDir)\VATopDownSearch\Parameters\
			$(SolutionDir)\VATopDownSearch\TDAttention\
	
	1.2.6 Add the following files to your project (Right-click on your project -> Add -> Existing Item)
			bloom_filter.hpp
			BStringTree.hpp
			BStringTree.cpp
			dir_names.hpp
			Homography_match.hpp
			Input_Output.hpp
			Parameters.h
			TDAttention;hpp
			main.cpp
			
	

	
2. Setting values in dir_names.h
	In dir_names.h you must set the following directories:
	  DIR_IMGS_TEST : directory where the scene images (in which each object is going to be searched) are located
	  DIR_OBECTS : directory where images of each object used as the target of the search are located
	  ROOT_DIR : root directory where experiment results and data are stored
	  
    The following subdirectories of ROOT_DIR can be left unchanged:
      DIR_TXT_RESULTS : time processing and accuracy resulted from the search using each object as target in all scene images (this folder is automatically created)
      DIR_KP_RESULTS : pré-computed keypoints for each object (this folder is automatically created)	  
	  DIR_DES_OBJECTS : pre-computed descriptors for each object (this folder is automatically created)
	  DIR_FILTERS : Set of Bloom Filters trained for each object (this folder is automatically created)
	  DIR_IMG_MATCHES : directory where image matches are stored (this folder is automatically created)
	  
	The following values may be left unchanged, especially if PONCE Object Recognition is used
	
	*ps: Our method was experimented with the Object Recognition Database, from Ponce Group [2], that contains 51 test scenes and a set of viewpoint images for 8 different objects.
	  If you wish to use this dataset, it can be downloaded from http://www-cvr.ai.uiuc.edu/ponce_grp/data/ 
	  
	  objs[numberOfObjects] : folder name of each object in DIR_OBJECTS; for each folder, a set of viewpoints for the given object may exist (can be left unchanged if PONCE Object Recognition database is used)
	  GT[numberOfObjects][numberOfTestScenes] : defines if i-th(first dimension) object is present at the j-th(second dimension) test image (can be left unchanged if PONCE Object Recognition database is used)
												These values are configures according to Object Recognition database from PONCE research group and need to be redefined if a different dataset is used
	  percKpSal[numberOfObjects] : percentual of keypoints selected to be prioritized during the search using baseline methods ('random' and 'sort by ratio')
				For a fair comparison these values should be set to keep the same proportion of prioritized descriptors selected by the tested method (Top Down attention using bloom filters, for the case of the method proposed in [1])
	  
	  
	  
3. Generating keypoints and descriptors from the object dataset

	In Parameters.h, change GENERATE_KP_DES to true to generate and store the keypoints and descriptors for each viewpoint of each object;
	The set of keypoints and descriptors for each object viewpoint will be saved in DIR_KP_RESULTS and DIR_DES_OBJECTS;
	
	Remember to set the this value back to false after keypoint and descriptor generation
	
4. Training Bloom Filters
	To populate the set of Bloom Filters for each object, set the value of TRAIN to 1, in Parameters.h. By default, the first NBITS(72, by default) bits  
    are used, and neighbours of distance less or equal than MAX_DIST (2, by default) will be stored in the Bloom Filter. NBITS and MAX_DIST are defined in Parameters.h
	The Bloom Filters of each object will be saved in ROOT_DIR\DIR_FILTERS\NBITS_<NBITS>_MAX_DIST_<MAX_DIST>.

5. Testing our implementation

	5.1 In Parameters.h, set TRAIN to 0, and GENERATE_KP_DES to false. Define the desired number of executions by setting NUM_EXEC.  
    After running the program, the results will be saved in ROOT_DIR\DIR_TXT_RESULTS. A text file wil be created for each object used
    as target. This text file contains, for all tested methods:
		The average time spent on the TopDown attention phase using Bloom Filters
		The average number of scene descriptors
		The recognition accuracy of each method (that are expected to be the same);
		a list of processing times considering all images of the scene images dataset;
	    the average processing time considering all scene images;
		the average processing time considering all only true positive cases (scenes in which all methods successfully recognized the target);
	
	5.2 To save images containing matched keypoints between the target object and the scene, set VERBOSE to true in Parameters.h. The match images
	will be saved in ROOT_DIR\DIR_IMG_MATCHES, with the following nomenclature:
		
		obj_<number of object>_scene_<name of tested scene>_<method>_<stage of match>.jpg
		
		<method> can be: 
				"CLASSIC_SEARCH", for the classic AKAZE, in which all descriptors of the scene are matched at once	
			or 
			    "SBR", for 'SORT BY RESPONSE' method, in which descriptors to be matched are selected to prioritize keypoints with greater response values, respecting the same average number of descriptors selected by the proposed method
			or 
			    "RND", for RANDOM AKAZE, in which descriptors to be matched are randomly selected, respecting the same average number of descriptors selected by the proposed method
			or	
				"_BloomFilter_TDAttention", for the proposed method

            if <method> is "CLASSICAL", the <stage of match> is "MatchAll", as all descriptors are always matched at once
			if <method> is _BloomFilter_TDAttention, SBR or RND, the <stage of match> can be:			
				"" (empty), if the target was found during the top-down attention stage
			or
				"Match_Near", if the target was found after the attention stage, but in a promising location (near a matched keypoint during attention stage)
			of
                "MatchAll", if the target was found neither during the attention stage nor at promising locations, but only when the remaining descriptors were matched


[1] R. G. Mesquita, C. A. B. Mello and P. L. Castilho, "Visual search guided by an efficient top-down attention approach", in  IEEE International Conference on Image Processing (ICIP), 2016

[2] F. Rothganger, S. Lazebnik, C. Schmid, and J.Ponce, “3D object modeling and recognition using
local affine-invariant image descriptors and multiview spatial constraints,” Int. J. Comput. Vis., vol.
66, no. 3, pp. 231–259, 2006.
