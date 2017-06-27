/*
	CILP++

	Copyright 2012 Manoel Vitor Macedo França <maneuvitor@gmail.com>

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#ifndef UTILITARIA_H_
#define UTILITARIA_H_

#define LINE_SIZE 4096

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <climits>
#include <cfloat>
#include <ctime>
#include <iostream>
#include <fstream>
#include <exception>
#include <sys/types.h>
#include "dirent.h"

using namespace std;

// Utility structure for storing general-purpose algorithms
typedef struct _Utilities
{
	int find(vector<string> vetor, string elemento)
	{
		unsigned int i;

		for(i = 0; i < vetor.size(); i++)
		{
			if(vetor.at(i).compare(elemento) == 0)
				return i;
		}
		return -1;
	}

	int find(vector<int> vetor, int elemento)
	{
		unsigned int i;

		for(i = 0; i < vetor.size(); i++)
		{
			if(vetor.at(i) == elemento)
				return i;
		}
		return -1;
	}

	void trimRight( std::string& str, const std::string& trimChars = " \f\n\r\t\v")
	{
	   std::string::size_type pos = str.find_last_not_of( trimChars );
	   str.erase( pos + 1 );
	}


	void trimLeft( std::string& str, const std::string& trimChars = " \f\n\r\t\v")
	{
	   std::string::size_type pos = str.find_first_not_of( trimChars );
	   str.erase( 0, pos );
	}


	string& trim(std::string& str, const std::string& trimChars = " \f\n\r\t\v")
	{
	   trimRight( str, trimChars );
	   trimLeft( str, trimChars );

	   return str;
	}


	bool fileExists(string fileName)
	{
		if(FILE *file = fopen(fileName.c_str(), "r"))
		{
			fclose(file);
			return true;
		}

		return false;
	}

	vector<string> readLines(string fileName)
	{
		unsigned int pos;
		char lineCharArray[LINE_SIZE];

		vector<string> lines;

		FILE *file;

		if ((file = fopen(fileName.c_str(), "r")) != NULL)
		{
			while (fgets(lineCharArray, LINE_SIZE, file) != NULL)
			{
				string line(lineCharArray);

				// Removing each formatting char
				while(line.find(" ") != string::npos)
				{
					pos = line.find(" ");
					line.erase(pos, 1);
				}

				while(line.find(".") != string::npos)
				{
					pos = line.find(".");
					line.erase(pos, 1);
				}

				while(line.find("\n") != string::npos)
				{
					pos = line.find("\n");
					line.erase(pos, 1);
				}

				while(line.find("\t") != string::npos)
				{
					pos = line.find("\t");
					line.erase(pos, 1);
				}

				if(line.size() > 0)
					lines.push_back(line);
			}
		}

		return lines;
	}

	vector<string> readParameters(string fileName)
	{
		unsigned int pos;
		char lineCharArray[LINE_SIZE];

		vector<string> lines;

		FILE *file;

		if ((file = fopen(fileName.c_str(), "r")) != NULL)
		{
			while (fgets(lineCharArray, LINE_SIZE, file) != NULL)
			{
				string line(lineCharArray);

				// Removing each formatting char
				while(line.find(" ") != string::npos)
				{
					pos = line.find(" ");
					line.erase(pos, 1);
				}

				while(line.find("\n") != string::npos)
				{
					pos = line.find("\n");
					line.erase(pos, 1);
				}

				while(line.find("\t") != string::npos)
				{
					pos = line.find("\t");
					line.erase(pos, 1);
				}

				// Lines starting with '#' are ignored
				if(line.size() > 0 && (line.at(0) != '#') && (std::find(lines.begin(), lines.end(), line) == lines.end()))
					lines.push_back(line);
			}
		}

		return lines;
	}

	vector<string> readClauses(string fileName)
	{
		unsigned int pos;
		char lineCharArray[LINE_SIZE];

		vector<string> lines;

		FILE *file;

		if ((file = fopen(fileName.c_str(), "r")) != NULL)
		{
			string finalLine;

			while (fgets(lineCharArray, LINE_SIZE, file) != NULL)
			{
				string line(lineCharArray);

				// Removing each formatting char
				while(line.find(" ") != string::npos)
				{
					pos = line.find(" ");
					line.erase(pos, 1);
				}

				while(line.find(".") != string::npos)
				{
					pos = line.find(".");
					line.erase(pos, 1);
				}

				while(line.find("\n") != string::npos)
				{
					pos = line.find("\n");
					line.erase(pos, 1);
				}

				while(line.find("\t") != string::npos)
				{
					pos = line.find("\t");
					line.erase(pos, 1);
				}

				// If the ":-" operator has been found (which means the current line is a new clause),
				// treat the line as a new clause; otherwise, append it to the last clause
				if(line.find(":-") != string::npos)
				{
					if(finalLine.size() > 0 && (std::find(lines.begin(), lines.end(), finalLine) == lines.end()))
						lines.push_back(finalLine);

					finalLine = line;
				}
				else
					finalLine.append(line);
			}

			if(finalLine.size() > 0 && (std::find(lines.begin(), lines.end(), finalLine) == lines.end()))
				lines.push_back(finalLine);
		}

		return lines;
	}

	template <class T>
	inline string convertToString(const T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	template <class T>
	bool convertFromString(T& t, const string& s, ios_base& (*f)(ios_base&))
	{
		istringstream iss(s);
		T& older = t;
		if((iss >> f >> t).fail())
		{
			t = older;
			return false;
		}
		else
			return true;
	}

	void clearAllFiles(string dirName, string extension)
	{
		DIR *dp;
		struct dirent *ep;
		string s = "", removedFile;
		unsigned int found;

		dp = opendir(s.append("./").append(dirName).c_str());

		while ((ep = readdir(dp)))
		{
			s = string(ep->d_name);

			found = s.find(".");
			if((found != string::npos) && (s.substr(found).compare(extension.c_str()) == 0))
			{
				removedFile = "";
				removedFile.append(dirName).append(s);
				remove(removedFile.c_str());
			}
		}

		(void) closedir(dp);
	}

	template <class T>
	vector<T> shuffleVector(vector<T> inputVector)
	{
		int currentSize = inputVector.size();
		vector<T> outputVector;

		srand (time(NULL));

		// Choosing random indexes
		for( ; currentSize; currentSize--)
		{
			int randomIndex = (int)floor(((float)rand()/RAND_MAX) * currentSize);
			if (randomIndex == currentSize)
				randomIndex--;

			outputVector.push_back(inputVector.at(randomIndex));
			inputVector.erase(inputVector.begin()+randomIndex);
		}

		return outputVector;
	}
}Utilities;

#endif /* UTILITARIA_H_ */
