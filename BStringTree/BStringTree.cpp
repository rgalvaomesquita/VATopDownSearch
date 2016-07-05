
#include <BStringTree.hpp>
#include <stack>
#include <bitset>
#include <Parameters.h>



void BStringTree::init(BStringTree::bitchain& bstring, int nbits, float ratio_knn, int maxDist)
{
	this->nbits = nbits;
	this->maxDist = maxDist;
	this->ratio_knn = ratio_knn;
	this->rootValue = bstring;
	this->root.edge.assign(this->nbits, NULL);
	this->root.exist = true;
}


BStringTree::BStringTree(int nbits, cv::Mat& descriptors, float ratio_knn, int maxDist, bloom_filter** filters, int numberOfFilters, bool binarized)
{

	this->numberOfFilters = numberOfFilters;
	this->filter = new bloom_filter*[this->numberOfFilters];
	for (int i = 0; i < numberOfFilters; i++)
	{
		this->filter[i] = filters[i];
	}

	uchar* ptr;
	std::vector<BStringTree::bitchain> bitStrings;

	BStringTree::bitchain tempBitString(NBITS, '0');
	int temp, firstBit, count;
	this->maxDist = maxDist;
	if (!binarized)
	{
		for (int i = 0; i < descriptors.rows; i++)
		{
			ptr = descriptors.ptr<uchar>(i);
			firstBit = 0;
			for (int j = descriptors.cols - 1; j >= 0; j--)
			{
				temp = ptr[j];

				for (int bit = firstBit; bit < firstBit + 8; bit++)
				{
					tempBitString[bit] = temp % 2 == 0 ? '0': '1';

					
					temp = temp / 2;
				}
				firstBit += 8;
			}
			bitStrings.push_back(tempBitString);
			
		}
	}
	else
	{

		for (int i = 0; i < descriptors.rows; i++)
		{
			ptr = descriptors.ptr<uchar>(i);
			int currentBit = 0;
			for (int j = 0; j < descriptors.cols; j++)
			{

				tempBitString[currentBit] = ptr[j] == 0 ? '0' : '1';

				currentBit++;
			}
			bitStrings.push_back(tempBitString);

		}
	}

	this->construct(nbits, bitStrings, ratio_knn);

	

}


void BStringTree::construct(int nbits, std::vector<BStringTree::bitchain>& bitStrings, float ratio_knn)
{
	this->nbits = nbits;
	this->ratio_knn = ratio_knn;
	this->rootValue = bitStrings[0];
	this->root.edge.assign(this->nbits, NULL);
	this->root.exist = true;
	
	this->populateNeighbours(bitStrings);
	

}


BStringTree::bitchain BStringTree::getRootValue()
{
	return this->rootValue;
}

BStringTree::TreeNode* BStringTree::getRoot()
{
	return &(this->root);
}


float BStringTree::exist(cv::Mat& descriptor, int row, int& d1, int& d2)
{
	uchar* ptr;
	BStringTree::bitchain bitString(0);
	int temp, firstBit, count;

	ptr = descriptor.ptr<uchar>(row);
	firstBit = 0;

	for (int j = descriptor.cols - 1; j >= 0; j--)
	{

		temp = ptr[j];

		for (int bit = firstBit; bit < firstBit + 8; bit++)
		{
			bitString[bit] = temp % 2;
			temp = temp / 2;
			
		}
		firstBit += 8;
	}



	return this->exist(bitString, 0, &this->root, d1, d2);
}

float BStringTree::exist(BStringTree::bitchain& nodeValue, int& d1, int &d2)
{
	return this->exist(nodeValue, 0, &this->root, d1, d2);
}

BStringTree::TreeNode* BStringTree::find(BStringTree::bitchain& nodeValue, int& d1, int &d2)
{
	return this->find(nodeValue, 0, &this->root, d1, d2);
}

BStringTree::TreeNode* BStringTree::find(BStringTree::bitchain& nodeValue, int firstBit, TreeNode* currentNode, int& d1, int &d2)
{

	for (int i = 0; i < this->nbits - firstBit; i++)
	{
		if (nodeValue[i + firstBit] != this->rootValue[i + firstBit])
		{

			if (currentNode->edge[i] == NULL)
			{
				return false;
			}
			return find(nodeValue, firstBit + i + 1, currentNode->edge[i], d1, d2);

		}
	}

	if (currentNode->exist)
	{
		d1 = currentNode->d1;
		d2 = currentNode->d2;
		return currentNode;
	}
	else
	{
		d1 = -1;
		d2 = -1;
		return NULL;
	}



}

float BStringTree::exist(BStringTree::bitchain& nodeValue, int firstBit, TreeNode* currentNode, int& d1, int &d2)
{
	TreeNode* t = find(nodeValue, firstBit, currentNode, d1, d2);
	if (t != NULL)
	{
		if (t->d2 == 0)
			return 0;
		return t->d1 / ((float)t->d2);
	}
	else
		return -1.0;
}





void BStringTree::populateNeighbours(std::vector<BStringTree::bitchain>& bitStrings)
{
	std::cout << "maxDist: " << this->maxDist << std::endl;

	int dist = 0;

	std::stack<BStringTree::BitStringStack> q;

	BitStringStack topNode;
	BitStringStack tempNode;
	BStringTree::bitchain currentBitSet;
	BStringTree::bitchain currentBitSetOriginal;

	

	for (int i = 0; i < bitStrings.size(); i++)
	{
		
		std::cout << "\r" << i + 1 << "/" << bitStrings.size();
		
		tempNode.bitSet = bitStrings[i];
		tempNode.firstBit = 0;
		tempNode.distToRoot = 0;
		q.push(tempNode);

		this->filter[0]->insert(bitStrings[i]);
		while (!q.empty())
		{
			bool exactBitSet = false;

			while (!q.empty())
			{

				if (q.top().firstBit<this->nbits)
					break;
				else
					q.pop();
			}

			if (q.empty())
				break;
			currentBitSetOriginal = q.top().bitSet;
			topNode = q.top();
			int b = topNode.firstBit;
			q.top().firstBit++;
			currentBitSet = currentBitSetOriginal;
			currentBitSet[b] == '0' ? '1' : '0';
			
			exactBitSet = false;
			

			
			int currentDistance;
			int d1 = q.top().distToRoot + 1;

			this->filter[d1]->insert(currentBitSet);

			if (d1 < this->maxDist)
			{
				//enqueue
				if (b < this->nbits - 1)
				{

					tempNode.bitSet = currentBitSet;
					tempNode.firstBit = b + 1;
					tempNode.distToRoot = d1;
					q.push(tempNode);

				}

			}





		}

	}

	

}


BStringTree::~BStringTree()
{
	delete[] this->filter;
}





