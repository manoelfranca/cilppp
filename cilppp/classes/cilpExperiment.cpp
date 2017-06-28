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

#include "cilpExperiment.h"

CilpExperiment::CilpExperiment(string backgroundPath, string trainingPath, string testingPath)
{
	parametersFile = NULL;

	// Initializing all timings
	setupTotalTime = 0.0;
	buildTotalTime = 0.0;
	trainTotalTime = 0.0;
	testTotalTime  = 0.0;

	// Reading all data
	time(&setupStart);
	{
		if(!util.greater(paramExamplesAsBKProportion, 0.0))
			backgroundData = util.readClauses(backgroundPath);
		trainData = util.readClauses(trainingPath);
		testData = util.readClauses(testingPath);
	}
	time(&setupEnd);
	setupTotalTime += difftime(setupEnd, setupStart);

	srand (time(NULL));
}

CilpExperiment::~CilpExperiment()
{}

// Method to set all general system parameters, according to 'parameters.txt':
void CilpExperiment::setVariables()
{
	unsigned int pos, index = 0;
	string tempStr;
	vector<string> parameterLines = util.readParameters("files/parameters.txt");

	// STOCHASTIC
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramStochastic, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: STOCHASTIC. Using system defaults...\n");
	index++;

	// BETA
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramBeta, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: BETA. Using system defaults...\n");
	index++;

	// MINIMAL_INCREMENT
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramMinimalIncrement, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: MINIMAL_INCREMENT. Using system defaults...\n");
	index++;

	// AMIN_THRESHOLD
	pos = parameterLines[index].find("=");
	if(!parameterLines[index].substr(pos+1).compare("max"))
		paramAminThreshold = FLT_MAX;
	else
	{
		if(!util.convertFromString<double>(paramAminThreshold, parameterLines[index].substr(pos+1), std::dec))
			printf("<Error> Invalid parameter: AMIN_THRESHOLD. Using system defaults...\n");
	}
	index++;

	// UNKNOWN_INPUTS_AS_FALSE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramUnknownInputsAsFalse, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("Invalid parameter: UNKNOWN_INPUTS_AS_FALSE. Using system defaults...\n");
	index++;

	// EXAMPLES_AS_BK_PROPORTION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramExamplesAsBKProportion, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: EXAMPLES_AS_BK_PROPORTION. Using system defaults...\n");
	index++;

	// USES_RECURSION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramUsesRecursion, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: USES_RECURSION. Using system defaults...\n");
	index++;

	// COMPLETE_INTERMEDIATE_CONCEPTS
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramCompleteIntermediateConcepts, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: COMPLETE_INTERMEDIATE_CONCEPTS. Using system defaults...\n");
	index++;

	// REPLICATE_BK
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramReplicateBk, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: REPLICATE_BK. Using system defaults...\n");
	index++;

	// MAX_REPLICATION_RECURSION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramMaxReplicationRecursion, parameterLines[index].substr(pos+1), std::dec))
		printf("Invalid parameter: MAX_REPLICATION_RECURSION. Using system defaults...\n");
	index++;

	// NEW_BCP
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramNewBcp, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: NEW_BCP. Using system defaults...\n");
	index++;

	// TRAINING_TYPE
	pos = parameterLines[index].find("=");
	int tempTrainingType;
	if(!util.convertFromString<int>(tempTrainingType, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: TRAINING_TYPE. Using system defaults...\n");
	else
		paramTrainingType = (stoppingCriteria)tempTrainingType;
	index++;

	// STRIP_SIZE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramStripSize, parameterLines[index].substr(pos+1), std::dec))
		printf("Invalid parameter: STRIP_SIZE. Using system defaults...\n");
	index++;

	// DEFAULT_LEARNING_RATE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramDefaultLearningRate, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: DEFAULT_LEARNING_RATE. Using system defaults...\n");
	index++;

	// DECAY_FACTOR
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramDecayFactor, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: DECAY_FACTOR. Using system defaults...\n");
	index++;

	// MOMENTUM
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramMomentum, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: MOMENTUM. Using system defaults...\n");
	index++;

	// BATCH_SIZE
	pos = parameterLines[index].find("=");
	if(!parameterLines[index].substr(pos+1).compare("max"))
		paramBatchSize = MAX_SIZE;
	else
	{
		if(!util.convertFromString<int>(paramBatchSize, parameterLines[index].substr(pos+1), std::dec))
			printf("<Error> Invalid parameter: BATCH_SIZE. Using system defaults...\n");
	}
	index++;

	// MRMR_FILTERING
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramMrmrFiltering, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: MRMR_FILTERING. Using system defaults...\n");
	index++;

	// FILTERING_PROPORTION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramFilteringProportion, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: FILTERING_PROPORTION. Using system defaults...\n");
	index++;

	// STARTING_HIDDEN_NEURONS
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramStartingHiddenNeurons, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: STARTING_HIDDEN_NEURONS. Using system defaults...\n");
	index++;

	// TRAIN_PORTION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramTrainPortion, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: TRAIN_PORTION. Using system defaults...\n");
	index++;

	// TEST_PORTION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramTestPortion, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: TEST_PORTION. Using system defaults...\n");
	index++;

	// STABLIZATION_MAX_ITERATIONS
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramStablizationMaxIterations, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: STABILIZATION_MAX_ITERATIONS. Using system defaults...\n");
	index++;

	// SIMPLE_STOP_CRITERION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramSimpleStopCriterion, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: SIMPLE_STOP_CRITERION. Using system defaults...\n");
	index++;

	// EPSILON
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramEpsilon, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: EPSILON. Using system defaults...\n");
	index++;

	// DELTA
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramDelta, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: DELTA. Using system defaults...\n");
	index++;

	// EPOCH_RUNS
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramEpochRuns, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: EPOCH_RUNS. Using system defaults...\n");
	index++;

	// CORRECTNESS_RANGE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramCorrectnessRange, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: CORRECTNESS_RANGE. Using system defaults...\n");
	index++;

	// CORRECTNESS_PROPORTION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramCorrectnessProportion, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: CORRECTNESS_PROPORTION. Using system defaults...\n");
	index++;

	// HIT_MARGIN
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramHitMargin, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: HIT_MARGIN. Using system defaults...\n");
	index++;

	// STABILIZATION_PROPORTION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramStabilizationProportion, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: STABILIZATION_PROPORTION. Using system defaults...\n");
	index++;

	// STABILIZATION_MAX_EPOCHS
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramStabilizationMaxEpochs, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: STABILIZATION_MAX_EPOCHS. Using system defaults...\n");
	index++;

	// WEIGHT_INITIAL_RANGE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramWeightInitialRange, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: WEIGHT_INITIAL_RANGE. Using system defaults...\n");
	index++;

	// NUMBER_OF_VALIDATION_FOLDS
	pos = parameterLines[index].find("=");
	if(!parameterLines[index].substr(pos+1).compare("max"))
		paramNumberOfValidationFolds = MAX_SIZE;
	else
	{
		if(!util.convertFromString<int>(paramNumberOfValidationFolds, parameterLines[index].substr(pos+1), std::dec))
			printf("<Error> Invalid parameter: NUMBER_OF_VALIDATION_FOLDS. Using system defaults...\n");
	}
	index++;

	// NUMBER_OF_TEST_FOLDS
	pos = parameterLines[index].find("=");
	if(!parameterLines[index].substr(pos+1).compare("max"))
		paramNumberOfTestFolds = MAX_SIZE;
	else
	{
		if(!util.convertFromString<int>(paramNumberOfTestFolds, parameterLines[index].substr(pos+1), std::dec))
			printf("<Error> Invalid parameter: NUMBER_OF_TEST_FOLDS. Using system defaults...\n");
	}
	index++;

	// PRE_SPLITTED_FOLDS
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramPreSplittedFolds, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("Invalid parameter: PRE_SPLITTED_FOLDS. Using system defaults...\n");
	index++;

	// INPUT_PRUNING_RATE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<double>(paramInputPruningVariation, parameterLines[index].substr(pos+1), std::dec))
		printf("Invalid parameter: INPUT_PRUNING_VARIATION. Using system defaults...\n");
	index++;

	// PRUNING_RATE_FACTOR
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramPruningRateFactor, parameterLines[index].substr(pos+1), std::dec))
		printf("Invalid parameter: PRUNING_RATE_FACTOR. Using system defaults...\n");
	index++;

	// HIDDEN_NEURONS_VARIATION
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramHiddenNeuronsVariation, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: HIDDEN_NEURONS_VARIATION. Using system defaults...\n");
	index++;

	// HIDDEN_NEURONS_FACTOR
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<int>(paramHiddenNeuronsFactor, parameterLines[index].substr(pos+1), std::dec))
		printf("<Error> Invalid parameter: HIDDEN_NEURONS_FACTOR. Using system defaults...\n");
	index++;

	// PRINT_TRAIN_ERROR
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramPrintTrainError, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: PRINT_TRAIN_ERROR. Using system defaults...\n");
	index++;

	// PRINT_TEST_ERROR
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramPrintTestError, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: PRINT_TEST_ERROR. Using system defaults...\n");
	index++;

	// DEBUG_MODE
	pos = parameterLines[index].find("=");
	if(!util.convertFromString<bool>(paramDebugMode, parameterLines[index].substr(pos+1), std::boolalpha))
		printf("<Error> Invalid parameter: DEBUG_MODE. Using system defaults...\n");
	index++;
}

void CilpExperiment::basicExperiment()
{
	unsigned int selected;

	Cilp *baseSystem = new Cilp(paramStochastic);
	Cilp copy = *baseSystem;
	FILE *logFile, *foldAccuracyFile;

	vector<double> mean, meanFMeasure;
	vector<string> artificialBK, bodiesTemp;
	vector<vector<string> > bodiesTrain, bodiesTest, headsTrain, headsTest;

	vector<pair<pair<double, double>, confusionTable> > testResults;
	pair<vector<vector<string> >, vector<vector<string> > > dataReadTrain, dataReadTest;
	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > datasets;

	// mRMR
	vector<int> selectedFeatures;
	int totalFeatures;
	vector<string> temp, featureTable;
	string bodyWithoutNegation;

	// 1. Cleaning all previous logs
	util.clearAllFiles("files/", ".log");
	util.clearAllFiles("files/trainError/", ".log");
	util.clearAllFiles("files/validationError/", ".log");

	foldAccuracyFile = fopen("files/testFoldAccuracy.log", "a");

	printf("<General> Using the background knowledge file to build an initial network...\n");

	// 2. Building the net
	time(&buildStart);
	{
		baseSystem->buildNet(backgroundData, true);
	}
	time(&buildEnd);
	buildTotalTime += difftime(buildEnd, buildStart);

	printf("<General> Finished building the initial network...\n\n");

	printf("<General> Reading training/testing data and splitting it into folds...\n");

	// 3. Reading the data file and splitting it into bodies and heads
	// Example format: T :- L1,L2,L3,...,Ln.
	time(&setupStart);
	{
		dataReadTrain = baseSystem->decodeData(trainData);
		bodiesTrain = dataReadTrain.first;
		headsTrain = dataReadTrain.second;

		// Filtering the inputs using mRMR
		if(paramMrmrFiltering && !paramPreSplittedFolds)
		{
			temp = util.clausesToFeatures(trainData);
			totalFeatures = count(temp.at(0).begin(), temp.at(0).end(), ',');
			selectedFeatures = mrmr(temp, totalFeatures);
			featureTable = util.csvStringToStringVector(temp.at(0));

			// Filtering body data
			for(unsigned int i = 0; i < bodiesTrain.size(); i++)
			{
				bodiesTemp.clear();

				for(unsigned int j = 0; j < bodiesTrain.at(i).size(); j++)
				{
					bodyWithoutNegation = bodiesTrain.at(i).at(j);
					if(bodiesTrain.at(i).at(j).at(0) == '~')
						bodyWithoutNegation = bodyWithoutNegation.substr(1);

					if(util.find(selectedFeatures, util.find(featureTable, bodyWithoutNegation)) >= 0)
						bodiesTemp.push_back(bodiesTrain.at(i).at(j));
				}

				bodiesTrain.at(i).clear();
				bodiesTrain.at(i) = bodiesTemp;
			}
		}

		if(testData.empty())
		{
			if(paramPreSplittedFolds)
				datasets = baseSystem->buildDatasetFromFolds();
			else
				datasets = baseSystem->splitDataByTarget(trainData, bodiesTrain, headsTrain, paramNumberOfTestFolds, paramTestPortion);
		}
		else
		{
			dataReadTest = baseSystem->decodeData(testData);

			bodiesTest = dataReadTest.first;
			headsTest = dataReadTest.second;

			datasets.push_back(make_pair(make_pair(trainData, make_pair(bodiesTrain, headsTrain)), make_pair(testData, make_pair(bodiesTest, headsTest))));
		}
	}
	time(&setupEnd);
	setupTotalTime += difftime(setupEnd, setupStart);

	printf("<General> Finished preparing training/testing data...\n");

	// Printing dataset info
	baseSystem->printDatasetInfo(datasets, true, "Main dataset");

	// 4. For each built train/test an experiment is done and the total error is collected
	printf("<General> Starting %d-fold cross validation on test...\n\n", (int)datasets.size());

	vector<Cilp> *systems = new vector<Cilp>(datasets.size(), *baseSystem);
	mean.resize(STABILIZATION_TYPE_SIZE, 0.0);
	meanFMeasure.resize(STABILIZATION_TYPE_SIZE, 0.0);

	//#pragma omp parallel for private(experimentsError)
	for(unsigned int i = 0; i < datasets.size(); i++)
	{
		if(paramMrmrFiltering && paramPreSplittedFolds)
		{
			temp = util.clausesToFeatures(datasets.at(i).first.first);
			totalFeatures = count(temp.at(0).begin(), temp.at(0).end(), ',');
			selectedFeatures = mrmr(temp, totalFeatures);
			featureTable = util.csvStringToStringVector(temp.at(0));

			// Filtering body data
			for(unsigned int i = 0; i < bodiesTrain.size(); i++)
			{
				bodiesTemp.clear();

				for(unsigned int j = 0; j < bodiesTrain.at(i).size(); j++)
				{
					bodyWithoutNegation = bodiesTrain.at(i).at(j);
					if(bodiesTrain.at(i).at(j).at(0) == '~')
						bodyWithoutNegation = bodyWithoutNegation.substr(1);

					if(util.find(selectedFeatures, util.find(featureTable, bodyWithoutNegation)) >= 0)
						bodiesTemp.push_back(bodiesTrain.at(i).at(j));
				}

				bodiesTrain.at(i).clear();
				bodiesTrain.at(i) = bodiesTemp;
			}
		}

		// 4.1. If there is no background knowledge, paramExamplesAsBKProportion is different from zero and BCP 2.0 is being used, an artificial BK will be made with training data
		if((systems->at(i)).noBackgroundKnowledge && !util.equals(paramExamplesAsBKProportion, 0.0) && !paramNewBcp)
		{
			printf("<General> No BK found: creating artificial BK for fold #%d...\n", i+1);

			artificialBK.clear();

			(systems->at(i)) = copy;
			(systems->at(i)).shuffleTrainData(datasets.at(i).first.first, datasets.at(i).first.second.first, datasets.at(i).first.second.second);

			selected = 0;

			time(&setupStart);
			{
				for(unsigned int index = 0; (selected < (unsigned int)floor(datasets.at(i).first.first.size() * paramExamplesAsBKProportion)) && (index < datasets.at(i).first.first.size()); index++)
				{
					if(datasets.at(i).first.first.at(index).at(0) != '~')
					{
						selected++;
						artificialBK.push_back(datasets.at(i).first.first.at(index));
					}
				}
			}
			time(&setupEnd);
			setupTotalTime += difftime(setupEnd, setupStart);

			time(&buildStart);
			{
				(systems->at(i)).buildNet(artificialBK, false);
			}
			time(&buildEnd);
			buildTotalTime += difftime(buildEnd, buildStart);

			// Commented - now the BK examples are reinforced during training
//			time(&setupStart);
//			{
//				sort(indexes.begin(), indexes.end());
//				for(int j = indexes.size()-1; j >= 0; j--)
//				{
//					datasets.at(i).first.first.erase(datasets.at(i).first.first.begin() + indexes.at(j));
//					datasets.at(i).first.second.first.erase(datasets.at(i).first.second.first.begin() + indexes.at(j));
//					datasets.at(i).first.second.second.erase(datasets.at(i).first.second.second.begin() + indexes.at(j));
//				}
//			}
//			time(&setupEnd);
//			setupTotalTime += difftime(setupEnd, setupStart);

			printf("<General> Finished creating artificial BK for fold #%d...\n\n", i+1);
		}

		printf("<General> Training fold #%d...\n", i+1);

		// 4.2. Training each system copy
		(systems->at(i)).trainNet(datasets.at(i).first.first, true);

		printf("<General> Finished training fold #%d\n\n", i+1);

		//(systems->at(i)).printNetWeights(true);

		printf("<General> Testing fold #%d...\n", i+1);

		time(&testStart);
		{
			testResults = (systems->at(i)).testNet(datasets.at(i).second.first, true);
			//experimentsError.resize(4, 0);
		}
		time(&testEnd);
		testTotalTime += difftime(testEnd, testStart);

		if(paramStochastic)
			fprintf(foldAccuracyFile, "%.2f %.2f %.2f %.2f\n", testResults.at(regularSimple).first.first,
															   testResults.at(regularStable).first.first,
															   testResults.at(stochasticSimple).first.first,
															   testResults.at(stochasticStable).first.first);
		else
			fprintf(foldAccuracyFile, "%.2f %.2f\n", testResults.at(regularSimple).first.first,
													 testResults.at(regularStable).first.first);

		mean.at(regularSimple) += testResults.at(regularSimple).first.first;
		mean.at(regularStable) += testResults.at(regularStable).first.first;
		
		meanFMeasure.at(regularSimple) += testResults.at(regularSimple).first.second;
		meanFMeasure.at(regularStable) += testResults.at(regularStable).first.second;

		if(paramStochastic)
		{
			mean.at(stochasticSimple) += testResults.at(stochasticSimple).first.first;
			mean.at(stochasticStable) += testResults.at(stochasticStable).first.first;

			meanFMeasure.at(stochasticSimple) += testResults.at(stochasticSimple).first.second;
			meanFMeasure.at(stochasticStable) += testResults.at(stochasticStable).first.second;
		}

		printf("\n<General> Finished testing fold #%d\n\n", i+1);
	}

	printf("<General> Finished %d-fold cross validation on test...\n\n", (int)datasets.size());

	// 5. Printing final statistics

	logFile = fopen("files/main.log", "a+");

	fprintf(logFile, "\nFinal Results:\n\n");
	printf("\nFinal Results:\n\n");

	fprintf(logFile, "Elapsed Time:\n");
	fprintf(logFile, "Set-up: %s\n", util.returnFormattedTime(setupTotalTime).c_str());
	fprintf(logFile, "Building: %s\n", util.returnFormattedTime(buildTotalTime).c_str());
	fprintf(logFile, "Training: %s\n", util.returnFormattedTime(trainTotalTime).c_str());
	fprintf(logFile, "Testing: %s\n", util.returnFormattedTime(testTotalTime).c_str());
	fprintf(logFile, "Total: %s\n", util.returnFormattedTime(setupTotalTime + buildTotalTime + trainTotalTime + testTotalTime).c_str());

	printf("Elapsed Time:\n");
	printf("Set-up: %s\n", util.returnFormattedTime(setupTotalTime).c_str());
	printf("Building: %s\n", util.returnFormattedTime(buildTotalTime).c_str());
	printf("Training: %s\n", util.returnFormattedTime(trainTotalTime).c_str());
	printf("Testing: %s\n", util.returnFormattedTime(testTotalTime).c_str());
	printf("Total: %s\n\n", util.returnFormattedTime(setupTotalTime + buildTotalTime + trainTotalTime + testTotalTime).c_str());

	fprintf(logFile, "\nAverage accuracy obtained for Regular Simple Evaluation: %0.2f%%\n", (double)(mean.at(regularSimple)/datasets.size()));
	fprintf(logFile, "Average accuracy obtained for Regular Stable Evaluation: %0.2f%%\n", (double)(mean.at(regularStable)/datasets.size()));
	printf("Average accuracy obtained for Regular Simple Evaluation: %0.2f%%\n", (double)(mean.at(regularSimple)/datasets.size()));
	printf("Average accuracy obtained for Regular Stable Evaluation: %0.2f%%\n", (double)(mean.at(regularStable)/datasets.size()));
	if(paramStochastic)
	{
		fprintf(logFile, "Average accuracy obtained for Stochastic Simple Evaluation: %0.2f%%\n", (double)(mean.at(stochasticSimple)/datasets.size()));
		fprintf(logFile, "Average accuracy obtained for Stochastic Stable Evaluation: %0.2f%%\n", (double)(mean.at(stochasticStable)/datasets.size()));
		printf("Average accuracy obtained for Stochastic Simple Evaluation: %0.2f%%\n", (double)(mean.at(stochasticSimple)/datasets.size()));
		printf("Average accuracy obtained for Stochastic Stable Evaluation: %0.2f%%\n\n", (double)(mean.at(stochasticStable)/datasets.size()));
	}

	fprintf(logFile, "\nAverage f-measure obtained for Regular Simple Evaluation: %f\n", (double)(meanFMeasure.at(regularSimple)/datasets.size()));
	fprintf(logFile, "Average f-measure obtained for Regular Stable Evaluation: %f\n", (double)(meanFMeasure.at(regularStable)/datasets.size()));
	printf("Average f-measure obtained for Regular Simple Evaluation: %f\n", (double)(meanFMeasure.at(regularSimple)/datasets.size()));
	printf("Average f-measure obtained for Regular Stable Evaluation: %f\n", (double)(meanFMeasure.at(regularStable)/datasets.size()));
	if(paramStochastic)
	{
		fprintf(logFile, "Average f-measure obtained for Stochastic Simple Evaluation: %f\n", (double)(meanFMeasure.at(stochasticSimple)/datasets.size()));
		fprintf(logFile, "Average f-measure obtained for Stochastic Stable Evaluation: %f\n", (double)(meanFMeasure.at(stochasticStable)/datasets.size()));
		printf("Average f-measure obtained for Stochastic Simple Evaluation: %f\n", (double)(meanFMeasure.at(stochasticSimple)/datasets.size()));
		printf("Average f-measure obtained for Stochastic Stable Evaluation: %f\n\n", (double)(meanFMeasure.at(stochasticStable)/datasets.size()));
	}

	fclose(logFile);
	fclose(foldAccuracyFile);

	delete(baseSystem);
	delete(systems);
}

void CilpExperiment::incrementalExperiment(int numberOfExperiments)
{
	int index;
	string datasetLabel;
	Cilp *baseSystem = new Cilp(paramStochastic);
	Cilp copy = *baseSystem;
	FILE *logFile, *foldAccuracyFile;

	vector<int> indexes;
	vector<double> mean;
	vector<string> headsTrain, headsTest, artificialBK;
	vector<vector<string> > bodiesTrain, bodiesTest;

	pair<vector<vector<string> >, vector<string> > dataReadTrain, dataReadTest;
	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > datasets;
	vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > incrementalDatasets;

	// 1. Cleaning all previous logs
	util.clearAllFiles("files/", ".log");

	foldAccuracyFile = fopen("files/testFoldAccuracy.log", "a");

	// 2. Reading the data file and splitting it into bodies and heads
	// Example format: T :- L1,L2,L3,...,Ln.
	time(&setupStart);
	{
		// TODO: if implementing incremental experiments, the commented line below needs to be adapted!
//		dataReadTrain = baseSystem->decodeData(trainData);
		bodiesTrain = dataReadTrain.first;
		headsTrain = dataReadTrain.second;
		if(testData.empty())
		{
			if(paramPreSplittedFolds)
				incrementalDatasets = baseSystem->buildDatasetFromIncrementalFolds(numberOfExperiments);
			else
				incrementalDatasets = baseSystem->splitDataIncrementallyByTarget(numberOfExperiments, trainData, bodiesTrain, headsTrain, paramNumberOfTestFolds);
		}
		else
		{
			// TODO: if implementing incremental experiments, the commented line below needs to be adapted!
//			dataReadTest = baseSystem->decodeData(testData);
			bodiesTest = dataReadTest.first;
			headsTest = dataReadTest.second;

			datasets.push_back(make_pair(make_pair(trainData, make_pair(bodiesTrain, headsTrain)), make_pair(testData, make_pair(bodiesTest, headsTest))));
			incrementalDatasets.push_back(datasets);

			numberOfExperiments = 1;
		}
	}
	time(&setupEnd);
	setupTotalTime += difftime(setupEnd, setupStart);

	for(int h = 0; h < numberOfExperiments; h++)
	{
		// 3. Building the net
		time(&buildStart);
		{
			baseSystem->buildNet(backgroundData, true);

			// Printing dataset info
			datasetLabel = "";
			datasetLabel.append("Dataset ").append(util.convertToString(h));
			// TODO: if implementing incremental experiments, the commented line below needs to be adapted!
//			baseSystem->printDatasetInfo(incrementalDatasets.at(h), true, datasetLabel);
		}
		time(&buildEnd);
		buildTotalTime += difftime(buildEnd, buildStart);

		// 4. For each built train/test an experiment is done and the total error is collected
		vector<Cilp> *systems = new vector<Cilp>(incrementalDatasets.at(h).size(), *baseSystem);
		mean.resize(STABILIZATION_TYPE_SIZE, 0.0);
		for(unsigned int i = 0; i < incrementalDatasets.at(h).size(); i++)
		{
			vector<int> experimentsError;

			// 4.1. If there is no background knowledge and paramExamplesAsBKProportion different from zero, an artificial BK will be made with training data
			if((systems->at(i)).noBackgroundKnowledge && paramExamplesAsBKProportion)
			{
				(systems->at(i)) = copy;

				time(&setupStart);
				{
					while(indexes.size() < (unsigned int)floor(incrementalDatasets.at(h).at(i).first.first.size() * paramExamplesAsBKProportion))
					{
						index = (int)(floor(((double)rand()/RAND_MAX) * incrementalDatasets.at(h).at(i).first.first.size()));

						if((util.find(indexes, index) < 0) && (incrementalDatasets.at(h).at(i).first.first.at(index).at(0) != '~'))
						{
							indexes.push_back(index);
							artificialBK.push_back(incrementalDatasets.at(h).at(i).first.first.at(index));
						}
					}
				}
				time(&setupEnd);
				setupTotalTime += difftime(setupEnd, setupStart);

				time(&buildStart);
				{
					(systems->at(i)).buildNet(artificialBK, false);
				}
				time(&buildEnd);
				buildTotalTime += difftime(buildEnd, buildStart);

				time(&setupStart);
				{
					sort(indexes.begin(), indexes.end());
					for(int j = indexes.size()-1; j >= 0; j--)
					{
						datasets.at(i).first.first.erase(incrementalDatasets.at(h).at(i).first.first.begin() + indexes.at(j));
						datasets.at(i).first.second.first.erase(incrementalDatasets.at(h).at(i).first.second.first.begin() + indexes.at(j));
						datasets.at(i).first.second.second.erase(incrementalDatasets.at(h).at(i).first.second.second.begin() + indexes.at(j));
					}
				}
				time(&setupEnd);
				setupTotalTime += difftime(setupEnd, setupStart);

				artificialBK.clear();
				indexes.clear();
			}

			// 4.2. Training each system copy
			(systems->at(i)).trainNet(incrementalDatasets.at(h).at(i).first.first, true);

			// 4.3. Testing each system copy
			time(&testStart);
			{
				//experimentsError = (systems->at(i)).testNet(incrementalDatasets.at(h).at(i).second.first, true);
			}
			time(&testEnd);
			testTotalTime += difftime(testEnd, testStart);

			if(paramStochastic)
				fprintf(foldAccuracyFile, "%.2f %.2f %.2f %.2f\n", ((double)experimentsError.at(regularSimple)/incrementalDatasets.at(h).at(i).second.first.size())*100,
																   ((double)experimentsError.at(regularStable)/incrementalDatasets.at(h).at(i).second.first.size())*100,
																   ((double)experimentsError.at(stochasticSimple)/incrementalDatasets.at(h).at(i).second.first.size())*100,
																   ((double)experimentsError.at(stochasticStable)/incrementalDatasets.at(h).at(i).second.first.size())*100);
			else
				fprintf(foldAccuracyFile, "%.2f %.2f\n", ((double)experimentsError.at(regularSimple)/incrementalDatasets.at(h).at(i).second.first.size())*100,
														 ((double)experimentsError.at(regularStable)/incrementalDatasets.at(h).at(i).second.first.size())*100);

			mean.at(regularSimple) += (double)experimentsError.at(regularSimple)/incrementalDatasets.at(h).at(i).second.first.size();
			mean.at(regularStable) += (double)experimentsError.at(regularStable)/incrementalDatasets.at(h).at(i).second.first.size();

			if(paramStochastic)
			{
				mean.at(stochasticSimple) += (double)experimentsError.at(stochasticSimple)/incrementalDatasets.at(h).at(i).second.first.size();
				mean.at(stochasticStable) += (double)experimentsError.at(stochasticStable)/incrementalDatasets.at(h).at(i).second.first.size();
			}

			experimentsError.clear();
		}
		fprintf(foldAccuracyFile, "\n");

		// 5. Printing final statistics
		logFile = fopen("files/main.log", "a+");

		fprintf(logFile, "\nFinal Results For Increment %d:\n\n", h+1);

		fprintf(logFile, "Elapsed Time:\n");
		fprintf(logFile, "Set-up: %s\n", util.returnFormattedTime(setupTotalTime).c_str());
		fprintf(logFile, "Building: %s\n", util.returnFormattedTime(buildTotalTime).c_str());
		fprintf(logFile, "Training: %s\n", util.returnFormattedTime(trainTotalTime).c_str());
		fprintf(logFile, "Testing: %s\n", util.returnFormattedTime(testTotalTime).c_str());
		fprintf(logFile, "Total: %s\n", util.returnFormattedTime(setupTotalTime + buildTotalTime + trainTotalTime + testTotalTime).c_str());

		fprintf(logFile, "\nAverage results obtained for Regular Simple Evaluation: %0.2f%%\n", (double)(mean.at(regularSimple)/incrementalDatasets.at(h).size()) * 100);
		fprintf(logFile, "Average results obtained for Regular Stable Evaluation: %0.2f%%\n", (double)(mean.at(regularStable)/incrementalDatasets.at(h).size()) * 100);
		if(paramStochastic)
		{
			fprintf(logFile, "Average results obtained for Stochastic Simple Evaluation: %0.2f%%\n", (double)(mean.at(stochasticSimple)/incrementalDatasets.at(h).size()) * 100);
			fprintf(logFile, "Average results obtained for Stochastic Stable Evaluation: %0.2f%%\n\n", (double)(mean.at(stochasticStable)/incrementalDatasets.at(h).size()) * 100);
		}

		fclose(logFile);
	}

	fclose(foldAccuracyFile);
}


void CilpExperiment::replicatedBkExperiment()
{
	Cilp *baseSystem = new Cilp(paramStochastic);
	Cilp copy = *baseSystem;
	FILE *logFile, *foldAccuracyFile;

	vector<double> mean, meanFMeasure;
	vector<string> artificialBK, bodiesTemp;
	vector<vector<string> > bodiesTrain, bodiesTest, headsTrain, headsTest;

	vector<pair<pair<double, double>, confusionTable> > testResults;
	pair<vector<vector<string> >, vector<vector<string> > > dataReadTrain, dataReadTest;
	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > datasets;

	// mRMR
	vector<int> selectedFeatures;
	int totalFeatures;
	vector<string> temp, featureTable;
	string bodyWithoutNegation;

	// 1. Cleaning all previous logs
	util.clearAllFiles("files/", ".log");
	util.clearAllFiles("files/trainError/", ".log");
	util.clearAllFiles("files/validationError/", ".log");

	foldAccuracyFile = fopen("files/testFoldAccuracy.log", "a");

	printf("<General> Using the background knowledge file to build an initial network...\n");

	// 2. Building the net
//	time(&buildStart);
//	{
//		baseSystem->buildReplicatedBKNet(backgroundData, trainData, true);
//	}
//	time(&buildEnd);
//	buildTotalTime += difftime(buildEnd, buildStart);
//
//	printf("<General> Finished building the initial network...\n\n");
//	printf("<General> Reading training/testing data and splitting it into folds...\n");

	// 3. Reading the data file and splitting it into bodies and heads
	// Example format: T :- L1,L2,L3,...,Ln.
	time(&setupStart);
	{
		dataReadTrain = baseSystem->decodeData(trainData);
		bodiesTrain = dataReadTrain.first;
		headsTrain = dataReadTrain.second;

		// Filtering the inputs using mRMR
		if(paramMrmrFiltering && !paramPreSplittedFolds)
		{
			temp = util.clausesToFeatures(trainData);
			totalFeatures = count(temp.at(0).begin(), temp.at(0).end(), ',');
			selectedFeatures = mrmr(temp, totalFeatures);
			featureTable = util.csvStringToStringVector(temp.at(0));

			// Filtering body data
			for(unsigned int i = 0; i < bodiesTrain.size(); i++)
			{
				bodiesTemp.clear();

				for(unsigned int j = 0; j < bodiesTrain.at(i).size(); j++)
				{
					bodyWithoutNegation = bodiesTrain.at(i).at(j);
					if(bodiesTrain.at(i).at(j).at(0) == '~')
						bodyWithoutNegation = bodyWithoutNegation.substr(1);

					if(util.find(selectedFeatures, util.find(featureTable, bodyWithoutNegation)) >= 0)
						bodiesTemp.push_back(bodiesTrain.at(i).at(j));
				}

				bodiesTrain.at(i).clear();
				bodiesTrain.at(i) = bodiesTemp;
			}
		}

		if(testData.empty())
		{
			if(paramPreSplittedFolds)
				datasets = baseSystem->buildDatasetFromFolds();
			else
				datasets = baseSystem->splitDataByTarget(trainData, bodiesTrain, headsTrain, paramNumberOfTestFolds, paramTestPortion);
		}
		else
		{
			dataReadTest = baseSystem->decodeData(testData);

			bodiesTest = dataReadTest.first;
			headsTest = dataReadTest.second;

			datasets.push_back(make_pair(make_pair(trainData, make_pair(bodiesTrain, headsTrain)), make_pair(testData, make_pair(bodiesTest, headsTest))));
		}
	}
	time(&setupEnd);
	setupTotalTime += difftime(setupEnd, setupStart);

	printf("<General> Finished preparing training/testing data...\n");

	// Printing dataset info
	baseSystem->printDatasetInfo(datasets, true, "Main dataset");

	// 4. For each built train/test an experiment is done and the total error is collected
	printf("<General> Starting %d-fold cross validation on test...\n\n", (int)datasets.size());

	vector<Cilp> *systems = new vector<Cilp>(datasets.size(), *baseSystem);
	mean.resize(STABILIZATION_TYPE_SIZE, 0.0);
	meanFMeasure.resize(STABILIZATION_TYPE_SIZE, 0.0);

	//#pragma omp parallel for private(experimentsError)
	for(unsigned int i = 0; i < datasets.size(); i++)
	{
		if(paramMrmrFiltering && paramPreSplittedFolds)
		{
			temp = util.clausesToFeatures(datasets.at(i).first.first);
			totalFeatures = count(temp.at(0).begin(), temp.at(0).end(), ',');
			selectedFeatures = mrmr(temp, totalFeatures);
			featureTable = util.csvStringToStringVector(temp.at(0));

			// Filtering body data
			for(unsigned int i = 0; i < bodiesTrain.size(); i++)
			{
				bodiesTemp.clear();

				for(unsigned int j = 0; j < bodiesTrain.at(i).size(); j++)
				{
					bodyWithoutNegation = bodiesTrain.at(i).at(j);
					if(bodiesTrain.at(i).at(j).at(0) == '~')
						bodyWithoutNegation = bodyWithoutNegation.substr(1);

					if(util.find(selectedFeatures, util.find(featureTable, bodyWithoutNegation)) >= 0)
						bodiesTemp.push_back(bodiesTrain.at(i).at(j));
				}

				bodiesTrain.at(i).clear();
				bodiesTrain.at(i) = bodiesTemp;
			}
		}

		(systems->at(i)) = copy;
		(systems->at(i)).shuffleTrainData(datasets.at(i).first.first, datasets.at(i).first.second.first, datasets.at(i).first.second.second);

		time(&buildStart);
		{
			(systems->at(i)).buildReplicatedBKNet(backgroundData, datasets.at(i).first.first, paramDebugMode);
		}
		time(&buildEnd);
		buildTotalTime += difftime(buildEnd, buildStart);

		printf("<General> Training fold #%d...\n", i+1);

		// 4.2. Training each system copy

		// Setting up the "absurd" targets for each example
		logFile = fopen("files/main.log", "a+");
		for(unsigned int k = 0; k < datasets.at(i).first.first.size(); k++)
		{
			(systems->at(i)).stabilizeNet(regularSimple, datasets.at(i).first.second.first.at(k), false, logFile);

			if(util.find((systems->at(i)).neuronsLabels.at(OUTPUT_LAYER), "F") >= 0)
			{
				if(util.greater((systems->at(i)).netStatus.at(OUTPUT_LAYER).at(util.find((systems->at(i)).neuronsLabels.at(OUTPUT_LAYER), "F")), 0.0) && (util.find(datasets.at(i).first.second.second.at(k), "F") < 0))
				{
					datasets.at(i).first.first.at(k) = "F," + datasets.at(i).first.first.at(k);
					datasets.at(i).first.second.second.at(k).push_back("F");
				}
			}

			if(paramDebugMode)
				fprintf(logFile, "\nExample %d:%s", k+1, datasets.at(i).first.first.at(k).c_str());

			fprintf(logFile, "\n");
		}
		fclose(logFile);

		(systems->at(i)).trainNet(datasets.at(i).first.first, true);

		printf("<General> Finished training fold #%d\n\n", i+1);

		//(systems->at(i)).printNetWeights(true);

		printf("<General> Testing fold #%d...\n", i+1);

		time(&testStart);
		{
			testResults = (systems->at(i)).testNet(datasets.at(i).second.first, true);
			//experimentsError.resize(4, 0);
		}
		time(&testEnd);
		testTotalTime += difftime(testEnd, testStart);

		if(paramStochastic)
			fprintf(foldAccuracyFile, "%.2f %.2f %.2f %.2f\n", testResults.at(regularSimple).first.first,
															   testResults.at(regularStable).first.first,
															   testResults.at(stochasticSimple).first.first,
															   testResults.at(stochasticStable).first.first);
		else
			fprintf(foldAccuracyFile, "%.2f %.2f\n", testResults.at(regularSimple).first.first,
													 testResults.at(regularStable).first.first);

		mean.at(regularSimple) += testResults.at(regularSimple).first.first;
		mean.at(regularStable) += testResults.at(regularStable).first.first;

		meanFMeasure.at(regularSimple) += testResults.at(regularSimple).first.second;
		meanFMeasure.at(regularStable) += testResults.at(regularStable).first.second;

		if(paramStochastic)
		{
			mean.at(stochasticSimple) += testResults.at(stochasticSimple).first.first;
			mean.at(stochasticStable) += testResults.at(stochasticStable).first.first;

			meanFMeasure.at(stochasticSimple) += testResults.at(stochasticSimple).first.second;
			meanFMeasure.at(stochasticStable) += testResults.at(stochasticStable).first.second;
		}

		printf("\n<General> Finished testing fold #%d\n\n", i+1);
	}

	printf("<General> Finished %d-fold cross validation on test...\n\n", (int)datasets.size());

	// 5. Printing final statistics

	logFile = fopen("files/main.log", "a+");

	fprintf(logFile, "\nFinal Results:\n\n");
	printf("\nFinal Results:\n\n");

	fprintf(logFile, "Elapsed Time:\n");
	fprintf(logFile, "Set-up: %s\n", util.returnFormattedTime(setupTotalTime).c_str());
	fprintf(logFile, "Building: %s\n", util.returnFormattedTime(buildTotalTime).c_str());
	fprintf(logFile, "Training: %s\n", util.returnFormattedTime(trainTotalTime).c_str());
	fprintf(logFile, "Testing: %s\n", util.returnFormattedTime(testTotalTime).c_str());
	fprintf(logFile, "Total: %s\n", util.returnFormattedTime(setupTotalTime + buildTotalTime + trainTotalTime + testTotalTime).c_str());

	printf("Elapsed Time:\n");
	printf("Set-up: %s\n", util.returnFormattedTime(setupTotalTime).c_str());
	printf("Building: %s\n", util.returnFormattedTime(buildTotalTime).c_str());
	printf("Training: %s\n", util.returnFormattedTime(trainTotalTime).c_str());
	printf("Testing: %s\n", util.returnFormattedTime(testTotalTime).c_str());
	printf("Total: %s\n\n", util.returnFormattedTime(setupTotalTime + buildTotalTime + trainTotalTime + testTotalTime).c_str());

	fprintf(logFile, "\nAverage accuracy obtained for Regular Simple Evaluation: %0.2f%%\n", (double)(mean.at(regularSimple)/datasets.size()));
	fprintf(logFile, "Average accuracy obtained for Regular Stable Evaluation: %0.2f%%\n", (double)(mean.at(regularStable)/datasets.size()));
	printf("Average accuracy obtained for Regular Simple Evaluation: %0.2f%%\n", (double)(mean.at(regularSimple)/datasets.size()));
	printf("Average accuracy obtained for Regular Stable Evaluation: %0.2f%%\n", (double)(mean.at(regularStable)/datasets.size()));
	if(paramStochastic)
	{
		fprintf(logFile, "Average accuracy obtained for Stochastic Simple Evaluation: %0.2f%%\n", (double)(mean.at(stochasticSimple)/datasets.size()));
		fprintf(logFile, "Average accuracy obtained for Stochastic Stable Evaluation: %0.2f%%\n", (double)(mean.at(stochasticStable)/datasets.size()));
		printf("Average accuracy obtained for Stochastic Simple Evaluation: %0.2f%%\n", (double)(mean.at(stochasticSimple)/datasets.size()));
		printf("Average accuracy obtained for Stochastic Stable Evaluation: %0.2f%%\n\n", (double)(mean.at(stochasticStable)/datasets.size()));
	}

	fprintf(logFile, "\nAverage f-measure obtained for Regular Simple Evaluation: %f\n", (double)(meanFMeasure.at(regularSimple)/datasets.size()));
	fprintf(logFile, "Average f-measure obtained for Regular Stable Evaluation: %f\n", (double)(meanFMeasure.at(regularStable)/datasets.size()));
	printf("Average f-measure obtained for Regular Simple Evaluation: %f\n", (double)(meanFMeasure.at(regularSimple)/datasets.size()));
	printf("Average f-measure obtained for Regular Stable Evaluation: %f\n", (double)(meanFMeasure.at(regularStable)/datasets.size()));
	if(paramStochastic)
	{
		fprintf(logFile, "Average f-measure obtained for Stochastic Simple Evaluation: %f\n", (double)(meanFMeasure.at(stochasticSimple)/datasets.size()));
		fprintf(logFile, "Average f-measure obtained for Stochastic Stable Evaluation: %f\n", (double)(meanFMeasure.at(stochasticStable)/datasets.size()));
		printf("Average f-measure obtained for Stochastic Simple Evaluation: %f\n", (double)(meanFMeasure.at(stochasticSimple)/datasets.size()));
		printf("Average f-measure obtained for Stochastic Stable Evaluation: %f\n\n", (double)(meanFMeasure.at(stochasticStable)/datasets.size()));
	}

	fclose(logFile);
	fclose(foldAccuracyFile);

	delete(baseSystem);
	delete(systems);
}
