
#include <opencv2/opencv.hpp>
#include <opencv\cxcore.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include "dirent.h"
#include <Parameters.h>
#include <serializablebloom\bloom_filter.hpp>

int readFileToMat(cv::Mat &I, std::string path);
void read_object(char fileKpObject[100], std::vector<cv::KeyPoint>& kp_object_vec, char fileDesObject[100], cv::Mat& des_object_vec);
int writeMatToFile(const cv::Mat &I, std::string path);
void readBloomFilterByObject(bloom_filter* filters[MAX_DIST + 1], std::stringstream& dirFilters);
bool generateKpDesPosesObject(cv::Ptr<cv::AKAZE>& akaze, char* objectsDir, char* kpObjectsDir, char* desObjectsDir);
void readPosesObject(std::stringstream& objectsDir, std::stringstream& kpObjectsDir, std::stringstream& desObjectsDir, std::string* objs, std::vector <std::vector<cv::KeyPoint>>& kp_obj_collection, cv::Mat*& des_obj_collection, cv::Mat*& img_obj_collection, std::vector<int>& sequence, bloom_filter* filters[MAX_DIST + 1], std::stringstream filtersDir[1]);
int myrandom(int i);



int myrandom(int i) 
{ 
	return std::rand() % i; 
}

void readPosesObject(std::stringstream& objectsDir, std::stringstream& kpObjectsDir, std::stringstream& desObjectsDir, std::string* objs, std::vector <std::vector<cv::KeyPoint>>& kp_obj_collection, cv::Mat*& des_obj_collection, cv::Mat*& img_obj_collection, std::vector<int>& sequence, bloom_filter* filters[MAX_DIST + 1], std::stringstream filtersDir[1])
{


	kp_obj_collection.clear();
	std::vector<cv::KeyPoint> kp_object_iter;
	cv::Mat des_object_iter;


	char fileKpObject[100];
	char fileDesObject[100];

	char fileObject[100];
	char fileBCT[100];
	char fileBCT2[100];
	struct dirent *dp;
	DIR *fd;

	if ((fd = opendir(objectsDir.str().c_str())) == NULL) {
		fprintf(stderr, "listdir: can't open %s\n", objectsDir);
		getch();
		exit(0);
	}

	//count how many files exists in the directory
	int objCounter = 0;
	while ((dp = readdir(fd)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;    /* skip self and parent */
		objCounter++;
	}
	closedir(fd);
	int qtObjects = objCounter;
	
	des_obj_collection = new cv::Mat[qtObjects];


	for (int i = 0; i < qtObjects; i++)
	{
		des_obj_collection[i].create(2, 2, CV_32F);
	}

	img_obj_collection = new cv::Mat[qtObjects];
	for (int i = 0; i < qtObjects; i++)
	{
		img_obj_collection[i].create(2, 2, CV_32F);
	}
	objCounter = 0;


	
	
	readBloomFilterByObject(filters, filtersDir[0]);


	char fileNameWithoutExtension[40];

	if ((fd = opendir(objectsDir.str().c_str())) == NULL) {
		fprintf(stderr, "listdir: can't open %s\n", objectsDir);
		getch();
		exit(0);
	}


	sequence.clear();
	while ((dp = readdir(fd)) != NULL)
	{
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;    /* skip self and parent */
		strcpy(fileObject, objectsDir.str().c_str());
		strcat(fileObject, dp->d_name);


		std::cout << "image file: " << fileObject << std::endl;
		img_obj_collection[objCounter] = cv::imread(fileObject, cv::IMREAD_GRAYSCALE);
		strcpy(fileNameWithoutExtension, dp->d_name);
		for (int i = 0; i < sizeof(fileNameWithoutExtension) / sizeof(char); i++)
		{
			if (fileNameWithoutExtension[i] == '.')
				fileNameWithoutExtension[i] = '\0';
		}

		strcpy(fileKpObject, kpObjectsDir.str().c_str());
		strcat(fileKpObject, fileNameWithoutExtension);
		strcpy(fileDesObject, desObjectsDir.str().c_str());
		strcat(fileDesObject, fileNameWithoutExtension);

		kp_object_iter.clear();
		read_object(fileKpObject, kp_object_iter, fileDesObject, des_obj_collection[objCounter]);
		kp_obj_collection.push_back(kp_object_iter);
		sequence.push_back(objCounter);
		objCounter++;

	}
	std::random_shuffle(sequence.begin(), sequence.end(), myrandom);
	closedir(fd);


}


bool generateKpDesPosesObject(cv::Ptr<cv::AKAZE>& akaze, char* objectsDir, char* dirKpObjects, char* desObjectsDir)
{

	cv::Mat object;
	cv::Mat descriptor;
	FILE *fptrKp;
	FILE *fptrDes;
	
	char fileObject[100];

	char fileKpObject[100];
	char fileDesObject[100];
	char fileName[100];
	struct dirent *dp;

	DIR *fd;

	int contObjects = 0;
	cv::Mat msumObject;
	cv::Mat sumObj;
	cv::Mat objectKP;
	std::vector<cv::KeyPoint> kp_object_iter;


	if ((fd = opendir(objectsDir)) == NULL) {
		fprintf(stderr, "listdir: can't open %s\n", objectsDir);
		return 0;
	}

	while ((dp = readdir(fd)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;    /* skip self and parent */

		strcpy(fileObject, objectsDir);

		strcat(fileObject, dp->d_name);


		std::cout << "dir obj: " << fileObject << std::endl;
	
		object = cv::imread(fileObject, cv::IMREAD_GRAYSCALE);



		akaze->detectAndCompute(object, cv::noArray(), kp_object_iter, descriptor);
		cv::normalize(descriptor, descriptor, 0, 255, cv::NORM_MINMAX, CV_8UC1);

		cv::KeyPoint* kp_vec = (cv::KeyPoint*)malloc(kp_object_iter.size()*sizeof(cv::KeyPoint));
		for (int i = 0; i < kp_object_iter.size(); i++)
			kp_vec[i] = kp_object_iter[i];
		strcpy(fileKpObject, dirKpObjects);
		strcpy(fileDesObject, desObjectsDir);
		strcpy(fileName, dp->d_name);
		for (int i = 0; i < sizeof(fileName) / sizeof(char); i++)
		{
			if (fileName[i] == '.')
				fileName[i] = '\0';
		}
		strcat(fileKpObject, fileName);
		strcat(fileDesObject, fileName);
		
		fptrKp = fopen(fileKpObject, "wb");
		fwrite(kp_vec, sizeof(cv::KeyPoint), kp_object_iter.size(), fptrKp);
		fclose(fptrKp);



		writeMatToFile(descriptor, fileDesObject);

		contObjects++;
	}
}

void readBloomFilterByObject(bloom_filter* filters[MAX_DIST + 1], std::stringstream& filtersDir)
{
	DIR *fd;
	struct dirent *dp;


	char fileFilter[MAX_DIST + 1][100];
	std::ifstream ifsFile[MAX_DIST + 1];
	std::string strfilter = "filterx";

	for (int i = 0; i < MAX_DIST + 1; i++)
	{
		strfilter[strfilter.size() - 1] = std::to_string(i)[0];
		strcpy(fileFilter[i], filtersDir.str().c_str());
		strcat(fileFilter[i], strfilter.c_str());
		ifsFile[i].open(fileFilter[i]);
		std::cout << fileFilter[i] << std::endl;
	}
	//Read BCTs



	if ((fd = opendir(filtersDir.str().c_str())) == NULL) {
		fprintf(stderr, "listdir: can't open %s\n", filtersDir);
		getch();
		exit(0);
	}





	for (int i = 0; i < MAX_DIST + 1; i++)
	{
		filters[i]->deserialize(fileFilter[i]);
	}




	closedir(fd);


}

int readFileToMat(cv::Mat &I, std::string path) {

	//declare image parameters
	int matWidth, matHeight, type;

	//declare values to be written
	float fvalue;
	double dvalue;
	cv::Vec3f vfvalue;
	cv::Vec3d vdvalue;

	//create the file stream
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
		return -1;

	//read type and size of the matrix first
	file.read((char*)&type, sizeof(type));
	file.read((char*)&matWidth, sizeof(matWidth));
	file.read((char*)&matHeight, sizeof(matHeight));

	//change Mat type
	I = cv::Mat::zeros(matHeight, matWidth, type);

	//write data depending on the image's type
	switch (type)
	{
	default:
		std::cout << "Error: wrong Mat type: must be CV_32F, CV_64F, CV_32FC3 or CV_64FC3" << std::endl;
		break;
		// UCHAR ONE CHANNEL
	case CV_8U:
		std::cout << "Reading CV_32F image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			file.read((char*)&fvalue, sizeof(fvalue));
			I.at<uchar>(i) = fvalue;
		}
		break;
		// FLOAT ONE CHANNEL
	case CV_32F:
		std::cout << "Reading CV_32F image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			file.read((char*)&fvalue, sizeof(fvalue));
			I.at<float>(i) = fvalue;
		}
		break;
		// DOUBLE ONE CHANNEL
	case CV_64F:
		std::cout << "Reading CV_64F image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			file.read((char*)&dvalue, sizeof(dvalue));
			I.at<double>(i) = dvalue;
		}
		break;

		// FLOAT THREE CHANNELS
	case CV_32FC3:
		std::cout << "Reading CV_32FC3 image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			file.read((char*)&vfvalue, sizeof(vfvalue));
			I.at<cv::Vec3f>(i) = vfvalue;
		}
		break;

		// DOUBLE THREE CHANNELS
	case CV_64FC3:
		std::cout << "Reading CV_64FC3 image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			file.read((char*)&vdvalue, sizeof(vdvalue));
			I.at<cv::Vec3d>(i) = vdvalue;
		}
		break;

	}

	//close file
	file.close();

	return 0;
}

void read_object(char fileKpObject[100], std::vector<cv::KeyPoint>& kp_object_vec, char fileDesObject[100], cv::Mat& des_object_vec)
{

	FILE* fptrKp;
	std::vector<cv::KeyPoint> kp_vec_temp;
	cv::KeyPoint kp_temp;
	cv::Mat des_temp;



	std::cout << "kp_objects file: " << fileKpObject << std::endl;
	fptrKp = fopen(fileKpObject, "rb");
	if (fptrKp == NULL)
	{
		std::cout << "error reading kp_object file " << std::endl;
		std::fclose(fptrKp);
		getch();
		exit(1);
	}

	while (!feof(fptrKp))
	{

		fread(&kp_temp, sizeof(cv::KeyPoint), 1, fptrKp);
		if (feof(fptrKp))
			break;
		kp_object_vec.push_back(kp_temp);
	}

	std::fclose(fptrKp);




	std::cout << "des_objects file: " << fileDesObject << std::endl;
	
	readFileToMat(des_object_vec, fileDesObject);



}

int writeMatToFile(const cv::Mat &I, std::string path) {

	//load the matrix size
	int matWidth = I.size().width, matHeight = I.size().height;

	//read type from Mat
	int type = I.type();

	//declare values to be written
	float fvalue;
	double dvalue;
	cv::Vec3f vfvalue;
	cv::Vec3d vdvalue;

	//create the file stream
	std::ofstream file(path.c_str(), std::ios::out | std::ios::binary);
	if (!file)
		return -1;

	//write type and size of the matrix first
	file.write((const char*)&type, sizeof(type));
	file.write((const char*)&matWidth, sizeof(matWidth));
	file.write((const char*)&matHeight, sizeof(matHeight));

	//write data depending on the image's type
	switch (type)
	{
	default:
		std::cout << "Error: wrong Mat type: must be CV_32F, CV_64F, CV_32FC3 or CV_64FC3" << std::endl;
		break;
		// UCHAR ONE CHANNEL
	case CV_8U:
		std::cout << "Writing CV_8U image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			fvalue = I.at<uchar>(i);
			file.write((const char*)&fvalue, sizeof(fvalue));
		}
		break;
		// FLOAT ONE CHANNEL
	case CV_32F:
		std::cout << "Writing CV_32F image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			fvalue = I.at<float>(i);
			file.write((const char*)&fvalue, sizeof(fvalue));
		}
		break;
		// DOUBLE ONE CHANNEL
	case CV_64F:
		std::cout << "Writing CV_64F image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			dvalue = I.at<double>(i);
			file.write((const char*)&dvalue, sizeof(dvalue));
		}
		break;

		// FLOAT THREE CHANNELS
	case CV_32FC3:
		std::cout << "Writing CV_32FC3 image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			vfvalue = I.at<cv::Vec3f>(i);
			file.write((const char*)&vfvalue, sizeof(vfvalue));
		}
		break;

		// DOUBLE THREE CHANNELS
	case CV_64FC3:
		std::cout << "Writing CV_64FC3 image" << std::endl;
		for (int i = 0; i < matWidth*matHeight; ++i) {
			vdvalue = I.at<cv::Vec3d>(i);
			file.write((const char*)&vdvalue, sizeof(vdvalue));
		}
		break;

	}

	//close file
	file.close();

	return 0;
}
