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

#define POSITIVE_FILE "muta.f"
#define NEGATIVE_FILE "muta.n"

#define NOISE 0.0
#define NOISE_ON_TEST false

// Method to shuffle a file
void foldsCreator(string filePathPositive, string filePathNegative, int numberOfFolds)
{
	unsigned int randIndex, positiveFoldSize, negativeFoldSize, positiveTrainingSize, positiveTestingSize;
	bool end;
	string positiveExtension, negativeExtension, fileName, temp;

	FILE *baseFile, *baseFilePositive, *baseFileNegative;
	Utilities util;

	vector<int> chosenNoise;
	vector<string> positiveLines, negativeLines, linesTempPositive, linesTempNegative, unifiedTrainLines, unifiedTestLines;

	srand(time(NULL));

	positiveLines = util.readLines(filePathPositive);
	negativeLines = util.readLines(filePathNegative);

	end = positiveLines.empty() && negativeLines.empty();

	// 1. Shuffling data
	while(!end)
	{
		end = true;

		if(positiveLines.size())
		{
			end = false;

			randIndex = (int) floor(((double)rand() / RAND_MAX) * positiveLines.size());
			while(randIndex == positiveLines.size())
				randIndex = (int) floor(((double)rand() / RAND_MAX) * positiveLines.size());

			linesTempPositive.push_back(positiveLines.at(randIndex));
			positiveLines.erase(positiveLines.begin() + randIndex);
		}

		if(negativeLines.size())
		{
			end = false;

			randIndex = (int) floor(((double)rand() / RAND_MAX) * negativeLines.size());
			while(randIndex == negativeLines.size())
				randIndex = (int) floor(((double)rand() / RAND_MAX) * negativeLines.size());

			linesTempNegative.push_back(negativeLines.at(randIndex));
			negativeLines.erase(negativeLines.begin() + randIndex);
		}
	}

	if(positiveLines.empty())
		positiveLines = linesTempPositive;

	if(negativeLines.empty())
		negativeLines = linesTempNegative;

	// 2. Getting the file extension
	positiveExtension = filePathPositive.substr(filePathPositive.find("."), filePathPositive.size() - filePathPositive.find("."));
	negativeExtension = filePathNegative.substr(filePathNegative.find("."), filePathNegative.size() - filePathNegative.find("."));

	// 3. Filling up the train/test folds
	if(numberOfFolds <= 0)
	{
		// If the number of folds is zero or less, leave-one-out will be performed
		for(unsigned int i = 0; i < positiveLines.size() + negativeLines.size(); i++)
		{
			// 3.1. Preparing each train fold
			fileName = "";
			fileName = fileName.append("files/train").append(util.convertToString(i+1)).append(positiveExtension);
			baseFilePositive = fopen(fileName.c_str(), "w");

			fileName = "";
			fileName = fileName.append("files/train").append(util.convertToString(i+1)).append(negativeExtension);
			baseFileNegative = fopen(fileName.c_str(), "w");

			for(unsigned int j = 0; j < positiveLines.size() + negativeLines.size(); j++)
			{
				if(i == j)
					continue;

				// If it was positive before, now is negative
				if(j < positiveLines.size())
					fprintf(baseFilePositive, "%s\n", positiveLines.at(j).c_str());
				// If it was negative before, now is positive
				else
					fprintf(baseFileNegative, "%s\n", negativeLines.at(j - positiveLines.size()).c_str());
			}

			fclose(baseFilePositive);
			fclose(baseFileNegative);

			// 3.2. Preparing each test fold
			fileName = "";
			fileName = fileName.append("files/test").append(util.convertToString(i+1)).append(positiveExtension);
			baseFilePositive = fopen(fileName.c_str(), "w");

			fileName = "";
			fileName = fileName.append("files/test").append(util.convertToString(i+1)).append(negativeExtension);
			baseFileNegative = fopen(fileName.c_str(), "w");

			if(i < positiveLines.size())
				fprintf(baseFilePositive, "%s\n", positiveLines.at(i).c_str());
			else
				fprintf(baseFileNegative, "%s\n", negativeLines.at(i - positiveLines.size()).c_str());

			fclose(baseFilePositive);
			fclose(baseFileNegative);
		}
	}
	else
	{
		// 4. Creating a number of train/test folds according to 'numberOfFolds'
		positiveFoldSize = positiveLines.size()/numberOfFolds;
		negativeFoldSize = negativeLines.size()/numberOfFolds;

		if(!positiveFoldSize || !negativeFoldSize)
		{
			cout << "Error: dataset is smaller than number of folds!" << endl;
			return;
		}

		for(int i = 0; i < numberOfFolds; i++)
		{
			// 4.1. Preparing each train fold (possibly with noise)
			unifiedTrainLines.clear();
			for(int j = 0; j < (i * positiveFoldSize); j++)
				unifiedTrainLines.push_back(positiveLines.at(j));

			for(unsigned int j = (i+1) * positiveFoldSize; (j < positiveLines.size()) && ((i+1) != numberOfFolds); j++)
				unifiedTrainLines.push_back(positiveLines.at(j));

			positiveTrainingSize = unifiedTrainLines.size();

			for(int j = 0; j < (i * negativeFoldSize); j++)
				unifiedTrainLines.push_back(negativeLines.at(j));

			for(unsigned int j = (i+1) * negativeFoldSize; (j < negativeLines.size()) && ((i+1) != numberOfFolds); j++)
				unifiedTrainLines.push_back(negativeLines.at(j));

			// 4.2. Preparing each test fold (possibly with noise)
			unifiedTestLines.clear();
			if(NOISE_ON_TEST)
			{
				for(int j = i * positiveFoldSize; j < ( ((i+1) == numberOfFolds) ? positiveLines.size() : ((i+1) * positiveFoldSize) ); j++)
					unifiedTestLines.push_back(positiveLines.at(j));

				positiveTestingSize = unifiedTestLines.size();

				for(int j = i * negativeFoldSize; j < ( ((i+1) == numberOfFolds) ? negativeLines.size() : ((i+1) * negativeFoldSize) ); j++)
					unifiedTestLines.push_back(negativeLines.at(j));
			}

			// 4.3. Choosing random lines to be "flipped"
			chosenNoise.clear();
			for(unsigned int j = 0; j < floor((unifiedTrainLines.size() + unifiedTestLines.size()) * NOISE); j++)
			{
				randIndex = (int) floor(((double)rand() / RAND_MAX) * (unifiedTrainLines.size() + unifiedTestLines.size()));
				while((randIndex == (unifiedTrainLines.size() + unifiedTestLines.size())) && (util.find(chosenNoise, randIndex) >= 0))
					randIndex = (int) floor(((double)rand() / RAND_MAX) * (unifiedTrainLines.size() + unifiedTestLines.size()));

				chosenNoise.push_back(randIndex);
			}

			// 4.4. Preparing each train fold (possibly with noise)
			fileName = "";
			fileName = fileName.append("files/train").append(util.convertToString(i+1)).append(positiveExtension);
			baseFilePositive = fopen(fileName.c_str(), "w");

			fileName = "";
			fileName = fileName.append("files/train").append(util.convertToString(i+1)).append(negativeExtension);
			baseFileNegative = fopen(fileName.c_str(), "w");

			for(unsigned int j = 0; j < unifiedTrainLines.size(); j++)
			{
				// If the current training data belongs to the set of noisy examples
				if(util.find(chosenNoise, j) >= 0)
				{
					// If it was positive before, now is negative
					if(j < positiveTrainingSize)
						fprintf(baseFileNegative, "%s\n", unifiedTrainLines.at(j).c_str());
					// If it was negative before, now is positive
					else
						fprintf(baseFilePositive, "%s\n", unifiedTrainLines.at(j).c_str());
				}
				else
				{
					// If it was positive before, now is negative
					if(j < positiveTrainingSize)
						fprintf(baseFilePositive, "%s\n", unifiedTrainLines.at(j).c_str());
					// If it was negative before, now is positive
					else
						fprintf(baseFileNegative, "%s\n", unifiedTrainLines.at(j).c_str());
				}
			}

			fclose(baseFilePositive);
			fclose(baseFileNegative);

			if(!NOISE_ON_TEST)
			{
				// 4.5. Preparing each test fold (possibly with noise)
				fileName = "";
				fileName = fileName.append("files/test").append(util.convertToString(i+1)).append(positiveExtension);
				baseFile = fopen(fileName.c_str(), "w");
				for(int j = i * positiveFoldSize; j < ( ((i+1) == numberOfFolds) ? positiveLines.size() : ((i+1) * positiveFoldSize) ); j++)
					fprintf(baseFile, "%s\n", positiveLines.at(j).c_str());
				fclose(baseFile);

				fileName = "";
				fileName = fileName.append("files/test").append(util.convertToString(i+1)).append(negativeExtension);
				baseFile = fopen(fileName.c_str(), "w");
				for(int j = i * negativeFoldSize; j < ( ((i+1) == numberOfFolds) ? negativeLines.size() : ((i+1) * negativeFoldSize) ); j++)
					fprintf(baseFile, "%s\n", negativeLines.at(j).c_str());
				fclose(baseFile);
			}
			else
			{
				// 4.6. Preparing each test fold (possibly with noise)
				fileName = "";
				fileName = fileName.append("files/test").append(util.convertToString(i+1)).append(positiveExtension);
				baseFilePositive = fopen(fileName.c_str(), "w");

				fileName = "";
				fileName = fileName.append("files/test").append(util.convertToString(i+1)).append(negativeExtension);
				baseFileNegative = fopen(fileName.c_str(), "w");

				for(unsigned int j = 0; j < unifiedTestLines.size(); j++)
				{
					// If the current testing data belongs to the set of noisy examples
					if(util.find(chosenNoise, j + unifiedTrainLines.size()) >= 0)
					{
						// If it was positive before, now is negative
						if(j < positiveTestingSize)
							fprintf(baseFileNegative, "%s\n", unifiedTestLines.at(j).c_str());
						// If it was negative before, now is positive
						else
							fprintf(baseFilePositive, "%s\n", unifiedTestLines.at(j).c_str());
					}
					else
					{
						// If it was positive before, now is negative
						if(j < positiveTestingSize)
							fprintf(baseFilePositive, "%s\n", unifiedTestLines.at(j).c_str());
						// If it was negative before, now is positive
						else
							fprintf(baseFileNegative, "%s\n", unifiedTestLines.at(j).c_str());
					}
				}

				fclose(baseFilePositive);
				fclose(baseFileNegative);
			}
		}
	}
}

void incrementalFoldsCreator(string filePathPositive, string filePathNegative, int numberOfExperiments, int numberOfFolds)
{
	unsigned int randIndex, positiveFoldSize, negativeFoldSize, positiveIncrementalSize, negativeIncrementalSize, positiveTrainingSize, positiveTestingSize;
	bool end;
	string positiveExtension, negativeExtension, fileName, dirName, temp;

	FILE *baseFile, *baseFilePositive, *baseFileNegative;
	Utilities util;

	vector<int> chosenNoise;
	vector<string> positiveLines, negativeLines, linesTempPositive, linesTempNegative, unifiedTrainLines, unifiedTestLines;

	srand(time(NULL));

	positiveLines = util.readLines(filePathPositive);
	negativeLines = util.readLines(filePathNegative);

	end = positiveLines.empty() && negativeLines.empty();

	// 1. Shuffling data
	while(!end)
	{
		end = true;

		if(positiveLines.size())
		{
			end = false;

			randIndex = (int) floor(((double)rand() / RAND_MAX) * positiveLines.size());
			while(randIndex == positiveLines.size())
				randIndex = (int) floor(((double)rand() / RAND_MAX) * positiveLines.size());

			linesTempPositive.push_back(positiveLines.at(randIndex));
			positiveLines.erase(positiveLines.begin() + randIndex);
		}

		if(negativeLines.size())
		{
			end = false;

			randIndex = (int) floor(((double)rand() / RAND_MAX) * negativeLines.size());
			while(randIndex == negativeLines.size())
				randIndex = (int) floor(((double)rand() / RAND_MAX) * negativeLines.size());

			linesTempNegative.push_back(negativeLines.at(randIndex));
			negativeLines.erase(negativeLines.begin() + randIndex);
		}
	}

	if(positiveLines.empty())
		positiveLines = linesTempPositive;

	if(negativeLines.empty())
		negativeLines = linesTempNegative;

	// 2. Getting the file extension
	positiveExtension = filePathPositive.substr(filePathPositive.find("."), filePathPositive.size() - filePathPositive.find("."));
	negativeExtension = filePathNegative.substr(filePathNegative.find("."), filePathNegative.size() - filePathNegative.find("."));

	// 3. Setting up the incremental folds size
	positiveIncrementalSize = positiveLines.size()/numberOfExperiments;
	negativeIncrementalSize = negativeLines.size()/numberOfExperiments;

	// 4. Creating the incremental folds
	for(int e = 0; e < numberOfExperiments; e++)
	{
		dirName = "";
		dirName.append("files/incremental").append(util.convertToString(e+1));
		mkdir(dirName.c_str());
		DIR *dir = opendir(dirName.c_str());
		chdir(dirName.c_str());

		positiveFoldSize = (positiveIncrementalSize * (e+1))/numberOfFolds;
		negativeFoldSize = (negativeIncrementalSize * (e+1))/numberOfFolds;

		for(int i = 0; i < numberOfFolds; i++)
		{
			// 4.1. Preparing each train fold (possibly with noise)
			unifiedTrainLines.clear();
			for(int j = 0; j < (i * positiveFoldSize); j++)
				unifiedTrainLines.push_back(positiveLines.at(j));

			for(unsigned int j = (i+1) * positiveFoldSize; (j < (positiveIncrementalSize * (e+1))) && ((i+1) != numberOfFolds); j++)
				unifiedTrainLines.push_back(positiveLines.at(j));

			positiveTrainingSize = unifiedTrainLines.size();

			for(int j = 0; j < (i * negativeFoldSize); j++)
				unifiedTrainLines.push_back(negativeLines.at(j));

			for(unsigned int j = (i+1) * negativeFoldSize; (j < (negativeIncrementalSize * (e+1))) && ((i+1) != numberOfFolds); j++)
				unifiedTrainLines.push_back(negativeLines.at(j));

			// 4.2. Preparing each test fold (possibly with noise)
			unifiedTestLines.clear();
			if(NOISE_ON_TEST)
			{
				for(int j = i * positiveFoldSize; j < ( ((i+1) == numberOfFolds) ? (positiveIncrementalSize * (e+1)) : ((i+1) * positiveFoldSize) ); j++)
					unifiedTestLines.push_back(positiveLines.at(j));

				positiveTestingSize = unifiedTestLines.size();

				for(int j = i * negativeFoldSize; j < ( ((i+1) == numberOfFolds) ? (negativeIncrementalSize * (e+1)) : ((i+1) * negativeFoldSize) ); j++)
					unifiedTestLines.push_back(negativeLines.at(j));
			}

			// 4.3. Choosing random lines to be "flipped"
			chosenNoise.clear();
			for(unsigned int j = 0; j < floor((unifiedTrainLines.size() + unifiedTestLines.size()) * NOISE); j++)
			{
				randIndex = (int) floor(((double)rand() / RAND_MAX) * (unifiedTrainLines.size() + unifiedTestLines.size()));
				while((randIndex == (unifiedTrainLines.size() + unifiedTestLines.size())) && (util.find(chosenNoise, randIndex) >= 0))
					randIndex = (int) floor(((double)rand() / RAND_MAX) * (unifiedTrainLines.size() + unifiedTestLines.size()));

				chosenNoise.push_back(randIndex);
			}

			// 4.4. Preparing each train fold (possibly with noise)
			fileName = "";
			fileName = fileName.append("train").append(util.convertToString(i+1)).append(positiveExtension);
			baseFilePositive = fopen(fileName.c_str(), "w");

			fileName = "";
			fileName = fileName.append("train").append(util.convertToString(i+1)).append(negativeExtension);
			baseFileNegative = fopen(fileName.c_str(), "w");

			for(unsigned int j = 0; j < unifiedTrainLines.size(); j++)
			{
				// If the current training data belongs to the set of noisy examples
				if(util.find(chosenNoise, j) >= 0)
				{
					// If it was positive before, now is negative
					if(j < positiveTrainingSize)
						fprintf(baseFileNegative, "%s\n", unifiedTrainLines.at(j).c_str());
					// If it was negative before, now is positive
					else
						fprintf(baseFilePositive, "%s\n", unifiedTrainLines.at(j).c_str());
				}
				else
				{
					// If it was positive before, now is negative
					if(j < positiveTrainingSize)
						fprintf(baseFilePositive, "%s\n", unifiedTrainLines.at(j).c_str());
					// If it was negative before, now is positive
					else
						fprintf(baseFileNegative, "%s\n", unifiedTrainLines.at(j).c_str());
				}
			}

			fclose(baseFilePositive);
			fclose(baseFileNegative);

			if(!NOISE_ON_TEST)
			{
				// 4.5. Preparing each test fold (possibly with noise)
				fileName = "";
				fileName = fileName.append("test").append(util.convertToString(i+1)).append(positiveExtension);
				baseFile = fopen(fileName.c_str(), "w");
				for(int j = i * positiveFoldSize; j < ( ((i+1) == numberOfFolds) ? (positiveIncrementalSize * (e+1)) : ((i+1) * positiveFoldSize) ); j++)
					fprintf(baseFile, "%s\n", positiveLines.at(j).c_str());
				fclose(baseFile);

				fileName = "";
				fileName = fileName.append("test").append(util.convertToString(i+1)).append(negativeExtension);
				baseFile = fopen(fileName.c_str(), "w");
				for(int j = i * negativeFoldSize; j < ( ((i+1) == numberOfFolds) ? (negativeIncrementalSize * (e+1)) : ((i+1) * negativeFoldSize) ); j++)
					fprintf(baseFile, "%s\n", negativeLines.at(j).c_str());
				fclose(baseFile);
			}
			else
			{
				// 4.6. Preparing each test fold (possibly with noise)
				fileName = "";
				fileName = fileName.append("test").append(util.convertToString(i+1)).append(positiveExtension);
				baseFilePositive = fopen(fileName.c_str(), "w");

				fileName = "";
				fileName = fileName.append("test").append(util.convertToString(i+1)).append(negativeExtension);
				baseFileNegative = fopen(fileName.c_str(), "w");

				for(unsigned int j = 0; j < unifiedTestLines.size(); j++)
				{
					// If the current testing data belongs to the set of noisy examples
					if(util.find(chosenNoise, j + unifiedTrainLines.size()) >= 0)
					{
						// If it was positive before, now is negative
						if(j < positiveTestingSize)
							fprintf(baseFileNegative, "%s\n", unifiedTestLines.at(j).c_str());
						// If it was negative before, now is positive
						else
							fprintf(baseFilePositive, "%s\n", unifiedTestLines.at(j).c_str());
					}
					else
					{
						// If it was positive before, now is negative
						if(j < positiveTestingSize)
							fprintf(baseFilePositive, "%s\n", unifiedTestLines.at(j).c_str());
						// If it was negative before, now is positive
						else
							fprintf(baseFileNegative, "%s\n", unifiedTestLines.at(j).c_str());
					}
				}

				fclose(baseFilePositive);
				fclose(baseFileNegative);
			}
		}

		chdir("../");
		chdir("../");
		closedir(dir);
	}
}

int main (int argc, char *argv[])
{
	string filePathPositive = "files/";
	string filePathNegative = "files/";

	filePathPositive.append(POSITIVE_FILE);
	filePathNegative.append(NEGATIVE_FILE);

	//incrementalFoldsCreator(filePathPositive, filePathNegative, 10, 10);
	foldsCreator(filePathPositive, filePathNegative, 0);

	return 0;
}
