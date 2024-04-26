#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

struct Suffix
{
	unsigned int index; // To store original index
	int rank1; // To store ranks and next rank pair
    int rank2; // To store ranks and next rank pair
    Suffix(unsigned int index, int rank1, int rank2): index(index), rank1(rank1), rank2(rank2) {}
};

// comparsion
int cmp(const Suffix& a, const Suffix& b)
{
	return (a.rank1 == b.rank1)? (a.rank2 < b.rank2 ? 1: 0):
			(a.rank1 < b.rank1 ? 1: 0);
}

// main function to use
std::vector<unsigned int> buildSuffixArray(std::u32string txt)
{
	std::vector<Suffix> suffixes; suffixes.reserve(txt.size());

	for (size_t i = 0; i < txt.size(); ++i) {
        suffixes.push_back(Suffix(i, txt[i] - 'a', ((i+1) < txt.size()) ? (txt[i + 1] - 'a'): -1));
	}
	std::sort(suffixes.begin(), suffixes.end(), cmp);

	std::vector<int> ind(txt.size(), 0);    
	for (size_t k = 4; k < 2*txt.size(); k = k*2)
	{
		int rank = 0;
		int prev_rank = suffixes[0].rank1;
		suffixes[0].rank1 = rank;
		ind[suffixes[0].index] = 0;

		for (size_t i = 1; i < txt.size(); ++i)
		{
			if (suffixes[i].rank1 == prev_rank &&
				suffixes[i].rank2 == suffixes[i-1].rank2)
			{
				prev_rank = suffixes[i].rank1;
				suffixes[i].rank1 = rank;
			} else {
				prev_rank = suffixes[i].rank1;
				suffixes[i].rank1 = ++rank;
			}
			ind[suffixes[i].index] = i;
		}

		unsigned int nextindex;
		for (size_t i = 0; i < txt.size(); ++i) {
			nextindex = suffixes[i].index + k/2;
			suffixes[i].rank2 = (nextindex < txt.size()) ?
								(suffixes[ind[nextindex]].rank1) : -1;
		}

		std::sort(suffixes.begin(), suffixes.end(), cmp);
	}
	ind.clear();

	std::vector<unsigned int> suffixArr; suffixArr.reserve(txt.size());
	for (size_t i = 0; i < txt.size(); ++i)
		suffixArr.push_back(suffixes[i].index);

	return suffixArr;
}

// END
