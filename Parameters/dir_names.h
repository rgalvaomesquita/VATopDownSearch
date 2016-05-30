#include <iostream>

//locations in which test scene images are stored
const std::string DIR_IMGS_TEST = "R:\\Doutorado\\busca visual\\bases de dados\\ponce\\imagens teste\\";
//locations in which object images are stored
const std::string DIR_OBJECTS = "R:\\Doutorado\\busca visual\\bases de dados\\ponce\\objects\\";

//file names for each object of the dataset
std::string objs[8] = { "01", "02", "03", "04", "05", "06", "07", "08" };
//percentual of keypoints selected to be prioritized during RANDOM_SEARCH 
float percKpSal[] = { 1.97, 0.08, 0.05, 0.04, 0.03, 0.05, 0.04, 0.02 };

const bool GT[8][51] = {
	{ true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, true, true, true, false, false, false, false, false, false, false, false, false, false, true, true, false, false, true, true, true, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true },
};


const std::string ROOT_DIR = "R:\\Doutorado\\ICIP 2016\\experimentos\\";
//text files results
const std::string DIR_TXT_RESULTS = "results\\";
//objects keypoints file
const std::string DIR_KP_OBJECTS = "AKAZE_kp_objects_th=0.001\\";
//objects descriptors file
const std::string DIR_DES_OBJECTS = "AKAZE_des_objects_th=0.001\\";
//objects bloom filters file
const std::string DIR_FILTERS = "filters_akaze\\";
//location to save images of matches between object and scene images, in case VERBOSE parameter is set to true
const std::string DIR_IMG_MATCHES = "img_matches\\";

const int expectedNumKp = 16 * 4000;
const float desiredFPProb = 0.001;
