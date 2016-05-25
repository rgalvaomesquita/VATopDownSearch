
#include <opencv2/opencv.hpp>
#include <opencv\cxcore.h>
#include <Parameters.h>


typedef struct insertedMatches{
	float distance;
	int good_matches_idx;
}InsertedMatches;


double mean_homography_error(std::vector<cv::Point> set11, std::vector<cv::Point> set12, std::vector<cv::Point> set21, std::vector<cv::Point> set22);
cv::Point H_x_Point(cv::Point point, cv::Mat& H);
double det3x3(cv::Mat H);
void invMat3x3(cv::Mat H, cv::Mat& Hinv);
bool testHomography(std::vector<cv::DMatch >& good_matches, std::vector<cv::KeyPoint>& kp_object, std::vector<cv::KeyPoint>& kp_image, std::string& msg, double numPixels);
double euclidianDistance(cv::Point p1, cv::Point p2);
bool applyMatch(cv::Mat des_object, cv::Mat des_image, std::vector<cv::KeyPoint>& kp_image, std::vector<cv::DMatch>& good_matches, float knn_ratio, std::vector<cv::KeyPoint>& kp_image_goodmatches, InsertedMatches*& trainIdxGoodMatches);




bool applyMatch(cv::Mat des_object, cv::Mat des_image, std::vector<cv::KeyPoint>& kp_image, std::vector<cv::DMatch>& good_matches, float knn_ratio, std::vector<cv::KeyPoint>& kp_image_goodmatches, InsertedMatches*& trainIdxGoodMatches)
{

	cv::BFMatcher matcher(cv::NORM_HAMMING);
	std::vector<std::vector<cv::DMatch > > matches;
	matcher.knnMatch(des_image, des_object, matches, 2);
	bool newMatchInserted = false;
	for (int i = 0; i < std::min(des_image.rows, (int)matches.size()); i++)
	{


		if (((matches[i][0].distance < knn_ratio*(matches[i][1].distance)) && ((int)matches[i].size() <= 2 && (int)matches[i].size()>0)))
		{
			int queryIdx_original = matches[i][0].queryIdx;

			if (trainIdxGoodMatches[matches[i][0].trainIdx].distance == -1)
			{
				matches[i][0].queryIdx = kp_image_goodmatches.size();
				newMatchInserted = true;
				good_matches.push_back(matches[i][0]);
				trainIdxGoodMatches[matches[i][0].trainIdx].distance = matches[i][0].distance;
				trainIdxGoodMatches[matches[i][0].trainIdx].good_matches_idx = good_matches.size() - 1;
				kp_image_goodmatches.push_back(kp_image[queryIdx_original]);
			}
			else if (matches[i][0].distance < trainIdxGoodMatches[matches[i][0].trainIdx].distance)
			{

				newMatchInserted = true;
				matches[i][0].queryIdx = good_matches[trainIdxGoodMatches[matches[i][0].trainIdx].good_matches_idx].queryIdx;
				good_matches[trainIdxGoodMatches[matches[i][0].trainIdx].good_matches_idx] = matches[i][0];
				trainIdxGoodMatches[matches[i][0].trainIdx].distance = matches[i][0].distance;
				kp_image_goodmatches[trainIdxGoodMatches[matches[i][0].trainIdx].good_matches_idx] = kp_image[queryIdx_original];

			}
			

		}
	}

	if (good_matches.size() >= NUM_MIN_MATCHES && newMatchInserted)
	{

		return true;
	}
	else
	{

		return false;
	}
}

double euclidianDistance(cv::Point p1, cv::Point p2)
{
	return sqrt(powf(p1.x - p2.x, 2) + (powf(p1.y - p2.y, 2)));
}

double mean_homography_error(std::vector<cv::Point> set11, std::vector<cv::Point> set12, std::vector<cv::Point> set21, std::vector<cv::Point> set22)
{
	double rep_error = 0;
	for (int i = 0; i < set11.size(); i++)
	{
		rep_error += euclidianDistance(set11[i], set12[i]);
		if (set21.size()>0)
			rep_error += euclidianDistance(set21[i], set22[i]);
	}
	if (set21.size()>0)
		return rep_error / (set11.size() * 2);
	else
		return rep_error / set11.size();
}

cv::Point H_x_Point(cv::Point point, cv::Mat& H)
{
	cv::Point point_H;
	double* ptr0 = H.ptr<double>(0);
	double* ptr1 = H.ptr<double>(1);
	double* ptr2 = H.ptr<double>(2);

	float ww = 1.0 / (ptr2[0] * point.x + ptr2[1] * point.y + ptr2[2]);
	point_H.x = (ptr0[0] * point.x + ptr0[1] * point.y + ptr0[2])*ww;
	point_H.y = (ptr1[0] * point.x + ptr1[1] * point.y + ptr1[2])*ww;

	return point_H;
}


double det3x3(cv::Mat H)
{
	double* ptr0 = H.ptr<double>(0);
	double* ptr1 = H.ptr<double>(1);
	double* ptr2 = H.ptr<double>(2);


	double detH = ptr0[0] * ptr1[1] * ptr2[2]
		+ ptr0[1] * ptr1[2] * ptr2[0]
		+ ptr0[2] * ptr1[0] * ptr2[1]
		+ ptr2[0] * ptr1[1] * ptr0[2]
		+ ptr2[1] * ptr1[2] * ptr0[0]
		+ ptr2[2] * ptr1[0] * ptr0[1];


	return detH;
}

void invMat3x3(cv::Mat H, cv::Mat& Hinv)
{
	if (Hinv.rows != H.rows || Hinv.cols != H.cols || H.rows != 3 || H.cols != 3)
	{
		std::cout << "Matrices should be 3x3!";
		exit(0);
	}

	double* ptr0 = H.ptr<double>(0);
	double* ptr1 = H.ptr<double>(1);
	double* ptr2 = H.ptr<double>(2);

	double* ptrinv0 = Hinv.ptr<double>(0);
	double* ptrinv1 = Hinv.ptr<double>(1);
	double* ptrinv2 = Hinv.ptr<double>(2);
	double det = det3x3(H);

	ptrinv0[0] = (ptr1[1] * ptr2[2] - ptr1[2] * ptr2[1]) / det;
	ptrinv0[1] = (ptr0[2] * ptr2[1] - ptr0[1] * ptr2[2]) / det;
	ptrinv0[2] = (ptr0[1] * ptr1[2] - ptr0[2] * ptr1[1]) / det;

	ptrinv1[0] = (ptr1[2] * ptr2[0] - ptr1[0] * ptr2[2]) / det;
	ptrinv1[1] = (ptr0[0] * ptr2[2] - ptr0[2] * ptr2[0]) / det;
	ptrinv1[2] = (ptr0[2] * ptr1[0] - ptr0[0] * ptr1[2]) / det;

	ptrinv2[0] = (ptr1[0] * ptr2[1] - ptr1[1] * ptr2[0]) / det;
	ptrinv2[1] = (ptr0[1] * ptr2[0] - ptr0[0] * ptr2[1]) / det;
	ptrinv2[2] = (ptr0[0] * ptr1[1] - ptr0[1] * ptr1[0]) / det;
}


bool testHomography(std::vector<cv::DMatch >& good_matches, std::vector<cv::KeyPoint>& kp_object, std::vector<cv::KeyPoint>& kp_image, std::string& msg, double numPixels)
{

	std::vector<cv::Point2f> obj;
	std::vector<cv::Point2f> scene;
	std::stringstream sstm;
	for (int i = 0; i < good_matches.size(); i++)
	{

		if (good_matches[i].trainIdx > kp_object.size() || good_matches[i].queryIdx > kp_image.size())
		{
			std::cout << "invalid keypoint detected in testHomography! press any key to finish! ";
			getchar();
			exit(0);
		}

		//Get the keypoints from the good matches
		obj.push_back(kp_object[good_matches[i].trainIdx].pt);
		scene.push_back(kp_image[good_matches[i].queryIdx].pt);

	}

	cv::Mat H;

	cv::Mat mask;
	H = findHomography(scene, obj, cv::RANSAC, 3.0, mask);

	if (H.empty())
	{
		
		return false;
	}

	std::vector<cv::Point> points_query;
	std::vector<cv::Point> points_trainInvH;
	std::vector<cv::Point> points_train;
	std::vector<cv::Point> points_queryH;

	

	cv::Mat Hinv(H.rows, H.cols, H.type());
	invMat3x3(H, Hinv);

	int nInliers = 0;
	double mean_sym_error = 0;
	uchar* pMask;

	for (int i = 0; i < mask.rows; i++)
	{

		if (mask.at<uchar>(i, 0)>0)
		{
			nInliers++;

			points_query.push_back(kp_image[good_matches[i].queryIdx].pt);
			points_trainInvH.push_back(H_x_Point(kp_object[good_matches[i].trainIdx].pt, Hinv));

			points_train.push_back(kp_object[good_matches[i].trainIdx].pt);
			points_queryH.push_back(H_x_Point(kp_image[good_matches[i].queryIdx].pt, H));

			
		}
	}


	double mean_rep_error2 = mean_homography_error(points_queryH, points_train, points_query, points_trainInvH);
	
	const double detH = det3x3(H);


	if (mean_rep_error2 <= log((float)nInliers) / log(4.0) /*&& fabs(detR) >= 0.01*/ && fabs(detH) >= 0.00001)
	{
		return true;
	}
	else
	{
		return false;
	}
}


