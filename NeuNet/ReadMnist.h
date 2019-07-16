#ifndef READMNIST_H
#define READMNIST_H


#endif // READMNIST_H
#include <string>
#include <cuchar>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
# include <windows.h>
#include "Hilbert.h"

template <typename T> void ReadMNISTMod(std::vector<std::vector<T>> &images, std::vector<int> &labels, bool train, bool hilbertCurve = false)
{

	char result[MAX_PATH];
	std::string(result, GetModuleFileNameA(NULL, result, MAX_PATH));


	unsigned int num, magic, rows, cols;
	ifstream icin;
	icin.open(train ? "../../NeuNet/MNIST/train-images.idx3-ubyte"
		: "../../NeuNet/MNIST/t10k-images.idx3-ubyte", ios::binary);/*
	icin.open(train ? "C:/Users/Misha/source/repos/NeuNet/NeuNet/MNIST/train-images.idx3-ubyte"
		: "C:/Users/Misha/source/repos/NeuNet/NeuNet/MNIST/t10k-images.idx3-ubyte", ios::binary);*/
	magic = in(icin, 4), num = in(icin, 4), rows = in(icin, 4), cols = in(icin, 4);
	std::vector<T> img;
	std::vector<std::vector<T>> img2;
	if (hilbertCurve) 
	{
		img2.resize(32);
		for (size_t i = 0; i < 32; i++)
		{
			img2[i].resize(32);
		}
	}
	for (long int i = 0; i < num; i++)
	{

		img.resize(rows * cols);
		for (int x = 0; x < rows; x++)
		{
			for (int y = 0; y < cols; y++)
			{
				if (!hilbertCurve) 
				{
					img[rows * x + y] = in(icin, 1);
				} 
				else 
				{
					img2[x+2][y+2] = in(icin, 1);
					;
				}
			}
		}
		if (hilbertCurve)
		{
			img.resize(1024);
			for (size_t i = 0; i < 32; i++)
			{
				for (size_t j = 0; j < 32; j++)
				{
					img[xy2d(5, i, j)] = img2[i][j];
				}
			}
		}
		images.push_back(img);
		img.clear();
	}

	icin.close();
	icin.open(train ? "../../NeuNet/MNIST/train-labels.idx1-ubyte"
		: "../../NeuNet/MNIST/t10k-labels.idx1-ubyte", ios::binary);/*
	icin.open(train ? "C:/Users/Misha/source/repos/NeuNet/NeuNet/MNIST/train-labels.idx1-ubyte"
		: "C:/Users/Misha/source/repos/NeuNet/NeuNet/MNIST/t10k-labels.idx1-ubyte", ios::binary);*/
	long int num2_ = num;
	magic = in(icin, 4), num2_ = in(icin, 4);
	for (long int i = 0; i < num; i++)
	{
		labels.push_back(in(icin, 1));
	}
}

unsigned int in(ifstream& icin, unsigned int size)
{
	unsigned int ans = 0;
	for (long int i = 0; i < size; i++)
	{
		unsigned char x;
		icin.read((char*)&x, 1);
		unsigned int temp = x;
		ans <<= 8;
		ans += temp;
	}
	return ans;
}

