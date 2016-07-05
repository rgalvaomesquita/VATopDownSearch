
#include <opencv2/opencv.hpp>
#include <opencv\cxcore.h>
#include "dirent.h"
#include <ctime>        // std::time

#include <Parameters.h>
#include <BStringTree.hpp>
#include <Homography_Match.hpp>
#include <bloom_filter.hpp>

typedef struct indexedResponses{
	float response;
	int index;
}IndexedResponses;



float timeTD;
int numDesc;
double FPprob;

void train(cv::Mat img, bloom_filter* filters[MAX_DIST + 1]);
int trainTrees(bloom_filter* filters[MAX_DIST + 1], std::stringstream& dirFilters, std::stringstream& dirObjects, std::string* objs, int contObjs);
bool runTDSearch(bloom_filter* filters[MAX_DIST + 1], cv::Mat& scene, cv::Mat* des_obj_collection, int qtObjColl, std::vector<std::vector<cv::KeyPoint>>& kp_obj_collection, float* processingTime, std::vector<int>& sequencePoses, int* qtDetect, cv::Mat*& img_obj_collection, bool verbose, char fileName[], std::string& obj, bool randomKp, float percSalKp, cv::Mat sceneColor, std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, bool orderByResponse, float& propPriorKp);
bool runClassicSearch(cv::Mat& scene, cv::Mat* des_obj_collection, int qtObjColl, std::vector<std::vector<cv::KeyPoint>>& kp_obj_collection, std::vector<int>& sequencePoses, cv::Mat*& img_obj_collection, bool verbose, char fileName[], std::string& obj, std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene);
void getKpOrderedByResponse(std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, float percSalKp, std::vector<cv::KeyPoint> selectedKeypoints[MAX_DIST + 1], cv::Mat descSelected[MAX_DIST + 1], std::vector<cv::KeyPoint>& kpNotSal, cv::Mat& descNotSal);
void getBCTKp(bloom_filter* filters[MAX_DIST + 1], int qtObjColl, std::vector<int>& sequencePoses, cv::Mat& descriptorsFirst, std::vector<cv::KeyPoint> kptsBCT[MAX_DIST + 1], cv::Mat descBCT[MAX_DIST + 1], std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, std::vector<cv::KeyPoint>& kpNotSal, cv::Mat& descNotSal);
void getRandomKp(std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, float percSalKp, std::vector<cv::KeyPoint> selectedKeypoints[MAX_DIST + 1], cv::Mat descSelected[MAX_DIST + 1], std::vector<cv::KeyPoint>& kpNotSal, cv::Mat& descNotSal);
void getKpOrderedByResponse(std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, float percSalKp, std::vector<cv::KeyPoint> selectedKeypoints[MAX_DIST + 1], cv::Mat descSelected[MAX_DIST + 1], std::vector<cv::KeyPoint>& kpNotSal, cv::Mat& descNotSal);
bool compareKeypointsResponses(IndexedResponses a, IndexedResponses b);



unsigned int factorial(unsigned int n)
{
	unsigned int retval = 1;
	for (int i = n; i > 1; --i)
		retval *= i;
	return retval;
}

bool setBloomFilterParameters(bloom_parameters parameters[MAX_DIST + 1], int expectedNumberOfKp, float desiredFPProb)
{
	for (int i = 0; i < MAX_DIST + 1; i++)
	{

		/*if (i <= 2)
			parameters[i].projected_element_count = 16 * 4000;
		else
			parameters[i].projected_element_count = 16 * 4000;*/

		parameters[i].projected_element_count = expectedNumberOfKp;

		for (int j = 0; j < i; j++)
			parameters[i].projected_element_count *= (NBITS - j);
		
		parameters[i].projected_element_count /= factorial(i);
		
		
		parameters[i].false_positive_probability = desiredFPProb;
		parameters[i].random_seed = 0xA5A5A5A5;
		
		if (!parameters[i])
		{
			std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
			return false;
		}

		parameters[i].compute_optimal_parameters();

	}
	//getchar();
	return true;
}

int trainTrees(bloom_filter* filters[MAX_DIST + 1], std::stringstream& dirFilters, std::stringstream& dirObjects, std::string* objs, int contObjs)
{


	char fileFilter[MAX_DIST + 1][100];
	std::ofstream ofsFile[MAX_DIST + 1];
	std::string strfilter = "filterx";

	for (int i = 0; i < MAX_DIST + 1; i++)
	{
		strfilter[strfilter.size() - 1] = std::to_string(i)[0];
		strcpy(fileFilter[i], dirFilters.str().c_str());
		strcat(fileFilter[i], strfilter.c_str());
		std::cout << fileFilter[i] << std::endl;
	}


	char fileObject[100];
	struct dirent *dp;
	DIR *fd;

	if ((fd = opendir(dirObjects.str().c_str())) == NULL) {
		fprintf(stderr, "listdir: can't open %s\n", dirObjects);
		return 0;
	}


	int objCounter = 0;

	while ((dp = readdir(fd)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;    /* skip self and parent */

		std::cout << "generating pose " << objCounter + 1 << std::endl;
		strcpy(fileObject, dirObjects.str().c_str());
		strcat(fileObject, dp->d_name);
		cv::Mat obj = cv::imread(fileObject, cv::IMREAD_GRAYSCALE);

		

		train(obj, filters);


		objCounter++;
		

	}
	closedir(fd);

	for (int i = 0; i < MAX_DIST + 1; i++)
	{
		filters[i]->serialize(fileFilter[i]);
	}

}
void train(cv::Mat img, bloom_filter* filters[MAX_DIST + 1])
{
	cv::Mat descriptor;


	cv::Ptr<cv::AKAZE> akaze = cv::AKAZE::create(5, 0, 3);

	std::vector<cv::KeyPoint> kpts;

	akaze->detectAndCompute(img, cv::noArray(), kpts, descriptor);

	cv::Mat descriptorRange(descriptor.rows, NBITS/8, descriptor.type());
	uchar* pDes;
	uchar* pDesRange;
	for (int i = 0; i < descriptorRange.rows; i++)
	{

		pDesRange = descriptorRange.ptr<uchar>(i);

		pDes = descriptor.ptr<uchar>(i);
		for (int j = 0; j < descriptorRange.cols; j++)
		{

			pDesRange[j] = pDes[j];

		}

	}

	BStringTree bstTrain(NBITS, descriptorRange, KNN_RATIO, MAX_DIST, filters, MAX_DIST + 1);


	std::cout << "FINISH!\n";


}


bool runClassicSearch( cv::Mat& scene, cv::Mat* des_obj_collection, int qtObjColl, std::vector<std::vector<cv::KeyPoint>>& kp_obj_collection, std::vector<int>& sequencePoses, cv::Mat*& img_obj_collection, bool verbose, char fileName[], std::string& obj, std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene)
{


	InsertedMatches* trainIdxGoodMatches;

	std::vector<cv::DMatch> good_matches;
	bool encontrou;
	
	for (int poseInd = 0; poseInd < qtObjColl; poseInd++)
	{
		int pose = sequencePoses[poseInd];
		
		good_matches.clear();
	

		trainIdxGoodMatches = (InsertedMatches*)malloc(kp_obj_collection[pose].size()*sizeof(InsertedMatches));
		for (int i = 0; i < kp_obj_collection[pose].size(); i++)
		{
			trainIdxGoodMatches[i].distance = -1;
			trainIdxGoodMatches[i].good_matches_idx = -1;
		}
		std::vector<cv::KeyPoint> kp_image_final;
		kp_image_final.clear();
		bool matchOcurred = applyMatch(des_obj_collection[pose], descriptorScene, kptsScene, good_matches, KNN_RATIO, kp_image_final, trainIdxGoodMatches);

		std::string msg;

		if (matchOcurred)
			encontrou = testHomography(good_matches, kp_obj_collection[pose], kp_image_final, msg, scene.rows*scene.cols);
		else
			continue;
		if (encontrou)
		{

			
			if (verbose)
			{


				
				cv::Mat matchImg;
				scene.copyTo(matchImg);
				drawMatches(scene, kp_image_final, img_obj_collection[pose], kp_obj_collection[pose], good_matches, matchImg);
				std::stringstream st2;
				st2 << ROOT_DIR.c_str() << DIR_IMG_MATCHES.c_str();
				CreateDirectoryA(st2.str().c_str(), NULL);
				st2 << "\\obj_" << obj << "_scene_" << fileName << "_CLASSIC_SEARCH.jpg";

				imwrite(st2.str(), matchImg);


			}
			free(trainIdxGoodMatches);
		

			return true;

		}
		free(trainIdxGoodMatches);
	}
	return false;

}

bool compareKeypointsResponses(IndexedResponses a, IndexedResponses b)
{

	return a.response > b.response;

}


void getKpOrderedByResponse(std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, float percSalKp, std::vector<cv::KeyPoint> prioritizedKpts[MAX_DIST + 1], cv::Mat prioritizedDesc[MAX_DIST + 1], std::vector<cv::KeyPoint>& remainingKpts, cv::Mat& remainingDesc)
{
	std::vector<IndexedResponses> sortedKp;
	for (int i = 0; i < kptsScene.size(); i++)
	{
		IndexedResponses ir;
		ir.index = i;
		ir.response = kptsScene[i].response;
		sortedKp.push_back(ir);
	}
	std::sort(sortedKp.begin(), sortedKp.end(), compareKeypointsResponses);


	int lastRndKp = kptsScene.size()*percSalKp;

	for (int i = 0; i < kptsScene.size(); i++)
	{
		bool flagInserted = false;
		for (int j = 1; j <= MAX_DIST + 1; j++)
		{
			if (i < j*lastRndKp / 3.0)
			{

				prioritizedKpts[j-1].push_back(kptsScene[sortedKp[i].index]);
				prioritizedDesc[j-1].push_back(descriptorScene.row(sortedKp[i].index));
				flagInserted = true;
				break;
			}
		}
		
		if(!flagInserted){
			remainingKpts.push_back(kptsScene[sortedKp[i].index]);
			remainingDesc.push_back(descriptorScene.row(sortedKp[i].index));
		}
	}
}


void getRandomKp(std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, float percSalKp, std::vector<cv::KeyPoint> prioritizedKpts[MAX_DIST + 1], cv::Mat prioritizedDesc[MAX_DIST + 1], std::vector<cv::KeyPoint>& remainingKpts, cv::Mat& remainingDesc)
{
	srand((unsigned)time(0));
	std::vector<int> rndKp;
	for (int i = 0; i < kptsScene.size(); i++)
		rndKp.push_back(i);

	std::random_shuffle(rndKp.begin(), rndKp.end());
	int lastRndKp = kptsScene.size()*percSalKp;

	for (int i = 0; i < kptsScene.size(); i++)
	{

		bool flagInserted = false;
		for (int j = 1; j <= MAX_DIST + 1; j++)
		{
			if (i < j*lastRndKp / 3.0)
			{

				prioritizedKpts[j-1].push_back(kptsScene[rndKp[i]]);
				prioritizedDesc[j-1].push_back(descriptorScene.row(rndKp[i]));
				flagInserted = true;
				break;
			}
		}

		if (!flagInserted){
			remainingKpts.push_back(kptsScene[rndKp[i]]);
			remainingDesc.push_back(descriptorScene.row(rndKp[i]));
		}

		
	}
}

void getBCTKp(bloom_filter* filters[MAX_DIST + 1], int qtObjColl, std::vector<int>& sequencePoses, cv::Mat& descriptorsFirst, std::vector<cv::KeyPoint> prioritizedKpts[MAX_DIST + 1], cv::Mat prioritizedDesc[MAX_DIST + 1], std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, std::vector<cv::KeyPoint>& remainingKpts, cv::Mat& remainingDesc)
{



	
	BStringTree::bitchain searchString(NBITS,'0');
	//BStringTree::bitchain searchString(0);
	for (int i = 0; i < descriptorsFirst.rows; i++)
	{


		uchar* ptr;

		int temp, firstBit;
		ptr = descriptorsFirst.ptr<uchar>(i);

		int sumD1AllChains = 0;
		bool existOnFilter = false;




		firstBit = 0;

		for (int j = NBITS/8 - 1; j >= 0; j--)
		{
			temp = ptr[j];

			for (int bit = firstBit; bit < firstBit + 8; bit++)
			{

				searchString[bit] = temp % 2 == 0? '0' : '1';
				temp = temp / 2;
			}
			firstBit += 8;
		}

		existOnFilter = false;
		for (int ind_filter = 0; ind_filter < MAX_DIST + 1; ind_filter++)
		{
			if (filters[ind_filter]->contains(searchString))
			{

				prioritizedKpts[ind_filter].push_back(kptsScene[i]);
				prioritizedDesc[ind_filter].push_back(descriptorScene.row(i));
				existOnFilter = true;

				break;
			}

		}

		if (!existOnFilter)
		{

			remainingKpts.push_back(kptsScene[i]);
			remainingDesc.push_back(descriptorScene.row(i));

		}

	}

}



bool runTDSearch(bloom_filter* filters[MAX_DIST + 1], cv::Mat& scene, cv::Mat* des_obj_collection, int qtObjColl, std::vector<std::vector<cv::KeyPoint>>& kp_obj_collection, float* processingTime, std::vector<int>& sequencePoses, cv::Mat*& img_obj_collection, bool verbose, char fileName[], std::string& obj, bool randomKp, float percSalKp, cv::Mat sceneColor, std::vector<cv::KeyPoint>& kptsScene, cv::Mat& descriptorScene, bool orderByResponse, float& propPriorKp)
{

	float propPriorKPScene = 0;

	float dist = std::min(scene.rows, scene.cols)*0.05;
	std::vector<cv::KeyPoint>* kp_image_final;
	kp_image_final = new std::vector<cv::KeyPoint>[qtObjColl];
	std::vector<cv::DMatch>* good_matches;
	good_matches = new std::vector<cv::DMatch>[qtObjColl];

	InsertedMatches** trainIdxGoodMatches;
	trainIdxGoodMatches = new InsertedMatches*[qtObjColl];


	cv::normalize(descriptorScene, descriptorScene, 0, 255, cv::NORM_MINMAX, CV_8UC1);


	std::vector<cv::KeyPoint> prioritizedKpts[MAX_DIST + 1];


	cv::Mat prioritizedDesc[MAX_DIST + 1];


	std::vector<cv::KeyPoint> remainingKpts;

	cv::Mat remainingDesc;

	std::vector<cv::KeyPoint> kptsFarMatched;

	cv::Mat descFarMatched;

	if (orderByResponse)
	{

		getKpOrderedByResponse(kptsScene, descriptorScene, percSalKp, prioritizedKpts, prioritizedDesc, remainingKpts, remainingDesc);

	}
	else if (randomKp){
		getRandomKp(kptsScene, descriptorScene, percSalKp, prioritizedKpts, prioritizedDesc, remainingKpts, remainingDesc);


	}
	else
	{
		cv::Mat descriptorsFirst(descriptorScene.rows, NBITS/8, descriptorScene.type());


		uchar* pDesFirst;
		uchar* pDesc1;
		for (int i = 0; i < descriptorsFirst.rows; i++)
		{

			pDesFirst = descriptorsFirst.ptr<uchar>(i);

			pDesc1 = descriptorScene.ptr<uchar>(i);
			for (int j = 0; j < descriptorsFirst.cols; j++)
			{
				pDesFirst[j] = pDesc1[j];

			}

		}
		int ttd0 = cv::getTickCount();
		getBCTKp(filters, qtObjColl, sequencePoses, descriptorsFirst, prioritizedKpts, prioritizedDesc, kptsScene, descriptorScene, remainingKpts, remainingDesc);
		int ttd1 = cv::getTickCount();
		timeTD += (ttd1 - ttd0) / cv::getTickFrequency();

		for (int d = 0; d < MAX_DIST + 1; d++)
		{
			propPriorKPScene += prioritizedKpts[d].size();
		}

		propPriorKPScene /= ((float)kptsScene.size());
		propPriorKp += propPriorKPScene;
		numDesc += descriptorScene.rows;
		
	}



	//search for prioritized keypoints


	for (int poseInd = 0; poseInd < qtObjColl; poseInd++)
	{
		int pose = sequencePoses[poseInd];

		bool encontrou = false;
		trainIdxGoodMatches[pose] = (InsertedMatches*)malloc(kp_obj_collection[pose].size()*sizeof(InsertedMatches));
		for (int i = 0; i < kp_obj_collection[pose].size(); i++)
		{
			trainIdxGoodMatches[pose][i].distance = -1;
			trainIdxGoodMatches[pose][i].good_matches_idx = -1;
		}

		for (int group = 0; group < MAX_DIST + 1; group++)
		{

			bool matchOcurred = applyMatch(des_obj_collection[pose], prioritizedDesc[group], prioritizedKpts[group], good_matches[pose], KNN_RATIO, kp_image_final[pose], trainIdxGoodMatches[pose]);

			std::string msg;

			if (matchOcurred)
			{
				encontrou = testHomography(good_matches[pose], kp_obj_collection[pose], kp_image_final[pose], msg, scene.rows*scene.cols);

			}
			else
			{

				continue;
			}
			if (encontrou)
			{
				if (verbose)
				{
					cv::Mat matchImg;
					scene.copyTo(matchImg);
					drawMatches(scene, kp_image_final[pose], img_obj_collection[pose], kp_obj_collection[pose], good_matches[pose], matchImg);
					std::stringstream st2;
					st2 << ROOT_DIR.c_str() << DIR_IMG_MATCHES.c_str();
					CreateDirectoryA(st2.str().c_str(), NULL);
					
					if (randomKp)
						st2 << "\\obj_" << obj << "_scene_" << fileName << "_RND.jpg";
					else if (orderByResponse)
						st2 << "\\obj_" << obj << "_scene_" << fileName << "_SBR.jpg";
					else
						st2 << "\\obj_" << obj << "_scene_" << fileName << "_BloomFilter_TDAttention.jpg";
					imwrite(st2.str(), matchImg);





				}

				for (int i = 0; i <= poseInd; i++)
					free(trainIdxGoodMatches[sequencePoses[i]]);
				delete[] trainIdxGoodMatches;

				return true;
			}
		}
	}




	std::vector<cv::KeyPoint> kptsNearMatches;
	cv::Mat descNearMatches;
	for (int i = 0; i < remainingKpts.size(); i++)
	{

		bool flagInsertedNearMatch = false;
		for (int poseInd = 0; poseInd < qtObjColl; poseInd++)
		{
			int pose = sequencePoses[poseInd];
			for (int j = 0; j < kp_image_final[pose].size(); j++)
			{

				if (sqrtf(powf(remainingKpts[i].pt.x - kp_image_final[pose][j].pt.x, 2) + powf(remainingKpts[i].pt.y - kp_image_final[pose][j].pt.y, 2)) < dist)
				{
					flagInsertedNearMatch = true;
					kptsNearMatches.push_back(remainingKpts[i]);
					descNearMatches.push_back(remainingDesc.row(i));
					break;
				}

			}
			if (flagInsertedNearMatch)
			{
				break;
			}

		}

		if (!flagInsertedNearMatch)
		{
			kptsFarMatched.push_back(kptsScene[i]);
			descFarMatched.push_back(descriptorScene.row(i));
		}


	}




	//search for keypoints near matches
	for (int poseInd = 0; poseInd < qtObjColl; poseInd++)
	{
		int pose = sequencePoses[poseInd];

		bool encontrou = false;


		bool matchOcurred = applyMatch(des_obj_collection[pose], descNearMatches, kptsNearMatches, good_matches[pose], KNN_RATIO, kp_image_final[pose], trainIdxGoodMatches[pose]);

		std::string msg;

		if (matchOcurred)
		{
			encontrou = testHomography(good_matches[pose], kp_obj_collection[pose], kp_image_final[pose], msg, scene.rows*scene.cols);

		}
		else
		{
			continue;
		}
		if (encontrou)
		{

			
			if (verbose)
			{
				cv::Mat matchImg;
				scene.copyTo(matchImg);
				drawMatches(scene, kp_image_final[pose], img_obj_collection[pose], kp_obj_collection[pose], good_matches[pose], matchImg);
				std::stringstream st2;
				st2 << ROOT_DIR.c_str() << DIR_IMG_MATCHES.c_str();
				CreateDirectoryA(st2.str().c_str(), NULL);

				if (randomKp)
					st2 << "\\obj_" << obj << "_scene_" << fileName << "_RND_MatchNear.jpg";
				else if (orderByResponse)
					st2 << "\\obj_" << obj << "_scene_" << fileName << "_SBR_MatchNear.jpg";
				else
					st2 << "\\obj_" << obj << "_scene_" << fileName << "_BloomFilter_TDAttention_MatchNear.jpg";
				imwrite(st2.str(), matchImg);




			}


			for (int i = 0; i < qtObjColl; i++)
				free(trainIdxGoodMatches[i]);
			delete[] trainIdxGoodMatches;

			return true;

		}





	}//close poses loop

	//----end-----search for keypoints near not matched BCT keypoints

	if (!SEARCH_ALL_KP)
		return  false;

	//search using the rest of the keypoints 
	for (int poseInd = 0; poseInd < qtObjColl; poseInd++)
	{
		int pose = sequencePoses[poseInd];

		bool encontrou = false;


		bool matchOcurred = applyMatch(des_obj_collection[pose], descFarMatched, kptsFarMatched, good_matches[pose], KNN_RATIO, kp_image_final[pose], trainIdxGoodMatches[pose]);

		std::string msg;

		if (matchOcurred)
		{
			encontrou = testHomography(good_matches[pose], kp_obj_collection[pose], kp_image_final[pose], msg, scene.rows*scene.cols);

		}
		else
		{
			continue;
		}
		if (encontrou)
		{


			
			if (verbose)
			{



				cv::Mat matchImg;
				scene.copyTo(matchImg);
				drawMatches(scene, kp_image_final[pose], img_obj_collection[pose], kp_obj_collection[pose], good_matches[pose], matchImg);
				std::stringstream st2;
				st2 << ROOT_DIR.c_str() << DIR_IMG_MATCHES.c_str();
				CreateDirectoryA(st2.str().c_str(), NULL);

				if (randomKp)
					st2 << "\\obj_" << obj << "_scene_" << fileName << "_RND_MatchAll.jpg";
				else if (orderByResponse)
					st2 << "\\obj_" << obj << "_scene_" << fileName << "_SBR_MatchAll.jpg";
				else
					st2 << "\\obj_" << obj << "_scene_" << fileName << "_BloomFilter_TDAttention_MatchAll.jpg";
				imwrite(st2.str(), matchImg);
			}


			for (int i = 0; i < qtObjColl; i++)
				free(trainIdxGoodMatches[i]);
			delete[] trainIdxGoodMatches;


			return true;

		}





	}//close poses loop




	for (int i = 0; i < qtObjColl; i++)
		free(trainIdxGoodMatches[i]);
	delete[] trainIdxGoodMatches;


	return false;


}
