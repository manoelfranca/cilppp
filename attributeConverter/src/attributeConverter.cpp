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

#include "utilities.h"

#define FOLDER_PATH "files/"
#define EXTENSION ".data"

int main (int argc, char *argv[])
{
	unsigned int pos = 0;
	string substring, literal, fileName, convertedLine;
	vector<string> files, lines, features, attributeMapping, usedFiles;
	vector<pair<string, string> > groupedFiles;

	FILE *convertedFile;

	Utilities util;

	files = util.getAllFiles(FOLDER_PATH, EXTENSION);

	for(unsigned int i = 0; i < files.size(); i++)
	{
		fileName = files.at(i);
		fileName = fileName.substr(0, files.at(i).find(EXTENSION));
		fileName.append("Converted").append(".con");

		convertedFile = fopen(fileName.c_str(), "w");
		lines = util.readClauses(files.at(i));

		if(files.at(i).find(".data") < 0)
		{
			pos = files.at(i).find(":-");

			if(util.trim(literal).at(0) == '~')
				attributeMapping.push_back(literal.substr(1));
			else
				attributeMapping.push_back(literal);
		}

		// 2. Collecting all attributes
		for(unsigned int j = 0; j < lines.size(); j++)
		{
			pos = lines.at(j).find(":-");
			substring = lines.at(j).substr(pos+2);

			pos = substring.find(",");
			while(pos != string::npos)
			{
				literal = substring.substr(0, pos);

				if(util.find(attributeMapping, literal) < 0)
				{
					if(util.trim(literal).at(0) == '~')
						attributeMapping.push_back(literal.substr(1));
					else
						attributeMapping.push_back(literal);
				}

				substring.replace(0, pos+1, "");
				pos = substring.find(",");
			}
		}

		// 3. Printing the labels
		convertedLine = "class,";
		convertedLine.append(util.stringVectorToCSVString(attributeMapping));
		fprintf(convertedFile, "%s\n", convertedLine.c_str());

		// 4. Creating and writing the vectors
		for(unsigned int j = 0; j < lines.size(); j++)
		{
			features.clear();
			features.resize(attributeMapping.size(), "0");

			if(util.trim(lines.at(j)).at(0) == '~')
				features.insert(features.begin(), "-1");
			else
				features.insert(features.begin(), "1");

			pos = lines.at(j).find(":-");

			substring = lines.at(j).substr(pos+2);

			pos = substring.find(",");
			while(pos != string::npos)
			{
				literal = substring.substr(0, pos);

				if(util.trim(literal).at(0) == '~')
					features.at(util.find(attributeMapping, literal.substr(1)) + 1) = "-1";
				else
					features.at(util.find(attributeMapping, literal) + 1) = "1";

				substring.replace(0, pos+1, "");
				pos = substring.find(",");
			}

			convertedLine = util.stringVectorToCSVString(features);
			fprintf(convertedFile, "%s\n", convertedLine.c_str());
		}

		fclose(convertedFile);
	}

	return 0;
}
