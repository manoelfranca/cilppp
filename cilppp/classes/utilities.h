/*
	CILP++

	Copyright 2012 Manoel Vitor Macedo Franca <maneuvitor@gmail.com>

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

#define LINE_SIZE 256

#include "mrmr/mrmr.h"
#include "smote/smote.h"

// Utility structure for storing general-purpose algorithms
typedef struct _Utilities
{
	vector<pair<string, vector<pair<int, double> > > > examplesBuffer;

	int find(vector<string> vector, string element)
	{
		unsigned int i;

		for(i = 0; i < vector.size(); i++)
		{
			if(vector.at(i).compare(element) == 0)
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

	int find(vector<int> vetor, int element)
	{
		unsigned int i;

		for(i = 0; i < vetor.size(); i++)
		{
			if(vetor.at(i) == element)
				return i;
		}
		return -1;
	}

	template <class T>
	int genericFind(vector<T> vetor, T element)
	{
		unsigned int i;

		for(i = 0; i < vetor.size(); i++)
		{
			if(vetor.at(i) == element)
				return i;
		}
		return -1;
	}

	template <class T1, class T2>
	int genericPairFind(pair<bool, bool> searchScope, vector<pair<T1, T2> > vetor, pair<T1, T2> element)
	{
		unsigned int i;

		if(searchScope.first)
		{
			if(searchScope.second)
			{
				for(i = 0; i < vetor.size(); i++)
				{
					if((vetor.at(i).first == element.first) && (vetor.at(i).second == element.second))
						return i;
				}
			}
			else
			{
				for(i = 0; i < vetor.size(); i++)
				{
					if(vetor.at(i).first == element.first)
						return i;
				}
			}
		}
		else if(searchScope.second)
		{
			for(i = 0; i < vetor.size(); i++)
			{
				if(vetor.at(i).second == element.second)
					return i;
			}
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

				while(line.find("\r") != string::npos)
				{
					pos = line.find("\r");
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

			fclose(file);
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

				while(line.find("\r") != string::npos)
				{
					pos = line.find("\r");
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

			fclose(file);
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

				while(line.find("\r") != string::npos)
				{
					pos = line.find("\r");
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

			if(finalLine.size() > 0)
				lines.push_back(finalLine);

			fclose(file);
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
			if((found != (unsigned int)string::npos) && (s.substr(found).compare(extension.c_str()) == 0))
			{
				removedFile = "";
				removedFile.append(dirName).append(s);
				remove(removedFile.c_str());
			}
		}

		(void) closedir(dp);
	}

	vector<string> getAllFiles(string dirName, string extension)
	{
		DIR *dp;
		struct dirent *ep;
		string s = "", selectedFile;
		unsigned int found;
		vector<string> files;

		dp = opendir(s.append("./").append(dirName).c_str());

		while ((ep = readdir(dp)))
		{
			s = string(ep->d_name);

			found = s.find(".");
			if((found != string::npos) && (s.substr(found).compare(extension.c_str()) == 0))
			{
				selectedFile = "";
				selectedFile.append(dirName).append(s);
				files.push_back(selectedFile);
			}
		}

		(void) closedir(dp);

		return files;
	}

	inline int getExpoBase2(double d)
	{
		int i = 0;
		short* shortD = (short*)(&d);
		short* shorti = (short*)(&i);

		shorti[0] = (shortD[3] & (short)32752); // _123456789ab____ & 0111111111110000
		return (i >> 4) - 1023;
	}

	bool equals(double d1, double d2)
	{
		if (d1 == d2)
			return true;
		int e1 = getExpoBase2(d1);
		int e2 = getExpoBase2(d2);
		int e3 = getExpoBase2(d1 - d2);
		if ((e3 - e2 < -48) && (e3 - e1 < -48))
			return true;
		return false;
	}

	int compare(double d1, double d2)
	{
		if (equals(d1, d2) == true)
			return 0;
		if (d1 > d2)
			return 1;
		return -1;
	}

	bool greater(double d1, double d2)
	{
		if (equals(d1, d2) == true)
			return false;
		if (d1 > d2)
			return true;
		return false;
	}

	bool greaterOrEqual(double d1, double d2)
	{
		if (equals(d1, d2) == true)
			return true;
		if (d1 > d2)
			return true;
		return false;
	}

	bool less(double d1, double d2)
	{
		if (equals(d1, d2) == true)
			return false;
		if (d1 < d2)
			return true;
		return false;
	}

	bool lessOrEqual(double d1, double d2)
	{
		if (equals(d1, d2) == true)
			return true;
		if (d1 < d2)
			return true;
		return false;
	}

	string returnFormattedTime(double secondsRaw)
	{
		int hours, minutes, seconds;
		char output[20];

		seconds = secondsRaw;
		hours   = seconds/3600;
		seconds = seconds%3600;
		minutes = seconds/60;
		seconds = seconds%60;

		sprintf (output, "%d:%02d:%02d", hours, minutes, seconds);

		return string(output);
	}

	string stringVectorToCsvString(vector<string> stringVector)
	{
		string output = "";

		for(unsigned int i = 0; i < stringVector.size(); i++)
			output.append(stringVector.at(i)).append(",");

		return output.substr(0, output.size()-1);
	}

	vector<string> csvStringToStringVector(string csvString)
	{
		unsigned int pos = 0;
		string literal;
		vector<string> features;

		pos = csvString.find(",");
		while(pos != string::npos)
		{
			literal = csvString.substr(0, pos);
			features.push_back(literal);

			csvString.replace(0, pos+1, "");
			pos = csvString.find(",");
		}

		features.push_back(literal);

		return features;
	}

	vector<string> clausesToFeatures(vector<string> lines)
	{
		unsigned int pos = 0;
		string substring, literal, fileName, convertedLine;
		vector<string> files, features, attributeMapping, convertedLines;
		vector<pair<string, string> > groupedFiles;

		// 1. Collecting all attributes
		for(unsigned int j = 0; j < lines.size(); j++)
		{
			pos = lines.at(j).find(":-");
			substring = lines.at(j).substr(pos+2);

			pos = substring.find(",");
			while(pos != string::npos)
			{
				literal = substring.substr(0, pos);

				if(findAtom(attributeMapping, literal) < 0)
				{
					if(literal.at(0) == '~')
						attributeMapping.push_back(literal.substr(1));
					else
						attributeMapping.push_back(literal);
				}

				substring.replace(0, pos+1, "");
				pos = substring.find(",");
			}
		}

		// 2. Printing the labels
		convertedLine = "class,";
		convertedLine.append(stringVectorToCsvString(attributeMapping));
		convertedLines.push_back(convertedLine);

		// 3. Creating and writing the vectors
		for(unsigned int j = 0; j < lines.size(); j++)
		{
			features.clear();
			features.resize(attributeMapping.size() + 1, "0");

			if(lines.at(j).at(0) == '~')
				features.at(0) = "-1";
			else
				features.at(0) = "1";

			pos = lines.at(j).find(":-");
			substring = lines.at(j).substr(pos+2);

			pos = substring.find(",");
			while(pos != string::npos)
			{
				literal = substring.substr(0, pos);

				if(literal.at(0) == '~')
					features.at(find(attributeMapping, literal.substr(1))) = "-1";
				else
					features.at(find(attributeMapping, literal)) = "1";

				substring.replace(0, pos+1, "");
				pos = substring.find(",");
			}

			convertedLine = stringVectorToCsvString(features);
			convertedLines.push_back(convertedLine);
		}

		return convertedLines;
	}

	// TODO: complete this method, if needed
	pair<string, pair<vector<string>, vector<string> > > readWeka(string fileName)
	{
		bool allIsDataNow = false;

		unsigned int pos, pos1, pos2;

		string relation, lineLowerCase, temp;
		vector<string> mapping, clauses;
		vector<string> lines = readClauses(fileName);

		for(unsigned int i = 0; i < lines.size(); i++)
		{
			if(!allIsDataNow)
			{
				lineLowerCase = toLower(lines.at(i));

				// Checking if it is a @relation
				pos = lineLowerCase.find("@relation");
				if(pos != string::npos)
					relation = trim(lineLowerCase.erase(pos, pos+9));

				// Checking if it is a @attribute
				pos = lineLowerCase.find("@attribute");
				if(pos != string::npos)
				{
					temp = trim(lineLowerCase.erase(pos, pos+10));

					pos1 = lineLowerCase.find("{");
					pos2 = lineLowerCase.find("}");
					mapping.push_back(trim(temp.erase(pos1, pos2+1)));
				}

				// Checking if it is a @data
				pos = lineLowerCase.find("@data");
				if(pos != string::npos)
					allIsDataNow = true;
			}
			else
			{
				//Decode data into clause
			}
		}

		// TODO: correct this line
		return make_pair(temp, make_pair(lines, lines));
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
	    	replace(internalString.begin(), internalString.end(), delim, '#');
	    	s.replace(pos, pos2-pos+1, internalString);
	    }

	    pos = s.find("[");
	    pos2 = s.find("]");
	    if((pos != string::npos) && (pos2 != string::npos) && (pos < pos2))
	    {
	    	internalString = s.substr(pos, pos2-pos+1);
	    	replace(internalString.begin(), internalString.end(), delim, '#');
	    	s.replace(pos, pos2-pos+1, internalString);
	    }

	    split(s, delim, elems);

	    for(unsigned int i = 0; i < elems.size(); i++)
	    	replace(elems.at(i).begin(), elems.at(i).end(), '#', delim);

	    return elems;
	}

	bool isVariable(string s)
	{
		return (((s.at(0) <= 'Z') && (s.at(0) >= 'A')) || (s.at(0) == '_'));
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

	// Method to split the dataset clauses into bodies and heads
	pair<vector<vector<string> >, vector<vector<string> > > decodeRules(vector<string> dataset)
	{
		unsigned int pos;

		string headStr, bodyStr, line, tempStr;

		vector<string> trainedExamples, temp;
		vector<vector<string> > bodies, heads;

		for(unsigned int i = 0; i < dataset.size(); i++)
		{
			line = dataset.at(i);

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

				pos = headStr.find("),");
				if(!temp.empty())
					temp.clear();
				while(pos != (unsigned int)string::npos)
				{
					tempStr = headStr.substr(0, pos+1);
					temp.push_back(trim(tempStr));

					headStr.erase(0, pos+2);
					pos = headStr.find("),");
				}
				temp.push_back(headStr);
				heads.push_back(temp);
			}
		}

		return make_pair(bodies, heads);
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

	string idToVariable(int id)
	{
		char factor 	 = (id/26) + 48;
		char featureChar = (id%26) + 65;

		string result = "";

		result.append(1, featureChar);
		if(factor != '0')
			result.append(1, factor);

		return result;
	}

}Utilities;

#endif /* UTILITARIA_H_ */
