
#include "windows.h"
#include <opencv2/opencv.hpp>
#include <opencv\cxcore.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <conio.h>
#include "opencv2/features2d.hpp"


#include <Input_Output.hpp>
#include <TDAttention.hpp>
#include <Parameters.h>
#include <dir_names.h>

using namespace std;




int main(void)
{


	int TP_classic, TN_classic, FP_classic, FN_classic;
	int TP_obr, TN_obr, FP_obr, FN_obr;
	int TP_rnd, TN_rnd, FP_rnd, FN_rnd;
	int TP_tdbf, TN_tdbf, FP_tdbf, FN_tdbf;
	ofstream outputExecTime;


	std::srand(unsigned(std::time(0)));

	vector<cv::KeyPoint> kpts1, kpts2;

	cv::Mat desc1, desc2;

	

	float th = 0.001;
	cv::Ptr<cv::AKAZE> akaze = cv::AKAZE::create(5, 0, 3, th);

	cv::Mat* des_obj_collection;

	std::vector<std::vector<cv::KeyPoint>> kp_obj_collection;
	cv::Mat* img_obj_collection;

	std::vector<int> sequence;

	
	

	/// bloom filter's settings
	bloom_parameters parameters[MAX_DIST + 1];
	for (int i = 0; i < MAX_DIST + 1; i++)
	{
		
		if (i <= 2)
			parameters[i].projected_element_count = 16 * 5000;
		else
			parameters[i].projected_element_count = 16 * 1000;

		for (int j = 0; j < i; j++)
			parameters[i].projected_element_count *= (NBITS - j);
		parameters[i].false_positive_probability = 0.001;
		parameters[i].random_seed = 0xA5A5A5A5;
		if (!parameters[i])
		{
			std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
			return 1;
		}

		parameters[i].compute_optimal_parameters();
		
	}

	CreateDirectoryA(ROOT_DIR.c_str(), NULL);

	for (int iter = 0; iter < 1; iter++)
		for (int contObjs = FIRST_OBJECT; contObjs < LAST_OBJECT; contObjs++)
		{


			std::vector<float> propBCT;
			int qtAcertosSal = 0;
			std::stringstream pathTxtExecTime;
			pathTxtExecTime.str(std::string());
			pathTxtExecTime << ROOT_DIR.c_str() << DIR_TXT_RESULTS.c_str();
			CreateDirectoryA(pathTxtExecTime.str().c_str(), NULL);
			pathTxtExecTime <<  objs[contObjs] << ".txt";
			
			
			std::stringstream dirObjects;
			dirObjects.str(std::string());
			dirObjects << DIR_OBJECTS.c_str() << objs[contObjs] << "\\";
			
			std::stringstream dirKpObjects;
			dirKpObjects.str(std::string());
			dirKpObjects << ROOT_DIR.c_str() << DIR_KP_OBJECTS.c_str();
			if (GENERATE_KP_DES)
				CreateDirectoryA(dirKpObjects.str().c_str(), NULL);
			dirKpObjects << objs[contObjs] << "\\";
			if (GENERATE_KP_DES)
				CreateDirectoryA(dirKpObjects.str().c_str(), NULL);
			
			std::stringstream dirDesObjects;
			dirDesObjects.str(std::string());
			dirDesObjects << ROOT_DIR.c_str() << DIR_DES_OBJECTS.c_str();
			if (GENERATE_KP_DES)
				CreateDirectoryA(dirDesObjects.str().c_str(), NULL);
			dirDesObjects << objs[contObjs] << "\\";
			if (GENERATE_KP_DES)
				CreateDirectoryA(dirDesObjects.str().c_str(), NULL);
			
			if (GENERATE_KP_DES)
			{
				generateKpDesPosesObject(akaze, &dirObjects.str()[0], &dirKpObjects.str()[0], &dirDesObjects.str()[0]);
				std::cout << "object generated";

				continue;
			}

			

			bloom_filter* filters[MAX_DIST + 1];
			for (int j = 0; j < MAX_DIST + 1; j++)
			{
				filters[j] = new bloom_filter(parameters[j]);

			}

			

			std::stringstream dirFilters;
			dirFilters.str(std::string());
			dirFilters << ROOT_DIR.c_str() << DIR_FILTERS.c_str();
			if (TRAIN)
				CreateDirectoryA(dirFilters.str().c_str(), NULL);
			dirFilters << "NBITS_" << NBITS << "_MAX_DIST_" << MAX_DIST << "\\";
			if (TRAIN)
				CreateDirectoryA(dirFilters.str().c_str(), NULL);
			dirFilters << objs[contObjs] << "\\";

			std::cout << "filters dir: " << dirFilters.str() << std::endl;

			if (TRAIN)
			{
				
				CreateDirectoryA(dirFilters.str().c_str(), NULL);
				trainTrees(filters, dirFilters, dirObjects, objs, contObjs);
				for (int j = 0; j < MAX_DIST + 1; j++)
				{
					std::cout << "expected false probability rate for filter " << j << ": " << filters[j]->effective_fpp() << std::endl;
					delete filters[j];
				}
				
				continue;
			}

			readPosesObject(dirObjects, dirKpObjects, dirDesObjects, objs, kp_obj_collection, des_obj_collection, img_obj_collection, sequence, filters, &dirFilters);
			


			TP_classic = 0; TP_obr = 0; TP_rnd = 0; TP_tdbf = 0;
			TN_classic = 0; TN_obr = 0; TN_rnd = 0; TN_tdbf = 0;
			FP_classic = 0; FP_obr = 0; FP_rnd = 0; FP_tdbf = 0;
			FN_classic = 0; FN_obr = 0; FN_rnd = 0; FN_tdbf = 0;


			std::vector<float> time_obr;
			std::vector<float> time_tdbf;
			std::vector<float> time_classic;
			std::vector<float> time_rnd;
			time_obr.clear();
			DIR *fdImgTeste;
			struct dirent *dp0;
			
			if ((fdImgTeste = opendir(DIR_IMGS_TEST.c_str())) == NULL)
			{
				fprintf(stderr, "listdir: can't open %s\n", dirObjects);
				return 0;
			}
			int contImg = 0;
			cv::Mat scene;
			std::stringstream imgFileTemp;
			float totalTimeClassic = 0;
			float totalTimeObr = 0;
			float totalTimeBctAll = 0;
			float totalTimeRnd = 0;

			float timeClassicTP = 0;
			float timeObrTP = 0;
			float timeBctAllTP = 0;
			float timeRndTP = 0;

			float processingTime = 0;
			int numberOfAgreedTP = 0;
			while ((dp0 = readdir(fdImgTeste)) != NULL)
			{
				if (!strcmp(dp0->d_name, ".") || !strcmp(dp0->d_name, "..") )
					continue;    /* skip self and parent */
				
				contImg++;

				std::cout << dp0->d_name << std::endl;
				

				std::vector<int> sequence;
				for (int i = 0; i < kp_obj_collection.size(); i++)
					sequence.push_back(i);
				std::random_shuffle(sequence.begin(), sequence.end());

				imgFileTemp.str(std::string());
				imgFileTemp << DIR_IMGS_TEST.c_str() << dp0->d_name;
				scene = cv::imread(imgFileTemp.str(), cv::IMREAD_GRAYSCALE);
				cv::Mat sceneColor = cv::imread(imgFileTemp.str(), cv::IMREAD_COLOR);

				vector<cv::KeyPoint> kptsScene;
				cv::Mat descriptorScene;


				akaze->detectAndCompute(scene, cv::noArray(), kptsScene, descriptorScene);

				

				bool flgCountTP = true;
				bool encontrou = false;
				
				int t0_classic = cv::getTickCount();
				encontrou = runClassicSearch( scene, des_obj_collection, kp_obj_collection.size(), kp_obj_collection, sequence, img_obj_collection, VERBOSE, dp0->d_name, objs[contObjs], kptsScene, descriptorScene);
				int tf_classic = cv::getTickCount();
				time_classic.push_back((tf_classic - t0_classic) / cv::getTickFrequency());


				flgCountTP = flgCountTP && encontrou;
				if (GT[contObjs][contImg - 1] == true)
				{
					if (encontrou)
						TP_classic += 1;
					else
						FN_classic += 1;

				}
				else{
					if (encontrou)
						FP_classic += 1;
					else
						TN_classic += 1;


				}


				if (VERBOSE)
				{
					if (encontrou)
					{
						std::cout << "[CLASSIC] OBJECT DETECTED\n";
					}
					else
						std::cout << "[CLASSIC] OBJECT NOT DETECTED\n";
				}

				int t0_rnd = cv::getTickCount();
				//RANDOM SEARCH
				encontrou = runTDSearch(filters, scene, des_obj_collection, kp_obj_collection.size(), kp_obj_collection, &processingTime, sequence, img_obj_collection, VERBOSE, dp0->d_name, objs[contObjs], true, true, percKpSal[contObjs], sceneColor, kptsScene, descriptorScene, false);

				int tf_rnd = cv::getTickCount();
				time_rnd.push_back((tf_rnd - t0_rnd) / cv::getTickFrequency());

				flgCountTP = flgCountTP && encontrou;
				if (GT[contObjs][contImg - 1] == true)
				{
					if (encontrou)
						TP_rnd += 1;
					else
						FN_rnd += 1;

				}
				else{
					if (encontrou)
						FP_rnd += 1;
					else
						TN_rnd += 1;


				}

				if (VERBOSE)
				{
					if (encontrou)
					{
						std::cout << "[RND] OBJECT DETECTED\n";
					}
					else
						std::cout << "[RND] OBJECT NOT DETECTED\n";
				}
				
				int t0_obr = cv::getTickCount();
				//SORT KEYPOINTS BY RESPONSE
				encontrou = runTDSearch(filters, scene, des_obj_collection, kp_obj_collection.size(), kp_obj_collection, &processingTime, sequence, img_obj_collection, VERBOSE, dp0->d_name, objs[contObjs], true, false, percKpSal[contObjs], sceneColor, kptsScene, descriptorScene, true);
				

				int tf_obr = cv::getTickCount();
				time_obr.push_back((tf_obr - t0_obr) / cv::getTickFrequency());
				
				if (GT[contObjs][contImg - 1] == true)
				{

					if (encontrou)
						TP_obr += 1;
					else
						FN_obr += 1;

					

				}
				else{
					if (encontrou)
						FP_obr += 1;
					else
						TN_obr += 1;

					

				}


				if (VERBOSE)
				{
					if (encontrou)
					{
						std::cout << "[SBR] OBJECT DETECTED\n";
					}
					else
						std::cout << "[SBR] OBJECT NOT DETECTED\n";
				}

				

				int t0_tdbf = cv::getTickCount();
				//TD attention using Bloom Filters
				encontrou = runTDSearch(filters, scene, des_obj_collection, kp_obj_collection.size(), kp_obj_collection, &processingTime, sequence, img_obj_collection, VERBOSE, dp0->d_name, objs[contObjs], true, false, 0.0, sceneColor, kptsScene, descriptorScene, false);
				int tf_tdbf = cv::getTickCount();

				time_tdbf.push_back((tf_tdbf - t0_tdbf) / cv::getTickFrequency());

				flgCountTP = flgCountTP && encontrou;
				if (GT[contObjs][contImg - 1] == true)
				{

					if (encontrou)
						TP_tdbf += 1;
					else
						FN_tdbf += 1;


					

				}
				else{
					if (encontrou)
						FP_tdbf += 1;
					else
						TN_tdbf += 1;

					

				}


				if (VERBOSE)
				{
					if (encontrou)
					{
						std::cout << "[TD_attention-BloomFilter] OBJECT DETECTED\n";
					}
					else
						std::cout << "[TD_attention-BloomFilter] OBJECT NOT DETECTED\n";
				}



				if (flgCountTP)
				{

					totalTimeClassic += time_classic[contImg - 1];
					totalTimeRnd += time_rnd[contImg - 1];
					totalTimeObr += time_obr[contImg - 1];
					totalTimeBctAll += time_tdbf[contImg - 1];

					numberOfAgreedTP++;
					timeClassicTP += time_classic[contImg - 1];
					timeRndTP += time_rnd[contImg - 1];
					timeObrTP += time_obr[contImg - 1];
					timeBctAllTP += time_tdbf[contImg - 1];

				}
				else
				{
					totalTimeClassic += time_classic[contImg - 1];
					totalTimeRnd += time_rnd[contImg - 1];
					totalTimeObr += time_obr[contImg - 1];
					totalTimeBctAll += time_tdbf[contImg - 1];
				}

			}//finish processing scenes

			std::cout << percKP / 51.0 << std::endl;


			std::cout << "Finished processing image scenes with object " << objs[contObjs] << "!";
			outputExecTime.open(pathTxtExecTime.str(), fstream::app);
			//getch();
			outputExecTime << std::endl << "time TD-Attention: " << timeTD / 51.0 << std::endl;
			timeTD = 0;
			outputExecTime << "avg. number of scene descriptors: " << numDesc / 51.0 << std::endl;
			numDesc = 0;
			outputExecTime << "FP Rate: " << FPprob << std::endl;
			numDesc = 0;
			outputExecTime << std::endl << percKP / 51.0 << std::endl;
			percKP = 0;
			outputExecTime << "\nTIMES CLASSIC METHOD: " << endl;
			float recall = 0;
			float precision = 0;
			outputExecTime << "\tTP: " << TP_classic << endl;
			outputExecTime << "\tFP: " << FP_classic << endl;
			outputExecTime << "\tTN: " << TN_classic << endl;
			outputExecTime << "\tFN: " << FN_classic << endl;

			if (TP_classic + FN_classic == 0)
				recall = 0;
			else
				recall = (TP_classic / (float)(TP_classic + FN_classic));


			if (TP_classic + FP_classic == 0)
				precision = 0;
			else
				precision = (TP_classic / (float)(TP_classic + FP_classic));

			if (recall + precision == 0)
				outputExecTime << "\tF-Measure: INDETERM." << endl;
			else
				outputExecTime << "\tF-Measure: " << 2 * recall*precision / (recall + precision) << endl;

			for (int i = 0; i < time_classic.size(); i++)
			{
				outputExecTime << time_classic[i] << " ";

			}
			totalTimeClassic /= time_classic.size();
			outputExecTime << "TOTAL TIME:" << totalTimeClassic << " TIME TRUE POSITIVE CASES:" << timeClassicTP / numberOfAgreedTP;

			outputExecTime << "\nTIMES RANDOM METHOD: " << endl;
			recall = 0;
			precision = 0;
			outputExecTime << "\tTP: " << TP_rnd << endl;
			outputExecTime << "\tFP: " << FP_rnd << endl;
			outputExecTime << "\tTN: " << TN_rnd << endl;
			outputExecTime << "\tFN: " << FN_rnd << endl;

			if (TP_rnd + FN_rnd == 0)
				recall = 0;
			else
				recall = (TP_rnd / (float)(TP_rnd + FN_rnd));


			if (TP_rnd + FP_rnd == 0)
				precision = 0;
			else
				precision = (TP_rnd / (float)(TP_rnd + FP_rnd));

			if (recall + precision == 0)
				outputExecTime << "\tF-Measure: INDETERM." << endl;
			else
				outputExecTime << "\tF-Measure: " << 2 * recall*precision / (recall + precision) << endl;

			for (int i = 0; i < time_rnd.size(); i++)
			{
				outputExecTime << time_rnd[i] << " ";

			}
			totalTimeRnd /= time_rnd.size();
			outputExecTime << "TOTAL TIME:" << totalTimeRnd << " TIME TRUE POSITIVE CASES:" << timeRndTP / numberOfAgreedTP;

			outputExecTime << "\nTIMES 'SORT BY RESPONSE' METHOD: " << endl;
			recall = 0;
			precision = 0;
			outputExecTime << "\tTP: " << TP_obr << endl;
			outputExecTime << "\tFP: " << FP_obr << endl;
			outputExecTime << "\tTN: " << TN_obr << endl;
			outputExecTime << "\tFN: " << FN_obr << endl;

			if (TP_obr + FN_obr == 0)
				recall = 0;
			else
				recall = (TP_obr / (float)(TP_obr + FN_obr));


			if (TP_obr + FP_obr == 0)
				precision = 0;
			else
				precision = (TP_obr / (float)(TP_obr + FP_obr));

			if (recall + precision == 0)
				outputExecTime << "\tF-Measure: INDETERM." << endl;
			else
				outputExecTime << "\tF-Measure: " << 2 * recall*precision / (recall + precision) << endl;

			for (int i = 0; i < time_obr.size(); i++)
			{
				outputExecTime << time_obr[i] << " ";

			}
			totalTimeObr /= time_obr.size();
			outputExecTime << "TOTAL TIME:" << totalTimeObr << " TIME TRUE POSITIVE CASES:" << timeObrTP / numberOfAgreedTP;

			outputExecTime << "\nTIME 'TD-ATTENTION BLOOM FILTER' METHOD: " << endl;
			recall = 0;
			precision = 0;
			outputExecTime << "\tTP: " << TP_tdbf << endl;
			outputExecTime << "\tFP: " << FP_tdbf << endl;
			outputExecTime << "\tTN: " << TN_tdbf << endl;
			outputExecTime << "\tFN: " << FN_tdbf << endl;

			if (TP_tdbf + FN_tdbf == 0)
				recall = 0;
			else
				recall = (TP_tdbf / (float)(TP_tdbf + FN_tdbf));


			if (TP_tdbf + FP_tdbf == 0)
				precision = 0;
			else
				precision = (TP_tdbf / (float)(TP_tdbf + FP_tdbf));

			if (recall + precision == 0)
				outputExecTime << "\tF-Measure: INDETERM." << endl;
			else
				outputExecTime << "\tF-Measure: " << 2 * recall*precision / (recall + precision) << endl;

			for (int i = 0; i < time_tdbf.size(); i++)
			{
				outputExecTime << time_tdbf[i] << " ";

			}
			totalTimeBctAll /= time_tdbf.size();
			outputExecTime << "TOTAL TIME:" << totalTimeBctAll << " TIME TRUE POSITIVE CASES:" << timeBctAllTP / numberOfAgreedTP;

			time_classic.clear();
			time_rnd.clear();
			time_obr.clear();
			time_tdbf.clear();

			outputExecTime.close();

			std::cout << "Cleaning memory...";

			for (int j = 0; j < MAX_DIST + 1; j++)
			{
				std::cout << "expected false probability rate for filter " << j << ": " << filters[j]->effective_fpp() << std::endl;
				delete filters[j];

			}
			
			delete[] des_obj_collection;
			delete[] img_obj_collection;
			
			std::cout << "finish!\n";
		
		}
	return 0;
}