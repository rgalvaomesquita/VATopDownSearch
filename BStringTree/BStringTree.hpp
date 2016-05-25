

#include <bitset>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv\cxcore.h>
#include <Parameters.h>
#include <serializablebloom\bloom_filter.hpp>

class BStringTree
{
public:

	typedef std::bitset<NBITS> bitchain;
	typedef struct TreeNode
	{
		bool exist;
		std::vector<TreeNode*> edge;
		int d1;
		int d2;
	};

	typedef struct{
		bitchain bitSet;
		int firstBit;
		int distToRoot;
	}BitStringStack;

	
	void init(BStringTree::bitchain& bstring, int nbits, float ratio_knn, int maxDist);
	BStringTree(int nbits, cv::Mat& descriptors, float ratio_knn, int maxDist, bloom_filter** filters, int numberOfFilters, bool binarized = false);
	void construct(int nbits, std::vector<bitchain>& bitStrings, float ratio_knn);
	void BStringTree::writeBSTtoFile(const TreeNode* node, std::ofstream& out, std::string& path);
	void BStringTree::printTree();
	bitchain getRootValue();
	TreeNode* getRoot();
	
	float exist(bitchain& nodeValue, int& d1, int &d2);
	float exist(cv::Mat& descriptor, int row, int& d1, int&d2);
	TreeNode* find(bitchain& nodeValue, int& d1, int &d2);
	~BStringTree();
private:

	int numberOfFilters;
	bloom_filter** filter;
	TreeNode root;
	int nbits;
	int maxDist;
	bitchain rootValue;
	float ratio_knn;
	float exist(bitchain& nodeValue, int level, TreeNode* currentNode, int& d1, int& d2);
	TreeNode* find(bitchain& nodeValue, int level, TreeNode* currentNode, int& d1, int &d2);
	void populateNeighbours(std::vector<bitchain>& bitStrings);



};
