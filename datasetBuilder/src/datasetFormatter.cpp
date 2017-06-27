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

enum datasetClass {none, debug, promoter, alzheimer, mutagenesis};
enum datasetType {backgroundKnowledge, training, testing};
#define SIZE_OF_DATASET_CLASS 5

#define DATASET alzheimer
#define NUMBER_OF_EXPERIMENTS 1
#define NUMBER_OF_FOLDS 42
#define CLASS "negative"
#define SHUFFLE false

// Method to build the formatted dataset, depending on the data class and type
void prepareDataset(datasetClass dataClass, string backgroundPath, string trainPath, string testPath, int index)
{
	unsigned int pos, position;

	string targetClass = CLASS;
	string sequenceData, connectedBodies, plusSignal, clause, bkOutputPath, trainOutputPath, testOutputPath, extension, emptyString;
	vector<string> backgroundData, trainData, testData, examples, heads, exampleTypes, temp;
	vector<vector<string> > bodies;

	Utilities util;

	FILE *backgroundKnowledgeFile, *trainFile, *testFile;

	bkOutputPath    = "files/backgroundKnowledge";
	trainOutputPath = "files/training";
	testOutputPath  = "files/testing";

	if(index)
	{
		bkOutputPath.append(CLASS);
		trainOutputPath.append(CLASS);
		testOutputPath.append(CLASS);
	}

	extension = ".data";
	emptyString = "";

	backgroundKnowledgeFile = fopen(bkOutputPath.append(index ? util.convertToString(index) : emptyString).append(extension).c_str(), "w");
	trainFile = fopen(trainOutputPath.append(index ? util.convertToString(index) : emptyString).append(extension).c_str(), "w");
	testFile = fopen(testOutputPath.append(index ? util.convertToString(index) : emptyString).append(extension).c_str(), "w");

	switch(dataClass)
	{
		case debug:
		case promoter:
		case mutagenesis:

			backgroundData = util.readClauses(backgroundPath);
			trainData = util.readLines(trainPath);
			testData = util.readLines(testPath);

			break;

		case alzheimer:

			backgroundData = util.readClauses(backgroundPath);
			trainData = util.readClauses(trainPath);
			testData = util.readClauses(testPath);

			break;

		default:

			backgroundData = util.readClauses(backgroundPath);
			trainData = util.readClauses(trainPath);
			testData = util.readClauses(testPath);

			break;
	}

	if(SHUFFLE)
	{
		backgroundData = util.shuffleVector(backgroundData);
		trainData = util.shuffleVector(trainData);
		testData = util.shuffleVector(testData);
	}

	for(unsigned int type = 0; type < SIZE_OF_DATASET_CLASS; type++)
	{
		switch(dataClass)
		{
			// 1. Promoter dataset: it needs a pre-process to add the nucleotide position to the input neuron labels
			case debug:
			case promoter:

				switch(type)
				{
					examples.clear();
					exampleTypes.clear();

					case backgroundKnowledge:
						for(unsigned int j = 0; j < backgroundData.size(); j++)
							fprintf(backgroundKnowledgeFile, "%s.\n", backgroundData.at(j).c_str());
						break;

					case training:
						// Transforming training/testing data into clauses with its targets as head
						// 1.1. Splitting and properly storaging each data line
						for(unsigned int i = 0; i < trainData.size(); i++)
						{
							pos = 0;
							string line = trainData.at(i);
							connectedBodies = "";

							if(line.size() > 0 && (find(examples.begin(), examples.end(), line) == examples.end()))
							{
								examples.push_back(line);

								// 1.1.1. First part of the line: example type ('+' or '-')
								exampleTypes.push_back(line.substr(0, 1));

								// 1.1.2. Second part of the line: irrelevant (just a name to identify the example)
								pos = line.find(",", pos+1);

								// 1.1.3. Third part of the line: example data (sequence of 53 nucleotides)
								pos = line.find(",", pos+1);
								sequenceData = line.substr(pos + 1, line.length() - pos);
								util.trim(sequenceData);

								temp.clear();
								for(unsigned int j = 0; j < sequenceData.length(); j++)
								{
									position = j;
									plusSignal = "";

									if((position-50) >= 0)
									{
										position = j + 1;
										plusSignal = "+";
									}

									if(!j)
										connectedBodies += "p" + plusSignal + util.convertToString(position-50) + "=" + util.convertToString(sequenceData.at(j));
									else
										connectedBodies += ",p" + plusSignal + util.convertToString(position-50) + "=" + util.convertToString(sequenceData.at(j));
								}

								// 1.1.5. Making the head
								if(!exampleTypes.at(i).compare("+"))
									fprintf(trainFile, "promoter:-%s.\n", connectedBodies.c_str());
								else
									fprintf(trainFile, "~promoter:-%s.\n", connectedBodies.c_str());
							}
						}
						break;

					case testing:
						// Transforming training/testing data into clauses with its targets as head
						// 1.1. Splitting and properly storaging each data line
						for(unsigned int i = 0; i < testData.size(); i++)
						{
							pos = 0;
							string line = testData.at(i);
							connectedBodies = "";

							if(line.size() > 0 && (find(examples.begin(), examples.end(), line) == examples.end()))
							{
								examples.push_back(line);

								// 1.1.1. First part of the line: example type ('+' or '-')
								exampleTypes.push_back(line.substr(0, 1));

								// 1.1.2. Second part of the line: irrelevant (just a name to identify the example)
								pos = line.find(",", pos+1);

								// 1.1.3. Third part of the line: example data (sequence of 53 nucleotides)
								pos = line.find(",", pos+1);
								sequenceData = line.substr(pos + 1, line.length() - pos);
								util.trim(sequenceData);

								temp.clear();
								for(unsigned int j = 0; j < sequenceData.length(); j++)
								{
									position = j;
									plusSignal = "";

									if((position-50) >= 0)
									{
										position = j + 1;
										plusSignal = "+";
									}

									if(!j)
										connectedBodies += "p" + plusSignal + util.convertToString(position-50) + "=" + util.convertToString(sequenceData.at(j));
									else
										connectedBodies += ",p" + plusSignal + util.convertToString(position-50) + "=" + util.convertToString(sequenceData.at(j));
								}

								// 1.1.5. Making the head
								if(!exampleTypes.at(i).compare("+"))
									fprintf(testFile, "promoter:-%s.\n", connectedBodies.c_str());
								else
									fprintf(testFile, "~promoter:-%s.\n", connectedBodies.c_str());
							}
						}
						break;

					default:
						break;
				}

				break;

			// 2. Alzeimer dataset: it needs a pre-process to remove the non-clause data of the read lines
			case alzheimer:

				switch(type)
				{
					case backgroundKnowledge:
						// 2.1. Splitting and properly storing each data line
						for(unsigned int i = 0; i < backgroundData.size(); i++)
						{
							pos = backgroundData.at(i).find("[");
							while((backgroundData.at(i).size() > 0) && (pos != string::npos))
							{
								position = backgroundData.at(i).find("]");
								backgroundData.at(i).replace(pos, position - pos + 1, "");

								pos = backgroundData.at(i).find("[");
							}

							pos = backgroundData.at(i).find("),");
							while((backgroundData.at(i).size() > 0) && (pos != string::npos))
							{
								backgroundData.at(i).replace(pos, 2, "#");
								pos = backgroundData.at(i).find("),");
							}

							pos = backgroundData.at(i).find(",");
							while((backgroundData.at(i).size() > 0) && (pos != string::npos))
							{
								backgroundData.at(i).replace(pos, 1, ";");
								pos = backgroundData.at(i).find(",");
							}

							pos = backgroundData.at(i).find("#");
							while((backgroundData.at(i).size() > 0) && (pos != string::npos))
							{
								backgroundData.at(i).replace(pos, 1, "),");
								pos = backgroundData.at(i).find("#");
							}

							if((backgroundData.at(i).size() > 0) && (backgroundData.at(i).find(":-") != string::npos))
							{

								if(!targetClass.compare("positive"))
									fprintf(backgroundKnowledgeFile, "%s.\n", backgroundData.at(i).c_str());
								else
									fprintf(backgroundKnowledgeFile, "~%s.\n", backgroundData.at(i).c_str());
							}
						}
						break;

					case training:
						// 2.2. Splitting and properly storing each data line
						for(unsigned int i = 0; i < trainData.size(); i++)
						{
							pos = trainData.at(i).find("[");
							while((trainData.at(i).size() > 0) && (pos != string::npos))
							{
								position = trainData.at(i).find("]");
								trainData.at(i).replace(pos, position - pos + 1, "");

								pos = trainData.at(i).find("[");
							}

							pos = trainData.at(i).find("),");
							while((trainData.at(i).size() > 0) && (pos != string::npos))
							{
								trainData.at(i).replace(pos, 2, "#");
								pos = trainData.at(i).find("),");
							}

							pos = trainData.at(i).find(",");
							while((trainData.at(i).size() > 0) && (pos != string::npos))
							{
								trainData.at(i).replace(pos, 1, ";");
								pos = trainData.at(i).find(",");
							}

							pos = trainData.at(i).find("#");
							while((trainData.at(i).size() > 0) && (pos != string::npos))
							{
								trainData.at(i).replace(pos, 1, "),");
								pos = trainData.at(i).find("#");
							}

							if((trainData.at(i).size() > 0) && (trainData.at(i).find(":-") != string::npos))
							{
								if(!targetClass.compare("positive"))
									fprintf(trainFile, "%s.\n", trainData.at(i).c_str());
								else
									fprintf(trainFile, "~%s.\n", trainData.at(i).c_str());
							}
						}
						break;

					case testing:
						// 2.3. Splitting and properly storing each data line
						for(unsigned int i = 0; i < testData.size(); i++)
						{
							pos = testData.at(i).find("[");
							while((testData.at(i).size() > 0) && (pos != string::npos))
							{
								position = testData.at(i).find("]");
								testData.at(i).replace(pos, position - pos + 1, "");

								pos = testData.at(i).find("[");
							}

							pos = testData.at(i).find("),");
							while((testData.at(i).size() > 0) && (pos != string::npos))
							{
								testData.at(i).replace(pos, 2, "#");
								pos = testData.at(i).find("),");
							}

							pos = testData.at(i).find(",");
							while((testData.at(i).size() > 0) && (pos != string::npos))
							{
								testData.at(i).replace(pos, 1, ";");
								pos = testData.at(i).find(",");
							}

							pos = testData.at(i).find("#");
							while((testData.at(i).size() > 0) && (pos != string::npos))
							{
								testData.at(i).replace(pos, 1, "),");
								pos = testData.at(i).find("#");
							}

							if((testData.at(i).size() > 0) && (testData.at(i).find(":-") != string::npos))
							{
								if(!targetClass.compare("positive"))
									fprintf(testFile, "%s.\n", testData.at(i).c_str());
								else
									fprintf(testFile, "~%s.\n", testData.at(i).c_str());
							}
						}
						break;

					default:
						break;
				}

				break;

				// 3. Mutagenesis dataset: uses Weka format on training and testing data, any future 'Weka-like' dataset can use this case
				case mutagenesis:

					switch(type)
					{
						case backgroundKnowledge:
							// 3.1. Splitting and properly storing each data line
							for(unsigned int i = 0; i < backgroundData.size(); i++)
							{
								pos = backgroundData.at(i).find("[");
								while((backgroundData.at(i).size() > 0) && (pos != string::npos))
								{
									position = backgroundData.at(i).find("]");
									backgroundData.at(i).replace(pos, position - pos + 1, "");

									pos = backgroundData.at(i).find("[");
								}

								pos = backgroundData.at(i).find("),");
								while((backgroundData.at(i).size() > 0) && (pos != string::npos))
								{
									backgroundData.at(i).replace(pos, 2, "#");
									pos = backgroundData.at(i).find("),");
								}

								pos = backgroundData.at(i).find(",");
								while((backgroundData.at(i).size() > 0) && (pos != string::npos))
								{
									backgroundData.at(i).replace(pos, 1, ";");
									pos = backgroundData.at(i).find(",");
								}

								pos = backgroundData.at(i).find("#");
								while((backgroundData.at(i).size() > 0) && (pos != string::npos))
								{
									backgroundData.at(i).replace(pos, 1, "),");
									pos = backgroundData.at(i).find("#");
								}

								if((backgroundData.at(i).size() > 0) && (backgroundData.at(i).find(":-") != string::npos))
									fprintf(backgroundKnowledgeFile, "%s.\n", backgroundData.at(i).c_str());
							}
							break;

						case training:
							// 3.2. Splitting and properly storing each data line
							for(unsigned int i = 0; i < trainData.size(); i++)
							{
								pos = trainData.at(i).find("pos");
								if(pos != string::npos)
									clause = "active:-";
								else
									clause = "~active:-";

								pos = trainData.at(i).find("+");
								position = trainData.at(i).find("-");
								for(unsigned int j = 0; (pos != string::npos) || (position != string::npos); j++)
								{
									if(j)
										clause.append(",");

									if(trainData.at(i).at(j) == '+')
									{
										clause.append("f(");
										clause.append(util.convertToString(j).c_str());
										clause.append(")");
									}
									else
									{
										clause.append("~f(");
										clause.append(util.convertToString(j).c_str());
										clause.append(")");
									}

									trainData.at(i).replace(j, 1, "");

									pos = trainData.at(i).find("+");
									position = trainData.at(i).find("-");
								}

								fprintf(trainFile, "%s.\n", clause.c_str());
							}

							break;

						case testing:
							// 3.2. Splitting and properly storing each data line
							for(unsigned int i = 0; i < testData.size(); i++)
							{
								pos = testData.at(i).find("pos");
								if(pos != string::npos)
									clause = "active:-";
								else
									clause = "~active:-";

								pos = testData.at(i).find("+");
								position = testData.at(i).find("-");
								for(unsigned int j = 0; (pos != string::npos) || (position != string::npos); j++)
								{
									if(j)
										clause.append(",");

									if(trainData.at(i).at(j) == '+')
									{
										clause.append("f(");
										clause.append(util.convertToString(j).c_str());
										clause.append(")");
									}
									else
									{
										clause.append("~f(");
										clause.append(util.convertToString(j).c_str());
										clause.append(")");
									}

									testData.at(i).replace(j, 1, "");

									pos = testData.at(i).find("+");
									position = testData.at(i).find("-");
								}

								fprintf(testFile, "%s.\n", clause.c_str());
							}

							break;

						default:
							break;
					}

					break;

			case none:

				switch(type)
				{
					examples.clear();
					exampleTypes.clear();

					case backgroundKnowledge:
						for(unsigned int j = 0; j < backgroundData.size(); j++)
							fprintf(backgroundKnowledgeFile, "%s.\n", backgroundData.at(j).c_str());
						break;

					case training:
						for(unsigned int j = 0; j < trainData.size(); j++)
							fprintf(trainFile, "%s.\n", trainData.at(j).c_str());
						break;

					case testing:
						for(unsigned int j = 0; j < testData.size(); j++)
							fprintf(testFile, "%s.\n", testData.at(j).c_str());
						break;

					default:
						break;
				}

				break;

			default:
				break;
		}
	}
}

int main (int argc, char *argv[])
{
	 //If the test file path is empty, the trainingData will be split by K-Fold Cross Validation
	string backgroundKnowledgeFilePath = "";
	string trainingDataFilePath = "";
	string testingDataFilePath = "";
	string incrementalFold;

	Utilities util;

	// Checking if the dataset is incremental
	if(NUMBER_OF_EXPERIMENTS <= 1)
	{
		// Checking if there is only one data file or if they are split into folds
		if(!NUMBER_OF_FOLDS)
		{
			switch(DATASET)
			{
				case debug:
					backgroundKnowledgeFilePath = "files/promoters.theory";
					trainingDataFilePath = "files/promoters2.data";

					break;

				case promoter:
					backgroundKnowledgeFilePath = "files/promoters.theory";
					trainingDataFilePath = "files/promoters.data";

					break;

				case alzheimer:
					backgroundKnowledgeFilePath = "files/bottomsReduce_bk.txt";
					trainingDataFilePath = "files/bottomsReduce_train.txt";

					break;

				case mutagenesis:
					trainingDataFilePath = "files/muta.data";

					break;

				case none:
				default:
					break;
			}

			prepareDataset(DATASET, backgroundKnowledgeFilePath, trainingDataFilePath, testingDataFilePath, 0);
		}
		else
		{
			for(int i = 1; i <= NUMBER_OF_FOLDS; i++)
			{

				trainingDataFilePath = "files/folds/inputFolds/bottoms" + util.convertToString(i) + "train" + CLASS + ".txt";
				testingDataFilePath =  "files/folds/inputFolds/bottoms" + util.convertToString(i) + "test" + CLASS + ".txt";

				prepareDataset(DATASET, backgroundKnowledgeFilePath, trainingDataFilePath, testingDataFilePath, i);
			}
		}
	}
	else
	{
		for(int i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
		{
			incrementalFold = "";
			incrementalFold.append("files/incremental").append(util.convertToString(i+1).append("/"));

			// Checking if there is only one data file or if they are splitted into folds
			if(!NUMBER_OF_FOLDS)
			{
				switch(DATASET)
				{
					case debug:
						backgroundKnowledgeFilePath = "files/promoters.theory";
						trainingDataFilePath = "files/promoters2.data";

						break;

					case promoter:
						backgroundKnowledgeFilePath = "files/promoters.theory";
						trainingDataFilePath = "files/promoters.data";

						break;

					case alzheimer:
						backgroundKnowledgeFilePath = "files/bottomsReduce_bk.txt";
						trainingDataFilePath = "files/bottomsReduce_train.txt";

						break;

					case mutagenesis:
						trainingDataFilePath = "files/muta.data";

						break;

					case none:
					default:
						break;
				}

				prepareDataset(DATASET, backgroundKnowledgeFilePath, trainingDataFilePath, testingDataFilePath, 0);
			}
			else
			{
				for(int i = 1; i <= NUMBER_OF_FOLDS; i++)
				{

					trainingDataFilePath = "files/folds/inputFolds/bottoms" + util.convertToString(i) + "train" + CLASS + ".txt";
					testingDataFilePath =  "files/folds/inputFolds/bottoms" + util.convertToString(i) + "test" + CLASS + ".txt";

					prepareDataset(DATASET, backgroundKnowledgeFilePath, trainingDataFilePath, testingDataFilePath, i);
				}
			}
		}
	}

	return 0;
}
