#pragma once

#include <string>
#include <set> // makes ordered set
#include <algorithm> // sorting

// return ordered alphabet from the string
wchar_t* Alphabet(const std::wstring& str);

// return index of the character in the alphabet
unsigned GetIndex(const wchar_t* alphabet, const size_t size, wchar_t c);

// return index of the character in the frequencies
unsigned GetIndex(const std::pair<wchar_t, double>* frequencies, const size_t size, wchar_t c);

// return index of the character in sorted alphabet
unsigned GetIndexInSorted(const wchar_t* alphabet, const size_t size, wchar_t c);

// return index of the character in sorted frequencies
unsigned GetIndexInSorted(const std::pair<wchar_t, double>* frequencies, const size_t size, wchar_t c);

// return frequencies of the characters
std::pair<wchar_t, double>* Frequencies(wchar_t* alphabet, const size_t size, const std::wstring& str);


// START IMPLEMENTATION

wchar_t* Alphabet(const std::wstring& str)
{
    const wchar_t* charStr = str.c_str();
    std::set<wchar_t> charsSet(charStr, charStr + wcslen(charStr));

    wchar_t* alphabet = new wchar_t[charsSet.size() + 1];
    int ind = 0;
    for (wchar_t c : charsSet) { 
        alphabet[ind++] = c;
    }
    alphabet[ind] = '\0'; // end of string

    std::sort(alphabet, alphabet + ind);

    return alphabet;
}

unsigned GetIndex(const wchar_t* alphabet, const size_t size, wchar_t c)
{
    for (int i = 0; i < size; ++i) {
        if (alphabet[i] == c) {
            return i;
        }
    }
    std::cout << "GetIndex() Error: character " << c << " was not found in the alphabet" << std::endl;
    return 0;
}

unsigned GetIndex(const std::pair<wchar_t, double>* frequencies, const size_t size, wchar_t c)
{
    for (int i = 0; i < size; ++i) {
        if (frequencies[i].first == c) {
            return i;
        }
    }
    return -1;
}

unsigned GetIndexInSorted(const wchar_t* alphabet, const size_t size, wchar_t c)
{
    // binary search
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (alphabet[mid] == c) {
            return mid;
        }
        if (alphabet[mid] < c) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

unsigned GetIndexInSorted(const std::pair<wchar_t, double>* frequencies, const size_t size, wchar_t c)
{
    // binary search
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (frequencies[mid].first == c) {
            return mid;
        }
        if (frequencies[mid].first < c) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

std::pair<wchar_t, double>* Frequencies(wchar_t* alphabet, const size_t size, const std::wstring& str)
{
    std::pair<wchar_t, double>* frequencies = new std::pair<wchar_t, double>[size];
    for (int i = 0; i < size; i++) {
        frequencies[i].first = alphabet[i];
        frequencies[i].second = 0;
    }

    size_t countAll = 0;
    for (wchar_t c : str) {
        ++frequencies[GetIndex(alphabet, size, c)].second;
        ++countAll;
    } for (int i = 0; i < size; i++) {
        frequencies[i].second /= countAll;
    }

    return frequencies;
}

// END IMPLEMENTATION