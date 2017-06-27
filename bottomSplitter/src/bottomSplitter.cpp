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

// Method to split a recently-built bottom-clause file into background knowledge and training,
// according to a "numberOfData" parameter
void splitIntoBKTraining(string bottomPath, double bkProportion, bool useNegatives)
{
	int index, multiplier;
	unsigned int pos, position;

	vector<int> positiveIndexes, negativeIndexes;
	vector<string> bottomData, positiveBottomData, negativeBottomData;

	Utilities util;

	FILE *backgroundKnowledgeFile, *dataFile;

	srand (time(NULL));

	backgroundKnowledgeFile = fopen("files/bottoms_bk.txt", "w");
	dataFile = fopen("files/bottoms_train.txt", "w");

	bottomData = util.readClauses(bottomPath);

	for(unsigned int i = 0; i < bottomData.size(); i++)
	{
		pos = bottomData.at(i).find("[");
		while((bottomData.at(i).size() > 0) && (pos != string::npos))
		{
			position = bottomData.at(i).find("]");
			bottomData.at(i).replace(pos, position - pos + 1, "");

			pos = bottomData.at(i).find("[");
		}

		pos = bottomData.at(i).find("),");
		while((bottomData.at(i).size() > 0) && (pos != string::npos))
		{
			bottomData.at(i).replace(pos, 2, "#");
			pos = bottomData.at(i).find("),");
		}

		pos = bottomData.at(i).find(",");
		while((bottomData.at(i).size() > 0) && (pos != string::npos))
		{
			bottomData.at(i).replace(pos, 1, ";");
			pos = bottomData.at(i).find(",");
		}

		pos = bottomData.at(i).find("#");
		while((bottomData.at(i).size() > 0) && (pos != string::npos))
		{
			bottomData.at(i).replace(pos, 1, "),");
			pos = bottomData.at(i).find("#");
		}

		if((bottomData.at(i).size() > 0) && (bottomData.at(i).find(":-") != string::npos))
		{
			if(bottomData.at(i).at(0) == '~')
				negativeBottomData.push_back(bottomData.at(i));
			else
				positiveBottomData.push_back(bottomData.at(i));
		}
	}

	if(useNegatives)
		multiplier = 1;
	else
		multiplier = 2;

	while(positiveIndexes.size() < (unsigned int)floor(positiveBottomData.size() * bkProportion * multiplier))
	{
		index = (int)(floor(((double)rand()/RAND_MAX) * positiveBottomData.size()));

		if(util.find(positiveIndexes, index) < 0)
		{
			positiveIndexes.push_back(index);
			fprintf(backgroundKnowledgeFile, "%s\n", positiveBottomData.at(index).c_str());
		}
	}

	while((negativeIndexes.size() < (unsigned int)floor(negativeBottomData.size() * bkProportion)) && useNegatives)
	{
		index = (int)(floor(((double)rand()/RAND_MAX) * negativeBottomData.size()));

		if(util.find(negativeIndexes, index) < 0)
		{
			negativeIndexes.push_back(index);
			fprintf(backgroundKnowledgeFile, "%s\n", negativeBottomData.at(index).c_str());
		}
	}

	sort(positiveIndexes.begin(), positiveIndexes.end());
	sort(negativeIndexes.begin(), negativeIndexes.end());

	for(int i = positiveIndexes.size() - 1; i >= 0; i--)
		positiveBottomData.erase(positiveBottomData.begin()+positiveIndexes.at(i));

	for(int i = negativeIndexes.size() - 1; i >= 0; i--)
		negativeBottomData.erase(negativeBottomData.begin()+negativeIndexes.at(i));

	for(unsigned int i = 0; i < positiveBottomData.size(); i++)
		fprintf(dataFile, "%s\n", positiveBottomData.at(i).c_str());

	for(unsigned int i = 0; i < negativeBottomData.size(); i++)
		fprintf(dataFile, "%s\n", negativeBottomData.at(i).c_str());
}

int main (int argc, char *argv[])
{
	string bottomFilePath = "files/bottomsFull.txt";

	splitIntoBKTraining(bottomFilePath, 0.05, false);

	return 0;
}

