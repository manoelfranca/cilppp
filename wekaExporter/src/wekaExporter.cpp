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

//============================================================================
// Name        : wekaExporter.cpp
// Author      : Manoel Vitor Macedo Franca
// Version     : 1.0
// Copyright   : Apache version 2.0
// Description : A small program to convert bottom clauses into .arff
//============================================================================

#include "utilities.h"

#define ATTRIBUTE_TYPE "{-1,0,1}"
#define CLASS_TYPE "{-1,0,1}"
#define NUMBER_OF_FOLDS 10

#define EXPORT_OR_IMPORT 'e';

#define CLASS "eastbound"
#define EXTENSION ".data"

// Method to build the formatted dataset, depending on the data class and type
void exportArff(string dataClass, vector<string> trainingData, vector<string> testingData)
{
	int index;
	bool isNegated;

	string outputFileName, bodyWithoutNegation, headWithoutNegation;
	vector<string> bodyMapping, pattern;

	Utilities util;

	outputFileName = "files/";
	outputFileName = outputFileName.append(dataClass).append("Training.arff");
	FILE *outputTrainingFile = fopen(outputFileName.c_str(), "w");

	outputFileName = "files/";
	outputFileName = outputFileName.append(dataClass).append("Testing.arff");
	FILE *outputTestingFile = fopen(outputFileName.c_str(), "w");

	pair<vector<vector<string> >, vector<string> > decodedTrainingData, decodedTestingData;

	////////////////////
	// Part 1: header //
	////////////////////

	fprintf(outputTrainingFile, "@Relation %s\n\n", dataClass.c_str());
	fprintf(outputTestingFile, "@Relation %s\n\n", dataClass.c_str());

	// Splitting the lines into bodies/heads
	decodedTrainingData = util.decodeData(trainingData);
	decodedTestingData = util.decodeData(testingData);

	// Creating a vector mapping for the found training bodies
	for(unsigned int i = 0; i < decodedTrainingData.first.size(); i++)
	{
		for(unsigned int j = 0; j < decodedTrainingData.first.at(i).size(); j++)
		{
			bodyWithoutNegation = ((decodedTrainingData.first.at(i).at(j).at(0) == '~') ? decodedTrainingData.first.at(i).at(j).substr(1, decodedTrainingData.first.at(i).at(j).size()-1) : decodedTrainingData.first.at(i).at(j));

			if(util.find(bodyMapping, bodyWithoutNegation) == -1)
			{
				fprintf(outputTrainingFile, "@Attribute %s %s\n", bodyWithoutNegation.c_str(), ATTRIBUTE_TYPE);
				fprintf(outputTestingFile, "@Attribute %s %s\n", bodyWithoutNegation.c_str(), ATTRIBUTE_TYPE);
				bodyMapping.push_back(bodyWithoutNegation);
			}
		}
	}

	// Verifying if the head is negated
	headWithoutNegation = ((decodedTrainingData.second.at(0).at(0) == '~') ? decodedTrainingData.second.at(0).substr(1, decodedTrainingData.second.at(0).size()-1) : decodedTrainingData.second.at(0));
	fprintf(outputTrainingFile, "@Attribute %s %s\n", headWithoutNegation.c_str(), CLASS_TYPE);

	headWithoutNegation = ((decodedTestingData.second.at(0).at(0) == '~') ? decodedTestingData.second.at(0).substr(1, decodedTestingData.second.at(0).size()-1) : decodedTestingData.second.at(0));
	fprintf(outputTestingFile, "@Attribute %s %s\n", headWithoutNegation.c_str(), CLASS_TYPE);

	fprintf(outputTrainingFile, "\n@Data\n");
	fprintf(outputTestingFile, "\n@Data\n");

	//////////////////
	// Part 2: data //
	//////////////////

	// Printing training data table
	for(unsigned int i = 0; i < decodedTrainingData.first.size(); i++)
	{
		pattern.clear();
		pattern.resize(bodyMapping.size(), "0");

		for(unsigned int j = 0; j < decodedTrainingData.first.at(i).size(); j++)
		{
			isNegated = (decodedTrainingData.first.at(i).at(j).at(0) == '~');
			bodyWithoutNegation = (isNegated ? decodedTrainingData.first.at(i).at(j).substr(1, decodedTrainingData.first.at(i).at(j).size()-1) : decodedTrainingData.first.at(i).at(j));

			if(isNegated)
				pattern.at(util.find(bodyMapping, bodyWithoutNegation)) = "-1";
			else
				pattern.at(util.find(bodyMapping, bodyWithoutNegation)) = "1";
		}

		isNegated = (decodedTrainingData.second.at(i).at(0) == '~');

		if(isNegated)
			fprintf(outputTrainingFile, "%s,-1\n", util.stringVectorToCsvString(pattern).c_str());
		else
			fprintf(outputTrainingFile, "%s,1\n", util.stringVectorToCsvString(pattern).c_str());
	}

	// Printing testing data table
	for(unsigned int i = 0; i < decodedTestingData.first.size(); i++)
	{
		pattern.clear();
		pattern.resize(bodyMapping.size(), "0");

		for(unsigned int j = 0; j < decodedTestingData.first.at(i).size(); j++)
		{
			isNegated = (decodedTestingData.first.at(i).at(j).at(0) == '~');
			bodyWithoutNegation = (isNegated ? decodedTestingData.first.at(i).at(j).substr(1, decodedTestingData.first.at(i).at(j).size()-1) : decodedTestingData.first.at(i).at(j));

			index = util.find(bodyMapping, bodyWithoutNegation);

			if(index >= 0)
			{
				if(isNegated)
					pattern.at(util.find(bodyMapping, bodyWithoutNegation)) = "-1";
				else
					pattern.at(util.find(bodyMapping, bodyWithoutNegation)) = "1";
			}
		}

		isNegated = (decodedTestingData.second.at(i).at(0) == '~');

		if(isNegated)
			fprintf(outputTestingFile, "%s,-1\n", util.stringVectorToCsvString(pattern).c_str());
		else
			fprintf(outputTestingFile, "%s,1\n", util.stringVectorToCsvString(pattern).c_str());
	}

	fclose(outputTrainingFile);
	fclose(outputTestingFile);
}

// Method to build the formatted dataset, depending on the data class and type (currently, only binary data -- {-1, 0, 1} is accepted)
void importArff(vector<string> lines, string type, int index)
{
	bool isData = false;
	unsigned int pos, pos1, pos2;

	string baseName, positiveName, negativeName, currentClause;
	vector<int> attributes;
	vector<string> bodyMapping;

	Utilities util;

	if(lines.empty())
		return;

	baseName = "files/";
	positiveName = negativeName = baseName.append(type);

	positiveName.append("positive").append(util.convertToString(index)).append(".data");
	negativeName.append("negative").append(util.convertToString(index)).append(".data");

	FILE *positiveFile = fopen(positiveName.c_str(), "w");
	FILE *negativeFile = fopen(negativeName.c_str(), "w");

	// Verifying each line and collecting all relevant data
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		if(!isData)
		{
			// Verifying if it is a relation (in this case, this line is ignored)
			if(util.toLower(lines.at(i)).find("@relation") != string::npos)
				continue;

			// Verifying if it is an attribute (an attribute vector is built)
			pos = util.toLower(lines.at(i)).find("@attribute");
			if(pos != string::npos)
			{
				// Extracting the attribute label
				lines.at(i).erase(pos, pos+10);

				pos1 = lines.at(i).find("{");
				pos2 = lines.at(i).find("}");

				lines.at(i).erase(lines.at(i).begin()+pos1, lines.at(i).begin()+pos2+1);

				bodyMapping.push_back(util.trim(lines.at(i)));
			}

			// Verifying if the data lines has been reached
			if(util.toLower(lines.at(i)).find("@data") != string::npos)
				isData = true;
		}
		else
		{
			// Writing the clausal form of the examples into the corresponding files
			attributes = util.csvStringToNumericalVector(lines.at(i));

			if(attributes.at(attributes.size()-1) == 1)
				currentClause = bodyMapping.at(bodyMapping.size()-1);
			else if(attributes.at(attributes.size()-1) == -1)
			{
				currentClause = "~";
				currentClause.append(bodyMapping.at(bodyMapping.size()-1));
			}
			currentClause = currentClause.append(":-");

			for(unsigned int j = 0; j < (attributes.size()-1); j++)
			{
				if(attributes.at(j) == 1)
				{
					currentClause.append(bodyMapping.at(j));
					currentClause = currentClause.append(",");
				}
				else if(attributes.at(j) == -1)
				{
					currentClause.append("~");
					currentClause.append(bodyMapping.at(j));
					currentClause = currentClause.append(",");
				}
			}
			// Removing the extra ','
			currentClause.erase(currentClause.size()-1, 1);

			// If the current clausal example is positive
			if(currentClause.at(0) != '~')
				fprintf(positiveFile, "%s\n", currentClause.c_str());
			else
				fprintf(negativeFile, "%s\n", currentClause.c_str());
		}
	}

	fclose(positiveFile);
	fclose(negativeFile);
}

int main(int argc, char *argv[])
{
	char exportOrImport = EXPORT_OR_IMPORT;
	string fileName;
	vector<string> clausesTrain, clausesTest, clausesTemp, lines;

	Utilities util;

	switch(exportOrImport)
	{
		case 'e':
			// Generating .arff data
			for(unsigned int i = 0; i < NUMBER_OF_FOLDS; i++)
			{
				// 1. Generating the i-th training fold
				if(!clausesTrain.empty())
					clausesTrain.clear();

				// 1.1. Reading negative training examples
				fileName = "files/trainingnegative";
				fileName = fileName.append(util.convertToString(i+1)).append(EXTENSION);

				if(!clausesTemp.empty())
					clausesTemp.clear();
				clausesTemp = util.readClauses(fileName);
				clausesTrain.insert(clausesTrain.begin(), clausesTemp.begin(), clausesTemp.end());

				// 1.1. Reading positive training examples
				fileName = "files/trainingpositive";
				fileName = fileName.append(util.convertToString(i+1)).append(EXTENSION);

				if(!clausesTemp.empty())
					clausesTemp.clear();
				clausesTemp = util.readClauses(fileName);
				clausesTrain.insert(clausesTrain.begin(), clausesTemp.begin(), clausesTemp.end());

				/////////////////////////////////////////////////////////////////////////////////

				// 2. Generating the i-th testing fold
				if(!clausesTest.empty())
					clausesTest.clear();

				// 2.1. Reading negative testing examples
				fileName = "files/testingnegative";
				fileName = fileName.append(util.convertToString(i+1)).append(EXTENSION);

				if(!clausesTemp.empty())
					clausesTemp.clear();
				clausesTemp = util.readClauses(fileName);
				clausesTest.insert(clausesTest.begin(), clausesTemp.begin(), clausesTemp.end());

				// 2.1. Reading positive training examples
				fileName = "files/testingpositive";
				fileName = fileName.append(util.convertToString(i+1)).append(EXTENSION);

				if(!clausesTemp.empty())
					clausesTemp.clear();
				clausesTemp = util.readClauses(fileName);
				clausesTest.insert(clausesTest.begin(), clausesTemp.begin(), clausesTemp.end());

				/////////////////////////////////////////////////////////////////////////////////

				// 3. Calling the .arff exporter
				fileName = CLASS;
				exportArff(fileName.append(util.convertToString(i+1)), clausesTrain, clausesTest);
			}

			break;
		case 'i':

			for(unsigned int i = 1; i <= NUMBER_OF_FOLDS; i++)
			{
				fileName = "files/";
				fileName = fileName.append(CLASS).append(util.convertToString(i)).append("Training").append(".arff");

				lines = util.readLines(fileName);
				importArff(lines, "training", i);

				fileName = "files/";
				fileName = fileName.append(CLASS).append(util.convertToString(i)).append("Testing").append(".arff");

				lines = util.readLines(fileName);
				importArff(lines, "testing", i);
			}

			break;
		default:
			break;
	}

	return 0;
}

