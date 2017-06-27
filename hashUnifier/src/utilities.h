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

	int findAtom(vector<string> vector, string element)
	{
		unsigned int i;
		string atomWithoutNegation, elementWithoutNegation;

		for(i = 0; i < vector.size(); i++)
		{
			atomWithoutNegation = (element.at(0) == '~' ? element.substr(1) : element);
			elementWithoutNegation = (vector.at(i).at(0) == '~' ? vector.at(i).substr(1) : vector.at(i));

			if(elementWithoutNegation.compare(atomWithoutNegation) == 0)
				return i;
		}
		return -1;
	}

	vector<string> getDistinctElements(vector<string> vetor)
	{
		vector<string> distinctElements;

		for(unsigned int i = 0; i < vetor.size(); i++)
		{
			if(find(distinctElements, vetor.at(i)) < 0)
				distinctElements.push_back(vetor.at(i));
		}
		return distinctElements;
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

				// Removing any formatting char
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
					if(finalLine.size() > 0)
						lines.push_back(finalLine);

					finalLine = line;
				}
				else
					finalLine.append(line);
			}

			if(finalLine.size() > 0)
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

	void clearAllFiles(string dirName, string extension, bool isRelativePath)
	{
		DIR *dp;
		struct dirent *ep;
		string s = "", removedFile;
		unsigned int found;

		if(isRelativePath)
			dp = opendir(s.append("./").append(dirName).c_str());
		else
			dp = opendir(s.append(dirName).c_str());

		while ((ep = readdir(dp)))
		{
			s = string(ep->d_name);

			found = s.find_last_of(".");
			if((found != string::npos) && (s.substr(found).compare(extension.c_str()) == 0))
			{
				removedFile = "";
				removedFile.append(dirName).append(s);
				remove(removedFile.c_str());
			}
		}

		(void) closedir(dp);
	}

	vector<string> getAllFiles(string dirName, string extension, bool isRelativePath)
	{
		DIR *dp;
		struct dirent *ep;
		string s = "", selectedFile;
		unsigned int found;
		vector<string> files;

		if(isRelativePath)
			dp = opendir(s.append("./").append(dirName).c_str());
		else
			dp = opendir(s.append(dirName).c_str());

		while ((ep = readdir(dp)))
		{
			s = string(ep->d_name);

			found = s.find(".");
			if((found != string::npos) && s.compare(".") && s.compare("..") && ((extension.compare("") == 0) || (s.substr(found).compare(extension.c_str()) == 0)))
			{
				selectedFile = "";
				selectedFile.append(dirName).append(s);
				files.push_back(selectedFile);
			}
		}

		(void) closedir(dp);

		return files;
	}

	// Method to split the dataset clauses into bodies and heads
	pair<vector<vector<string> >, vector<string> > decodeData(vector<string> dataset)
	{
		unsigned int pos;

		string headStr, bodyStr, line, tempStr;

		vector<string> trainedExamples, heads, temp;
		vector<vector<string> > bodies;

		for(unsigned int i = 0; i < dataset.size(); i++)
		{
			line = dataset.at(i);

			// This line was generating an error on the algorithm by making clauses to have a different size than bodies and heads
			// if(util.trim(line).size() > 0 && (find(trainedExamples.begin(), trainedExamples.end(), line) == trainedExamples.end()))
			if(trim(line).size() > 0)
			{
				trainedExamples.push_back(line);
				pos = line.find(":-");

				if (pos != (unsigned int)string::npos)
				{
					tempStr = line.substr(0, pos);
					headStr = trim(tempStr);

					tempStr = line.substr(pos+2, line.size()-pos+1);
					bodyStr = trim(tempStr);
				}
				else
				{
					headStr  = line;
					bodyStr = "T";
				}

				pos = bodyStr.find(",");
				if(!temp.empty())
					temp.clear();
				while(pos != (unsigned int)string::npos)
				{
					tempStr = bodyStr.substr(0, pos);
					temp.push_back(trim(tempStr));

					bodyStr.erase(0, pos+1);
					pos = bodyStr.find(",");
				}
				temp.push_back(bodyStr);
				bodies.push_back(temp);

				heads.push_back(headStr);
			}
		}

		return make_pair(bodies, heads);
	}

	// Method to split the dataset clauses into bodies and heads
	pair<vector<vector<string> >, vector<string> > decodeRules(vector<string> dataset)
	{
		unsigned int pos;

		string headStr, bodyStr, line, tempStr;

		vector<string> trainedExamples, heads, temp;
		vector<vector<string> > bodies;

		for(unsigned int i = 0; i < dataset.size(); i++)
		{
			line = dataset.at(i);

			// This line was generating an error on the algorithm by making clauses to have a different size than bodies and heads
			// if(util.trim(line).size() > 0 && (find(trainedExamples.begin(), trainedExamples.end(), line) == trainedExamples.end()))
			if(trim(line).size() > 0)
			{
				trainedExamples.push_back(line);
				pos = line.find(":-");

				if (pos != (unsigned int)string::npos)
				{
					tempStr = line.substr(0, pos);
					headStr = trim(tempStr);

					tempStr = line.substr(pos+2, line.size()-pos+1);
					bodyStr = trim(tempStr);
				}
				else
				{
					headStr  = line;
					bodyStr = "true";
				}

				pos = bodyStr.find("),");
				if(!temp.empty())
					temp.clear();
				while(pos != (unsigned int)string::npos)
				{
					tempStr = bodyStr.substr(0, pos+1);
					temp.push_back(trim(tempStr));

					bodyStr.erase(0, pos+2);
					pos = bodyStr.find("),");
				}
				temp.push_back(bodyStr);
				bodies.push_back(temp);

				heads.push_back(headStr);
			}
		}

		return make_pair(bodies, heads);
	}

	string stringVectorToCsvString(vector<string> stringVector)
	{
		string output = "";

		for(unsigned int i = 0; i < stringVector.size(); i++)
			output.append(stringVector.at(i)).append(",");

		return output.substr(0, output.size()-1);
	}

	vector<int> csvStringToNumericalVector(string csvString)
	{
		unsigned int pos = 0;
		int result;
		string literal;
		vector<int> features;

		pos = csvString.find(",");
		while(pos != string::npos)
		{
			literal = csvString.substr(0, pos);
			convertFromString(result, literal, std::dec);
			features.push_back(result);

			csvString.replace(0, pos+1, "");
			pos = csvString.find(",");
		}

		convertFromString(result, csvString, std::dec);
		features.push_back(result);

		return features;
	}

	string toLower(string text)
	{
		string result = "";

		for(unsigned int i = 0; i < text.size(); i++)
			result.insert(result.end(), tolower(text.at(i)));

		return result;
	}

	string toUpper(string text)
	{
		string result = "";

		for(unsigned int i = 0; i < text.size(); i++)
			result.insert(result.end(), toupper(text.at(i)));

		return result;
	}

	vector<string> &split(const string &s, char delim, vector<string> &elems)
	{
	    stringstream ss(s);
	    string item;
	    while (getline(ss, item, delim))
	        elems.push_back(item);
	    return elems;
	}


	vector<string> split(const string &s, char delim)
	{
	    vector<string> elems;
	    split(s, delim, elems);
	    return elems;
	}

	vector<string> splitDelimited(string s, char delim)
	{
		unsigned int pos, pos2;
		string internalString;
	    vector<string> elems;

	    pos = s.find("(");
	    pos2 = s.find(")");
	    if((pos != string::npos) && (pos2 != string::npos) && (pos < pos2))
	    {
	    	internalString = s.substr(pos, pos2-pos+1);
	    	replace(internalString.begin(), internalString.end(), ',', ';');
	    	s.replace(pos, pos2-pos+1, internalString);
	    }

	    pos = s.find("[");
	    pos2 = s.find("]");
	    if((pos != string::npos) && (pos2 != string::npos) && (pos < pos2))
	    {
	    	internalString = s.substr(pos, pos2-pos+1);
	    	replace(internalString.begin(), internalString.end(), ',', ';');
	    	s.replace(pos, pos2-pos+1, internalString);
	    }

	    split(s, delim, elems);

	    for(unsigned int i = 0; i < elems.size(); i++)
	    	replace(elems.at(i).begin(), elems.at(i).end(), ';', ',');

	    return elems;
	}

	bool isVariable(string s)
	{
		return ((s.at(0) <= 'Z') && (s.at(0) >= 'A'));
	}

	void formatRule(string &rule)
	{
		int index, index2, size;
		string ignoredCharacters;

		// 1. Removing extra parenthesis
		index = 0;
		size = rule.size();
		ignoredCharacters = " ,()";
		while(index < size)
		{
			if(rule.at(index) != '(')
				index++;
			else
			{
				index2 = size-1;
				while(index2 > index)
				{
					if(rule.at(index2) != ')')
						index2--;
					else
					{
						// If there is no useful content, delete the entire content of the parenthesis
						if(rule.substr(index+1, (index2-index)-1).find_first_not_of(ignoredCharacters) == string::npos)
						{
							rule.erase(index, (index2-index)+1);
							size = rule.size();
						}

						index2 = index;
					}
				}

				index++;
			}
		}

		// 2. Removing extra commas
		index = 0;
		size = rule.size();
		ignoredCharacters = " ,";
		while(index < size)
		{
			if(rule.at(index) != ',')
				index++;
			else
			{
				if(rule.substr(index).find_first_not_of(ignoredCharacters) != string::npos)
				{
					rule.erase(index+1, rule.substr(index+1).find_first_not_of(ignoredCharacters));
					size = rule.size();
				}

				index++;
			}
		}
		index2 = rule.find(":-");
		index = rule.substr(index2+2).find_first_not_of(ignoredCharacters);
		if((unsigned int)index != string::npos)
			rule.erase(index2+3, index-1);

		// 3. Removing wrong combinations of commas and parenthesis
		index = 0;
		size = rule.size();
		ignoredCharacters = " ";
		while(index < size)
		{
			if(rule.at(index) != '(')
				index++;
			else
			{
				if((rule.substr(index+1).find_first_not_of(ignoredCharacters) != string::npos) && (rule.at(index+1+rule.substr(index+1).find_first_not_of(ignoredCharacters)) == ','))
				{
					rule.erase(index+1, rule.substr(index+2).find_first_not_of(ignoredCharacters)+1);
					size = rule.size();
				}

				index++;
			}
		}
		index = 0;
		size = rule.size();
		while(index < size)
		{
			if(rule.at(index) != ',')
				index++;
			else
			{
				if((rule.substr(index+1).find_first_not_of(ignoredCharacters) != string::npos) && (rule.at(index+1+rule.substr(index+1).find_first_not_of(ignoredCharacters)) == ')'))
				{
					rule.erase(index, rule.substr(index+1).find_first_not_of(ignoredCharacters)+1);
					size = rule.size();

					index--;
				}

				index++;
			}
		}

		// 4. Removing extra spaces
		index = 0;
		size = rule.size();
		ignoredCharacters = " ";
		while(index < size)
		{
			if(rule.at(index) != ' ')
				index++;
			else
			{
				if(rule.substr(index).find_first_not_of(ignoredCharacters) != string::npos)
				{
					rule.erase(index+1, rule.substr(index+1).find_first_not_of(ignoredCharacters));
					size = rule.size();
				}

				index++;
			}
		}
	}

}Utilities;

#endif /* UTILITARIA_H_ */
