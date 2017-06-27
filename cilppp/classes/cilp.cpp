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

#include "cilp.h"

Cilp::Cilp(bool isStochastic)
{
	// 1. Keeping the number of layers
	numberOfLayers = NUMBER_OF_LAYERS;

	// 2. Starting the decay factor
	learningRate = paramDefaultLearningRate;

	// 3. Allocating the weight vector
	connectionsWeights.resize(numberOfLayers);

	// 4. Allocating the weight labels
	neuronsLabels.resize(numberOfLayers);

	// 5. Allocating the biases and thresholds
	thresholds.resize(numberOfLayers);
	layersBias.resize(numberOfLayers);

	// 6. Setting the stochastic parameter
	stochastic = isStochastic;

	// 7. Allocating the normalization vectors
	normalizationFactors.resize(numberOfLayers);
	
	// 8. Setting the boolean attributes
	noBackgroundKnowledge = false;
	haveIntermediateConcepts = false;

	w = 0;
	amin = 0;
}

Cilp::~Cilp()
{
	unsigned int i, j;

	// 1. Emptying the number of neurons
	numberOfNeurons.clear();

	// 2. Emptying layersBias
	for(i = 0; i < layersBias.size(); i++)
		layersBias.at(i).clear();
	layersBias.clear();

	// 3. Emptying the connections weights
	for(i = 0; i < connectionsWeights.size(); i++)
	{
		for(j = 0; j < connectionsWeights.at(i).size(); j++)
			connectionsWeights.at(i).at(j).clear();

		connectionsWeights.at(i).clear();
	}
	connectionsWeights.clear();

	// 4. Emptying the neurons labels
	for(i = 0; i < neuronsLabels.size(); i++)
		neuronsLabels.at(i).clear();
	neuronsLabels.clear();

	// 5. Emptying the normalized amins
	for(i = 0; i < normalizedAmins.size(); i++)
		normalizedAmins.at(i).clear();
	normalizedAmins.clear();

	// 6. Emptying the thresholds
	for(i = 0; i < thresholds.size(); i++)
		thresholds.at(i).clear();
	thresholds.clear();

	// 7. Emptying the network status
	for(i = 0; i < netStatus.size(); i++)
		netStatus.at(i).clear();
	netStatus.clear();

	// 8. Emptying all the normalization factors
	for(i = 0; i < normalizationFactors.size(); i++)
		normalizationFactors.at(i).clear();
	normalizationFactors.clear();

	for(i = 0; i < bcpFeatures.size(); i++)
		bcpFeatures.at(i).clear();
	bcpFeatures.clear();

	bcpFeatureLabels.clear();
}

// Method responsible to build a neural net using the set of clauses passed by parameter
void Cilp::buildNet(vector<string> clauses, bool printLog)
{
	FILE *logFile = NULL;

	double weightW = 0.0;
	bool connectionExists, fileExists, patternFound;
	int maxP_k_m, index, find1, find2, size;

	string line, bodies, head, content, content2, predicate, predicate2, newBKClause, newHeadPart;
	time_t secondsQt;
	struct tm *currentTimestamp;

	vector<int> k, m;
	vector<string> temp, iteratedVector, variables, variables2, literalsList, newBK;
	vector<vector<string> > heads, bodiesSep;
	vector<pair<int, double> > newWeightVector;
	vector<double> newStatusVector;
	pair<vector<vector<string> >, vector<vector<string> > > dataRead, decodedBK;

	// 1. Initialization
	if(paramDebugMode) printLog = true;

	k.reserve(1);
	m.reserve(1);
	heads.reserve(1);
	temp.reserve(1);
	bodiesSep.reserve(1);
	newWeightVector.reserve(1);
	newStatusVector.reserve(1);

	// Initializing the log file
	fileExists = util.fileExists("files/main.log");
	if(printLog)
		logFile = fopen("files/main.log", "a");

	time(&secondsQt);
	currentTimestamp = localtime(&secondsQt);

	if(!fileExists)
	{
		if(printLog) fprintf(logFile, "CILP++, Version %0.1f\n", VERSION);
		if(printLog) fprintf(logFile, "Author: %s\n\n", AUTHOR);
		if(printLog) fprintf(logFile, "*Execution Log* %s\n", asctime(currentTimestamp));
	}

	if(printLog) fprintf(logFile, "|-----------------|\n");
	if(printLog) fprintf(logFile, "|Building the net:|\n");
	if(printLog) fprintf(logFile, "|-----------------|\n\n");

	// 2. Decoding the clauses and calculating the net parameters

	// New pre-processing: dealing with disjunction in the body
	decodedBK = util.decodeRules(clauses);
	size = clauses.size();

	for(int i = 0; i < size; i++)
	{
		// Collecting body mappings
		for(unsigned int j = 0; j < decodedBK.first.at(i).size(); j++)
		{
			literalsList.clear();
			if(decodedBK.first.at(i).at(j).find("|") != string::npos)
			{
				literalsList = util.splitDelimited(decodedBK.first.at(i).at(j), '|');

				for(unsigned int k = 0; k < literalsList.size(); k++)
				{
					newHeadPart = clauses.at(i).substr(0, clauses.at(i).find(":-")+2);
					newBKClause = clauses.at(i).substr(clauses.at(i).find(":-")+3);

					newBKClause.replace(newBKClause.find(decodedBK.first.at(i).at(j)), decodedBK.first.at(i).at(j).size(), literalsList.at(k));

					clauses.push_back(newHeadPart + newBKClause);
					size++;
				}
			}
		}
	}

	// Removing identical clauses
	clauses = util.getDistinctElements(clauses);

	// 2.1. Reading the program file and splitting it into bodies and heads
	dataRead = decodedBK;
	bodiesSep = dataRead.first;
	heads = dataRead.second;

	// 2.2. Calculating maxP_k_m
	for(unsigned int i = 0; i < clauses.size(); i++)
	{
		// 2.2.1. Calculating k
		if(bodiesSep.at(i).at(0).compare("T") != 0)
			k.push_back(bodiesSep.at(i).size());
		else
			k.push_back(0);

		// 2.2.2. Calculating m
		m.push_back(0);
		for(unsigned int j = 0; j < heads.size(); j++)
		{
			if(!heads.at(i).at(0).compare(heads.at(j).at(0)))
				m.at(i)++;
		}
	}

	// 2.2.3. Obtaining maxP_k_m
	vector<int> kOrd(k.begin(), k.end());
	vector<int> mOrd(m.begin(), m.end());

	sort(kOrd.begin(), kOrd.end());
	sort(mOrd.begin(), mOrd.end());

	if(!mOrd.empty())
		maxP_k_m = (kOrd.back() > mOrd.back()) ? kOrd.back() : mOrd.back();
	else
		maxP_k_m = 0;

	// 2.4. Calculating Amin
	// The Amin should be bigger than (maxP_k_m - 1) / (maxP_k_m + 1) and smaller than 1
	amin = (double) (maxP_k_m - 1) / (maxP_k_m + 1);
	amin += (1.0 - amin) / 2.0;

	// If the amin value is too high, it is returned to his original value added of a minimal value
	if(util.greater(amin, paramAminThreshold))
		amin = ((double) (maxP_k_m - 1) / (maxP_k_m + 1)) + paramMinimalIncrement;

	// 2.5. Calculating W
	w = ((double)2/paramBeta) * ((double)(log(1 + amin) - log(1 - amin)) / ((maxP_k_m * (amin - 1)) + amin + 1));
	w += paramMinimalIncrement;

	if(printLog) fprintf(logFile, "maxP_k_m = %d; amin = %f; w = %f\n\n", maxP_k_m, amin, w);

	// 3. For each clause, build the net using them
	for(unsigned int i = 0; i < clauses.size(); i++)
	{
		// 3.1. Adding the new neuron in the hidden layer
		neuronsLabels.at(HIDDEN_LAYER).push_back(clauses.at(i));

		if(printLog) fprintf(logFile, "Hidden layer neuron created: (%d) %s \n", i + 1, clauses.at(i).c_str());

		// 3.2. Building the input layer
		for(unsigned int j = 0; j < bodiesSep.at(i).size(); j++)
		{
			// 3.2.1. Checking if is a negative literal
			if(bodiesSep.at(i).at(j).at(0) == '~')
			{
				bodiesSep.at(i).at(j).erase(0, 1);
				weightW = -w;
			}
			else
				weightW = w;

			if(find(neuronsLabels.at(INPUT_LAYER).begin(), neuronsLabels.at(INPUT_LAYER).end(), bodiesSep.at(i).at(j)) == neuronsLabels.at(INPUT_LAYER).end())
			{
				neuronsLabels.at(INPUT_LAYER).push_back(bodiesSep.at(i).at(j));

				if(printLog) fprintf(logFile, "Input neuron created and pointing to %d: %s \n", i + 1, bodiesSep.at(i).at(j).c_str());

				if(!newWeightVector.empty())
					newWeightVector.clear();
				newWeightVector.push_back(make_pair(neuronsLabels.at(HIDDEN_LAYER).size()-1, weightW));
				connectionsWeights.at(INPUT_LAYER).push_back(newWeightVector);
			}
			else
			{
				if(printLog) fprintf(logFile, "Input neuron pointing to %d: %s \n", i + 1, bodiesSep.at(i).at(j).c_str());
				connectionsWeights.at(INPUT_LAYER).at(util.find(neuronsLabels.at(INPUT_LAYER), bodiesSep.at(i).at(j))).push_back(make_pair(neuronsLabels.at(HIDDEN_LAYER).size()-1, weightW));
			}
			
			if(!newWeightVector.empty())
				newWeightVector.clear();
			if(util.find(neuronsLabels.at(OUTPUT_LAYER), bodiesSep.at(i).at(j)) != -1)
			{
				if(printLog) fprintf(logFile, "Output neuron points to his related in the input layer: %s\n", bodiesSep.at(i).at(j).c_str());

				index = util.find(neuronsLabels.at(INPUT_LAYER), bodiesSep.at(i).at(j));
				connectionsWeights.at(OUTPUT_LAYER).at(index).push_back(make_pair(index, 1.0));

				haveIntermediateConcepts = true;
			}
		}

		// 3.3.1. Connecting the output layer to the input layer
		if(find(neuronsLabels.at(OUTPUT_LAYER).begin(), neuronsLabels.at(OUTPUT_LAYER).end(), heads.at(i).at(0)) == neuronsLabels.at(OUTPUT_LAYER).end())
		{
			neuronsLabels.at(OUTPUT_LAYER).push_back(heads.at(i).at(0));
			if(printLog) fprintf(logFile, "Output neuron created: %s\n", heads.at(i).at(0).c_str());

			if(!newWeightVector.empty())
				newWeightVector.clear();
			if(util.find(neuronsLabels.at(INPUT_LAYER), heads.at(i).at(0)) != -1)
			{
				if(printLog) fprintf(logFile, "Output neuron points to his related in the input layer: %s\n", heads.at(i).at(0).c_str());
				newWeightVector.push_back(make_pair(util.find(neuronsLabels.at(INPUT_LAYER), heads.at(i).at(0)), 1.0));

				haveIntermediateConcepts = true;
			}
			else if(heads.at(i).at(0).find("|") != string::npos)
			{
				// 3.3.1.1. If the correspondent input node does not exist and the current output neuron represents a disjunction, it is created
				neuronsLabels.at(INPUT_LAYER).push_back(heads.at(i).at(0));
				connectionsWeights.at(INPUT_LAYER).push_back(newWeightVector);

				if(printLog) fprintf(logFile, "The output neuron caused the creation of one neuron %s in the input\n", heads.at(i).at(0).c_str());

				newWeightVector.push_back(make_pair(neuronsLabels.at(INPUT_LAYER).size()-1, 1.0));

				haveIntermediateConcepts = true;
			}

			connectionsWeights.at(OUTPUT_LAYER).push_back(newWeightVector);
		}

		// 3.3.2. Next, the relation of the hidden neuron with the output is done
		if(!newWeightVector.empty())
			newWeightVector.clear();
		newWeightVector.push_back(make_pair(util.find(neuronsLabels.at(OUTPUT_LAYER), heads.at(i).at(0)), w));
		connectionsWeights.at(HIDDEN_LAYER).push_back(newWeightVector);

		if(printLog) fprintf(logFile, "Hidden layer neuron %d points to: %s \n\n", i + 1, heads.at(i).at(0).c_str());
	}

	// 3.4. Making the net fully-connected
	for(int i = 0; i < numberOfLayers - 1; i++)
	{
		for(unsigned int j = 0; j < neuronsLabels.at(i).size(); j++)
		{
			for(unsigned int k = 0; k < neuronsLabels.at(i+1).size(); k++)
			{
				connectionExists = false;
				int k2 = k;

				// 3.4.1. Adding weight 0 connections, to the input and hidden layers
				for(unsigned int l = 0; l < connectionsWeights.at(i).at(j).size(); l++)
				{
					if(connectionsWeights.at(i).at(j).at(l).first == k2)
					{
						connectionExists = true;
						break;
					}
				}
				if(!connectionExists)
					connectionsWeights.at(i).at(j).push_back(make_pair(k, 0.0));
			}
		}
	}

	// 3.5. Initializing the net status
	if(!netStatus.empty())
		netStatus.clear();
	for(int i = 0; i < numberOfLayers; i++)
	{
		if(!newStatusVector.empty())
			newStatusVector.clear();
		netStatus.push_back(newStatusVector);

		for(unsigned int j = 0; j < neuronsLabels.at(i).size(); j++)
			netStatus.at(i).push_back(0.0);
	}

	// Initializing the amins per layer into the net
	if(!normalizedAmins.empty())
		normalizedAmins.clear();
	for(int i = 0; i < numberOfLayers; i++)
	{
		vector<double> tempNormalizedAmins(connectionsWeights.at(i).size(), amin);
		normalizedAmins.push_back(tempNormalizedAmins);
	}

	// 3.6. The biases and thresholds are calculated
	for(unsigned int i = 0; i < neuronsLabels.size(); i++)
	{
		vector<string> iteratedVector = neuronsLabels.at(i);

		for(unsigned int j = 0; j < iteratedVector.size(); j++)
		{
			/*
			 * Thresholds inclusion:
			 *
			 * INPUT: threshold = 0
			 * HIDDEN: threshold = (1+amin)*('k'-1)*w/2
			 * OUTPUT: threshold = (1+amin)*(1-'m')*w/2
			 */
			switch(i)
			{
				case INPUT_LAYER:
					thresholds.at(i).push_back(0.0);
					layersBias.at(i).push_back(0.0);
					if(printLog) fprintf(logFile, "Threshold of the input layer neuron %s: %f\n", iteratedVector.at(j).c_str(), 0.0);
					break;
				case HIDDEN_LAYER:
					thresholds.at(i).push_back((1+amin)*(k.at(j)-1)*((double)w/2));
					layersBias.at(i).push_back((-1.0)*(1+amin)*(k.at(j)-1)*((double)w/2));
					if(printLog) fprintf(logFile, "Threshold of the hidden layer neuron %s: %f\n", iteratedVector.at(j).c_str(), (1+amin)*(k.at(j)-1)*((double)w/2));
					break;
				case OUTPUT_LAYER:
					thresholds.at(i).push_back((1+amin)*(1-m.at(j))*((double)w/2));
					layersBias.at(i).push_back((-1.0)*(1+amin)*(1-m.at(j))*((double)w/2));
					if(printLog) fprintf(logFile, "Threshold of the output layer neuron %s: %f\n", iteratedVector.at(j).c_str(), (1+amin)*(1-m.at(j))*((double)w/2));
					break;
				default:
					break;
			}
		}
	}

	// 3.7. Relational learning case: if the output layer has a "general unifying variable", it needs to be treated
	for(unsigned int i = 0; i < neuronsLabels.at(OUTPUT_LAYER).size(); i++)
	{
		// Verifying if the literal represents a disjunction
		literalsList.clear();
		if(neuronsLabels.at(OUTPUT_LAYER).at(i).find("|") != string::npos)
			literalsList = util.splitDelimited(neuronsLabels.at(OUTPUT_LAYER).at(i), '|');
		else
			literalsList.push_back(neuronsLabels.at(OUTPUT_LAYER).at(i));

		for(unsigned int a = 0; a < literalsList.size(); a++)
		{
			find1 = literalsList.at(a).find("(");
			find2 = literalsList.at(a).find(")");

			if((find1 == (int)string::npos) || (find2 == (int)string::npos))
				break;

			predicate = literalsList.at(a).substr(0, find1);
			content   = literalsList.at(a).substr(find1+1, find2-find1-1);

			variables = util.splitDelimited(content, ';');

			for(unsigned int j = 0; j < variables.size(); j++)
			{
				// Checking if the checked variable is generic
				if(!variables.at(j).compare("_"))
				{
					// If P(_,B) exists in the output, all P("something", B) needs to be true in the input.
					for(unsigned int k = 0; k < neuronsLabels.at(INPUT_LAYER).size(); k++)
					{
						patternFound = true;

						find1 = neuronsLabels.at(INPUT_LAYER).at(k).find("(");
						predicate2 = neuronsLabels.at(INPUT_LAYER).at(k).substr(0, find1);

						if(!predicate.compare(predicate2))
						{
							find2 = neuronsLabels.at(INPUT_LAYER).at(k).find(")");
							content2   = neuronsLabels.at(INPUT_LAYER).at(k).substr(find1+1, find2-find1-1);
							variables2 = util.splitDelimited(content, ';');

							for(unsigned int l = 0; l < variables2.size(); l++)
							{
								// Checking if the other variables of the literals being compared matches
								if((l != j) && variables.at(l).compare("_") && variables2.at(l).compare("_") && variables.at(l).compare(variables2.at(l)))
								{
									patternFound = false;
									break;
								}
							}

							// If they match, a recursive connection needs to be included, between them
							if(patternFound)
							{
								if(printLog) fprintf(logFile, "Output neuron %s points to his related in the input layer: %s\n", neuronsLabels.at(OUTPUT_LAYER).at(i).c_str(), neuronsLabels.at(INPUT_LAYER).at(k).c_str());

								haveIntermediateConcepts = true;
								connectionsWeights.at(OUTPUT_LAYER).at(i).push_back(make_pair(k, 1.0));
							}
						}
					}
				}
			}
		}
	}

	// 3.8. Setting the "just training" flag
	noBackgroundKnowledge = clauses.empty();

	//Normalizing the net
	normalizeWeights(NORMALIZE_WEIGHTS_ONLY);

	// 3.9. Adding the defined starting hidden neurons into the network
	addNewHiddenNeurons(paramStartingHiddenNeurons - connectionsWeights.at(HIDDEN_LAYER).size());

	if(printLog) fprintf(logFile, "\n");
	if(printLog) fclose(logFile);
}

// Method responsible for processing the inputs until the fixed point is found
int Cilp::stabilizeNet(stabilizationType type, vector<string> activatedNeurons, bool printLog, FILE *logFile)
{
	int iteration;
	bool fileExists, stabilized, isSimple = false;
	double justAdouble;
	string labelWithoutNegation;

	vector<double> comingWeightsSum, neuronOutputValues;
	activationFunction usedFunction;

	time_t secondsQt;
	struct tm *currentTimestamp;

	// Initializing the log file
	fileExists = util.fileExists("files/main.log");

	if(paramDebugMode) printLog = true;

	if(printLog) logFile = fopen("files/main.log", "a");

	if(!fileExists)
	{
		time(&secondsQt);
		currentTimestamp = localtime(&secondsQt);

		if(printLog) fprintf(logFile, "CILP++, Version %0.1f\n", VERSION);
		if(printLog) fprintf(logFile, "Author: %s\n\n", AUTHOR);
		if(printLog) fprintf(logFile, "*Execution* %s\n\n", asctime(currentTimestamp));
	}

	if(printLog) fprintf(logFile, "|--------------------|\n");
	if(printLog) fprintf(logFile, "|Stabilizing the net:|\n");
	if(printLog) fprintf(logFile, "|--------------------|\n\n");

	// 1. Initializing the net with the neurons passed as true at input
	for(unsigned int i = 0; i < neuronsLabels.at(INPUT_LAYER).size(); i++)
		neuronOutputValues.push_back(0.0);

	if(printLog) fprintf(logFile, "Inputs: ");
	for(unsigned int i = 0; i < activatedNeurons.size(); i++)
	{
		labelWithoutNegation = (activatedNeurons.at(i).at(0) == '~' ? activatedNeurons.at(i).substr(1, activatedNeurons.at(i).size()) : activatedNeurons.at(i));

		if(util.find(neuronsLabels.at(INPUT_LAYER), labelWithoutNegation) != -1)
		{
			if(activatedNeurons.at(i).at(0) != '~')
			{
				netStatus.at(INPUT_LAYER).at(util.find(neuronsLabels.at(INPUT_LAYER), labelWithoutNegation)) = 1.0;
				neuronOutputValues.at(util.find(neuronsLabels.at(INPUT_LAYER), labelWithoutNegation)) = 1.0;
			}
			else
			{
				netStatus.at(INPUT_LAYER).at(util.find(neuronsLabels.at(INPUT_LAYER), labelWithoutNegation)) = -1.0;
				neuronOutputValues.at(util.find(neuronsLabels.at(INPUT_LAYER), labelWithoutNegation)) = -1.0;
			}

			if(printLog) fprintf(logFile, "%s, ", activatedNeurons.at(i).c_str());
		}
	}
	if(printLog) fprintf(logFile, "\n");

	// 2. Marking the neurons with label "T" as activated
	for(unsigned int i = 0; i < neuronsLabels.at(INPUT_LAYER).size(); i++)
	{
		// If is the node with label "T", it is marked as activated
		if(neuronsLabels.at(INPUT_LAYER).at(i).compare("T") == 0)
		{
			if(printLog) fprintf(logFile, "Neuron 'T' activated \n\n");
			netStatus.at(INPUT_LAYER).at(i) = 1.0;
		}
	}

	// 3. Main loop: while the input and output activation state are not equal, the signals will be propagated by the net
	stabilized = false;
	if(printLog) fprintf(logFile, "Stabilization Process:\n\n");

	// If the stabilization type is 'simple', just one iteration will be made
	if((type == regularSimple) || (type == stochasticSimple))
		isSimple = true;

	for(iteration = 0; !stabilized; iteration++)
	{
		if(isSimple)
			stabilized = true;

		// If the number of iterations has reached a very big amount, stop
		if(iteration >= paramStablizationMaxIterations)
			break;

		// 3.1. Propagating the activation to the next layer
		for(int i = 0; i < numberOfLayers; i++)
		{
			if(!stabilized || (i != OUTPUT_LAYER))
			{
				if(printLog) fprintf(logFile, "Layer %d:\n", i + 1);

				// If the layer to receive the signals is the input layer, the activation function will be linear, otherwise, it is bipolar
				usedFunction = (i == OUTPUT_LAYER ? linear : bipolar);

				// 3.1.1. Resetting the vector containing the total sum of the weights getting at one node.
				// If the node is a 'T', it will receive total sum 1
				if(!comingWeightsSum.empty())
					comingWeightsSum.clear();
				for(unsigned int j = 0; j < neuronsLabels.at((i+1) % numberOfLayers).size(); j++)
				{
					if(neuronsLabels.at((i+1) % numberOfLayers).at(j).compare("T") == 0)
						comingWeightsSum.push_back(1.0);
					else
						comingWeightsSum.push_back(0.0);
				}

				// 3.1.2. To each neuron of the next layer, add the weights of their connections with the neurons of the previous layers
				//		  multiplied by the output vectors
				for(unsigned int j = 0; j < neuronOutputValues.size(); j++)
				{
					for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
						comingWeightsSum.at(connectionsWeights.at(i).at(j).at(k).first) += (connectionsWeights.at(i).at(j).at(k).second * neuronOutputValues.at(j));
				}

				// 3.1.3. The activation function is applied to each sum to check if the neuron will be activated
				if(!neuronOutputValues.empty())
					neuronOutputValues.clear();
				for(unsigned int j = 0; j < comingWeightsSum.size(); j++)
				{
					// If the node is a non-fed input neuron, it will keep its initial activation state
					if((i == OUTPUT_LAYER) && (util.find(neuronsLabels.at(i), neuronsLabels.at((i+1) % numberOfLayers).at(j)) == -1) && (util.find(activatedNeurons, neuronsLabels.at((i+1) % numberOfLayers).at(j)) != -1))
						comingWeightsSum.at(j) = 1.0;

					if(printLog) fprintf(logFile, "Sum coming in the neuron %s of the layer %d: %.6f\n", neuronsLabels.at((i+1) % numberOfLayers).at(j).c_str(), (i + 1) % numberOfLayers + 1, comingWeightsSum.at(j));

					switch(usedFunction)
					{
						case linear:
							if(printLog) fprintf(logFile, "Activation function: linear\n");
							break;
						case binary:
							if(printLog) fprintf(logFile, "Activation function: binary\n");
							break;
						case hyperbolic:
							if(printLog) fprintf(logFile, "Activation function: hyperbolic\n");
							break;
						case bipolar:
							if(printLog) fprintf(logFile, "Activation function: bipolar\n");
							break;
						default:
							break;
					}
					if(printLog) fprintf(logFile, "Activation function result: %f\n", calculateActivationFunction(comingWeightsSum.at(j) + layersBias.at((i+1) % numberOfLayers).at(j), usedFunction));
					if(printLog) fprintf(logFile, "Bias: %f\n", layersBias.at((i+1) % numberOfLayers).at(j));

					// 3.1.3.1. Calculating the output values and updating the net status
					neuronOutputValues.push_back(calculateActivationFunction(comingWeightsSum.at(j) + layersBias.at((i+1) % numberOfLayers).at(j), usedFunction));

					if(printLog) fprintf(logFile, "Previous Neuron status: %s\n", returnNeuronStatusString((i+1) % numberOfLayers, neuronsLabels.at((i+1) % numberOfLayers).at(j), false, justAdouble, justAdouble).c_str());
					netStatus.at((i+1) % numberOfLayers).at(j) = calculateActivationFunction(comingWeightsSum.at(j) + layersBias.at((i+1) % numberOfLayers).at(j), usedFunction);
					if(printLog) fprintf(logFile, "New neuron status: %s\n", returnNeuronStatusString((i+1) % numberOfLayers, neuronsLabels.at((i+1) % numberOfLayers).at(j), false, justAdouble, justAdouble).c_str());

					if(i == OUTPUT_LAYER)
					{
						if(returnNeuronStatus((i+1) % numberOfLayers, neuronsLabels.at((i+1) % numberOfLayers).at(j), false, justAdouble, justAdouble) == returnNeuronStatus(i, neuronsLabels.at((i+1) % numberOfLayers).at(j), false, justAdouble, justAdouble))
						{
							if(printLog) fprintf(logFile, "Neuron is stabilized\n");
						}
						else if(returnNeuronStatus(i, neuronsLabels.at((i+1) % numberOfLayers).at(j), false, justAdouble, justAdouble) != doesNotExist)
						{
							if(printLog) fprintf(logFile, "Neuron is not stabilized\n");
						}
					}

					if(printLog) fprintf(logFile, "\n");
				}
			}
			
			// 3.1.4. Checking if the system stabilized (if the output layer contains activation equal to the input)
			if(!isSimple && (i == HIDDEN_LAYER))
			{
				stabilized = isStabilized(type);
				if(stabilized)
					iteration--;
			}
		}
	}

	if(printLog) fprintf(logFile, "*System is stabilized*\n\n");
	if(printLog) fclose(logFile);

	return iteration;
}

// Method to apply the activation function to the weights sum passed as parameter
double Cilp::calculateActivationFunction(double weightSum, activationFunction function)
{
	switch(function)
	{
		case linear:
			return weightSum;
			break;
		case binary:
			if(util.less(weightSum, 0.0))
				return 0.0;
			else
				return 1.0;
			break;
		case hyperbolic:
			return tanh(weightSum);
			break;
		case bipolar:
			return ((double) 2 / (1 + exp(-1 * weightSum * paramBeta))) - 1;
			break;
		default:
			cout << "[Error] This activation function is not implemented!" << endl;
			break;
	}

	return 0.0;
}

// Method to train the net with N-fold Cross Validation
void Cilp::trainNet(vector<string> trainedExamples, bool printLog)
{
	int numberOfValidationFolds, totalRight, numberOfStochasticEvaluations = 0, index;
	double totalError, currentTotalInput, confidence, cumulativeConfidence = 0.0, initialValue;
	string newLiteral;

	bool fileExists, isRight;

	FILE *logFile = NULL, *foldAccuracyFile;
	vector<FILE*>  trainErrorFile, trainTestErrorFile;

	time_t secondsQt;
	struct tm *currentTimestamp;

	string headStr, bodyStr, headWithoutNegation, bodyWithoutNegation, fileName;

	vector<int> leastErrorIndexes;
	vector<double> currentInput, currentOutput, currentM, errorVector, totalErrorNets, newDelta, newBias, alpha, lastAlpha, errorFunction, targetVector, errorValues;
	vector<pair<int, double> > buffer;
	vector<vector<double> > input, output, m, deltaBias, previousDeltaBias, error, totalOutputInputs;
	vector<vector<vector<double> > > deltaW, previousDeltaW;

	vector<string> trainingHeads, validatingHeads, filteredFeatures, headsWithoutNegation;
	vector<vector<string> > bodies, heads, trainingBodies, validatingBodies;

	vector<pair<int, double> > newWeightVector;
	vector<vector<pair<int, double> > > newNodeWeightVector;

	pair<vector<vector<string> >, vector<vector<string> > > dataRead;
	pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > datasetTrain, datasetValidation;
	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > datasets, mainDataset;

	// mRMR
//	vector<int> selectedFeatures;
//	int totalFeatures;
//	vector<string> temp, featureTable;

	time(&trainStart);

	// 1. Initializing the log files
	fileExists = util.fileExists("files/main.log");

	if(paramDebugMode) printLog = true;

	if(printLog)
		logFile = fopen("files/main.log", "a");

	if(!fileExists)
	{
		time(&secondsQt);
		currentTimestamp = localtime(&secondsQt);

		if(printLog) fprintf(logFile, "CILP++, Version %0.1f\n", VERSION);
		if(printLog) fprintf(logFile, "Author: %s\n\n", AUTHOR);
		if(printLog) fprintf(logFile, "*Execution Log* %s\n\n", asctime(currentTimestamp));
	}

	// Printing the network structure
	printf("\nNetwork structure before receiving training data:\n");
	printf("Number of Input neurons: %d", (int)neuronsLabels.at(0).size());
	printf("\nNumber of Hidden neurons: %d", (int)neuronsLabels.at(1).size());
	printf("\nNumber of Output neurons: %d\n\n", (int)neuronsLabels.at(2).size());

	foldAccuracyFile = fopen("files/trainFoldAccuracy.log", "a");

	// 2. Analyzing the training examples
	if(printLog) fprintf(logFile, "|-----------------|\n");
	if(printLog) fprintf(logFile, "|Training the net:|\n");
	if(printLog) fprintf(logFile, "|-----------------|\n\n");

	// 2.1. Balancing the datasetss
	// TODO -- Implement SMOTE

	// 2.2. Reading the program data and splitting it into bodies and heads
	// Example of input format: T :- L1,L2,L3,...,Ln, in which T represents the target to classify. It can be positive or negative

	dataRead = decodeData(trainedExamples);

	time(&trainEnd);
	trainTotalTime += difftime(trainEnd, trainStart);

	// Separating the main dataset into training and validation
	mainDataset = splitDataByTarget(trainedExamples, dataRead.first, dataRead.second, 1, paramTrainPortion);

	// BCP propositionalization
	if(paramNewBcp)
		bcp(mainDataset.at(0).first.second);
	else
	{
		for(unsigned int i = 0; i < mainDataset.at(0).first.second.first.size(); i++)
			bcpFeatureLabels.insert(bcpFeatureLabels.begin(), mainDataset.at(0).first.second.first.at(i).begin(), mainDataset.at(0).first.second.first.at(i).end());

		bcpFeatureLabels = util.getDistinctElements(bcpFeatureLabels);
		for(unsigned int i = 0; i < bcpFeatureLabels.size(); i++)
		{
			vector<string> tempVector;
			tempVector.push_back(bcpFeatureLabels.at(i));
			bcpFeatures.push_back(tempVector);
		}
	}

	time(&trainStart);

	// Filtering the inputs using mRMR
//	if(paramMrmrFiltering)
//	{
//		temp = util.clausesToFeatures(mainDataset.at(0).first.first);
//		totalFeatures = count(temp.at(0).begin(), temp.at(0).end(), ',');
//		selectedFeatures = mrmr(temp, totalFeatures);
//		featureTable = util.csvStringToStringVector(temp.at(0));
//	}
	
	// 2.3. Completing intermediary concepts in the examples with answers from the network
	util.examplesBuffer.clear();
	for(unsigned int i = 0; paramUsesRecursion && paramCompleteIntermediateConcepts && haveIntermediateConcepts && (i < mainDataset.at(0).first.second.first.size()); i++)
	{
		clearNetStatus();
		stabilizeNet(DEFAULT_TRANING_STABILIZATION, mainDataset.at(0).first.second.first.at(i), paramDebugMode, logFile);

		buffer.clear();
		for(unsigned int k = 0; k < neuronsLabels.at(OUTPUT_LAYER).size(); k++)
		{
			// If an intermediary concept is not present in an example, its activation after stabilization is saved
			index = util.findAtom(mainDataset.at(0).first.second.first.at(i), neuronsLabels.at(OUTPUT_LAYER).at(k));
			if((util.find(neuronsLabels.at(INPUT_LAYER), neuronsLabels.at(OUTPUT_LAYER).at(k)) >= 0) && (index < 0))
				buffer.push_back(make_pair(k, netStatus.at(OUTPUT_LAYER).at(k)));
		}

		util.examplesBuffer.push_back(make_pair(mainDataset.at(0).first.first.at(i), buffer));
	}

	// 2.4. Adding the (possible) new concepts presented in the training set
	for(unsigned int i = 0; i < mainDataset.at(0).first.first.size(); i++)
	{
		headsWithoutNegation.clear();
		for(unsigned int j = 0; j < mainDataset.at(0).first.second.second.at(i).size(); j++)
			headsWithoutNegation.push_back((mainDataset.at(0).first.second.second.at(i).at(j).at(0) == '~') ? mainDataset.at(0).first.second.second.at(i).at(j).substr(1, mainDataset.at(0).first.second.second.at(i).at(j).size()) : mainDataset.at(0).first.second.second.at(i).at(j));

		// 2.4.1. If a body clause isn't mapped at the input layer, it is created and fully connected with the hidden layer
		for(unsigned int j = 0; j < mainDataset.at(0).first.second.first.at(i).size(); j++)
		{
			bodyWithoutNegation = (mainDataset.at(0).first.second.first.at(i).at(j).at(0) == '~' ? mainDataset.at(0).first.second.first.at(i).at(j).substr(1, mainDataset.at(0).first.second.first.at(i).at(j).size()) : mainDataset.at(0).first.second.first.at(i).at(j));
			bool part1 = (util.find(neuronsLabels.at(INPUT_LAYER), bodyWithoutNegation) == -1);
			//bool part2 = (selectedFeatures.empty() || util.find(selectedFeatures, util.find(featureTable, bodyWithoutNegation)) >= 0);
			bool part2 = true;

			if(part1 && part2)
				addNewInputNeuron(bodyWithoutNegation);
		}

		// 2.4.2. If a head clause isn't mapped at the output layer, it is connected with his correspondent at the input layer, if exists;
		for(unsigned int j = 0; j < headsWithoutNegation.size(); j++)
		{
			if(util.find(neuronsLabels.at(OUTPUT_LAYER), headsWithoutNegation.at(j)) == -1)
			{
				addNewOutputNeuron(headsWithoutNegation.at(j));
				
				// 2.4.3. Connecting all the hidden neurons with the new output neuron
				for(unsigned int k = 0; k < neuronsLabels.at(HIDDEN_LAYER).size(); k++)
				{
					if(noBackgroundKnowledge)
					{
						initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
						if(util.less(abs(initialValue), 0.1 * (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange))
							initialValue *= 10.0;
					}
					else
						initialValue =	0.0;

					connectionsWeights.at(HIDDEN_LAYER).at(k).push_back(make_pair(util.find(neuronsLabels.at(OUTPUT_LAYER), headsWithoutNegation.at(j)), initialValue));
				}
			}
		}
	}

	for(unsigned int i = 0; paramPrintTrainError && (i < neuronsLabels.at(OUTPUT_LAYER).size()); i++)
	{
		fileName = "";
		fileName = fileName.append("files/trainError/").append(neuronsLabels.at(OUTPUT_LAYER).at(i)).append(".log");
		trainErrorFile.push_back(fopen(fileName.c_str(), "a"));
	}

	time(&trainEnd);
	trainTotalTime += difftime(trainEnd, trainStart);

	for(unsigned int i = 0; paramPrintTestError && (i < neuronsLabels.at(OUTPUT_LAYER).size()); i++)
	{
		fileName = "";
		fileName = fileName.append("files/validationError/").append(neuronsLabels.at(OUTPUT_LAYER).at(i)).append(".log");
		trainTestErrorFile.push_back(fopen(fileName.c_str(), "a"));
	}

	time(&trainStart);

	// If the system is not using hidden neuron validation and a starting number has not been defined, it is added as a proportion of (5% of number of input parameters + 5% of training set size)
	if((!paramHiddenNeuronsVariation || !paramNumberOfValidationFolds) && !paramStartingHiddenNeurons)
		addNewHiddenNeurons(floor(neuronsLabels.at(INPUT_LAYER).size() * 0.05) + floor(mainDataset.at(0).first.first.size() * 0.05) - neuronsLabels.at(HIDDEN_LAYER).size());

	time(&trainEnd);
	trainTotalTime += difftime(trainEnd, trainStart);

	// 2.5. Obtaining the training/validation datasets
	unsigned int numberTemp = paramNumberOfValidationFolds;
	numberOfValidationFolds = ((numberTemp > mainDataset.at(0).first.first.size()) ? mainDataset.at(0).first.first.size() : numberTemp);
	if(numberOfValidationFolds > 0)
		datasets = splitDataByTarget(mainDataset.at(0).first.first, mainDataset.at(0).first.second.first, mainDataset.at(0).first.second.second, numberOfValidationFolds, paramTrainPortion);

	time(&trainStart);

	// Applying a small disturbance into the weights, to avoid the symmetry problem
	for(unsigned int i = 0; i < connectionsWeights.size() - 1; i++)
	{
		for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
		{
			for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
				connectionsWeights.at(i).at(j).at(k).second += (((double)rand() / RAND_MAX) - 0.5) * 2.0 * paramWeightInitialRange;
		}
	}
	for(unsigned int i = 0; i < connectionsWeights.size() - 1; i++)
	{
		for(unsigned int j = 0; j < connectionsWeights.at(i+1).size(); j++)
			layersBias.at(i+1).at(j) += (((double)rand() / RAND_MAX) - 0.5) * 2.0 * paramWeightInitialRange;
	}

	// 2.6. Doing the cross-validation routine to choose better parameters

	// The following parameters are being optimized:
	// - number of input neurons
	// - number of hidden neurons

	// Internally (within each fold training), the stop criterion are the following:
	// a) The training stabilized (the weight updates became very small)
	// b) The maximum number of iterations has been used

	// Printing the current dataset info
	printDatasetInfo(datasets, false, "Training/validation dataset");

	time(&trainEnd);
	trainTotalTime += difftime(trainEnd, trainStart);

	if((paramPruningRateFactor > 0) && !util.equals(paramInputPruningVariation, 0.0))
	{
		printf("\n<General> Starting validation on the number of input neurons...\n");
		if(printLog) fprintf(logFile, "\nStarting validation on the number of input neurons...\n");
	}

	// Creating a set of training nets for feature selection
	for(int f = 0; (f < paramPruningRateFactor) && (paramNumberOfValidationFolds != 0); f++)
	{
		Cilp modelChoosingCilp = *this;

		// 2.6.2.1. Applying the chosen validation parameter to train the net
		modelChoosingCilp.pruneInputNeurons((int)(f * paramInputPruningVariation * modelChoosingCilp.connectionsWeights.at(INPUT_LAYER).size()));

		printf("\nValidating %d input neurons...\n", (int)modelChoosingCilp.connectionsWeights.at(INPUT_LAYER).size());
		if(printLog) fprintf(logFile, "\nValidating %d input neurons...\n", (int)modelChoosingCilp.connectionsWeights.at(INPUT_LAYER).size());

		errorValues.push_back(0.0);
		for(int i = 0; i < numberOfValidationFolds; i++)
		{
			Cilp validationCilp = modelChoosingCilp;

			// 2.6.2. Training call
			//trainingNets.at(i).trainExamples(datasets.at(i).first.first, datasets.at(i).first.second.first, datasets.at(i).first.second.second, false, trainErrorFile);
			validationCilp.trainExamples(datasets.at(i), false, false, trainErrorFile, trainTestErrorFile);

			for(unsigned int j = 0; paramPrintTrainError && (j < trainErrorFile.size()); j++)
				fprintf(trainErrorFile.at(j), "\n");

			for(unsigned int j = 0; paramPrintTestError && (j < trainTestErrorFile.size()); j++)
				fprintf(trainTestErrorFile.at(j), "\n");

			// 2.6.3. Evaluating the validation data and calculating total error obtained
			totalError = 0.0;
			totalRight = 0;

			if(!output.empty())
				output.clear();
			if(!errorFunction.empty())
				errorFunction.clear();
			errorFunction.resize(datasets.at(i).second.second.first.size(), 0.0);
			for(unsigned int j = 0; j < datasets.at(i).second.second.first.size(); j++)
			{
				isRight = true;

				validationCilp.clearNetStatus();
				validationCilp.stabilizeNet(DEFAULT_TRANING_STABILIZATION, datasets.at(i).second.second.first.at(j), false, logFile);

				vector<string> data;
				for(unsigned int k = 0; k < datasets.at(i).second.second.second.at(j).size(); k++)
					data.push_back(datasets.at(i).second.second.second.at(j).at(k));

				targetVector = validationCilp.convertData(OUTPUT_LAYER, data);

				// Obtaining the inputs for the output neurons to further analysis
				if(NORMALIZE_WEIGHTS_ONLY)
				{
					if(!currentOutput.empty())
						currentOutput.clear();
					for(unsigned int k = 0; k < validationCilp.netStatus.at(OUTPUT_LAYER).size(); k++)
					{
						currentTotalInput = validationCilp.calculateTotalInput(OUTPUT_LAYER, k, false);
						currentOutput.push_back(currentTotalInput);

						if(!util.equals(targetVector.at(k), 0.0) && (util.equals(validationCilp.evaluateActivation(validationCilp.netStatus.at(OUTPUT_LAYER).at(k), OUTPUT_LAYER, k, currentTotalInput, true, confidence), targetVector.at(k))))
						{
							if(util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cumulativeConfidence += confidence;
							}
						}
						else if(!util.equals(validationCilp.evaluateActivation(validationCilp.netStatus.at(OUTPUT_LAYER).at(k), OUTPUT_LAYER, k, currentTotalInput, true, confidence), targetVector.at(k)))
							isRight = false;

						if(!util.equals(targetVector.at(k), 0.0))
						{
							errorFunction.at(j) += (double)pow(validationCilp.netStatus.at(OUTPUT_LAYER).at(k) - targetVector.at(k), 2)/2.0;
							totalError += (double)errorFunction.at(j)/(double)datasets.at(i).second.second.first.size();
						}
					}

					if(isRight)
						totalRight++;

					totalOutputInputs.push_back(currentOutput);
				}

				output.push_back(validationCilp.netStatus.at(OUTPUT_LAYER));
			}

			//output = trainingNets.at(i).evaluateData(datasets.at(i).second.second.first);

			// 2.6.4. Calculating the total error of the current validation set
	//		totalError = 0.0;
	//		totalRight = 0;
	//		isRight = false;
	//
	//		errorFunction.clear();
	//		errorFunction.resize(output.size(), 0.0);
	//		for(unsigned int j = 0; j < output.size(); j++)
	//		{
	//			vector<string> data(datasets.at(i).second.second.second.begin()+j, datasets.at(i).second.second.second.begin()+j+1);
	//			targetVector = validationCilp.convertData(OUTPUT, data);
	//
	//			for(unsigned int k = 0; k < output.at(j).size(); k++)
	//			{
	//				if(!util.equals(targetVector.at(k), 0.0) && (util.equals(evaluateActivation(output.at(j).at(k), OUTPUT, k, totalOutputInputs.at(j).at(k), true, confidence), targetVector.at(k))))
	//				{
	//					if(stochastic && (util.greater(confidence, 0.0)))
	//					{
	//						numberOfStochasticEvaluations++;
	//						cumulativeConfidence += confidence;
	//					}
	//
	//					isRight = true;
	//				}
	//
	//				errorFunction.at(j) += (double)pow(output.at(j).at(k) - targetVector.at(k), 2)/2.0;
	//				totalError += errorFunction.at(j);
	//			}
	//
	//			if(isRight)
	//				totalRight++;
	//		}

			/////////////////////
			// 2.6.5. Printing the training accuracy
			fprintf(foldAccuracyFile, "%.2f ", ((double)totalRight/output.size())*100.0);

			if(printLog) fprintf(logFile, "Training Fold number %d accuracy: %.2f ", i+1, ((double)totalRight/output.size())*100.0);
			if(stochastic && util.greater(cumulativeConfidence, 0.0))
			{
				if(printLog) fprintf(logFile, "Result Certainty: %.2f", ((double)cumulativeConfidence/numberOfStochasticEvaluations));
			}
			if(printLog) fprintf(logFile, "\n\n");
			////////////////////

			errorValues.at(f) += (double)totalError/paramNumberOfValidationFolds;
		}

		printf("\nFinished validating %d input neurons. MSE obtained = %f\n", (int)modelChoosingCilp.connectionsWeights.at(INPUT_LAYER).size(), errorValues.at(f));
		if(printLog) fprintf(logFile, "\nFinished validating %d input neurons. MSE obtained = %f\n", (int)modelChoosingCilp.connectionsWeights.at(INPUT_LAYER).size(), errorValues.at(f));

	}

	// Applying the found validation amount to the main training network
	if(printLog && ((paramPruningRateFactor > 0) && !util.equals(paramInputPruningVariation, 0.0))) fprintf(logFile, "Input Neuron Pruning Rate Validation Choosing: %.2f\n", (min_element(errorValues.begin(), errorValues.end()) - errorValues.begin()) * paramInputPruningVariation);
	pruneInputNeurons((int)((min_element(errorValues.begin(), errorValues.end()) - errorValues.begin()) * paramInputPruningVariation * connectionsWeights.at(INPUT_LAYER).size()));
	if((paramPruningRateFactor > 0) && !util.equals(paramInputPruningVariation, 0.0)) printf("\n<General> Input neuron amount chosen: %d\n", (int)connectionsWeights.at(INPUT_LAYER).size());

	if(!errorValues.empty())
		errorValues.clear();

	if((paramHiddenNeuronsFactor > 0) && (paramHiddenNeuronsVariation != 0))
	{
		printf("\n<General> Starting validation on the number of hidden neurons...\n");
		if(printLog) fprintf(logFile, "\nStarting validation on the number of hidden neurons...\n");
	}

	// 2.7. Creating a set of training nets for model selection
	for(int f = 0; (f < paramHiddenNeuronsFactor) && (paramNumberOfValidationFolds != 0); f++)
	{
		Cilp modelChoosingCilp = *this;

		// 2.7.1. Applying the chosen validation parameter to train the net
		modelChoosingCilp.addNewHiddenNeurons((int)(f * paramHiddenNeuronsVariation));

		printf("\nValidating %d hidden neurons...\n", (int)modelChoosingCilp.connectionsWeights.at(HIDDEN_LAYER).size());
		if(printLog) fprintf(logFile, "\nValidating %d hidden neurons...\n", (int)modelChoosingCilp.connectionsWeights.at(HIDDEN_LAYER).size());

		errorValues.push_back(0.0);
		for(int i = 0; i < numberOfValidationFolds; i++)
		{
			Cilp validationCilp = modelChoosingCilp;

			// 2.7.2. Training call
			//trainingNets.at(i).trainExamples(datasets.at(i).first.first, datasets.at(i).first.second.first, datasets.at(i).first.second.second, false, trainErrorFile);
			validationCilp.trainExamples(datasets.at(i), false, false, trainErrorFile, trainTestErrorFile);

			for(unsigned int j = 0; paramPrintTrainError && (j < trainErrorFile.size()); j++)
				fprintf(trainErrorFile.at(j), "\n");

			for(unsigned int j = 0; paramPrintTestError && (j < trainTestErrorFile.size()); j++)
				fprintf(trainTestErrorFile.at(j), "\n");

			// 2.7.3. Evaluating the validation data and calculating total error obtained
			totalError = 0.0;
			totalRight = 0;

			if(!output.empty())
				output.clear();
			if(!errorFunction.empty())
				errorFunction.clear();
			errorFunction.resize(datasets.at(i).second.second.first.size(), 0.0);
			for(unsigned int j = 0; j < datasets.at(i).second.second.first.size(); j++)
			{
				isRight = true;

				validationCilp.clearNetStatus();
				validationCilp.stabilizeNet(DEFAULT_TRANING_STABILIZATION, datasets.at(i).second.second.first.at(j), false, logFile);

				vector<string> data;
				for(unsigned int k = 0; k < datasets.at(i).second.second.second.at(j).size(); k++)
					data.push_back(datasets.at(i).second.second.second.at(j).at(k));

				targetVector = validationCilp.convertData(OUTPUT_LAYER, data);

				// Obtaining the inputs for the output neurons to further analysis
				if(NORMALIZE_WEIGHTS_ONLY)
				{
					if(!currentOutput.empty())
						currentOutput.clear();
					for(unsigned int k = 0; k < validationCilp.netStatus.at(OUTPUT_LAYER).size(); k++)
					{
						currentTotalInput = validationCilp.calculateTotalInput(OUTPUT_LAYER, k, false);
						currentOutput.push_back(currentTotalInput);

						if(!util.equals(targetVector.at(k), 0.0) && (util.equals(validationCilp.evaluateActivation(validationCilp.netStatus.at(OUTPUT_LAYER).at(k), OUTPUT_LAYER, k, currentTotalInput, true, confidence), targetVector.at(k))))
						{
							if(util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cumulativeConfidence += confidence;
							}
						}
						else if(!util.equals(validationCilp.evaluateActivation(validationCilp.netStatus.at(OUTPUT_LAYER).at(k), OUTPUT_LAYER, k, currentTotalInput, true, confidence), targetVector.at(k)))
							isRight = false;

						if(!util.equals(targetVector.at(k), 0.0))
						{
							errorFunction.at(j) += (double)pow(validationCilp.netStatus.at(OUTPUT_LAYER).at(k) - targetVector.at(k), 2)/2.0;
							totalError += (double)errorFunction.at(j)/(double)datasets.at(i).second.second.first.size();
						}
					}

					if(isRight)
						totalRight++;

					totalOutputInputs.push_back(currentOutput);
				}

				output.push_back(validationCilp.netStatus.at(OUTPUT_LAYER));
			}

			//output = trainingNets.at(i).evaluateData(datasets.at(i).second.second.first);

			// 2.7.3. Calculating the total error of the current validation set
	//		totalError = 0.0;
	//		totalRight = 0;
	//		isRight = false;
	//
	//		errorFunction.clear();
	//		errorFunction.resize(output.size(), 0.0);
	//		for(unsigned int j = 0; j < output.size(); j++)
	//		{
	//			vector<string> data(datasets.at(i).second.second.second.begin()+j, datasets.at(i).second.second.second.begin()+j+1);
	//			targetVector = validationCilp.convertData(OUTPUT, data);
	//
	//			for(unsigned int k = 0; k < output.at(j).size(); k++)
	//			{
	//				if(!util.equals(targetVector.at(k), 0.0) && (util.equals(evaluateActivation(output.at(j).at(k), OUTPUT, k, totalOutputInputs.at(j).at(k), true, confidence), targetVector.at(k))))
	//				{
	//					if(stochastic && (util.greater(confidence, 0.0)))
	//					{
	//						numberOfStochasticEvaluations++;
	//						cumulativeConfidence += confidence;
	//					}
	//
	//					isRight = true;
	//				}
	//
	//				errorFunction.at(j) += (double)pow(output.at(j).at(k) - targetVector.at(k), 2)/2.0;
	//				totalError += errorFunction.at(j);
	//			}
	//
	//			if(isRight)
	//				totalRight++;
	//		}

			/////////////////////
			// 2.7.4. Printing the training accuracy
			fprintf(foldAccuracyFile, "%.2f ", ((double)totalRight/output.size())*100.0);

			if(printLog) fprintf(logFile, "Training Fold number %d accuracy: %.2f ", i+1, ((double)totalRight/output.size())*100.0);

			if(stochastic && util.greater(cumulativeConfidence, 0.0))
			{
				if(printLog) fprintf(logFile, "Result Certainty: %.2f", ((double)cumulativeConfidence/numberOfStochasticEvaluations));
			}
			if(printLog) fprintf(logFile, "\n\n");
			////////////////////

			errorValues.at(f) += (double)totalError/paramNumberOfValidationFolds;
		}

		printf("\nFinished validating %d hidden neurons. MSE obtained = %f\n", (int)modelChoosingCilp.connectionsWeights.at(HIDDEN_LAYER).size(), errorValues.at(f));
		if(printLog) fprintf(logFile, "\nFinished validating %d hidden neurons. MSE obtained = %f\n", (int)modelChoosingCilp.connectionsWeights.at(HIDDEN_LAYER).size(), errorValues.at(f));
	}

	// Applying the found validation amount to the main training network
	if(printLog && ((paramHiddenNeuronsFactor > 0) && (paramHiddenNeuronsVariation != 0))) fprintf(logFile, "Number of added hidden neurons: %d\n", (int)(min_element(errorValues.begin(), errorValues.end()) - errorValues.begin()) * paramHiddenNeuronsVariation);
	addNewHiddenNeurons((min_element(errorValues.begin(), errorValues.end()) - errorValues.begin()) * paramHiddenNeuronsVariation);
	if((paramHiddenNeuronsFactor > 0) && (paramHiddenNeuronsVariation != 0)) printf("\n<General> Input neuron amount chosen: %d\n\n", (int)connectionsWeights.at(HIDDEN_LAYER).size());

	time(&trainStart);

	// Printing the network structure
	printf("Network structure after receiving training data:\n");
	printf("Number of Input neurons: %d\n", (int)neuronsLabels.at(0).size());
	printf("Number of Hidden neurons: %d\n", (int)neuronsLabels.at(1).size());
	printf("Number of Output neurons: %d\n\n", (int)neuronsLabels.at(2).size());

	if(printLog) fprintf(logFile, "\n");

	trainExamples(mainDataset.at(0), paramDebugMode, true, trainErrorFile, trainTestErrorFile);

	for(unsigned int j = 0; paramPrintTrainError && (j < trainErrorFile.size()); j++)
	{
		fprintf(trainErrorFile.at(j), "\n\n");
		fclose(trainErrorFile.at(j));
	}

	for(unsigned int j = 0; paramPrintTestError && (j < trainTestErrorFile.size()); j++)
	{
		fprintf(trainTestErrorFile.at(j), "\n\n");
	 	fclose(trainTestErrorFile.at(j));
	}

	if(printLog) fclose(logFile);
	fclose(foldAccuracyFile);

	time(&trainEnd);
	trainTotalTime += difftime(trainEnd, trainStart);
}

// Method responsible to train the net with example files
void Cilp::trainExamples(pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > dataset, bool printLog, bool printProgress, vector<FILE*> trainErrorFile, vector<FILE*> trainTestErrorFile)
{
	int trainingIndex, updateIndex, batchSize, epochRuns, chosenEpoch, epochRunsSoFar = 1;
	bool fileExists, canContinue, end = false;
	double initialLearningRate, leastValidationError;

	FILE *logFile = NULL;

	time_t secondsQt;
	struct tm *currentTimestamp;

	string headStr, bodyStr;

	vector<int> numberOfImprovements, lastNumberOfHits;
	vector<double> currentM, errorVector, newDelta, newBias, alpha, lastAlpha, effectiveTrainError, errorValidation, testingVector, trainingVector, newVector, cummulativeTestsetError, cummulativeTrainsetError;

	// Data balancing parameters: uncomment this after implementing the commented part below
	vector<double> minorityClass, majorityClass, currentInput, currentOutput;

	vector<vector<double> > input, positiveInput, negativeInput, output, outputs, m, deltaBias, previousBiasMomentum, currentBiasMomentum, labels, outputInputs, trainingVectors, errorFunction, bestLayersBias, lastLayersBias;
	vector<vector<vector<double> > > deltaW, previousBiasWeights, currentWeightMomentum;
	vector<vector<vector<pair<int, double> > > > bestConnectionsWeights, lastConnectionsWeights;

	vector<string> temp, trainedExamples, testedExamples;
	vector<vector<string> > bodies, testedBodies, heads, testedHeads;

	vector<pair<int, double> > newWeightVector;
	vector<vector<pair<int, double> > > newNodeWeightVector;

	activationFunction usedFunction;

	// 0. Splitting the dataset into training data and testing data
	trainedExamples = dataset.first.first;
	bodies = dataset.first.second.first;
	heads = dataset.first.second.second;

	testedExamples = dataset.second.first;
	testedBodies = dataset.second.second.first;
	testedHeads = dataset.second.second.second;

	// 1. Initializing the training logs
	fileExists = util.fileExists("files/main.log");

	if(paramDebugMode) printLog = true;

	if(printLog) logFile = fopen("files/main.log", "a");

	if(!fileExists)
	{
		time(&secondsQt);
		currentTimestamp = localtime(&secondsQt);

		if(printLog) fprintf(logFile, "CILP++, Version %0.1f\n", VERSION);
		if(printLog) fprintf(logFile, "Author: %s\n\n", AUTHOR);
		if(printLog) fprintf(logFile, "*Execution Log* %s\n\n", asctime(currentTimestamp));
	}

	// 2. Normalizing the net weights
	//normalizeWeights();
//	if(printLog) fprintf(logFile, "Normalizing the weights\n");
//	if(printLog) fprintf(logFile, "Weight factors:\n");
//	for(unsigned int i = 0; i < normalizationFactors.size(); i++)
//	{
//		if(printLog) fprintf(logFile, "Layer %d\n", i+1);
//
//		for(unsigned int j = 0; j < normalizationFactors.at(i).size(); j++)
//		{
//			if(printLog) fprintf(logFile, "%s: %f\n", neuronsLabels.at(i).at(j).c_str(), normalizationFactors.at(i).at(j));
//		}
//
//		if(printLog) fprintf(logFile, "\n");
//	}

	// 3. Doing the net training, using the defined parameters
	trainingIndex = 0;
	chosenEpoch = -1;
	updateIndex = 0;
	epochRuns = 0;

	numberOfImprovements.clear();
	numberOfImprovements.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0);

	lastNumberOfHits.clear();
	lastNumberOfHits.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0);

	initialLearningRate = learningRate;

	newVector.clear();
	newVector.push_back(0.0);
	errorFunction.clear();
	errorFunction.resize(neuronsLabels.at(OUTPUT_LAYER).size(), newVector);

	unsigned int sizeTemp = paramBatchSize;
	batchSize = ((sizeTemp > bodies.size()) ? bodies.size() : sizeTemp);
	leastValidationError = FLT_MAX;

	// Converting all data to input vectors
//	for(unsigned int i = 0; i < bodies.size(); i++)
//	{
//		input.push_back(convertInputData(INPUT_LAYER, bodies.at(i), trainedExamples.at(i)));

		// Uncomment here for implementing data balancing

//		// Collecting examples to verify if over-sampling will be required (currently applicable only for binary classification)
//		if(heads.size() == 1)
//		{
//			if(heads.at(0).at(0) == '~')
//				negativeInput.push_back(input.at(i));
//			else
//				positiveInput.push_back(input.at(i));
//		}
//	}

	// Uncomment here for implementing data balancing

	// Verifying if over-sampling will be required
//	minorityClass = (positiveInput.size() < negativeInput.size() ? positiveInput.size(): negativeInput.size());
//	majorityClass = (positiveInput.size() > negativeInput.size() ? positiveInput.size(): negativeInput.size());
//
//	// Verifying if data balancing is required
//	if((majorityClass.size() - minorityClass.size()) > abs(((3 * minorityClass.size())/2) - majorityClass.size()))
//	{
//
//
//	}

 	while(!end)
	{
		if(printLog) fprintf(logFile, "Update number %d\n\n", updateIndex+1);
		canContinue = true;

		for(int e = 0; e < batchSize && canContinue; e++)
		{
			if(!input.empty())
				input.clear();
			if(!output.empty())
				output.clear();
			if(!m.empty())
				m.clear();

			// 3.1. Converting the example to a numeric vector
			currentInput = convertInputData(INPUT_LAYER, bodies.at(trainingIndex), trainedExamples.at(trainingIndex));

			// 3.2. Feed-forward iteration (doing generalized delta rule, for training time reduction)
			for(int i = 0; i < numberOfLayers; i++)
			{
				if(!currentOutput.empty())
					currentOutput.clear();
				if(!currentM.empty())
					currentM.clear();

				usedFunction = (i == (INPUT_LAYER) ? linear : bipolar);

				// 3.2.1. Calculating the outputs for the current layer and the derivatives
				for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
				{
					currentOutput.push_back(calculateActivationFunction(currentInput.at(j) + layersBias.at(i).at(j), usedFunction));
					currentM.push_back(calculateActivationDerivative(currentInput.at(j) + layersBias.at(i).at(j), usedFunction));
				}
				output.push_back(currentOutput);
				m.push_back(currentM);

				// 3.2.2. Calculating the inputs for the next layer if there is a next one
				if(i != (OUTPUT_LAYER))
				{
					if(!currentInput.empty())
						currentInput.clear();
					currentInput.resize(connectionsWeights.at((i+1) % numberOfLayers).size(), 0.0);

					for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
					{
						for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
							currentInput.at(connectionsWeights.at(i).at(j).at(k).first) += (connectionsWeights.at(i).at(j).at(k).second * currentOutput.at(j));
					}
				}
			}

			if(printLog) fprintf(logFile, "Feed-forwarding of training data %d is done\n", trainingIndex);

			// 3.3. Calculating the error and cost function total
			// 		  Cost function = ((e*e)^2)/2
			if(!temp.empty())
				temp.clear();
			for(unsigned int i = 0; i < heads.at(trainingIndex).size(); i++)
			{
				temp.push_back(heads.at(trainingIndex).at(i));
			}
			errorVector = convertData(OUTPUT_LAYER, temp);

			if(!trainingIndex)
			{
				if(!labels.empty())
					labels.clear();
				if(!outputs.empty())
					outputs.clear();
			}

			labels.push_back(errorVector);
			outputs.push_back(output.at(OUTPUT_LAYER));

			for(unsigned int i = 0; i < errorVector.size(); i++)
			{
				if(!util.equals(errorVector.at(i), 0.0))
					errorVector.at(i) -= output.at(OUTPUT_LAYER).at(i);

				errorFunction.at(i).at(updateIndex) += ((double)pow(errorVector.at(i), 2)/(2.0 * (double)batchSize));
			}

			// 3.4. Back-propagation iteration
			for(int i = numberOfLayers; i >= 1; i--)
			{
				if(!alpha.empty())
					alpha.clear();

				// 3.4.1. Preparing the initial data for BP loop
				if(i == numberOfLayers)
				{
					if(!lastAlpha.empty())
						lastAlpha.clear();

					lastAlpha.resize(errorVector.size());
					copy(errorVector.begin(), errorVector.end(), lastAlpha.begin());

					vector<vector<double> > tempDeltaW;
					if(!deltaW.empty())
						deltaW.clear();
					deltaW.resize(numberOfLayers, tempDeltaW);

					if(!deltaBias.empty())
						deltaBias.clear();

					for(int j = 0; j < numberOfLayers; j++)
					{
						if(!newDelta.empty())
							newDelta.clear();
						if(!newBias.empty())
							newBias.clear();

						for(unsigned int k = 0; k < connectionsWeights.at(j).size(); k++)
						{
							if(!newDelta.empty())
								newDelta.clear();
							newDelta.resize(connectionsWeights.at(j).at(k).size(), 0.0);

							deltaW.at(j).push_back(newDelta);
						}

						if(!newBias.empty())
							newBias.clear();
						newBias.resize(connectionsWeights.at(j).size(), 0.0);

						deltaBias.push_back(newBias);
					}
				}
				else
				{
					// 3.4.2. Adding the current update to the update total
					for(unsigned int k = 0; k < neuronsLabels.at(i).size(); k++)
					{
						alpha.push_back(0.0);
						for(unsigned int l = 0; l < connectionsWeights.at(i).at(k).size() || ((i == OUTPUT_LAYER) && l < 1); l++)
						{
							alpha.at(k) = alpha.at(k) + m.at(i).at(k) * (i == OUTPUT_LAYER ? 1.0 : connectionsWeights.at(i).at(k).at(l).second) * lastAlpha.at((i != OUTPUT_LAYER) ? connectionsWeights.at(i).at(k).at(l).first : k);
						}

						deltaBias.at(i).at(k) += ((double)alpha.at(k));

						for(unsigned int m = 0; m < connectionsWeights.at((i-1) % numberOfLayers).size(); m++)
							deltaW.at((i-1) % numberOfLayers).at(m).at(k) += ((double)(alpha.at(k) * output.at((i-1) % numberOfLayers).at(m)));
					}

					// 3.4.3. Preparing the next iteration
					if(!lastAlpha.empty())
						lastAlpha.clear();
					lastAlpha.resize(alpha.size());
					copy(alpha.begin(), alpha.end(), lastAlpha.begin());
				}
			}


			if(printLog) fprintf(logFile, "Back-propagation of training data %d is done\nNumber of examples = %d, Data = %s\n\n", trainingIndex, (int)bodies.size(), trainedExamples.at(trainingIndex).c_str());

			trainingIndex++;
			if(trainingIndex >= ((int)bodies.size()))
			{
				// Correcting the error function (if the batch size is not multiple of training set size)
				for(unsigned int i = 0; i < errorFunction.size(); i++)
				{
					if(trainingIndex % batchSize)
						errorFunction.at(i).at(updateIndex) *= ((double)batchSize/(trainingIndex % batchSize));
				}

				// If the whole data is already used, it will be reshuffled
				if(batchSize != (int)bodies.size())
					shuffleTrainData(trainedExamples, heads, bodies);
				epochRuns++;
				trainingIndex = 0;
				canContinue = false;

				// Printing on-screen progress
				if(printProgress && (epochRuns >= ((paramEpochRuns / 10) * epochRunsSoFar)) && (epochRunsSoFar < 10))
				{
					printf("%d%% of max epochs trained so far...\n", epochRunsSoFar * 10);
					epochRunsSoFar++;
				}

				if(printProgress && (epochRuns == paramEpochRuns))
					printf("100%% of epochs have been trained, stopping training...\n");
			}
		}

		// 3.5. Applying the updates
		if(!currentWeightMomentum.empty())
			currentWeightMomentum.clear();
		if(!currentBiasMomentum.empty())
			currentBiasMomentum.clear();

		if(printLog) fprintf(logFile, "Updating the net\n\n");

		for(int i = 0; i < numberOfLayers; i++)
		{
			vector<vector<double> > tempWeightMomentum(connectionsWeights.at(i).size());
			currentWeightMomentum.push_back(tempWeightMomentum);

			vector<double> tempBiasMomentum(connectionsWeights.at(i).size());
			currentBiasMomentum.push_back(tempBiasMomentum);

			for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
			{
				vector<double> tempWeightMomentum2(connectionsWeights.at(i).at(j).size());
				currentWeightMomentum.at(i).at(j) = tempWeightMomentum2;

				// 3.5.1. Updating the biases
				currentBiasMomentum.at(i).at(j) = (learningRate * ((double)deltaBias.at(i).at(j))) + (paramMomentum * (!updateIndex ? 0.0 : ((double)previousBiasMomentum.at(i).at(j))));
				layersBias.at(i).at(j) += currentBiasMomentum.at(i).at(j);

				if(!util.equals(deltaBias.at(i).at(j), 0.0))
				{
					if(printLog) fprintf(logFile, "Current bias for layer %d, neuron %s: %f\n", i, neuronsLabels.at(i).at(j).c_str(), layersBias.at(i).at(j));
					if(printLog) fprintf(logFile, "Delta bias for layer %d, neuron %s: %f\n\n", i, neuronsLabels.at(i).at(j).c_str(), currentBiasMomentum.at(i).at(j));
				}

				// 3.5.2. Updating the weights
				for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
				{
					currentWeightMomentum.at(i).at(j).at(k) = (learningRate * ((double)deltaW.at(i).at(j).at(k))) + (paramMomentum * (!updateIndex ? 0.0 : ((double)previousBiasWeights.at(i).at(j).at(k))));
					connectionsWeights.at(i).at(j).at(k).second += currentWeightMomentum.at(i).at(j).at(k);

					if(!util.equals(deltaW.at(i).at(j).at(k), 0.0))
					{
						if(printLog) fprintf(logFile, "Current weight for layer %d, neurons %s -- %s: %f\n", i, neuronsLabels.at(i).at(j).c_str(), neuronsLabels.at((i+1)%3).at(connectionsWeights.at(i).at(j).at(k).first).c_str(), connectionsWeights.at(i).at(j).at(k).second);
						if(printLog) fprintf(logFile, "Delta weight for layer %d, neurons %s -- %s: %f\n\n", i, neuronsLabels.at(i).at(j).c_str(), neuronsLabels.at((i+1)%3).at(connectionsWeights.at(i).at(j).at(k).first).c_str(), currentWeightMomentum.at(i).at(j).at(k));
					}
				}
			}
		}

		if(printLog) fprintf(logFile, "Net updating is finished\n\n");

		// 3.6. Testing the last update with testing data
		cummulativeTestsetError.clear();
		cummulativeTestsetError.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0.0);
		if(!trainingIndex && (paramPrintTestError || (paramSimpleStopCriterion && (paramTrainingType == earlyStopping))))
		{
			for(unsigned int d = 0; d < testedBodies.size(); d++)
			{
				if(!output.empty())
					output.clear();
				currentInput = convertInputData(INPUT_LAYER, testedBodies.at(d), testedExamples.at(d));

				// 3.6.1. Doing a feed-forward iteration to get the testing result
				for(int i = 0; i < numberOfLayers; i++)
				{
					if(!currentOutput.empty())
						currentOutput.clear();
					usedFunction = (i == (INPUT_LAYER) ? linear : bipolar);

					// 3.6.1.1 Calculating the outputs for the current layer and the derivatives
					for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
						currentOutput.push_back(calculateActivationFunction(currentInput.at(j) + layersBias.at(i).at(j), usedFunction));

					output.push_back(currentOutput);

					// 3.6.1.2. Calculating the inputs for the next layer if there is a next one
					if(i != (OUTPUT_LAYER))
					{
						if(!currentInput.empty())
							currentInput.clear();
						currentInput.resize(connectionsWeights.at((i+1) % numberOfLayers).size(), 0.0);

						for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
						{
							for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
								currentInput.at(connectionsWeights.at(i).at(j).at(k).first) += (connectionsWeights.at(i).at(j).at(k).second * currentOutput.at(j));
						}
					}
				}

				// 3.6.2. Getting the average output error for the testing data
				if(!temp.empty())
					temp.clear();
				for(unsigned int i = 0; i < testedHeads.at(d).size(); i++)
					temp.push_back(testedHeads.at(d).at(i));

				testingVector = convertData(OUTPUT_LAYER, temp);

				for(unsigned int i = 0; i < testingVector.size(); i++)
				{
					if(!util.equals(testingVector.at(i), 0.0))
						testingVector.at(i) -= output.at(OUTPUT_LAYER).at(i);

					testingVector.at(i) = ((double)pow(testingVector.at(i), 2)/2.0);
					cummulativeTestsetError.at(i) += testingVector.at(i);
				}
			}
		}

		// Keeping the best validation error per epoch obtained
		if(!trainingIndex && paramSimpleStopCriterion && (paramTrainingType == earlyStopping))
		{
			double averageTestsetError = 0.0;

			errorValidation.push_back(0.0);
			for(unsigned int i = 0; i < testingVector.size(); i++)
			{
				errorValidation.at(errorValidation.size()-1) += (double)cummulativeTestsetError.at(i)/(testingVector.size() * testedBodies.size());
				averageTestsetError += (double)cummulativeTestsetError.at(i)/testingVector.size();
			}

			if(util.less((double)averageTestsetError/testedBodies.size(), leastValidationError))
			{
				leastValidationError = (double)averageTestsetError/testedBodies.size();
				bestConnectionsWeights = connectionsWeights;
				bestLayersBias = layersBias;
				chosenEpoch = epochRuns;
			}
		}

		if(!trainingIndex && paramPrintTestError)
		{
			for(unsigned int i = 0; i < trainTestErrorFile.size(); i++)
				fprintf(trainTestErrorFile.at(i), "%f ", (double)cummulativeTestsetError.at(i)/(testingVector.size() * testedBodies.size()));
		}

		// 3.7. Testing the last update with training data
		cummulativeTrainsetError.clear();
		cummulativeTrainsetError.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0.0);

		if(!trainingVectors.empty())
			trainingVectors.clear();
		if((!paramSimpleStopCriterion || paramPrintTrainError) && !trainingIndex)
		{
			outputs.clear();
			labels.clear();

			for(unsigned int d = 0; d < bodies.size(); d++)
			{
				if(!output.empty())
					output.clear();
				currentInput = convertInputData(INPUT_LAYER, bodies.at(d), trainedExamples.at(d));

				// 3.7.1. Doing a feed-forward iteration to get the testing result
				for(int i = 0; i < numberOfLayers; i++)
				{
					if(!currentOutput.empty())
						currentOutput.clear();
					usedFunction = (i == (INPUT_LAYER) ? linear : bipolar);

					// 3.7.1.1 Calculating the outputs for the current layer and the derivatives
					for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
						currentOutput.push_back(calculateActivationFunction(currentInput.at(j) + layersBias.at(i).at(j), usedFunction));

					output.push_back(currentOutput);

					// 3.7.1.2. Calculating the inputs for the next layer if there is a next one
					if(i != (OUTPUT_LAYER))
					{
						if(!currentInput.empty())
							currentInput.clear();
						currentInput.resize(connectionsWeights.at((i+1) % numberOfLayers).size(), 0.0);

						for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
						{
							for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
								currentInput.at(connectionsWeights.at(i).at(j).at(k).first) += (connectionsWeights.at(i).at(j).at(k).second * currentOutput.at(j));
						}
					}
				}

				if(!temp.empty())
					temp.clear();
				for(unsigned int i = 0; i < heads.at(d).size(); i++)
					temp.push_back(heads.at(d).at(i));
				trainingVector = convertData(OUTPUT_LAYER, temp);

				labels.push_back(trainingVector);
				outputs.push_back(output.at(OUTPUT_LAYER));

				for(unsigned int i = 0; i < trainingVector.size(); i++)
				{
					if(!util.equals(trainingVector.at(i), 0.0))
						trainingVector.at(i) -= output.at(OUTPUT_LAYER).at(i);

					trainingVector.at(i) = ((double)pow(trainingVector.at(i), 2)/2.0);
					cummulativeTrainsetError.at(i) += trainingVector.at(i);
				}

				trainingVectors.push_back(trainingVector);
			}
		}
		else
		{
			for(unsigned int i = 0; i < errorFunction.size(); i++)
				cummulativeTrainsetError.at(i) = errorFunction.at(i).at(updateIndex) * bodies.size();
		}

		if(paramPrintTrainError && !trainingIndex)
		{
			for(unsigned int i = 0; i < cummulativeTrainsetError.size(); i++)
				fprintf(trainErrorFile.at(i), "%f ", (double)cummulativeTrainsetError.at(i)/(bodies.size() * errorFunction.size()));
		}

		// 3.8. Keeping the last update made
		if(!previousBiasMomentum.empty())
			previousBiasMomentum.clear();
		previousBiasMomentum = currentBiasMomentum;

		if(!previousBiasWeights.empty())
			previousBiasWeights.clear();
		previousBiasWeights = currentWeightMomentum;

		// 3.9. Checking stop criterion
		if(!paramSimpleStopCriterion)
		{
			if (checkStopCriterion(labels, outputs, trainingVectors, trainingIndex, epochRuns, numberOfImprovements, lastNumberOfHits))
				end = true;
			else
			{
				updateIndex++;
				for(unsigned int i = 0; i < errorFunction.size(); i++)
					errorFunction.at(i).push_back(0.0);
			}
		}
		else
		{
			if (checkStopCriterionSimple(paramTrainingType, errorFunction, errorValidation, leastValidationError, trainingIndex, epochRuns))
			{
				// 3.9.1. Getting the best weight vector obtained so far, regarding validation set
				if(!util.equals(leastValidationError, FLT_MAX) && (paramTrainingType == earlyStopping))
				{
					connectionsWeights = bestConnectionsWeights;
					layersBias = bestLayersBias;

					if(printLog && util.less(paramTrainPortion, 1.0)) fprintf(logFile, "Chosen Epoch: %d\n\n", chosenEpoch);
				}

				end = true;
			}
			else
			{
				updateIndex++;
				for(unsigned int i = 0; i < errorFunction.size(); i++)
					errorFunction.at(i).push_back(0.0);
			}
		}

		// 3.10. Updating the learning rate (if an epoch has been executed)
		if(!trainingIndex)
			learningRate *= paramDecayFactor;
	}

 	// 4. Finishihg the training
 	// 4.1 Returning all the weights beck to their normal values
 	//revertWeights();

 	// 4.2. Doing the rest
 	learningRate = initialLearningRate;

 	if(printProgress)
 	{
 		if(epochRuns < paramEpochRuns)
 			printf("Training stopped at epoch #%d\n", epochRuns);

 		printf("\n");
 	}

 	if(printLog) fprintf(logFile, "Training ended\n\n");
 	if(printLog) fclose(logFile);
}

// Method to test data in the trained net
vector<vector<double> > Cilp::evaluateData(vector<vector<string> > bodies)
{
//	bool fileExists;
//
//	FILE *logFile;
//
//	time_t secondsQt;
//	struct tm *currentTimestamp;

	string bodyStr;

	vector<double> currentInput, currentOutput;
	vector<vector<double> > input, output;

	vector<string> temp;

	activationFunction usedFunction;

	// Initialization
	output.reserve(1);

	// 1. Initializing the file log
//	fileExists = util.fileExists("files/main.log");
//	logFile = fopen("files/main.log", "a");
//
//	if(!fileExists)
//	{
//		time(&secondsQt);
//		currentTimestamp = localtime(&secondsQt);
//
//		if(printLog) fprintf(logFile, "CILP++, Version %0.1f\n", VERSION);
//		if(printLog) fprintf(logFile, "Author: Manoel Vitor Macedo Franca\n\n");
//		if(printLog) fprintf(logFile, "*Execution Log* %s\n\n", asctime(currentTimestamp));
//	}
//
//	if(printLog) fprintf(logFile, "|--------------------|\n");
//	if(printLog) fprintf(logFile, "|Evaluating the data:|\n");
//	if(printLog) fprintf(logFile, "|--------------------|\n\n");

	// 2.1. Doing the net evaluation, using the defined parameters
	for(unsigned int testIndex = 0; testIndex < bodies.size(); testIndex++)
	{
		// 2.1.1. Converting the example to a numeric vector
		currentInput = convertInputData(INPUT_LAYER, bodies.at(testIndex), "");

		// 2.1.2. Feed-forward iteration
		for(int i = 0; i < numberOfLayers; i++)
		{
			if(!currentOutput.empty())
				currentOutput.clear();

			usedFunction = (i == (INPUT_LAYER) ? linear : bipolar);

			// 2.1.2.1. Calculating the outputs for the current layer
			for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
				currentOutput.push_back(calculateActivationFunction(currentInput.at(j) + layersBias.at(i).at(j), usedFunction));

			// 2.1.2.2. Calculating the inputs for the next layer if there is a next one
			if(i != (OUTPUT_LAYER))
			{
				currentInput = *(new vector<double>(connectionsWeights.at((i+1) % numberOfLayers).size()));
				fill(currentInput.begin(), currentInput.end(), 0.0);
				for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
				{
					for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
						currentInput.at(connectionsWeights.at(i).at(j).at(k).first) += (connectionsWeights.at(i).at(j).at(k).second * currentOutput.at(j));
				}
			}
		}

		output.push_back(currentOutput);
	}

	return output;
}

// Method to test data in the trained net
vector<pair<pair<double, double>, confusionTable> > Cilp::testNet(vector<string> testedExamples, bool printFile)
{
	bool fileExists;
	int numberOfPositiveHits, numberOfNegativeHits, numberOfMixedHits, numberOfFalsePositives, numberOfFalseNegatives, numberOfMixedAnswers, numberOfStochasticEvaluations, currentNumberOfPositiveHits, currentNumberOfNegativeHits, currentNumberOfFalsePositives, currentNumberOfFalseNegatives;
	double confidence, normalizedAmin, cummulativeConfidence, precision, recall;

	FILE *logFile = NULL;

	time_t secondsQt;
	struct tm *currentTimestamp;

	vector<pair<pair<double, double>, confusionTable> > results;

	string bodyStr;

	vector<int> numberOfFalsePositivesPerTest, numberOfFalseNegativesPerTest, numberOfMixedAnswersPerTest, numberOfStochasticEvaluationsPerTest, positiveHitsPerTest, negativeHitsPerTest, mixedHitsPerTest, exampleCountPerHead, numberOfIterations;
	vector<vector<int> >  exampleCountsPerHead;

	vector<double> currentInput, currentOutput, currentM, newDelta, newBias, alpha, lastAlpha, cummulativeConfidencePerTest, cummulativeFMeasurePerTest, hitProportion;
	vector<vector<double> > input, output, m, deltaBias, previousDeltaBias, headsHitProportion;
	vector<vector<vector<double> > > deltaW, previousDeltaW;

	vector<string> temp, headsWithoutNegation, hitProportionLabels;
	vector<vector<string> > bodies, heads;

	vector<pair<int, double> > newWeightVector;
	vector<vector<pair<int, double> > > newNodeWeightVector;

	pair<vector<vector<string> >, vector<vector<string> > > dataRead;

	neuronStatus status;

	// 1. Initializing the files
	fileExists = util.fileExists("files/main.log");

	if(paramDebugMode) printFile = true;

	if(printFile) logFile = fopen("files/main.log", "a");

	if(!fileExists)
	{
		time(&secondsQt);
		currentTimestamp = localtime(&secondsQt);

		if(printFile) fprintf(logFile, "CILP++, Version %0.1f\n", VERSION);
		if(printFile) fprintf(logFile, "Author: %s\n\n", AUTHOR);
		if(printFile) fprintf(logFile, "*Execution log* %s\n\n", asctime(currentTimestamp));
	}

	// 2. Stabilizing the net
	//stabilizeNet("");

	// 3. Analyzing the testing examples file
	if(printFile) fprintf(logFile, "|----------------|\n");
	if(printFile) fprintf(logFile, "|Testing the net:|\n");
	if(printFile) fprintf(logFile, "|----------------|\n\n");

	// Test clause format: T :- L1,L2,L3,...,Ln
	if(printFile) fprintf(logFile, "Testing examples, please wait...\n");
	dataRead = decodeData(testedExamples);
	bodies = dataRead.first;
	heads = dataRead.second;

	hitProportion.clear();
	hitProportion.resize(neuronsLabels.at(OUTPUT_LAYER).size(), -1);

	exampleCountPerHead.clear();
	exampleCountPerHead.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0);

	headsHitProportion.resize(STABILIZATION_TYPE_SIZE, hitProportion);
	exampleCountsPerHead.resize(STABILIZATION_TYPE_SIZE, exampleCountPerHead);

	positiveHitsPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	negativeHitsPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	mixedHitsPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	numberOfFalsePositivesPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	numberOfFalseNegativesPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	numberOfMixedAnswersPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	numberOfStochasticEvaluationsPerTest.resize(STABILIZATION_TYPE_SIZE, 0);
	cummulativeConfidencePerTest.resize(STABILIZATION_TYPE_SIZE, 0.0);
	cummulativeFMeasurePerTest.resize(STABILIZATION_TYPE_SIZE, 0.0);

	// 4. Doing 4 testings using the testing data:
	// - regular simple test;
	// - regular stochastic test;
	// - stable simple test;
	// - stable stochastic test;
	for(unsigned int i = 0; i < bodies.size(); i++)
	{
		//////////////////////////////
		// 4.1. Regular Simple test //
		//////////////////////////////

		numberOfPositiveHits = 0;
		numberOfNegativeHits = 0;
		numberOfMixedHits = 0;
		numberOfFalseNegatives = 0;
		numberOfFalsePositives = 0;
		numberOfMixedAnswers = 0;
		numberOfStochasticEvaluations = 0;
		cummulativeConfidence = 0.0;

		currentNumberOfPositiveHits = 0;
		currentNumberOfNegativeHits = 0;
		currentNumberOfFalseNegatives = 0;
		currentNumberOfFalsePositives = 0;

		fprintf(logFile, "Doing Regular Simple testing:\n\n");

		clearNetStatus();
		numberOfIterations.push_back(stabilizeNet(regularSimple, bodies.at(i), paramDebugMode, logFile));

		if(printFile) fprintf(logFile, "Test example %s:", testedExamples.at(i).c_str());

		for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
		{
			headsWithoutNegation.clear();
			for(unsigned int k = 0; k < heads.at(i).size(); k++)
				headsWithoutNegation.push_back(((heads.at(i).at(k).at(0) == '~') ? heads.at(i).at(k).substr(1) : heads.at(i).at(k)));

			for(unsigned int k = 0; k < heads.at(i).size(); k++)
			{
				if(!headsWithoutNegation.at(k).compare(neuronsLabels.at(OUTPUT_LAYER).at(j)))
				{
					status = returnNeuronStatus(OUTPUT_LAYER, neuronsLabels.at(OUTPUT_LAYER).at(j), false, confidence, normalizedAmin);
					if(printFile) fprintf(logFile, "\nActivation: %.2f\n", netStatus.at(OUTPUT_LAYER).at(j));
					if(printFile) fprintf(logFile, "Used Amin: %.2f\n", normalizedAmin);

					if((status == activated) && (heads.at(i).at(k).at(0) != '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfPositiveHits++;

						if(util.less(headsHitProportion.at(regularSimple).at(k), 0.0)) headsHitProportion.at(regularSimple).at(k) = 0.0;
						exampleCountsPerHead.at(regularSimple).at(k) += 1;
						headsHitProportion.at(regularSimple).at(k) += 1.0;
					}
					else if((status == desactivated) && (heads.at(i).at(k).at(0) == '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfNegativeHits++;

						if(util.less(headsHitProportion.at(regularSimple).at(k), 0.0)) headsHitProportion.at(regularSimple).at(k) = 0.0;
						exampleCountsPerHead.at(regularSimple).at(k) += 1;
						headsHitProportion.at(regularSimple).at(k) += 1.0;
					}
					else if((status == activated) && (heads.at(i).at(k).at(0) == '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfFalsePositives++;

						if(util.less(headsHitProportion.at(regularSimple).at(k), 0.0)) headsHitProportion.at(regularSimple).at(k) = 0.0;
						exampleCountsPerHead.at(regularSimple).at(k) += 1;
					}
					else if((status == desactivated) && (heads.at(i).at(k).at(0) != '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfFalseNegatives++;

						if(util.less(headsHitProportion.at(regularSimple).at(k), 0.0)) headsHitProportion.at(regularSimple).at(k) = 0.0;
						exampleCountsPerHead.at(regularSimple).at(k) += 1;
					}
				}
			}
		}

		// Checking if all the heads are guessed right
		if((currentNumberOfPositiveHits + currentNumberOfNegativeHits) == (int)heads.at(i).size())
		{
			if(stochastic && util.greater(confidence, 0.0))
			{
				if(printFile) fprintf(logFile, " hit (%.2f%% certainty)\n", cummulativeConfidence);
			}
			else
			{
				if(printFile) fprintf(logFile, " hit\n");
			}

			// Verifying if the analyzed example has multiple heads with multiple classes
			if(currentNumberOfPositiveHits && currentNumberOfNegativeHits)
				numberOfMixedHits++;
			else if(currentNumberOfPositiveHits)
				numberOfPositiveHits++;
			else if(currentNumberOfNegativeHits)
				numberOfNegativeHits++;
		}
		else
		{
			if(stochastic && util.greater(confidence, 0.0))
			{
				if(printFile) fprintf(logFile, " miss (%.2f%% certainty)\n", cummulativeConfidence);
			}
			else
			{
				if(printFile) fprintf(logFile, " miss\n");
			}

			if((currentNumberOfFalseNegatives > 0) && (currentNumberOfFalsePositives > 0))
				numberOfMixedAnswers++;
			else
			{
				if(currentNumberOfFalseNegatives > 0)
					numberOfFalseNegatives++;
				else if(currentNumberOfFalsePositives > 0)
					numberOfFalsePositives++;
			}
		}

		if(printFile) fprintf(logFile, "\n");

		positiveHitsPerTest.at(regularSimple) += numberOfPositiveHits;
		negativeHitsPerTest.at(regularSimple) += numberOfNegativeHits;
		mixedHitsPerTest.at(regularSimple) += numberOfMixedHits;
		numberOfFalsePositivesPerTest.at(regularSimple) += numberOfFalsePositives;
		numberOfFalseNegativesPerTest.at(regularSimple) += numberOfFalseNegatives;
		numberOfMixedAnswersPerTest.at(regularSimple) += numberOfMixedAnswers;
		numberOfStochasticEvaluationsPerTest.at(regularSimple) += numberOfStochasticEvaluations;
		cummulativeConfidencePerTest.at(regularSimple) += cummulativeConfidence;

		//////////////////////////////
		// 4.2. Regular Stable test //
		//////////////////////////////

		numberOfPositiveHits = 0;
		numberOfNegativeHits = 0;
		numberOfMixedHits = 0;
		numberOfFalseNegatives = 0;
		numberOfFalsePositives = 0;
		numberOfMixedAnswers = 0;
		numberOfStochasticEvaluations = 0;
		cummulativeConfidence = 0.0;

		currentNumberOfPositiveHits = 0;
		currentNumberOfNegativeHits = 0;
		currentNumberOfFalseNegatives = 0;
		currentNumberOfFalsePositives = 0;

		fprintf(logFile, "Doing Regular Stable testing:\n\n");

		clearNetStatus();
		numberOfIterations.push_back(stabilizeNet(regularStable, bodies.at(i), paramDebugMode, logFile));

		if(printFile) fprintf(logFile, "Test example %s:", testedExamples.at(i).c_str());

		for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
		{
			headsWithoutNegation.clear();
			for(unsigned int k = 0; k < heads.at(i).size(); k++)
				headsWithoutNegation.push_back(((heads.at(i).at(k).at(0) == '~') ? heads.at(i).at(k).substr(1) : heads.at(i).at(k)));

			for(unsigned int k = 0; k < heads.at(i).size(); k++)
			{
				if(!headsWithoutNegation.at(k).compare(neuronsLabels.at(OUTPUT_LAYER).at(j)))
				{
					status = returnNeuronStatus(OUTPUT_LAYER, neuronsLabels.at(OUTPUT_LAYER).at(j), false, confidence, normalizedAmin);
					if(printFile) fprintf(logFile, "\nActivation: %.2f\n", netStatus.at(OUTPUT_LAYER).at(j));
					if(printFile) fprintf(logFile, "Used Amin: %.2f\n", normalizedAmin);

					if((status == activated) && (heads.at(i).at(k).at(0) != '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfPositiveHits++;

						if(util.less(headsHitProportion.at(regularStable).at(k), 0.0)) headsHitProportion.at(regularStable).at(k) = 0.0;
						exampleCountsPerHead.at(regularStable).at(k) += 1;
						headsHitProportion.at(regularStable).at(k) += 1.0;
					}
					else if((status == desactivated) && (heads.at(i).at(k).at(0) == '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfNegativeHits++;

						if(util.less(headsHitProportion.at(regularStable).at(k), 0.0)) headsHitProportion.at(regularStable).at(k) = 0.0;
						exampleCountsPerHead.at(regularStable).at(k) += 1;
						headsHitProportion.at(regularStable).at(k) += 1.0;
					}
					else if((status == activated) && (heads.at(i).at(k).at(0) == '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfFalsePositives++;

						if(util.less(headsHitProportion.at(regularStable).at(k), 0.0)) headsHitProportion.at(regularStable).at(k) = 0.0;
						exampleCountsPerHead.at(regularStable).at(k) += 1;
					}
					else if((status == desactivated) && (heads.at(i).at(k).at(0) != '~'))
					{
						if(stochastic && util.greater(confidence, 0.0))
						{
							numberOfStochasticEvaluations++;
							cummulativeConfidence *= confidence;
						}

						currentNumberOfFalseNegatives++;

						if(util.less(headsHitProportion.at(regularStable).at(k), 0.0)) headsHitProportion.at(regularStable).at(k) = 0.0;
						exampleCountsPerHead.at(regularStable).at(k) += 1;
					}
				}
			}
		}

		// Checking if all the heads are guessed right
		if((currentNumberOfPositiveHits + currentNumberOfNegativeHits) == (int)heads.at(i).size())
		{
			if(stochastic && util.greater(confidence, 0.0))
			{
				if(printFile) fprintf(logFile, " hit (%.2f%% certainty)\n", cummulativeConfidence);
			}
			else
			{
				if(printFile) fprintf(logFile, " hit\n");
			}

			// Verifying if the analyzed example has multiple heads with multiple classes
			if(currentNumberOfPositiveHits && currentNumberOfNegativeHits)
				numberOfMixedHits++;
			else if(currentNumberOfPositiveHits)
				numberOfPositiveHits++;
			else if(currentNumberOfNegativeHits)
				numberOfNegativeHits++;
		}
		else
		{
			if(stochastic && util.greater(confidence, 0.0))
			{
				if(printFile) fprintf(logFile, " miss (%.2f%% certainty)\n", cummulativeConfidence);
			}
			else
			{
				if(printFile) fprintf(logFile, " miss\n");
			}

			if((currentNumberOfFalseNegatives > 0) && (currentNumberOfFalsePositives > 0))
				numberOfMixedAnswers++;
			else
			{
				if(currentNumberOfFalseNegatives > 0)
					numberOfFalseNegatives++;
				else if(currentNumberOfFalsePositives > 0)
					numberOfFalsePositives++;
			}
		}

		if(printFile) fprintf(logFile, "\n");

		positiveHitsPerTest.at(regularStable) += numberOfPositiveHits;
		negativeHitsPerTest.at(regularStable) += numberOfNegativeHits;
		mixedHitsPerTest.at(regularStable) += numberOfMixedHits;
		numberOfFalsePositivesPerTest.at(regularStable) += numberOfFalsePositives;
		numberOfFalseNegativesPerTest.at(regularStable) += numberOfFalseNegatives;
		numberOfMixedAnswersPerTest.at(regularStable) += numberOfMixedAnswers;
		numberOfStochasticEvaluationsPerTest.at(regularStable) += numberOfStochasticEvaluations;
		cummulativeConfidencePerTest.at(regularStable) += cummulativeConfidence;

		if(stochastic)
		{
			/////////////////////////////////
			// 4.3. Stochastic Simple test //
			/////////////////////////////////

			numberOfPositiveHits = 0;
			numberOfNegativeHits = 0;
			numberOfMixedHits = 0;
			numberOfFalseNegatives = 0;
			numberOfFalsePositives = 0;
			numberOfMixedAnswers = 0;
			numberOfStochasticEvaluations = 0;
			cummulativeConfidence = 0.0;

			currentNumberOfPositiveHits = 0;
			currentNumberOfNegativeHits = 0;
			currentNumberOfFalseNegatives = 0;
			currentNumberOfFalsePositives = 0;

			fprintf(logFile, "Doing Stochastic Simple testing:\n\n");

			clearNetStatus();
			numberOfIterations.push_back(stabilizeNet(stochasticSimple, bodies.at(i), paramDebugMode, logFile));

			if(printFile) fprintf(logFile, "Test example %s:", testedExamples.at(i).c_str());

			for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
			{
				headsWithoutNegation.clear();
				for(unsigned int k = 0; k < heads.at(i).size(); k++)
					headsWithoutNegation.push_back(((heads.at(i).at(k).at(0) == '~') ? heads.at(i).at(k).substr(1) : heads.at(i).at(k)));

				for(unsigned int k = 0; k < heads.at(i).size(); k++)
				{
					if(!headsWithoutNegation.at(k).compare(neuronsLabels.at(OUTPUT_LAYER).at(j)))
					{
						status = returnNeuronStatus(OUTPUT_LAYER, neuronsLabels.at(OUTPUT_LAYER).at(j), false, confidence, normalizedAmin);
						if(printFile) fprintf(logFile, "\nActivation: %.2f\n", netStatus.at(OUTPUT_LAYER).at(j));
						if(printFile) fprintf(logFile, "Used Amin: %.2f\n", normalizedAmin);

						if((status == activated) && (heads.at(i).at(k).at(0) != '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfPositiveHits++;

							if(util.less(headsHitProportion.at(stochasticSimple).at(k), 0.0)) headsHitProportion.at(stochasticSimple).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticSimple).at(k) += 1;
							headsHitProportion.at(stochasticSimple).at(k) += 1.0;
						}
						else if((status == desactivated) && (heads.at(i).at(k).at(0) == '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfNegativeHits++;

							if(util.less(headsHitProportion.at(stochasticSimple).at(k), 0.0)) headsHitProportion.at(stochasticSimple).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticSimple).at(k) += 1;
							headsHitProportion.at(stochasticSimple).at(k) += 1.0;
						}
						else if((status == activated) && (heads.at(i).at(k).at(0) == '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfFalsePositives++;

							if(util.less(headsHitProportion.at(stochasticSimple).at(k), 0.0)) headsHitProportion.at(regularStable).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticSimple).at(k) += 1;
						}
						else if((status == desactivated) && (heads.at(i).at(k).at(0) != '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfFalseNegatives++;

							if(util.less(headsHitProportion.at(stochasticSimple).at(k), 0.0)) headsHitProportion.at(stochasticSimple).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticSimple).at(k) += 1;
						}
					}
				}
			}

			// Checking if all the heads are guessed right
			if((currentNumberOfPositiveHits + currentNumberOfNegativeHits) == (int)heads.at(i).size())
			{
				if(stochastic && util.greater(confidence, 0.0))
				{
					if(printFile) fprintf(logFile, " hit (%.2f%% certainty)\n", cummulativeConfidence);
				}
				else
				{
					if(printFile) fprintf(logFile, " hit\n");
				}

				// Verifying if the analyzed example has multiple heads with multiple classes
				if(currentNumberOfPositiveHits && currentNumberOfNegativeHits)
					numberOfMixedHits++;
				else if(currentNumberOfPositiveHits)
					numberOfPositiveHits++;
				else if(currentNumberOfNegativeHits)
					numberOfNegativeHits++;
			}
			else
			{
				if(stochastic && util.greater(confidence, 0.0))
				{
					if(printFile) fprintf(logFile, " miss (%.2f%% certainty)\n", cummulativeConfidence);
				}
				else
				{
					if(printFile) fprintf(logFile, " miss\n");
				}

				if((currentNumberOfFalseNegatives > 0) && (currentNumberOfFalsePositives > 0))
					numberOfMixedAnswers++;
				else
				{
					if(currentNumberOfFalseNegatives > 0)
						numberOfFalseNegatives++;
					else if(currentNumberOfFalsePositives > 0)
						numberOfFalsePositives++;
				}
			}

			if(printFile) fprintf(logFile, "\n");

			positiveHitsPerTest.at(stochasticSimple) += numberOfPositiveHits;
			negativeHitsPerTest.at(stochasticSimple) += numberOfNegativeHits;
			mixedHitsPerTest.at(stochasticSimple) += numberOfMixedHits;
			numberOfFalsePositivesPerTest.at(stochasticSimple) += numberOfFalsePositives;
			numberOfFalseNegativesPerTest.at(stochasticSimple) += numberOfFalseNegatives;
			numberOfMixedAnswersPerTest.at(stochasticSimple) += numberOfMixedAnswers;
			numberOfStochasticEvaluationsPerTest.at(stochasticSimple) += numberOfStochasticEvaluations;
			cummulativeConfidencePerTest.at(stochasticSimple) += cummulativeConfidence;

			/////////////////////////////////
			// 4.4. Stochastic Stable test //
			/////////////////////////////////

			numberOfPositiveHits = 0;
			numberOfNegativeHits = 0;
			numberOfMixedHits = 0;
			numberOfFalseNegatives = 0;
			numberOfFalsePositives = 0;
			numberOfMixedAnswers = 0;
			numberOfStochasticEvaluations = 0;
			cummulativeConfidence = 0.0;

			currentNumberOfPositiveHits = 0;
			currentNumberOfNegativeHits = 0;
			currentNumberOfFalseNegatives = 0;
			currentNumberOfFalsePositives = 0;

			fprintf(logFile, "Doing Stochastic Stable testing:\n\n");

			clearNetStatus();
			numberOfIterations.push_back(stabilizeNet(stochasticStable, bodies.at(i), paramDebugMode, logFile));

			if(printFile) fprintf(logFile, "Test example %s:", testedExamples.at(i).c_str());

			for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
			{
				headsWithoutNegation.clear();
				for(unsigned int k = 0; k < heads.at(i).size(); k++)
					headsWithoutNegation.push_back(((heads.at(i).at(k).at(0) == '~') ? heads.at(i).at(k).substr(1) : heads.at(i).at(k)));

				for(unsigned int k = 0; k < heads.at(i).size(); k++)
				{
					if(!headsWithoutNegation.at(k).compare(neuronsLabels.at(OUTPUT_LAYER).at(j)))
					{
						status = returnNeuronStatus(OUTPUT_LAYER, neuronsLabels.at(OUTPUT_LAYER).at(j), false, confidence, normalizedAmin);
						if(printFile) fprintf(logFile, "\nActivation: %.2f\n", netStatus.at(OUTPUT_LAYER).at(j));
						if(printFile) fprintf(logFile, "Used Amin: %.2f\n", normalizedAmin);

						if((status == activated) && (heads.at(i).at(k).at(0) != '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfPositiveHits++;

							if(util.less(headsHitProportion.at(stochasticStable).at(k), 0.0)) headsHitProportion.at(stochasticStable).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticStable).at(k) += 1;
							headsHitProportion.at(stochasticStable).at(k) += 1.0;
						}
						else if((status == desactivated) && (heads.at(i).at(k).at(0) == '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfNegativeHits++;

							if(util.less(headsHitProportion.at(stochasticStable).at(k), 0.0)) headsHitProportion.at(stochasticStable).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticStable).at(k) += 1;
							headsHitProportion.at(stochasticStable).at(k) += 1.0;
						}
						else if((status == activated) && (heads.at(i).at(k).at(0) == '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfFalsePositives++;

							if(util.less(headsHitProportion.at(stochasticStable).at(k), 0.0)) headsHitProportion.at(stochasticStable).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticStable).at(k) += 1;
						}
						else if((status == desactivated) && (heads.at(i).at(k).at(0) != '~'))
						{
							if(stochastic && util.greater(confidence, 0.0))
							{
								numberOfStochasticEvaluations++;
								cummulativeConfidence *= confidence;
							}

							currentNumberOfFalseNegatives++;

							if(util.less(headsHitProportion.at(stochasticStable).at(k), 0.0)) headsHitProportion.at(stochasticStable).at(k) = 0.0;
							exampleCountsPerHead.at(stochasticStable).at(k) += 1;
						}
					}
				}
			}

			// Checking if all the heads are guessed right
			if((currentNumberOfPositiveHits + currentNumberOfNegativeHits) == (int)heads.at(i).size())
			{
				if(stochastic && util.greater(confidence, 0.0))
				{
					if(printFile) fprintf(logFile, " hit (%.2f%% certainty)\n", cummulativeConfidence);
				}
				else
				{
					if(printFile) fprintf(logFile, " hit\n");
				}

				// Verifying if the analyzed example has multiple heads with multiple classes
				if(currentNumberOfPositiveHits && currentNumberOfNegativeHits)
					numberOfMixedHits++;
				else if(currentNumberOfPositiveHits)
					numberOfPositiveHits++;
				else if(currentNumberOfNegativeHits)
					numberOfNegativeHits++;
			}
			else
			{
				if(stochastic && util.greater(confidence, 0.0))
				{
					if(printFile) fprintf(logFile, " miss (%.2f%% certainty)\n", cummulativeConfidence);
				}
				else
				{
					if(printFile) fprintf(logFile, " miss\n");
				}

				if((currentNumberOfFalseNegatives > 0) && (currentNumberOfFalsePositives > 0))
					numberOfMixedAnswers++;
				else
				{
					if(currentNumberOfFalseNegatives > 0)
						numberOfFalseNegatives++;
					else if(currentNumberOfFalsePositives > 0)
						numberOfFalsePositives++;
				}
			}

			if(printFile) fprintf(logFile, "\n");

			positiveHitsPerTest.at(stochasticStable) += numberOfPositiveHits;
			negativeHitsPerTest.at(stochasticStable) += numberOfNegativeHits;
			mixedHitsPerTest.at(stochasticStable) += numberOfMixedHits;
			numberOfFalsePositivesPerTest.at(stochasticStable) += numberOfFalsePositives;
			numberOfFalseNegativesPerTest.at(stochasticStable) += numberOfFalseNegatives;
			numberOfMixedAnswersPerTest.at(stochasticStable) += numberOfMixedAnswers;
			numberOfStochasticEvaluationsPerTest.at(stochasticStable) += numberOfStochasticEvaluations;
			cummulativeConfidencePerTest.at(stochasticStable) += cummulativeConfidence;
		}
	}

	// 5. Printing test statistics (false-positives, false-negatives)
	if(printFile) fprintf(logFile, "\nTest Statistics for current fold:\n");
	if(printFile) fprintf(logFile, "Test size: %d\n", (int)testedExamples.size());

	for(unsigned int i = 0; i < STABILIZATION_TYPE_SIZE; i++)
	{
		if(!stochastic && (i >= 2))
			break;

		switch(i)
		{
			// Case of regular simple testing
			case regularSimple:
				if(printFile) fprintf(logFile, "\nResults for regular simple testing:\n");
				printf("\nResults for regular simple testing:\n");
				break;

			// Case of regular stochastic testing
			case regularStable:
				if(printFile) fprintf(logFile, "\nResults for regular stable testing:\n");
				printf("\nResults for regular stable testing:\n");
				break;

			// Case of stable simple testing
			case stochasticSimple:
				if(stochastic)
				{
					if(printFile) fprintf(logFile, "\nResults for stochastic simple testing:\n");
					printf("\nResults for stochastic simple testing:\n");
				}
				break;

			// Case of stable stochastic testing
			case stochasticStable:
				if(stochastic)
				{
					if(printFile) fprintf(logFile, "\nResults for stochastic stable testing:\n");
					printf("\nResults for stochastic stable testing:\n");
				}
				break;

			default:
				break;
		}

		// Precision, recall and F-Measure (ignores mixed data)
		precision = (double)positiveHitsPerTest.at(i)/((positiveHitsPerTest.at(i)+numberOfFalsePositivesPerTest.at(i)) == 0 ? -1.0 : (positiveHitsPerTest.at(i)+numberOfFalsePositivesPerTest.at(i)));
		if(util.less(precision, 0.0)) precision = 0.0;

		recall = (double)positiveHitsPerTest.at(i)/((positiveHitsPerTest.at(i)+numberOfFalseNegativesPerTest.at(i)) == 0 ? -1.0 : (positiveHitsPerTest.at(i)+numberOfFalseNegativesPerTest.at(i)));
		if(util.less(recall, 0.0)) recall = 0.0;

		if(!util.equals(precision + recall, 0.0))
			cummulativeFMeasurePerTest.at(i) = (double)(2.0 * (precision * recall)) / (precision + recall);

		if(printFile)
		{
			fprintf(logFile, "Number of iterations: %d\n", numberOfIterations.at(i));

			fprintf(logFile, "Number of true positives: %d\n", positiveHitsPerTest.at(i));
			fprintf(logFile, "Number of false positives: %d\n", numberOfFalsePositivesPerTest.at(i));
			fprintf(logFile, "Number of true negatives: %d\n", negativeHitsPerTest.at(i));
			fprintf(logFile, "Number of false negatives: %d\n", numberOfFalseNegativesPerTest.at(i));
			fprintf(logFile, "Number of mixed hits: %d\n", mixedHitsPerTest.at(i));
			fprintf(logFile, "Number of mixed errors: %d\n\n", numberOfMixedAnswersPerTest.at(i));

			fprintf(logFile, "Fold F-Measure: %f\n", ((double)cummulativeFMeasurePerTest.at(i)));

			fprintf(logFile, "Fold Accuracy: %.2f%%\n", ((double)(positiveHitsPerTest.at(i) + negativeHitsPerTest.at(i) + mixedHitsPerTest.at(i))/bodies.size())*100);
			if(stochastic && util.greater(cummulativeConfidencePerTest.at(i), 0.0))
				fprintf(logFile, "Cumulative certainty of results: %.2f%%\n", ((double)cummulativeConfidencePerTest.at(i)/numberOfStochasticEvaluationsPerTest.at(i)));

			fprintf(logFile, "Accuracy per target:\n");
			for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
			{
				if(headsHitProportion.at(i).at(j) < 0) continue;

				fprintf(logFile, "%s: %.0f/%d (%.2f%%)\n", neuronsLabels.at(OUTPUT_LAYER).at(j).c_str(), headsHitProportion.at(i).at(j), exampleCountsPerHead.at(i).at(j), ((double)headsHitProportion.at(i).at(j)/exampleCountsPerHead.at(i).at(j))*100.0);
			}
		}

		printf("Number of iterations: %d\n", numberOfIterations.at(i));

		printf("Number of true positives: %d\n", positiveHitsPerTest.at(i));
		printf("Number of false positives: %d\n", numberOfFalsePositivesPerTest.at(i));
		printf("Number of true negatives: %d\n", negativeHitsPerTest.at(i));
		printf("Number of false negatives: %d\n", numberOfFalseNegativesPerTest.at(i));
		printf("Number of mixed hits: %d\n", mixedHitsPerTest.at(i));
		printf("Number of mixed errors: %d\n\n", numberOfMixedAnswersPerTest.at(i));
		printf("Fold F-Measure: %f\n", (double)cummulativeFMeasurePerTest.at(i));

		printf("Fold Accuracy: %.2f%%\n", ((double)(positiveHitsPerTest.at(i) + negativeHitsPerTest.at(i) + mixedHitsPerTest.at(i))/bodies.size())*100);
		if(stochastic && util.greater(cummulativeConfidencePerTest.at(i), 0.0))
			printf("Cumulative certainty of results: %.2f%%\n\n", ((double)cummulativeConfidencePerTest.at(i)/numberOfStochasticEvaluationsPerTest.at(i)));

		printf("Accuracy per target:\n");
		for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
		{
			if(headsHitProportion.at(i).at(j) < 0) continue;

			printf("%s: %.0f/%d (%.2f%%)\n", neuronsLabels.at(OUTPUT_LAYER).at(j).c_str(), headsHitProportion.at(i).at(j), exampleCountsPerHead.at(i).at(j), ((double)headsHitProportion.at(i).at(j)/exampleCountsPerHead.at(i).at(j))*100.0);
		}

		// Preparing the folds confusion table
		confusionTable result;
		result.falseNegatives = numberOfFalseNegativesPerTest.at(i);
		result.trueNegatives  = negativeHitsPerTest.at(i);
		result.falsePositives = numberOfFalseNegativesPerTest.at(i);
		result.truePositives  = positiveHitsPerTest.at(i);
		result.falseMixed 	  = numberOfMixedAnswersPerTest.at(i);
		result.trueMixed  	  = mixedHitsPerTest.at(i);

		results.push_back(make_pair(make_pair(((double)(positiveHitsPerTest.at(i) + negativeHitsPerTest.at(i) + mixedHitsPerTest.at(i))/bodies.size())*100, (double)cummulativeFMeasurePerTest.at(i)), result));
	}

	if(printFile) fprintf(logFile, "\n");
	if(printFile) fclose(logFile);

	return results;
}

// Method to calculate the number of clauses that has the node passed as head literal
int Cilp::calculateOutputConnections(string neuronLabel)
{
	int total = 0;
	pair<int, double> connection;

	for(unsigned int i = 0; i < connectionsWeights.at(HIDDEN_LAYER).size(); i++)
	{
		for(unsigned int j = 0; j < connectionsWeights.at(HIDDEN_LAYER).at(i).size(); j++)
		{
			connection = connectionsWeights.at(HIDDEN_LAYER).at(i).at(j);

			if(util.find(neuronsLabels.at(OUTPUT_LAYER), neuronLabel) == connection.first)
			{
				total++;
				break;
			}
		}
	}

	return total;
}

// Method to clear the net status
void Cilp::clearNetStatus()
{
	for(unsigned int i = 0; i < netStatus.size(); i++)
	{
		for(unsigned int j = 0; j < netStatus.at(i).size(); j++)
			netStatus.at(i).at(j) = -1.0;
	}
}

// Method to return the neuron status as an enum
neuronStatus Cilp::returnNeuronStatus(int layer, string label, bool canBeStochastic, double& stochasticConfidence, double& correctAmin)
{
	int neuronPosition;
	double randomNumber, neuronActivationLevel, totalInput, aminNormalizationFactor;

	stochasticConfidence = -1.0;

	if(util.equals(util.find(neuronsLabels.at(layer), label), -1.0))
		return doesNotExist;

	neuronPosition = util.find(neuronsLabels.at(layer), label);
	neuronActivationLevel = netStatus.at(layer).at(neuronPosition);

	// Calculating the Amin normalization factor (if NORMALIZE_WEIGHTS_ONLY = true)
	correctAmin = normalizedAmins.at(layer).at(neuronPosition);
	if(NORMALIZE_WEIGHTS_ONLY)
	{
		// Obtaining the input total of the neuron
		totalInput = calculateTotalInput(layer, neuronPosition, false);
		aminNormalizationFactor = (double)(1.0 - exp(-1.0 * paramBeta * totalInput * normalizationFactors.at(layer).at(neuronPosition))) / (1.0 - exp(-1.0 * paramBeta * totalInput));
		correctAmin *= aminNormalizationFactor;
	}

	if(util.lessOrEqual(neuronActivationLevel, 1.0) && util.greaterOrEqual(neuronActivationLevel, -1.0))
	{
		if(util.greaterOrEqual(neuronActivationLevel, correctAmin))
			return activated;
		else if(util.lessOrEqual(neuronActivationLevel, -correctAmin))
			return desactivated;
		else if(stochastic && canBeStochastic)
		{
			randomNumber = ((double)rand() / RAND_MAX);
			if(util.greaterOrEqual(abs((double)(neuronActivationLevel + correctAmin) / (correctAmin * 2.0)), randomNumber))
			{
				stochasticConfidence = abs((double)neuronActivationLevel / correctAmin) * 100;
				return activated;
			}
			else
			{
				stochasticConfidence = abs((double)neuronActivationLevel / correctAmin) * 100;
				return desactivated;
			}
		}
		else
		{
			if(util.greaterOrEqual(neuronActivationLevel, 0.0))
				return activated;
			else
				return desactivated;
		}
	}
	else
		return outOfBounds;
}

// Method to return the neuron status as a string
string Cilp::returnNeuronStatusString(int layer, string label, bool canBeStochastic, double& stochasticConfidence, double& correctAmin)
{
	int neuronPosition;
	double randomNumber, neuronActivationLevel, totalInput, aminNormalizationFactor;

	stochasticConfidence = -1.0;

	if(util.equals(util.find(neuronsLabels.at(layer), label), -1.0))
		return "doesNotExist";

	neuronPosition = util.find(neuronsLabels.at(layer), label);
	neuronActivationLevel = netStatus.at(layer).at(neuronPosition);

	// Calculating the Amin normalization factor (if NORMALIZE_WEIGHTS_ONLY = true)
	correctAmin = normalizedAmins.at(layer).at(neuronPosition);
	if(NORMALIZE_WEIGHTS_ONLY)
	{
		// Obtaining the input total of the neuron
		totalInput = calculateTotalInput(layer, neuronPosition, false);
		aminNormalizationFactor = (double)(1.0 - exp(-1.0 * paramBeta * totalInput * normalizationFactors.at(layer).at(neuronPosition))) / (1.0 - exp(-1.0 * paramBeta * totalInput));
		correctAmin *= aminNormalizationFactor;
	}

	if(util.lessOrEqual(neuronActivationLevel, 1.0) && util.greaterOrEqual(neuronActivationLevel, -1.0))
	{
		if(util.greaterOrEqual(neuronActivationLevel, correctAmin))
			return "activated";
		else if(util.lessOrEqual(neuronActivationLevel, -correctAmin))
			return "desactivated";
		else if(stochastic && canBeStochastic)
		{
			randomNumber = ((double)rand() / RAND_MAX);
			if(util.greaterOrEqual(abs((double)(neuronActivationLevel + correctAmin) / (correctAmin * 2.0)), randomNumber))
			{
				stochasticConfidence = abs((double)neuronActivationLevel / correctAmin) * 100;
				return "activated";
			}
			else
			{
				stochasticConfidence = abs((double)neuronActivationLevel / correctAmin) * 100;
				return "desactivated";
			}
		}
		else
		{
			if(util.greaterOrEqual(neuronActivationLevel, 0.0))
				return "activated";
			else
				return "desactivated";
		}
	}
	else
		return "outOfBounds";
}

// Method to verify if the net is stabilized
bool Cilp::isStabilized(stabilizationType type)
{
	bool canBeStochastic;
	double justAdouble;

	// Verifying if the status analysis will be stochastic or not
	if((type == stochasticSimple) || (type == stochasticStable))
		canBeStochastic = true;
	else
		canBeStochastic = false;

	// If the input and the corresponding output nodes status are equal, the system stabilized
	for(unsigned int i = 0; i < neuronsLabels.at(OUTPUT_LAYER).size(); i++)
	{
		// If the current output neuron does not have a correspondant on input, continue
		if(util.find(neuronsLabels.at(INPUT_LAYER), neuronsLabels.at(OUTPUT_LAYER).at(i)) == -1)
			continue;

		// If the output neuron 'i' has different status than the correspondant, it is not stabilized
		if(returnNeuronStatus(OUTPUT_LAYER, neuronsLabels.at(OUTPUT_LAYER).at(i), canBeStochastic, justAdouble, justAdouble) != returnNeuronStatus(INPUT_LAYER, neuronsLabels.at(OUTPUT_LAYER).at(i), canBeStochastic, justAdouble, justAdouble))
			return false;
	}

	return true;
}

// Method responsible to print all the system status
void Cilp::printNetStatus(bool printOnFile)
{
	double confidenceInterval, justAdouble;
	FILE *outputFile = NULL;

	if(printOnFile)
		outputFile = fopen("files/netStatus.log", "a");

	if(printOnFile)
		fprintf(outputFile, "\nCurrent system neurons stati: \n\n");
	else
		cout << endl << "Current system neurons stati: " << endl << endl;

	for(unsigned int i = 0; i < netStatus.size(); i++)
	{
		if(printOnFile)
			fprintf(outputFile, "Layer: %d\n", i+1);
		else
			cout << "Layer: " << i+1 << endl;

		for(unsigned int j = 0; j < netStatus.at(i).size(); j++)
		{
			if(printOnFile)
				fprintf(outputFile, "%s - (%s) %f", neuronsLabels.at(i).at(j).c_str(), returnNeuronStatusString(i, neuronsLabels.at(i).at(j), false, confidenceInterval, justAdouble).c_str(), netStatus.at(i).at(j));
			else
				cout << neuronsLabels.at(i).at(j) << " - (" << returnNeuronStatusString(i, neuronsLabels.at(i).at(j), false, confidenceInterval, justAdouble) << ") " << netStatus.at(i).at(j);

			if(stochastic && util.greater(confidenceInterval, 0.0))
			{
				if(printOnFile)
					fprintf(outputFile, ", with a probability of %f", confidenceInterval);
				else
					cout << ", with a probability of " << confidenceInterval;
			}

			if(printOnFile)
				fprintf(outputFile, "\n");
			else
				cout << endl;
		}

		if(printOnFile)
			fprintf(outputFile, "\n");
		else
			cout << endl;
	}

	if(printOnFile)
		fclose(outputFile);
}

// Method responsible to print all the system weights
void Cilp::printNetWeights(bool printOnFile)
{
	FILE *outputFile = NULL;

	if(printOnFile)
		outputFile = fopen("files/netWeights.log", "a");

	if(printOnFile)
		fprintf(outputFile, "\nCurrent system neurons weights: \n\n");
	else
		cout << endl << "Current system neurons weights: " << endl << endl;

	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
	{
		if(printOnFile)
			fprintf(outputFile, "Layer: %d\n", i+1);
		else
			cout << "Layer: " << i+1 << endl;

		for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
		{
			for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
			{
				if(printOnFile)
					fprintf(outputFile, "Connection %s -- %s: %f\n", neuronsLabels.at(i).at(j).c_str(), neuronsLabels.at((i+1)%connectionsWeights.size()).at(connectionsWeights.at(i).at(j).at(k).first).c_str(), connectionsWeights.at(i).at(j).at(k).second);
				else
					cout << "Connection " << neuronsLabels.at(i).at(j) << " -- " << neuronsLabels.at((i+1)%connectionsWeights.size()).at(connectionsWeights.at(i).at(j).at(k).first) << ": " << connectionsWeights.at(i).at(j).at(k).second << endl;
			}
		}

		if(printOnFile)
			fprintf(outputFile, "\n");
		else
			cout << endl;
	}

	if(printOnFile)
		fclose(outputFile);
}

// Method to convert a string example to a numeric vector with 'unknowns' equals to 0, for output comparisons
vector<double> Cilp::convertData(int layer, vector<string> stringVector)
{
	string headWithoutNegation;
	double currentValue;
	vector<double> numericVector;

	for(unsigned int i = 0; i < neuronsLabels.at(layer).size(); i++)
		numericVector.push_back(0.0);

	for(unsigned int i = 0; i < stringVector.size(); i++)
	{
		if(stringVector.at(i).at(0) == '~')
			currentValue = -1.0;
		else
			currentValue = 1.0;

		headWithoutNegation = ((stringVector.at(i).at(0) == '~') ? stringVector.at(i).substr(1, stringVector.at(i).size()-1) : stringVector.at(i));

		if(util.find(neuronsLabels.at(layer), headWithoutNegation) >= 0)
			numericVector.at(util.find(neuronsLabels.at(layer), headWithoutNegation)) = currentValue;
	}

	return numericVector;
}

// Method to convert a string example to a numeric vector where 'unknowns' equals -1, to allow proper network training
vector<double> Cilp::convertInputData(int layer, vector<string> stringVector, string exampleLabel)
{
	int index;
	bool match;
	string literal, predicate, content, literal2, predicate2, content2;
	vector<double> numericVector;
	vector<string> literalsList, variables, variables2;

	for(unsigned int i = 0; i < neuronsLabels.at(layer).size(); i++)
	{
		if(paramUnknownInputsAsFalse)
			numericVector.push_back(-1.0);
		else
			numericVector.push_back(0.0);
	}
	
	// Using intermediate info to initialize their values in the example
	if((layer == INPUT_LAYER) && !util.examplesBuffer.empty())
	{
		for(unsigned int i = 0; i < util.examplesBuffer.size(); i++)
		{
			if(!exampleLabel.compare(util.examplesBuffer.at(i).first))
			{
				for(unsigned int j = 0; j < util.examplesBuffer.at(i).second.size(); j++)
					numericVector.at(util.examplesBuffer.at(i).second.at(j).first) = util.examplesBuffer.at(i).second.at(j).second;
			}
		}
	}

	for(unsigned int i = 0; i < stringVector.size(); i++)
	{
		if(util.find(neuronsLabels.at(layer), stringVector.at(i)) >= 0)
		{
			if(stringVector.at(i).at(0) != '~')
				numericVector.at(util.find(neuronsLabels.at(layer), stringVector.at(i))) = 1.0;
			else
				numericVector.at(util.find(neuronsLabels.at(layer), stringVector.at(i).substr(1, stringVector.at(i).size()))) = -1.0;
		}
	}

	// For all disjunctions in the input layer: if any of its members is true, the disjunction must also be true
	for(unsigned int i = 0; (layer == INPUT_LAYER) && (i < neuronsLabels.at(layer).size()); i++)
	{
		// Disjunction check
		if(neuronsLabels.at(layer).at(i).find("|") != string::npos)
		{
			literalsList = util.splitDelimited(neuronsLabels.at(layer).at(i), '|');

			for(unsigned int j = 0; j < literalsList.size(); j++)
			{
				index = util.find(neuronsLabels.at(layer), literalsList.at(j));

				if((index >= 0) && util.equals(numericVector.at(index), 1.0))
				{
					numericVector.at(i) = 1.0;
					break;
				}
			}
		}
	}

	// For all literals with generic variable, if any feature which matches the literal can be found, then they are true
	for(unsigned int i = 0; (layer == INPUT_LAYER) && (i < stringVector.size()); i++)
	{
		if(stringVector.at(i).find("_") == string::npos)
			continue;

		predicate = stringVector.at(i).substr(0, stringVector.at(i).find("("));
		content   = stringVector.at(i).substr(stringVector.at(i).find("(")+1, stringVector.at(i).find(")")-stringVector.at(i).find("(")-1);

		variables = util.splitDelimited(content, ';');

		for(unsigned int j = 0; j < neuronsLabels.at(layer).size(); j++)
		{
			if(!util.equals(numericVector.at(j), 1.0))
				continue;

			predicate2 = neuronsLabels.at(layer).at(j).substr(0, neuronsLabels.at(layer).at(j).find("("));
			content2   = neuronsLabels.at(layer).at(j).substr(neuronsLabels.at(layer).at(j).find("(")+1, neuronsLabels.at(layer).at(j).find(")")-neuronsLabels.at(layer).at(j).find("(")-1);

			variables2 = util.splitDelimited(content2, ';');

			match = true;
			if(!predicate.compare(predicate2) && (variables.size() == variables2.size()))
			{
				for(unsigned int k = 0; k < variables.size(); k++)
				{
					if(variables.at(k).compare(variables2.at(k)) && variables.at(k).compare("_") && variables2.at(k).compare("_"))
					{
						match = false;
						break;
					}
				}
			}

			if(match)
				numericVector.at(j) = 1.0;
		}
	}

	return numericVector;
}

// Method to shuffle the training data
void Cilp::shuffleTrainData(vector<string>& trainingExamples, vector<vector<string> >& heads, vector<vector<string> >& bodies)
{
	vector<string> examplesTemp;
	vector<vector<string> > bodiesTemp, headsTemp;

	unsigned randIndex;
	bool end = trainingExamples.empty();

	while(!end)
	{
		randIndex = (int) floor(((double)rand() / RAND_MAX) * trainingExamples.size());
		while(randIndex == trainingExamples.size())
			randIndex = (int) floor(((double)rand() / RAND_MAX) * trainingExamples.size());

		examplesTemp.push_back(trainingExamples.at(randIndex));
		headsTemp.push_back(heads.at(randIndex));
		bodiesTemp.push_back(bodies.at(randIndex));

		trainingExamples.erase(trainingExamples.begin() + randIndex);
		heads.erase(heads.begin() + randIndex);
		bodies.erase(bodies.begin() + randIndex);

		if(!trainingExamples.size())
			end = true;
	}

	if(trainingExamples.empty())
	{
		trainingExamples = examplesTemp;
		heads = headsTemp;
		bodies = bodiesTemp;
	}
}

// Method that calculates the derivative of the semi-linear activation function
double Cilp::calculateActivationDerivative(double input, activationFunction function)
{
	switch(function)
	{
		case linear:
			return 1.0;
			break;
		case binary:
			cout << "<Error> Binary activation function is not derivable!" << endl;
			break;
		case hyperbolic:
			return 1 - pow(tanh(input), 2);
			break;
		case bipolar:
			return ((double)paramBeta/2) * (1 - pow(calculateActivationFunction((double)input * paramBeta, function), 2));
			break;
		default:
			cout << "<Error> This activation function is not implemented!" << endl;
			break;
	}

	return 0.0;
}

// Method to add new hidden neurons to the net
void Cilp::addNewHiddenNeurons(int numberOfHiddenNeurons)
{
	double initialValue = 0.0;
	vector<pair<int, double> > newWeightVector;

	for(int i = 0; i < numberOfHiddenNeurons; i++)
	{
		// 1. Adding the new neuron
		neuronsLabels.at(HIDDEN_LAYER).push_back("<new" + util.convertToString(neuronsLabels.at(HIDDEN_LAYER).size()) + ">");
		normalizationFactors.at(HIDDEN_LAYER).push_back(1.0);
		normalizedAmins.at(HIDDEN_LAYER).push_back(amin);
		netStatus.at(HIDDEN_LAYER).push_back(0.0);

		// 2. Setting the initialized threshold
		if(noBackgroundKnowledge)
			initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
		else
			initialValue = 0.0;
		thresholds.at(HIDDEN_LAYER).push_back(initialValue);
		layersBias.at(HIDDEN_LAYER).push_back(-initialValue);

		// 3. Connecting all the input neurons to this new neuron with a small non-zero weight
		for(unsigned int j = 0; j < connectionsWeights.at(INPUT_LAYER).size(); j++)
		{
			if(noBackgroundKnowledge)
				initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
			else
				initialValue = 0.0;

			connectionsWeights.at(INPUT_LAYER).at(j).push_back(make_pair(neuronsLabels.at(HIDDEN_LAYER).size()-1, initialValue));
		}

		// 4. Connecting the new hidden neuron to all output neurons, with a small random value
		if(!newWeightVector.empty())
			newWeightVector.clear();
		for(unsigned int j = 0; j < neuronsLabels.at(OUTPUT_LAYER).size(); j++)
		{
			if(noBackgroundKnowledge)
				initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
			else
				initialValue = 0.0;

			newWeightVector.push_back(make_pair(j, initialValue));
		}

		connectionsWeights.at(HIDDEN_LAYER).push_back(newWeightVector);
	}
}

// Method to add a new labeled input neuron
void Cilp::addNewInputNeuron(string label)
{
	double initialValue = 0.0;
	vector<pair<int, double> > newWeightVector;

	// 1. Creating the new input node
	neuronsLabels.at(INPUT_LAYER).push_back(label);
	normalizationFactors.at(INPUT_LAYER).push_back(1.0);
	normalizedAmins.at(INPUT_LAYER).push_back(amin);
	netStatus.at(INPUT_LAYER).push_back(0.0);
	thresholds.at(INPUT_LAYER).push_back(0.0);
	layersBias.at(INPUT_LAYER).push_back(0.0);

	// 2. Linking it with all the other neurons at the hidden layer
	if(!newWeightVector.empty())
		newWeightVector.clear();
	for(unsigned int k = 0; k < neuronsLabels.at(HIDDEN_LAYER).size(); k++)
	{
		// 1. Setting the initialized threshold
		if(noBackgroundKnowledge)
			initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
		else
			initialValue = 0.0;
		newWeightVector.push_back(make_pair(k, initialValue));
	}

	connectionsWeights.at(INPUT_LAYER).push_back(newWeightVector);

	// 3. If exists an output neuron with this same label, it is linked with it
	if(util.find(neuronsLabels.at(OUTPUT_LAYER), label) >= 0)
		connectionsWeights.at(OUTPUT_LAYER).at(util.find(neuronsLabels.at(OUTPUT_LAYER), label)).push_back(make_pair(util.find(neuronsLabels.at(INPUT_LAYER), label), 1.0));

}

// Method to add a new labeled output neuron
void Cilp::addNewOutputNeuron(string label)
{
	double initialValue = 0.0;
	vector<pair<int, double> > newWeightVector;

	neuronsLabels.at(OUTPUT_LAYER).push_back(label);
	neuronsLabels.at(INPUT_LAYER).push_back(label);

	// 1. Setting the initialized threshold
	if(noBackgroundKnowledge)
		initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
	else
		initialValue = 0.0;
	thresholds.at(OUTPUT_LAYER).push_back(initialValue);
	layersBias.at(OUTPUT_LAYER).push_back(-initialValue);

	thresholds.at(INPUT_LAYER).push_back(0.0);
	layersBias.at(INPUT_LAYER).push_back(0.0);

	netStatus.at(INPUT_LAYER).push_back(0.0);
	netStatus.at(OUTPUT_LAYER).push_back(0.0);

	normalizationFactors.at(INPUT_LAYER).push_back(1.0);
	normalizationFactors.at(OUTPUT_LAYER).push_back(1.0);

	normalizedAmins.at(INPUT_LAYER).push_back(amin);
	normalizedAmins.at(OUTPUT_LAYER).push_back(amin);

	// 1. Setting the new output neuron
	if(!newWeightVector.empty())
		newWeightVector.clear();
	newWeightVector.push_back(make_pair(util.find(neuronsLabels.at(INPUT_LAYER), label), 1.0));

	connectionsWeights.at(OUTPUT_LAYER).push_back(newWeightVector);

	// 2. Setting the new input neuron
	if(!newWeightVector.empty())
		newWeightVector.clear();
	for(unsigned int j = 0; j < neuronsLabels.at(HIDDEN_LAYER).size(); j++)
	{
		if(noBackgroundKnowledge)
			initialValue = (((double)rand() / RAND_MAX) - 0.5) * paramWeightInitialRange;
		else
			initialValue = 0.0;
		newWeightVector.push_back(make_pair(j, initialValue));
	}

	connectionsWeights.at(INPUT_LAYER).push_back(newWeightVector);
}

// Method to split properly the training data into folds, according to stratified k-fold cross-validation
vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > Cilp::splitDataByTarget(vector<string> dataset, vector<vector<string> > bodies, vector<vector<string> > heads, int numberOfFolds, double portion)
{
	bool moreFoldsThanData = false;
	int mixedFoldSize;
	vector<int> positiveFoldSize, negativeFoldSize;
	double tempParamTrainPortion;

	pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > datasetTrain, datasetValidation;
	vector<string> headsPositiveLabels, headsNegativeLabels, mixedClauses, mixedClausesShuffled, unitedClauses;
	vector<vector<vector<string> > > positiveBodies, negativeBodies, positiveBodiesShuffled, negativeBodiesShuffled, positiveHeads, negativeHeads, positiveHeadsShuffled, negativeHeadsShuffled;
	vector<vector<string> > positiveClauses, negativeClauses, positiveClausesShuffled, negativeClausesShuffled, mixedBodies, mixedBodiesShuffled, mixedHeads, mixedHeadsShuffled, unitedHeads, unitedBodies;

	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > datasets;

	if(numberOfFolds < 0) return datasets;

	// 1.  Shuffling and splitting the data by occurrences of positive and negative examples
	shuffleTrainData(dataset, heads, bodies);
	for(unsigned int i = 0; i < heads.size(); i++)
	{
		// Only one head in the clause: can be stratified
		if(heads.at(i).size() == 1)
		{
			vector<string> emptyVector;
			vector<vector<string> > emptyMatrix;
			int index = -1;
	
			if(heads.at(i).at(0).find("~") == string::npos)
				index = util.find(headsPositiveLabels, heads.at(i).at(0));
			else
				index = util.find(headsNegativeLabels, heads.at(i).at(0));
	
			if(index < 0)
			{
				if(heads.at(i).at(0).find("~") == string::npos)
				{
					headsPositiveLabels.push_back(heads.at(i).at(0));

					positiveHeads.push_back(emptyMatrix);
					positiveBodies.push_back(emptyMatrix);
					positiveClauses.push_back(emptyVector);

					index = positiveHeads.size()-1;
				}
				else
				{
					headsNegativeLabels.push_back(heads.at(i).at(0));

					negativeHeads.push_back(emptyMatrix);
					negativeBodies.push_back(emptyMatrix);
					negativeClauses.push_back(emptyVector);

					index = negativeHeads.size()-1;
				}
			}
			
			if(heads.at(i).at(0).find("~") != string::npos)
			{
				negativeHeads.at(index).push_back(heads.at(i));
				negativeBodies.at(index).push_back(bodies.at(i));
				negativeClauses.at(index).push_back(dataset.at(i));
			}
			else
			{
				positiveHeads.at(index).push_back(heads.at(i));
				positiveBodies.at(index).push_back(bodies.at(i));
				positiveClauses.at(index).push_back(dataset.at(i));
			}
		}
		// In this case, the data is put into a separate set
		else
		{
			mixedHeads.push_back(heads.at(i));
			mixedBodies.push_back(bodies.at(i));
			mixedClauses.push_back(dataset.at(i));
		}
	}

	// 2. Calculating both groups sizes
	int totalPositiveSize = 0, totalNegativeSize = 0;
	if(numberOfFolds > 1)
	{
		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
			positiveFoldSize.push_back(positiveBodies.at(i).size() / numberOfFolds);
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
			negativeFoldSize.push_back(negativeBodies.at(i).size() / numberOfFolds);

		mixedFoldSize = mixedBodies.size() / numberOfFolds;

		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
			totalPositiveSize += positiveFoldSize.at(i);

		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
			totalNegativeSize += negativeFoldSize.at(i);

		if(!totalPositiveSize && !totalNegativeSize && !mixedFoldSize)
			moreFoldsThanData = true;
	}
	else
	{
		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
			positiveFoldSize.push_back(positiveBodies.at(i).size());

		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
			negativeFoldSize.push_back(negativeBodies.at(i).size());

		mixedFoldSize = mixedBodies.size();
	}

	// 3. Iterating through all the examples and putting them into the <numberOfFolds> folds
	if(numberOfFolds > 1)
	{
		// Checking if none of the positive or negative examples are bigger than the number of folds
		if(moreFoldsThanData)
		{
			for(unsigned int d = 0; d < dataset.size(); d++)
			{
				// In this case, all examples will be shuffled together and then, distributed among the folds.
				// Leave-one-out cross-validation will be used.
				for(unsigned int i = 0; i < dataset.size(); i++)
				{
					if(i != d)
					{
						unitedBodies.push_back(bodies.at(i));
						unitedHeads.push_back(heads.at(i));
						unitedClauses.push_back(dataset.at(i));
					}
				}
				datasetTrain = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();

				unitedBodies.push_back(bodies.at(d));
				unitedHeads.push_back(heads.at(d));
				unitedClauses.push_back(dataset.at(d));

				datasetValidation = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				datasets.push_back(make_pair(datasetTrain, datasetValidation));

				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();
			}
		}
		else
		{
			for(int i = 0; i < numberOfFolds; i++)
			{
				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();

				positiveBodiesShuffled.clear();
				positiveHeadsShuffled.clear();
				positiveClausesShuffled.clear();

				negativeBodiesShuffled.clear();
				negativeHeadsShuffled.clear();
				negativeClausesShuffled.clear();

				mixedBodiesShuffled.clear();
				mixedHeadsShuffled.clear();
				mixedClausesShuffled.clear();

				// 3.1. Getting the validation portion of the positive and negative examples
				if(i != (numberOfFolds - 1))
				{
					for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.at(j).begin() + (i * positiveFoldSize.at(j)), positiveBodies.at(j).begin() + ((i + 1) * positiveFoldSize.at(j)));
						positiveBodiesShuffled.push_back(tempPositiveBodiesShuffled);

						vector<vector<string> > tempPositiveHeadsShuffled(positiveHeads.at(j).begin() + (i * positiveFoldSize.at(j)), positiveHeads.at(j).begin() + ((i + 1) * positiveFoldSize.at(j)));
						positiveHeadsShuffled.push_back(tempPositiveHeadsShuffled);

						vector<string> tempPositiveClausesShuffled(positiveClauses.at(j).begin() + (i * positiveFoldSize.at(j)), positiveClauses.at(j).begin() + ((i + 1) * positiveFoldSize.at(j)));
						positiveClausesShuffled.push_back(tempPositiveClausesShuffled);
					}

					for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.at(j).begin() + (i * negativeFoldSize.at(j)), negativeBodies.at(j).begin() + ((i + 1) * negativeFoldSize.at(j)));
						negativeBodiesShuffled.push_back(tempNegativeBodiesShuffled);

						vector<vector<string> > tempNegativeHeadsShuffled(negativeHeads.at(j).begin() + (i * negativeFoldSize.at(j)), negativeHeads.at(j).begin() + ((i + 1) * negativeFoldSize.at(j)));
						negativeHeadsShuffled.push_back(tempNegativeHeadsShuffled);

						vector<string> tempNegativeClausesShuffled(negativeClauses.at(j).begin() + (i * negativeFoldSize.at(j)), negativeClauses.at(j).begin() + ((i + 1) * negativeFoldSize.at(j)));
						negativeClausesShuffled.push_back(tempNegativeClausesShuffled);
					}
					
					if((int)mixedBodies.size() > (int)i)
					{
						vector<vector<string> > tempMixedBodiesShuffled(mixedBodies.begin() + (i * mixedFoldSize), mixedBodies.begin() + ((i + 1) * mixedFoldSize));
						mixedBodiesShuffled = tempMixedBodiesShuffled;

						vector<vector<string> > tempMixedHeadsShuffled(mixedHeads.begin() + (i * mixedFoldSize), mixedHeads.begin() + ((i + 1) * mixedFoldSize));
						mixedHeadsShuffled = tempMixedHeadsShuffled;

						vector<string> tempMixedClausesShuffled(mixedClauses.begin() + (i * mixedFoldSize), mixedClauses.begin() + ((i + 1) * mixedFoldSize));
						mixedClausesShuffled = tempMixedClausesShuffled;
					}
					else
					{
						mixedBodiesShuffled.clear();
						mixedHeadsShuffled.clear();
						mixedClausesShuffled.clear();
					}
				}
				else
				{
					for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.at(j).begin() + (i * positiveFoldSize.at(j)), positiveBodies.at(j).end());
						positiveBodiesShuffled.push_back(tempPositiveBodiesShuffled);

						vector<vector<string> > tempPositiveHeadsShuffled(positiveHeads.at(j).begin() + (i * positiveFoldSize.at(j)), positiveHeads.at(j).end());
						positiveHeadsShuffled.push_back(tempPositiveHeadsShuffled);

						vector<string> tempPositiveClausesShuffled(positiveClauses.at(j).begin() + (i * positiveFoldSize.at(j)), positiveClauses.at(j).end());
						positiveClausesShuffled.push_back(tempPositiveClausesShuffled);
					}

					for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.at(j).begin() + (i * negativeFoldSize.at(j)), negativeBodies.at(j).end());
						negativeBodiesShuffled.push_back(tempNegativeBodiesShuffled);

						vector<vector<string> > tempNegativeHeadsShuffled(negativeHeads.at(j).begin() + (i * negativeFoldSize.at(j)), negativeHeads.at(j).end());
						negativeHeadsShuffled.push_back(tempNegativeHeadsShuffled);

						vector<string> tempNegativeClausesShuffled(negativeClauses.at(j).begin() + (i * negativeFoldSize.at(j)), negativeClauses.at(j).end());
						negativeClausesShuffled.push_back(tempNegativeClausesShuffled);
					}
					
					vector<vector<string> > tempMixedBodiesShuffled(mixedBodies.begin() + (i * mixedFoldSize), mixedBodies.end());
					mixedBodiesShuffled = tempMixedBodiesShuffled;

					vector<vector<string> > tempMixedHeadsShuffled(mixedHeads.begin() + (i * mixedFoldSize), mixedHeads.end());
					mixedHeadsShuffled = tempMixedHeadsShuffled;

					vector<string> tempMixedClausesShuffled(mixedClauses.begin() + (i * mixedFoldSize), mixedClauses.end());
					mixedClausesShuffled = tempMixedClausesShuffled;
				}

				// 3.2. Joining the data
				for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
				{
					unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.at(j).begin(), positiveBodiesShuffled.at(j).end());
					unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.at(j).begin(), positiveHeadsShuffled.at(j).end());
					unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.at(j).begin(), positiveClausesShuffled.at(j).end());
				}

				for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
				{
					unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.at(j).begin(), negativeBodiesShuffled.at(j).end());
					unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.at(j).begin(), negativeHeadsShuffled.at(j).end());
					unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.at(j).begin(), negativeClausesShuffled.at(j).end());
				}

				unitedBodies.insert(unitedBodies.begin(), mixedBodiesShuffled.begin(), mixedBodiesShuffled.end());
				unitedHeads.insert(unitedHeads.begin(), mixedHeadsShuffled.begin(), mixedHeadsShuffled.end());
				unitedClauses.insert(unitedClauses.begin(), mixedClausesShuffled.begin(), mixedClausesShuffled.end());

				// 3.3. Shuffling the data and putting them into the training dataset
				shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
				datasetValidation = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				// 3.4. Getting the training portion of the positive and negative examples before the training part
				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();

				positiveBodiesShuffled.clear();
				positiveHeadsShuffled.clear();
				positiveClausesShuffled.clear();

				negativeBodiesShuffled.clear();
				negativeHeadsShuffled.clear();
				negativeClausesShuffled.clear();

				mixedBodiesShuffled.clear();
				mixedHeadsShuffled.clear();
				mixedClausesShuffled.clear();

				if(i != 0)
				{
					for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.at(j).begin(), positiveBodies.at(j).begin() + (i * positiveFoldSize.at(j)));
						positiveBodiesShuffled.push_back(tempPositiveBodiesShuffled);

						vector<vector<string> > tempPositiveHeadsShuffled(positiveHeads.at(j).begin(), positiveHeads.at(j).begin() + (i * positiveFoldSize.at(j)));
						positiveHeadsShuffled.push_back(tempPositiveHeadsShuffled);

						vector<string> tempPositiveClausesShuffled(positiveClauses.at(j).begin(), positiveClauses.at(j).begin() + (i * positiveFoldSize.at(j)));
						positiveClausesShuffled.push_back(tempPositiveClausesShuffled);
					}

					for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.at(j).begin(), negativeBodies.at(j).begin() + (i * negativeFoldSize.at(j)));
						negativeBodiesShuffled.push_back(tempNegativeBodiesShuffled);

						vector<vector<string> > tempNegativeHeadsShuffled(negativeHeads.at(j).begin(), negativeHeads.at(j).begin() + (i * negativeFoldSize.at(j)));
						negativeHeadsShuffled.push_back(tempNegativeHeadsShuffled);

						vector<string> tempNegativeClausesShuffled(negativeClauses.at(j).begin(), negativeClauses.at(j).begin() + (i * negativeFoldSize.at(j)));
						negativeClausesShuffled.push_back(tempNegativeClausesShuffled);
					}
					
					vector<vector<string> > tempMixedBodiesShuffled(mixedBodies.begin(), mixedBodies.begin() + (i * mixedFoldSize));
					mixedBodiesShuffled = tempMixedBodiesShuffled;

					vector<vector<string> > tempMixedHeadsShuffled(mixedHeads.begin(), mixedHeads.begin() + (i * mixedFoldSize));
					mixedHeadsShuffled = tempMixedHeadsShuffled;

					vector<string> tempMixedClausesShuffled(mixedClauses.begin(), mixedClauses.begin() + (i * mixedFoldSize));
					mixedClausesShuffled = tempMixedClausesShuffled;

					for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
					{
						unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.at(j).begin(), positiveBodiesShuffled.at(j).end());
						unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.at(j).begin(), positiveHeadsShuffled.at(j).end());
						unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.at(j).begin(), positiveClausesShuffled.at(j).end());
					}

					for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
					{
						unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.at(j).begin(), negativeBodiesShuffled.at(j).end());
						unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.at(j).begin(), negativeHeadsShuffled.at(j).end());
						unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.at(j).begin(), negativeClausesShuffled.at(j).end());
					}

					unitedBodies.insert(unitedBodies.begin(), mixedBodiesShuffled.begin(), mixedBodiesShuffled.end());
					unitedHeads.insert(unitedHeads.begin(), mixedHeadsShuffled.begin(), mixedHeadsShuffled.end());
					unitedClauses.insert(unitedClauses.begin(), mixedClausesShuffled.begin(), mixedClausesShuffled.end());
				}

				positiveBodiesShuffled.clear();
				positiveHeadsShuffled.clear();
				positiveClausesShuffled.clear();

				negativeBodiesShuffled.clear();
				negativeHeadsShuffled.clear();
				negativeClausesShuffled.clear();

				mixedBodiesShuffled.clear();
				mixedHeadsShuffled.clear();
				mixedClausesShuffled.clear();

				// 3.5. Getting the validation portion of the positive and negative examples after the training part
				if(i != (numberOfFolds - 1))
				{
					for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.at(j).begin() + ((i + 1) * positiveFoldSize.at(j)), positiveBodies.at(j).end());
						positiveBodiesShuffled.push_back(tempPositiveBodiesShuffled);

						vector<vector<string> > tempPositiveHeadsShuffled(positiveHeads.at(j).begin() + ((i + 1) * positiveFoldSize.at(j)), positiveHeads.at(j).end());
						positiveHeadsShuffled.push_back(tempPositiveHeadsShuffled);

						vector<string> tempPositiveClausesShuffled(positiveClauses.at(j).begin() + ((i + 1) * positiveFoldSize.at(j)), positiveClauses.at(j).end());
						positiveClausesShuffled.push_back(tempPositiveClausesShuffled);
					}

					for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.at(j).begin() + ((i + 1) * negativeFoldSize.at(j)), negativeBodies.at(j).end());
						negativeBodiesShuffled.push_back(tempNegativeBodiesShuffled);

						vector<vector<string> > tempNegativeHeadsShuffled(negativeHeads.at(j).begin() + ((i + 1) * negativeFoldSize.at(j)), negativeHeads.at(j).end());
						negativeHeadsShuffled.push_back(tempNegativeHeadsShuffled);

						vector<string> tempNegativeClausesShuffled(negativeClauses.at(j).begin() + ((i + 1) * negativeFoldSize.at(j)), negativeClauses.at(j).end());
						negativeClausesShuffled.push_back(tempNegativeClausesShuffled);
					}

					if((int)mixedBodies.size() > (int)i)
					{
						vector<vector<string> > tempMixedBodiesShuffled(mixedBodies.begin() + ((i + 1) * mixedFoldSize), mixedBodies.end());
						mixedBodiesShuffled = tempMixedBodiesShuffled;

						vector<vector<string> > tempMixedHeadsShuffled(mixedHeads.begin() + ((i + 1) * mixedFoldSize), mixedHeads.end());
						mixedHeadsShuffled = tempMixedHeadsShuffled;

						vector<string> tempMixedClausesShuffled(mixedClauses.begin() + ((i + 1) * mixedFoldSize), mixedClauses.end());
						mixedClausesShuffled = tempMixedClausesShuffled;
					}

					for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
					{
						unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.at(j).begin(), positiveBodiesShuffled.at(j).end());
						unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.at(j).begin(), positiveHeadsShuffled.at(j).end());
						unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.at(j).begin(), positiveClausesShuffled.at(j).end());
					}

					for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
					{
						unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.at(j).begin(), negativeBodiesShuffled.at(j).end());
						unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.at(j).begin(), negativeHeadsShuffled.at(j).end());
						unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.at(j).begin(), negativeClausesShuffled.at(j).end());
					}

					unitedBodies.insert(unitedBodies.begin(), mixedBodiesShuffled.begin(), mixedBodiesShuffled.end());
					unitedHeads.insert(unitedHeads.begin(), mixedHeadsShuffled.begin(), mixedHeadsShuffled.end());
					unitedClauses.insert(unitedClauses.begin(), mixedClausesShuffled.begin(), mixedClausesShuffled.end());
				}

				// 3.6. Shuffling the data and putting them into the training dataset
				shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
				datasetTrain = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				// 3.7. Putting the current dataset into the dataset list
				datasets.push_back(make_pair(datasetTrain, datasetValidation));
			}
		}
	}
	else
	{
		// Case of no splitting: in this case, the "train portion" or "test portion" parameters will define the train set size

		if(!unitedBodies.empty())
			unitedBodies.clear();
		if(!unitedHeads.empty())
			unitedHeads.clear();
		if(!unitedClauses.empty())
			unitedClauses.clear();

		// If the train portion is 0.0, a rule of thumb presented in Haykin, 2nd ed. will be used:
		// train_portion ~ 1 - (1 / sqrt(2*s)), where size is the number of connections
		if(util.equals(portion, 0.0))
		{
			tempParamTrainPortion = 1.0 - (double)1.0/sqrt(2.0 * (((neuronsLabels.at(INPUT_LAYER).size() + 1) * neuronsLabels.at(HIDDEN_LAYER).size()) + ((neuronsLabels.at(HIDDEN_LAYER).size() + 1) * neuronsLabels.at(OUTPUT_LAYER).size())));
			if(util.lessOrEqual(floor((totalPositiveSize + totalNegativeSize + mixedFoldSize) * tempParamTrainPortion), 1.0))
				tempParamTrainPortion = (double)1.0/(totalPositiveSize + totalNegativeSize + mixedFoldSize);
		}
		else
			tempParamTrainPortion = portion;

		// 3.1. Getting the validation portion of the positive and negative examples
		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
		{
			vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.at(i).begin() + ceil(tempParamTrainPortion * positiveFoldSize.at(i)), positiveBodies.at(i).end());
			positiveBodiesShuffled.push_back(tempPositiveBodiesShuffled);
		}
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
		{
			vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.at(i).begin() + ceil(tempParamTrainPortion * negativeFoldSize.at(i)), negativeBodies.at(i).end());
			negativeBodiesShuffled.push_back(tempNegativeBodiesShuffled);
		}
		vector<vector<string> > tempMixedBodiesShuffled(mixedBodies.begin() + ceil(tempParamTrainPortion * mixedFoldSize), mixedBodies.end());
		mixedBodiesShuffled = tempMixedBodiesShuffled;

		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
		{
			vector<vector<string> > tempPositiveHeadsShuffled(positiveHeads.at(i).begin() + ceil(tempParamTrainPortion * positiveFoldSize.at(i)), positiveHeads.at(i).end());
			positiveHeadsShuffled.push_back(tempPositiveHeadsShuffled);
		}
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
		{
			vector<vector<string> > tempNegativeHeadsShuffled(negativeHeads.at(i).begin() + ceil(tempParamTrainPortion * negativeFoldSize.at(i)), negativeHeads.at(i).end());
			negativeHeadsShuffled.push_back(tempNegativeHeadsShuffled);
		}
		vector<vector<string> > tempMixedHeadsShuffled(mixedHeads.begin() + ceil(tempParamTrainPortion * mixedFoldSize), mixedHeads.end());
		mixedHeadsShuffled = tempMixedHeadsShuffled;

		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
		{
			vector<string> tempPositiveClausesShuffled(positiveClauses.at(i).begin() + ceil(tempParamTrainPortion * positiveFoldSize.at(i)), positiveClauses.at(i).end());
			positiveClausesShuffled.push_back(tempPositiveClausesShuffled);
		}
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
		{
			vector<string> tempNegativeClausesShuffled(negativeClauses.at(i).begin() + ceil(tempParamTrainPortion * negativeFoldSize.at(i)), negativeClauses.at(i).end());
			negativeClausesShuffled.push_back(tempNegativeClausesShuffled);
		}
		vector<string> tempMixedClausesShuffled(mixedClauses.begin() + ceil(tempParamTrainPortion * mixedFoldSize), mixedClauses.end());
		mixedClausesShuffled = tempMixedClausesShuffled;

		// 3.2. Joining the data
		for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
		{
			unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.at(j).begin(), positiveBodiesShuffled.at(j).end());
			unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.at(j).begin(), positiveHeadsShuffled.at(j).end());
			unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.at(j).begin(), positiveClausesShuffled.at(j).end());
		}

		for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
		{
			unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.at(j).begin(), negativeBodiesShuffled.at(j).end());
			unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.at(j).begin(), negativeHeadsShuffled.at(j).end());
			unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.at(j).begin(), negativeClausesShuffled.at(j).end());
		}

		unitedBodies.insert(unitedBodies.begin(), mixedBodiesShuffled.begin(), mixedBodiesShuffled.end());
		unitedHeads.insert(unitedHeads.begin(), mixedHeadsShuffled.begin(), mixedHeadsShuffled.end());
		unitedClauses.insert(unitedClauses.begin(), mixedClausesShuffled.begin(), mixedClausesShuffled.end());

		// 3.3. Shuffling the data and putting them into the training dataset
		shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
		datasetValidation = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

		// 3.4. Getting the training portion of the positive and negative examples before the training part
		if(!unitedBodies.empty())
			unitedBodies.clear();
		if(!unitedHeads.empty())
			unitedHeads.clear();
		if(!unitedClauses.empty())
			unitedClauses.clear();

		positiveBodiesShuffled.clear();
		positiveHeadsShuffled.clear();
		positiveClausesShuffled.clear();

		negativeBodiesShuffled.clear();
		negativeHeadsShuffled.clear();
		negativeClausesShuffled.clear();

		mixedBodiesShuffled.clear();
		mixedHeadsShuffled.clear();
		mixedClausesShuffled.clear();

		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
		{
			vector<vector<string> > tempPositiveBodiesShuffled2(positiveBodies.at(i).begin(), positiveBodies.at(i).begin() + ceil(tempParamTrainPortion * positiveFoldSize.at(i)));
			positiveBodiesShuffled.push_back(tempPositiveBodiesShuffled2);
		}
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
		{
			vector<vector<string> > tempNegativeBodiesShuffled2(negativeBodies.at(i).begin(), negativeBodies.at(i).begin() + ceil(tempParamTrainPortion * negativeFoldSize.at(i)));
			negativeBodiesShuffled.push_back(tempNegativeBodiesShuffled2);
		}
		vector<vector<string> > tempMixedBodiesShuffled2(mixedBodies.begin(), mixedBodies.begin() + ceil(tempParamTrainPortion * mixedFoldSize));
		mixedBodiesShuffled = tempMixedBodiesShuffled2;

		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
		{
			vector<vector<string> > tempPositiveHeadsShuffled2(positiveHeads.at(i).begin(), positiveHeads.at(i).begin() + ceil(tempParamTrainPortion * positiveFoldSize.at(i)));
			positiveHeadsShuffled.push_back(tempPositiveHeadsShuffled2);
		}
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
		{
			vector<vector<string> > tempNegativeHeadsShuffled2(negativeHeads.at(i).begin(), negativeHeads.at(i).begin() + ceil(tempParamTrainPortion * negativeFoldSize.at(i)));
			negativeHeadsShuffled.push_back(tempNegativeHeadsShuffled2);
		}
		vector<vector<string> > tempMixedHeadsShuffled2(mixedHeads.begin(), mixedHeads.begin() + ceil(tempParamTrainPortion * mixedFoldSize));
		mixedHeadsShuffled = tempMixedHeadsShuffled2;

		for(unsigned int i = 0; i < headsPositiveLabels.size(); i++)
		{
			vector<string> tempPositiveClausesShuffled2(positiveClauses.at(i).begin(), positiveClauses.at(i).begin() + ceil(tempParamTrainPortion * positiveFoldSize.at(i)));
			positiveClausesShuffled.push_back(tempPositiveClausesShuffled2);
		}
		for(unsigned int i = 0; i < headsNegativeLabels.size(); i++)
		{
			vector<string> tempNegativeClausesShuffled2(negativeClauses.at(i).begin(), negativeClauses.at(i).begin() + ceil(tempParamTrainPortion * negativeFoldSize.at(i)));
			negativeClausesShuffled.push_back(tempNegativeClausesShuffled2);
		}
		vector<string> tempMixedClausesShuffled2(mixedClauses.begin(), mixedClauses.begin() + ceil(tempParamTrainPortion * mixedFoldSize));
		mixedClausesShuffled = tempMixedClausesShuffled2;

		for(unsigned int j = 0; j < headsPositiveLabels.size(); j++)
		{
			unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.at(j).begin(), positiveBodiesShuffled.at(j).end());
			unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.at(j).begin(), positiveHeadsShuffled.at(j).end());
			unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.at(j).begin(), positiveClausesShuffled.at(j).end());
		}

		for(unsigned int j = 0; j < headsNegativeLabels.size(); j++)
		{
			unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.at(j).begin(), negativeBodiesShuffled.at(j).end());
			unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.at(j).begin(), negativeHeadsShuffled.at(j).end());
			unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.at(j).begin(), negativeClausesShuffled.at(j).end());
		}

		unitedBodies.insert(unitedBodies.begin(), mixedBodiesShuffled.begin(), mixedBodiesShuffled.end());
		unitedHeads.insert(unitedHeads.begin(), mixedHeadsShuffled.begin(), mixedHeadsShuffled.end());
		unitedClauses.insert(unitedClauses.begin(), mixedClausesShuffled.begin(), mixedClausesShuffled.end());

		// 3.6. Shuffling the data and putting them into the training dataset
		shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
		datasetTrain = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

		// 3.7. Putting the current dataset into the dataset list
		datasets.push_back(make_pair(datasetTrain, datasetValidation));
	}

	return datasets;
}

// Method to build a complete dataset from pre-splitted folds
vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > Cilp::buildDatasetFromFolds()
{
	string filePath;
	vector<string> rawLinesTrainingPositive, rawLinesTestingPositive, rawLinesTrainingNegative, rawLinesTestingNegative, rawLinesTraining, rawLinesTesting;

	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > dataset;

	pair<vector<vector<string> >, vector<vector<string> > > decodedExamples1;
	vector<string> features1;

	Utilities util;

	for(int i = 1; i <= paramNumberOfTestFolds; i++)
	{
		switch(FOLD_FILES_FORMAT)
		{
			case 'a':
				filePath = "files/folds/";
				filePath = filePath.append("trainingpositive").append(util.convertToString(i)).append(".data");
				rawLinesTrainingPositive = util.readClauses(filePath);

				filePath = "files/folds/";
				filePath = filePath.append("testingpositive").append(util.convertToString(i)).append(".data");
				rawLinesTestingPositive = util.readClauses(filePath);

				filePath = "files/folds/";
				filePath = filePath.append("trainingnegative").append(util.convertToString(i)).append(".data");
				rawLinesTrainingNegative = util.readClauses(filePath);

				filePath = "files/folds/";
				filePath = filePath.append("testingnegative").append(util.convertToString(i)).append(".data");
				rawLinesTestingNegative = util.readClauses(filePath);

				// Combining the positive and negative examples
				if(!rawLinesTraining.empty())
					rawLinesTraining.clear();
				rawLinesTraining.insert(rawLinesTraining.begin(), rawLinesTrainingPositive.begin(), rawLinesTrainingPositive.end());
				rawLinesTraining.insert(rawLinesTraining.begin(), rawLinesTrainingNegative.begin(), rawLinesTrainingNegative.end());

				if(!rawLinesTesting.empty())
					rawLinesTesting.clear();
				rawLinesTesting.insert(rawLinesTesting.begin(), rawLinesTestingPositive.begin(), rawLinesTestingPositive.end());
				rawLinesTesting.insert(rawLinesTesting.begin(), rawLinesTestingNegative.begin(), rawLinesTestingNegative.end());

				////
				// 1. Obtaining the features table of the example set
//				decodedExamples1 = util.decodeRules(rawLinesTraining);
//
//				for(unsigned int i1 = 0; i1 < rawLinesTraining.size(); i1++)
//					features1.insert(features1.begin(), decodedExamples1.first.at(i1).begin(), decodedExamples1.first.at(i1).end());
//
//				features1 = util.getDistinctElements(features1);
//
//				cout << endl << "Number of BCP Features: " << features1.size() << endl << endl;
				////

				break;

			case 's':
				filePath = "files/folds/";
				filePath = filePath.append("training").append(util.convertToString(i)).append(".data");
				rawLinesTraining = util.readClauses(filePath);

				filePath = "files/folds/";
				filePath = filePath.append("testing").append(util.convertToString(i)).append(".data");
				rawLinesTesting = util.readClauses(filePath);

				break;
		}

		dataset.push_back(make_pair(make_pair(rawLinesTraining, decodeData(rawLinesTraining)), make_pair(rawLinesTesting, decodeData(rawLinesTesting))));
	}

	return dataset;
}

// Method to split the dataset clauses into bodies and heads
pair<vector<vector<string> >, vector<vector<string> > > Cilp::decodeData(vector<string> dataset)
{
	unsigned int pos;

	string headStr, bodyStr, line, tempStr;

	vector<string> trainedExamples, temp;
	vector<vector<string> > bodies, heads;

	for(unsigned int i = 0; i < dataset.size(); i++)
	{
		line = dataset.at(i);

		// This line was generating an error on the algorithm by making clauses to have a different size than bodies and heads
		// if(util.trim(line).size() > 0 && (find(trainedExamples.begin(), trainedExamples.end(), line) == trainedExamples.end()))
		if(util.trim(line).size() > 0)
		{
			trainedExamples.push_back(line);
			pos = line.find(":-");

			if (pos != (unsigned int)string::npos)
			{
				tempStr = line.substr(0, pos);
				headStr = util.trim(tempStr);

				tempStr = line.substr(pos+2, line.size()-pos+1);
				bodyStr = util.trim(tempStr);
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
				temp.push_back(util.trim(tempStr));

				bodyStr.erase(0, pos+1);
				pos = bodyStr.find(",");
			}
			temp.push_back(bodyStr);
			bodies.push_back(temp);

			pos = headStr.find(",");
			temp.clear();
			while(pos != (unsigned int)string::npos)
			{
				tempStr = headStr.substr(0, pos);
				temp.push_back(util.trim(tempStr));

				headStr.erase(0, pos+1);
				pos = headStr.find(",");
			}
			temp.push_back(headStr);
			heads.push_back(temp);
		}
	}

	return make_pair(bodies, heads);
}

// Method to normalize the net weights before training
void Cilp::normalizeWeights(bool weightsOnly)
{
//	int totalWeights = 0;
//	double numerator = 0.0;
//	double denominator;
    double maxW = 0.0;

	// In one iteration through all the net neurons, all normalization factors will be calculated and and applied to them.
	// Using the regular initialization algorithm, |w| <= 1 / (|Aj|)^(1/2), where Aj is the number of connections entering neuron j

	// Obtaining the maximum absolute weight value
	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
	{
		for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
		{
			for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
			{
				if(util.greater(fabs(connectionsWeights.at(i).at(j).at(k).second), maxW))
					maxW = fabs(connectionsWeights.at(i).at(j).at(k).second);
			}
		}
	}

//	weightsMean = 0.0;
//	weightsStd = 0.0;

	// Obtaining the average of the weights
//	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
//	{
//		for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
//		{
//			totalWeights += 1.0;
//			weightsMean += layersBias.at(i).at(j);
//
//			for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
//			{
//				totalWeights += 1.0;
//				weightsMean += connectionsWeights.at(i).at(j).at(k).second;
//			}
//		}
//	}
//	weightsMean = (double)weightsMean/totalWeights;
//
//	// Obtaining the standard deviation of the weights
//	denominator = totalWeights;
//	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
//	{
//		for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
//		{
//			numerator += pow(layersBias.at(i).at(j) - weightsMean, 2);
//
//			for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
//				numerator += pow(connectionsWeights.at(i).at(j).at(k).second - weightsMean, 2);
//		}
//	}
//	weightsStd = sqrt((double)numerator/denominator);

	normalizationFactors.clear();

	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
	{
		if(i != INPUT_LAYER)
		{
			vector<double> tempNormalizationFactors(connectionsWeights.at(i).size(), (double)1.0 / (sqrt((double)connectionsWeights.at(i-1).size()+1.0) * maxW));
			normalizationFactors.push_back(tempNormalizationFactors);
		}
		else
		{
			vector<double> tempNormalizationFactors(connectionsWeights.at(i).size(), 1.0);
			normalizationFactors.push_back(tempNormalizationFactors);
		}
	}

	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
	{
		// Since the net is fully connected, |Aj| is the size of the previous layer
		for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
		{
			for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
				connectionsWeights.at(i).at(j).at(k).second *= normalizationFactors.at((i+1)%3).at(connectionsWeights.at(i).at(j).at(k).first);
		}

		for(unsigned int j = 0; j < connectionsWeights.at((i+1)%3).size(); j++)
		{
			if(!weightsOnly)
				// Normalizing the amins to keep the activation inequalty in the same proportion
				normalizedAmins.at((i+1)%3).at(j) *= normalizationFactors.at((i+1)%3).at(j);
				//normalizedAmins.at((i+1)%3).at(j) *= 1.0;

			// Normalizing the biases
			layersBias.at((i+1)%3).at(j) *= normalizationFactors.at((i+1)%3).at(j);
		}
	}
}

// Method to transform back all the normalized neurons
void Cilp::revertWeights(bool weightsOnly)
{
	if(!normalizedAmins.empty())
		normalizedAmins.clear();

	// Reverting all the weights
	for(unsigned int i = 0; i < connectionsWeights.size(); i++)
	{
		// Ignoring the connections of the output layer, they are fixed
		if(i != OUTPUT_LAYER)
		{
			for(unsigned int j = 0; j < connectionsWeights.at(i).size(); j++)
			{
				for(unsigned int k = 0; k < connectionsWeights.at(i).at(j).size(); k++)
					connectionsWeights.at(i).at(j).at(k).second = (double)connectionsWeights.at(i).at(j).at(k).second / normalizationFactors.at(i+1).at(connectionsWeights.at(i).at(j).at(k).first);
			}

			// Reverting the biases
			for(unsigned int j = 0; j < connectionsWeights.at(i+1).size(); j++)
				layersBias.at(i+1).at(j) = (double)layersBias.at(i+1).at(j) / normalizationFactors.at(i+1).at(j);

			if(!weightsOnly)
			{
				// Reverting the amins
				vector<double> tempNormalizedAmins(connectionsWeights.at(i).size(), amin);
				normalizedAmins.push_back(tempNormalizedAmins);
			}
		}
	}

	if(!normalizationFactors.empty())
		normalizationFactors.clear();
}

// Method to print a dataset into a file named 'dataset.log'. If newFile == true, a new file is made. Otherwise, the data is appended at the end of the existing one.
void Cilp::printDatasetInfo(vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > dataset, bool newFile, string label)
{
	FILE *logFile;

	// 1. Initializing the log file
	if(newFile)
		logFile = fopen("files/dataset.log", "w");
	else
		logFile = fopen("files/dataset.log", "a");

	fprintf(logFile, "*** %s ***\n\n", label.c_str());
	fprintf(logFile, "Dataset size: %d\n\n", (int)dataset.size());

	for(unsigned int i = 0; i < dataset.size(); i++)
	{
		fprintf(logFile, "Dataset %d: %d training data, %d testing data.\n", i+1, (int)dataset.at(i).first.first.size(), (int)dataset.at(i).second.first.size());

		if((dataset.at(i).first.first.size() ==  dataset.at(i).first.second.first.size()) && (dataset.at(i).first.first.size() == dataset.at(i).first.second.second.size()))
			fprintf(logFile, "Bodies, heads and clauses of current training data are of same size? %s\n", "true");
		else
			fprintf(logFile, "Bodies, heads and clauses of current training data are of same size? %s\n", "false");

		if((dataset.at(i).second.first.size() ==  dataset.at(i).second.second.first.size()) && (dataset.at(i).second.first.size() == dataset.at(i).second.second.second.size()))
			fprintf(logFile, "Bodies, heads and clauses of current testing data are of same size? %s\n", "true");
		else
			fprintf(logFile, "Bodies, heads and clauses of current testing data are of same size? %s\n", "false");
	}

	fclose(logFile);
}

// Method to evaluate the back-propagation training stop
bool Cilp::checkStopCriterion(vector<vector<double> > labels, vector<vector<double> > outputs, vector<vector<double> > targetVectors, int trainingIndex, int epochRuns, vector<int>& numberOfStableEpochs, vector<int>& lastNumberOfHits)

{
	vector<int> amountOfHits, dataWithinRange;
	bool isStabilized;

	////////////////////// NON-EPOCH STOPPING CRITERIAS HERE //////////////////////////////////

	///////////////////// END OF NON-EPOCH STOPPING CRITERIAS /////////////////////////////////

	// If a complete run across all training data hasn't been made, training continues
	if(trainingIndex || epochRuns < 2)
		return false;

	amountOfHits.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0);
	dataWithinRange.resize(neuronsLabels.at(OUTPUT_LAYER).size(), 0);

	////////////////////// PER-EPOCH STOPPING CRITERIAS HERE //////////////////////////////////

	// Collecting some statistics
	for(unsigned int i = 0; i < labels.size(); i++)
	{
		for(unsigned int j = 0; j < labels.at(i).size(); j++)
		{
			// The data is considered "a hit" when the error is within HIT_MARGIN
			if(!labels.at(i).at(j) || util.lessOrEqual(abs(labels.at(i).at(j) - outputs.at(i).at(j)), paramHitMargin))
				amountOfHits.at(j)++;

			// The data is considered "within range" when the error is within CORRECTNESS_RANGE
			if(!labels.at(i).at(j) || util.lessOrEqual(abs(labels.at(i).at(j) - outputs.at(i).at(j)), paramCorrectnessRange))
				dataWithinRange.at(j)++;
		}
	}

	for(unsigned int i = 0; i < lastNumberOfHits.size(); i++)
	{
		if(lastNumberOfHits.at(i) < amountOfHits.at(i))
			numberOfStableEpochs.at(i) = 0;
		else
			numberOfStableEpochs.at(i)++;

		lastNumberOfHits.at(i) = amountOfHits.at(i);
	}

	// Criteria 1: if CORRECTNESS_PROPORTION of training data is within CORRECTNESS_RANGE of correct answer, stop
	isStabilized = true;
	for(unsigned int i = 0; i < dataWithinRange.size(); i++)
	{
		if (util.less((double)dataWithinRange.at(i)/labels.size(), paramCorrectnessProportion))
		{
			isStabilized = false;
			break;
		}
	}
	if(isStabilized) return true;

	// Criteria 2: if EPOCH_RUNS times of training epochs has been done, stop
	if (epochRuns >= paramEpochRuns)
		return true;

	// Criteria 3: if the network classifies at least STABILIZATION_PROPORTION of the training examples correctly
	//             but has not improved its classification ability for STABILIZATION_MAX_EPOCHS epochs, stop
	isStabilized = true;
	for(unsigned int i = 0; i < amountOfHits.size(); i++)
	{
		if (!(util.greaterOrEqual((double)amountOfHits.at(i)/labels.size(), paramStabilizationProportion) && (numberOfStableEpochs.at(i) >= paramStabilizationMaxEpochs)))
		{
			isStabilized = false;
			break;
		}
	}
	if(isStabilized) return true;

	///////////////////// END OF PER-EPOCH STOPPING CRITERIAS /////////////////////////////////

	return false;
}

// Method to evaluate the back-propagation training stop (just error value)
bool Cilp::checkStopCriterionSimple(stoppingCriteria kind, vector<vector<double> > trainingErrorFunction, vector<double> validationErrorFunction, double bestValidationError, int trainingIndex, int epochRuns)
{
	int epochSize;
	bool increasesStripTimes;
	double generalizationLoss, averagePreviousTrainingError = 0.0, averageCurrentTrainingError = 0.0;
//	double stripError;
	vector<double> trainingErrorPerEpoch, totalPreviousTrainingError, totalCurrentTrainingError;
	
	totalPreviousTrainingError.resize(trainingErrorFunction.size(), 0.0);
	totalCurrentTrainingError.resize(trainingErrorFunction.size(), 0.0);

	////////////////////// NON-EPOCH STOPPING CRITERIAS HERE //////////////////////////////////

	///////////////////// END OF NON-EPOCH STOPPING CRITERIAS /////////////////////////////////

	// If a complete run across all training data hasn't been made, training continues
	if(trainingIndex || (epochRuns < 2))
		return false;

	// If EPOCH_RUNS times of training epochs has been done, stop
	if (epochRuns >= paramEpochRuns)
		return true;

	////////////////////// PER-EPOCH STOPPING CRITERIAS HERE //////////////////////////////////

	// Calculating current epoch size
	epochSize = trainingErrorFunction.at(0).size() / epochRuns;

	if(kind == standard)
	{
		// Calculating total error from previous epoch
		for(int i = epochSize * (epochRuns-2); i < epochSize * (epochRuns-1); i++)
		{
			for(unsigned int j = 0; j < trainingErrorFunction.size(); j++)
				totalPreviousTrainingError.at(j) += (double)trainingErrorFunction.at(j).at(i)/(double)epochSize;

			for(unsigned int j = 0; j < totalPreviousTrainingError.size(); j++)
				averagePreviousTrainingError += (double)totalPreviousTrainingError.at(j)/(double)totalPreviousTrainingError.size();
		}

		// Calculating total error from current epoch
		for(unsigned int i = epochSize * (epochRuns-1); i < trainingErrorFunction.size(); i++)
		{
			for(unsigned int j = 0; j < trainingErrorFunction.size(); j++)
				totalCurrentTrainingError.at(j) += (double)trainingErrorFunction.at(j).at(i)/(double)epochSize;

			for(unsigned int j = 0; j < totalCurrentTrainingError.size(); j++)
				averageCurrentTrainingError += (double)totalCurrentTrainingError.at(j)/(double)totalCurrentTrainingError.size();
		}

		// Criteria 1: if the whole current epoch error is less than EPSILON and it is proportionally smaller than the total from the last one in a factor of DELTA, stop
//		for(unsigned int i = 0; i < trainingErrorFunction.size(); i++)
//		{
//			if (!(util.lessOrEqual((float)abs(totalCurrentTrainingError.at(i) - totalPreviousTrainingError.at(i)), paramDelta) && util.lessOrEqual(totalCurrentTrainingError.at(i), paramEpsilon)))
//			{
//				isConverged = false;
//				break;
//			}
//		}
//		if(isConverged) return true;

		if (util.lessOrEqual((float)abs(averageCurrentTrainingError - averagePreviousTrainingError), paramDelta) && util.lessOrEqual(averageCurrentTrainingError, paramEpsilon))
			return true;
	}
//	else if((kind == earlyStopping) && paramStripSize && !(epochRuns % paramStripSize))
//	{
//		trainingErrorPerEpoch.resize(paramStripSize, 0.0);
//
//		// Calculating the total stripError
//		for(unsigned int i = epochSize * (epochRuns-paramStripSize); i < trainingErrorFunction.size(); i++)
//		{
//			totalCurrentTrainingError += trainingErrorFunction.at(i);
//			trainingErrorPerEpoch.at(floor((double)(i - (epochSize * (epochRuns-paramStripSize)))/epochSize)) += (double)trainingErrorFunction.at(i)/epochSize;
//		}
//		sort(trainingErrorPerEpoch.begin(), trainingErrorPerEpoch.end());
//		stripError = 10.0 * (((double)totalCurrentTrainingError/(paramStripSize * trainingErrorPerEpoch.at(0))) - 1.0);
//
//		// Calculating total generalization loss
//		generalizationLoss =  100.0 * (((double)validationErrorFunction.at(validationErrorFunction.size()-1) / bestValidationError) - 1.0);
//
//		// Criteria 1: if the generalization does not compensate the training reduction error or the minimum error has been reached, stop
//		if (util.greater((double)generalizationLoss/stripError, paramDelta))
//			return true;
//	}

	else if((kind == earlyStopping) && paramStripSize && (epochRuns > paramStripSize))
	{
		increasesStripTimes = true;

		for(unsigned int i = (validationErrorFunction.size() - paramStripSize); i < validationErrorFunction.size(); i++)
		{
			if(util.greaterOrEqual(validationErrorFunction.at(i-1), validationErrorFunction.at(i)))
			{
				increasesStripTimes = false;
				break;
			}
		}

		if(increasesStripTimes)
			return true;
	}
	else if((kind == earlyStopping) && !paramStripSize)
	{
		// Calculating total generalization loss
		generalizationLoss = 1.0 * (((double)validationErrorFunction.at(validationErrorFunction.size()-1) / bestValidationError) - 1.0);

		// If the generalization loss is bigger than X, stop
		if (util.greater((double)generalizationLoss, paramDelta) && (util.lessOrEqual(bestValidationError, 0.5)))
			return true;
	}

	///////////////////// END OF PER-EPOCH STOPPING CRITERIAS /////////////////////////////////

	return false;
}

// Method to evaluate if 'activation' is considered by the system as activated
// Returns 1.0 if active, 0 if it is unknown and -1.0 if it is inactive
double Cilp::evaluateActivation(double activation, int layer, int neuronIndex, double totalInput, bool canBeStochastic, double& stochasticConfidence)
{
	double randomNumber, correctAmin, aminNormalizationFactor;

	stochasticConfidence = -1.0;

	// Calculating the Amin normalization factor (if NORMALIZE_WEIGHTS_ONLY = true)
	correctAmin = normalizedAmins.at(layer).at(neuronIndex);
	if(NORMALIZE_WEIGHTS_ONLY)
	{
		// Obtaining the input total of the neuron
		totalInput = calculateTotalInput(layer, neuronIndex, false);
		aminNormalizationFactor = (double)(1.0 - exp(-1.0 * paramBeta * totalInput * normalizationFactors.at(layer).at(neuronIndex))) / (1.0 - exp(-1.0 * paramBeta * totalInput));
		correctAmin *= aminNormalizationFactor;
	}

	if(util.lessOrEqual(activation, 1.0) && util.greaterOrEqual(activation, -1.0))
	{
		if(util.greaterOrEqual(activation, correctAmin))
			return 1.0;
		else if(util.lessOrEqual(activation, -correctAmin))
			return -1.0;
		else if(stochastic && canBeStochastic)
		{
			randomNumber = ((double)rand() / RAND_MAX);
			if(util.greaterOrEqual(abs((double)(activation + correctAmin) / (correctAmin * 2.0)), randomNumber))
			{
				stochasticConfidence = abs((double)activation / correctAmin) * 100;
				return 1.0;
			}
			else
			{
				stochasticConfidence = abs((double)activation / correctAmin) * 100;
				return -1.0;
			}
		}
		else
		{
			if(util.greaterOrEqual(activation, 0.0))
				return 1.0;
			else
				return -1.0;
		}
	}
	else
		return 0.0;
}

// Method to calculate the total input in the 'neuronIndex' neuron of layer 'layer'
// before applying its activation function
double Cilp::calculateTotalInput(int layer, int neuronIndex, bool normalized)
{
	int previousLayer;

	double currentWeight, currentBias;
	double totalInput = 0.0;

	// Summing up all the products of weights (unnormalized) and inputs for the neuron
	previousLayer = (((layer-1) < 0) ? 2 : (layer-1));
	for(unsigned int i = 0; i < connectionsWeights.at(previousLayer).size(); i++)
	{
		for(unsigned int j = 0; j < connectionsWeights.at(previousLayer).at(i).size(); j++)
		{
			if(connectionsWeights.at(previousLayer).at(i).at(j).first == neuronIndex)
			{
				currentWeight = connectionsWeights.at(previousLayer).at(i).at(j).second;
				currentWeight = (double)currentWeight / (normalized ? 1.0 : normalizationFactors.at(layer).at(neuronIndex));

				totalInput += (currentWeight * netStatus.at(previousLayer).at(i));
				break;
			}
		}
	}

	// Adding the bias (unnormalized)
	currentBias = layersBias.at(layer).at(neuronIndex);
	currentBias = (double)currentBias / (normalized ? 1.0 : normalizationFactors.at(layer).at(neuronIndex));

	totalInput += currentBias;

	return totalInput;
}

// Method to filter input data using (...) (cannot be used on data with background knowledge!)
pair<vector<vector<string> >, vector<string> > Cilp::filterInputData(pair<vector<vector<string> >, vector<string> > splittedDataset)
{
//	double meanY, varianceY, sumXY, currentSumXY;
//	map<string, double> meansX, varianceX, correlation;
//	pair<map<string, double>::iterator, bool> returnFlag;
//
//	vector<string> dataset;
//
////	Utilities util;
////
////	FILE *filteredTrainData, *bannedListData, *correlationTableData;
//
//	// 1. Collecting the means of all data (inputs and output)
//	meanY = 0.0;
//	for(unsigned int i = 0; i < splittedDataset.first.size(); i++)
//	{
//		if(splittedDataset.second.at(i).at(0) == '~')
//			meanY += ((double) -1.0 / splittedDataset.first.size());
//		else
//			meanY += ((double) 1.0 / splittedDataset.first.size());
//
//		for(unsigned int j = 0; j < splittedDataset.first.at(i).size(); j++)
//		{
//			if(splittedDataset.first.at(i).at(j).at(0) == '~')
//				returnFlag = meansX.insert(pair<string, double>(returnNonnegatedLiteral(splittedDataset.first.at(i).at(j)), (double)-1.0 / splittedDataset.first.size()));
//			else
//				returnFlag = meansX.insert(pair<string, double>(returnNonnegatedLiteral(splittedDataset.first.at(i).at(j)), (double)1.0 / splittedDataset.first.size()));
//
//			if(!returnFlag.second)
//			{
//				if(splittedDataset.first.at(i).at(j).at(0) == '~')
//					(*returnFlag.first).second += ((double)-1.0 / splittedDataset.first.size());
//				else
//					(*returnFlag.first).second += ((double)1.0 / splittedDataset.first.size());
//			}
//		}
//	}
//
//	// 2. Collecting the variances of all data (inputs and output)
//	varianceY = 0.0;
//	for(unsigned int i = 0; i < splittedDataset.first.size(); i++)
//	{
//		if(splittedDataset.second.at(i).at(0) == '~')
//			varianceY += ((double) pow(-1.0 - meanY, 2.0) / (splittedDataset.first.size() - 1));
//		else
//			varianceY += ((double) pow(1.0 - meanY, 2.0) / (splittedDataset.first.size() - 1));
//
//		for(unsigned int j = 0; j < splittedDataset.first.at(i).size(); j++)
//		{
//			if(splittedDataset.first.at(i).at(j).at(0) == '~')
//				returnFlag = varianceX.insert(pair<string, double>(returnNonnegatedLiteral(splittedDataset.first.at(i).at(j)), (double) pow(-1.0 - meansX[returnNonnegatedLiteral(splittedDataset.first.at(i).at(j))], 2.0) / (splittedDataset.first.size() - 1)));
//			else
//				returnFlag = varianceX.insert(pair<string, double>(returnNonnegatedLiteral(splittedDataset.first.at(i).at(j)), (double) pow(1.0 - meansX[returnNonnegatedLiteral(splittedDataset.first.at(i).at(j))], 2.0) / (splittedDataset.first.size() - 1)));
//
//			if(!returnFlag.second)
//			{
//				if(splittedDataset.first.at(i).at(j).at(0) == '~')
//					(*returnFlag.first).second += ((double) pow(-1.0 - meansX[returnNonnegatedLiteral(splittedDataset.first.at(i).at(j))], 2.0) / (splittedDataset.first.size() - 1));
//				else
//					(*returnFlag.first).second += ((double) pow(1.0 - meansX[returnNonnegatedLiteral(splittedDataset.first.at(i).at(j))], 2.0) / (splittedDataset.first.size() - 1));
//			}
//		}
//	}
//
//	// 3. Calculating pearson's correlation for each input, with regard to the output
////	map<string, map<string, double>>::iterator it;
////	for(it = meansX.begin(); iterator != meansX.end(); it++)
////	{
////		    it->first = key
////		    it->second = value
////
////		sumXY = 0.0;
////
////		// TO-DO: see how it will be added to the correlation matrix
////
////		for(unsigned int i = 0; i < splittedDataset.first.size(); i++)
////		{
////			if(splittedDataset.second.at(i).at(0) == '~')
////				currentSumXY = -1.0;
////			else
////				currentSumXY = 1.0;
////			currentSumXY -= meanY;
////
////			for(unsigned int j = 0; j < splittedDataset.first.at(i).size(); j++)
////			{
////				if(splittedDataset.first.at(i).at(j).at(0) == '~')
////					returnFlag = meansX.insert(pair<string, double>(returnNonnegatedLiteral(splittedDataset.first.at(i).at(j)), (double)-1.0 / splittedDataset.first.size()));
////				else
////					returnFlag = meansX.insert(pair<string, double>(returnNonnegatedLiteral(splittedDataset.first.at(i).at(j)), (double)1.0 / splittedDataset.first.size()));
////
////				if(!returnFlag.second)
////				{
////					if(splittedDataset.first.at(i).at(j).at(0) == '~')
////						(*returnFlag.first).second += ((double)-1.0 / splittedDataset.first.size());
////					else
////						(*returnFlag.first).second += ((double)1.0 / splittedDataset.first.size());
////				}
////			}
////		}
////	}
//
	return splittedDataset;
}

// Method to return a non negated literal of the given input
string Cilp::returnNonnegatedLiteral(string data)
{
	if(data.at(0) == '~')
		data = data.substr(1);

	return data;
}

// Method to prune input neurons from the already built neural network, in which half will be pruned from the beginning and half from the end.
// This is used as a second validation parameter
void Cilp::pruneInputNeurons(int pruningSize)
{
	int pruneSplitting = pruningSize/2;
	int index, connectionIndex, numberOfJumpedInputs;
	vector<int> inputNeuronsToBeDeleted, outputConnectionsToBeAdjusted;

//	double initialValue = 0.0;
//	vector<pair<int, double> > newWeightVector;

	if(!pruneSplitting)
		return;

	// 1. Choosing ending indexes to be deleted
	numberOfJumpedInputs = 0;
	index = neuronsLabels.at(INPUT_LAYER).size()-1;
	for(int i = 0; i < pruneSplitting; index--)
	{
		// If there are output neuron with same label as the current input one, it cannot be deleted
		connectionIndex = util.find(neuronsLabels.at(OUTPUT_LAYER), neuronsLabels.at(INPUT_LAYER).at(index));
		if(connectionIndex < 0)
		{
			if(util.find(inputNeuronsToBeDeleted, index) < 0)
				inputNeuronsToBeDeleted.push_back(index);
			i++;
		}
		else
			outputConnectionsToBeAdjusted.push_back(connectionIndex);
	}
	index = 0;
	for(int i = outputConnectionsToBeAdjusted.size()-1; i >= 0; i--)
	{
		(connectionsWeights.at(OUTPUT_LAYER).at(outputConnectionsToBeAdjusted.at(i)).at(0).first) = neuronsLabels.at(INPUT_LAYER).size() - pruneSplitting - outputConnectionsToBeAdjusted.size() + index;
		index++;
	}

	// 2. Removing each chosen input neuron
	for(unsigned int i = 0; i < inputNeuronsToBeDeleted.size(); i++)
	{
		neuronsLabels.at(INPUT_LAYER).erase(neuronsLabels.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		normalizationFactors.at(INPUT_LAYER).erase(normalizationFactors.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		normalizedAmins.at(INPUT_LAYER).erase(normalizedAmins.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		netStatus.at(INPUT_LAYER).erase(netStatus.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		thresholds.at(INPUT_LAYER).erase(thresholds.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		layersBias.at(INPUT_LAYER).erase(layersBias.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		connectionsWeights.at(INPUT_LAYER).erase(connectionsWeights.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
	}

	// 3. Choosing starting indexes to be deleted (and updating the ones that will not be deleted)
	if(!inputNeuronsToBeDeleted.empty())
		inputNeuronsToBeDeleted.clear();
	index = 0;
	numberOfJumpedInputs = 0;
	for(int i = 0; i < pruneSplitting; index++)
	{
		// If there are output neuron with same label as the current input one, it cannot be deleted
		connectionIndex = util.find(neuronsLabels.at(OUTPUT_LAYER), neuronsLabels.at(INPUT_LAYER).at(index));
		if(connectionIndex < 0)
		{
			if(util.find(inputNeuronsToBeDeleted, index) < 0)
				inputNeuronsToBeDeleted.push_back(index);
			i++;
		}
		else
		{
			(connectionsWeights.at(OUTPUT_LAYER).at(connectionIndex).at(0).first) -= index;
			(connectionsWeights.at(OUTPUT_LAYER).at(connectionIndex).at(0).first) += numberOfJumpedInputs;
			numberOfJumpedInputs++;
		}
	}

	// 4. Removing chosen starting input neurons, starting from the last one
	for(int i = inputNeuronsToBeDeleted.size()-1; i >= 0; i--)
	{
		neuronsLabels.at(INPUT_LAYER).erase(neuronsLabels.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		normalizationFactors.at(INPUT_LAYER).erase(normalizationFactors.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		normalizedAmins.at(INPUT_LAYER).erase(normalizedAmins.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		netStatus.at(INPUT_LAYER).erase(netStatus.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		thresholds.at(INPUT_LAYER).erase(thresholds.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		layersBias.at(INPUT_LAYER).erase(layersBias.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
		connectionsWeights.at(INPUT_LAYER).erase(connectionsWeights.at(INPUT_LAYER).begin()+inputNeuronsToBeDeleted.at(i));
	}

	// 5. Correcting the remaining output connections
	for(unsigned int i = numberOfJumpedInputs; i < connectionsWeights.at(OUTPUT_LAYER).size(); i++)
		(connectionsWeights.at(OUTPUT_LAYER).at(i).at(0).first) -= pruneSplitting;
}

vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > Cilp::buildDatasetFromIncrementalFolds(int numberOfExperiments)
{
	// TODO Adapt this method

	string filePath;
	vector<string> rawLinesTrainingPositive, rawLinesTestingPositive, rawLinesTrainingNegative, rawLinesTestingNegative, rawLinesTraining, rawLinesTesting;

	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > dataset;
	vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > datasetsIncremental;

	Utilities util;

	for(int i = 1; i <= paramNumberOfTestFolds; i++)
	{
		filePath = "files/folds/";
		filePath = filePath.append("trainingpositive").append(util.convertToString(i)).append(".data");
		rawLinesTrainingPositive = util.readClauses(filePath);

		filePath = "files/folds/";
		filePath = filePath.append("testingpositive").append(util.convertToString(i)).append(".data");
		rawLinesTestingPositive = util.readClauses(filePath);

		filePath = "files/folds/";
		filePath = filePath.append("trainingnegative").append(util.convertToString(i)).append(".data");
		rawLinesTrainingNegative = util.readClauses(filePath);

		filePath = "files/folds/";
		filePath = filePath.append("testingnegative").append(util.convertToString(i)).append(".data");
		rawLinesTestingNegative = util.readClauses(filePath);

		// Combining the positive and negative examples
		if(!rawLinesTraining.empty())
			rawLinesTraining.clear();
		rawLinesTraining.insert(rawLinesTraining.begin(), rawLinesTrainingPositive.begin(), rawLinesTrainingPositive.end());
		rawLinesTraining.insert(rawLinesTraining.begin(), rawLinesTrainingNegative.begin(), rawLinesTrainingNegative.end());

		if(!rawLinesTesting.empty())
			rawLinesTesting.clear();
		rawLinesTesting.insert(rawLinesTesting.begin(), rawLinesTestingPositive.begin(), rawLinesTestingPositive.end());
		rawLinesTesting.insert(rawLinesTesting.begin(), rawLinesTestingNegative.begin(), rawLinesTestingNegative.end());

		// TODO: if implementing incremental experiments, this line needs to be fixed!
//		dataset.push_back(make_pair(make_pair(rawLinesTraining, decodeData(rawLinesTraining)), make_pair(rawLinesTesting, decodeData(rawLinesTesting))));
	}

	return datasetsIncremental;
//	return dataset;
}

// Method to generate TREPAN-formatted files for generating decision trees
void Cilp::trepanExporter()
{

}

vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > Cilp::splitDataIncrementallyByTarget(int numberOfExperiments, vector<string> dataset, vector<vector<string> > bodies, vector<string> heads, int numberOfFolds)
{
	// TODO Adapt this method

	bool moreFoldsThanData = false;
	int positiveFoldSize, negativeFoldSize;
	double tempParamTrainPortion;

	pair<vector<string>, pair<vector<vector<string> >, vector<string> > > datasetTrain, datasetValidation;
	vector<vector<string> > positiveBodies, negativeBodies, positiveBodiesShuffled, negativeBodiesShuffled, unitedBodies;
	vector<string> positiveHeads, negativeHeads, positiveClauses, negativeClauses, positiveHeadsShuffled, negativeHeadsShuffled, positiveClausesShuffled, negativeClausesShuffled, unitedClauses, unitedHeads;

	vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > datasets;
	vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > datasetsIncremental;

	// 1.  Shuffling and splitting the data by occurrences of positive and negative examples
	//if(numberOfFolds < 0) return datasets;
	if(numberOfFolds < 0) return datasetsIncremental;

	// TODO: when dealing with incremental experiments, adaptation to multiple heads is required
//	shuffleTrainData(dataset, heads, bodies);
	for(unsigned int i = 0; i < heads.size(); i++)
	{
		if(heads.at(i).find("~") != string::npos)
		{
			negativeHeads.push_back(heads.at(i));
			negativeBodies.push_back(bodies.at(i));
			negativeClauses.push_back(dataset.at(i));
		}
		else
		{
			positiveHeads.push_back(heads.at(i));
			positiveBodies.push_back(bodies.at(i));
			positiveClauses.push_back(dataset.at(i));
		}
	}

	// 2. Calculating both groups sizes
	if(numberOfFolds > 1)
	{
		positiveFoldSize = positiveBodies.size() / numberOfFolds;
		negativeFoldSize = negativeBodies.size() / numberOfFolds;

		if(!positiveFoldSize && !negativeFoldSize)
			moreFoldsThanData = true;

		if(!positiveFoldSize)
			positiveFoldSize++;
		if(!negativeFoldSize)
			negativeFoldSize++;
	}
	else
	{
		positiveFoldSize = positiveBodies.size();
		negativeFoldSize = negativeBodies.size();
	}

	// 3. Iterating through all the examples and putting them into the <numberOfFolds> folds
	if(numberOfFolds > 1)
	{
		// Checking if none of the positive or negative examples are bigger than the number of folds
		if(moreFoldsThanData)
		{
			for(unsigned int d = 0; d < dataset.size(); d++)
			{
				// In this case, all examples will be shuffled together and then, distributed among the folds.
				// Leave-one-out cross-validation will be used.
				for(unsigned int i = 0; i < dataset.size(); i++)
				{
					if(i != d)
					{
						if(!unitedBodies.empty())
							unitedBodies.clear();
						if(!unitedHeads.empty())
							unitedHeads.clear();
						if(!unitedClauses.empty())
							unitedClauses.clear();
					}
				}
				datasetTrain = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();

				unitedBodies.push_back(bodies.at(d));
				unitedHeads.push_back(heads.at(d));
				unitedClauses.push_back(dataset.at(d));

				datasetValidation = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				datasets.push_back(make_pair(datasetTrain, datasetValidation));

				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();
			}
		}
		else
		{
			for(int i = 0; i < numberOfFolds; i++)
			{
				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();

				// 3.1. Getting the validation portion of the positive and negative examples
				if(i != (numberOfFolds - 1))
				{
					if((int)positiveBodies.size() > (int)i)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.begin() + (i * positiveFoldSize), positiveBodies.begin() + ((i + 1) * positiveFoldSize));
						positiveBodiesShuffled = tempPositiveBodiesShuffled;

						vector<string> tempPositiveHeadsShuffled(positiveHeads.begin() + (i * positiveFoldSize), positiveHeads.begin() + ((i + 1) * positiveFoldSize));
						positiveHeadsShuffled = tempPositiveHeadsShuffled;

						vector<string> tempPositiveClausesShuffled(positiveClauses.begin() + (i * positiveFoldSize), positiveClauses.begin() + ((i + 1) * positiveFoldSize));
						positiveClausesShuffled = tempPositiveClausesShuffled;
					}
					else
					{
						if(!positiveBodiesShuffled.empty())
							positiveBodiesShuffled.clear();
						if(!positiveHeadsShuffled.empty())
							positiveHeadsShuffled.clear();
						if(!positiveClausesShuffled.empty())
							positiveClausesShuffled.clear();
					}

					if((int)negativeBodies.size() > (int)i)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.begin() + (i * negativeFoldSize), negativeBodies.begin() + ((i + 1) * negativeFoldSize));
						negativeBodiesShuffled = tempNegativeBodiesShuffled;

						vector<string> tempNegativeHeadsShuffled(negativeHeads.begin() + (i * negativeFoldSize), negativeHeads.begin() + ((i + 1) * negativeFoldSize));
						negativeHeadsShuffled = tempNegativeHeadsShuffled;

						vector<string> tempNegativeClausesShuffled(negativeClauses.begin() + (i * negativeFoldSize), negativeClauses.begin() + ((i + 1) * negativeFoldSize));
						negativeClausesShuffled = tempNegativeClausesShuffled;
					}
					else
					{
						if(!negativeBodiesShuffled.empty())
							negativeBodiesShuffled.clear();
						if(!negativeHeadsShuffled.empty())
							negativeHeadsShuffled.clear();
						if(!negativeClausesShuffled.empty())
							negativeClausesShuffled.clear();
					}
				}
				else
				{
					if((int)positiveBodies.size() > (int)i)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.begin() + (i * positiveFoldSize), positiveBodies.end());
						positiveBodiesShuffled = tempPositiveBodiesShuffled;

						vector<string> tempPositiveHeadsShuffled(positiveHeads.begin() + (i * positiveFoldSize), positiveHeads.end());
						positiveHeadsShuffled = tempPositiveHeadsShuffled;

						vector<string> tempPositiveClausesShuffled(positiveClauses.begin() + (i * positiveFoldSize), positiveClauses.end());
						positiveClausesShuffled = tempPositiveClausesShuffled;
					}
					else
					{
						if(!positiveBodiesShuffled.empty())
							positiveBodiesShuffled.clear();
						if(!positiveHeadsShuffled.empty())
							positiveHeadsShuffled.clear();
						if(!positiveClausesShuffled.empty())
							positiveClausesShuffled.clear();
					}

					if((int)negativeBodies.size() > (int)i)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.begin() + (i * negativeFoldSize), negativeBodies.end());
						negativeBodiesShuffled = tempNegativeBodiesShuffled;

						vector<string> tempNegativeHeadsShuffled(negativeHeads.begin() + (i * negativeFoldSize), negativeHeads.end());
						negativeHeadsShuffled = tempNegativeHeadsShuffled;

						vector<string> tempNegativeClausesShuffled(negativeClauses.begin() + (i * negativeFoldSize), negativeClauses.end());
						negativeClausesShuffled = tempNegativeClausesShuffled;
					}
					else
					{
						if(!negativeBodiesShuffled.empty())
							negativeBodiesShuffled.clear();
						if(!negativeHeadsShuffled.empty())
							negativeHeadsShuffled.clear();
						if(!negativeClausesShuffled.empty())
							negativeClausesShuffled.clear();
					}
				}

				// 3.2. Joining the data
				unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.begin(), positiveBodiesShuffled.end());
				unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.begin(), negativeBodiesShuffled.end());
				unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.begin(), positiveHeadsShuffled.end());
				unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.begin(), negativeHeadsShuffled.end());
				unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.begin(), positiveClausesShuffled.end());
				unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.begin(), negativeClausesShuffled.end());

				// 3.3. Shuffling the data and putting them into the training dataset
				// TODO: adapt this method to work with multiple heads/logs
//				shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
				datasetValidation = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				// 3.4. Getting the training portion of the positive and negative examples before the training part
				if(!unitedBodies.empty())
					unitedBodies.clear();
				if(!unitedHeads.empty())
					unitedHeads.clear();
				if(!unitedClauses.empty())
					unitedClauses.clear();

				if(i != 0)
				{
					if((int)positiveBodies.size() > (int)i)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.begin(), positiveBodies.begin() + (i * positiveFoldSize));
						positiveBodiesShuffled = tempPositiveBodiesShuffled;

						vector<string> tempPositiveHeadsShuffled(positiveHeads.begin(), positiveHeads.begin() + (i * positiveFoldSize));
						positiveHeadsShuffled = tempPositiveHeadsShuffled;

						vector<string> tempPositiveClausesShuffled(positiveClauses.begin(), positiveClauses.begin() + (i * positiveFoldSize));
						positiveClausesShuffled = tempPositiveClausesShuffled;
					}
					else
					{
						if(!positiveBodiesShuffled.empty())
							positiveBodiesShuffled.clear();
						if(!positiveHeadsShuffled.empty())
							positiveHeadsShuffled.clear();
						if(!positiveClausesShuffled.empty())
							positiveClausesShuffled.clear();
					}

					if((int)negativeBodies.size() > (int)i)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.begin(), negativeBodies.begin() + (i * negativeFoldSize));
						negativeBodiesShuffled = tempNegativeBodiesShuffled;

						vector<string> tempNegativeHeadsShuffled(negativeHeads.begin(), negativeHeads.begin() + (i * negativeFoldSize));
						negativeHeadsShuffled = tempNegativeHeadsShuffled;

						vector<string> tempNegativeClausesShuffled(negativeClauses.begin(), negativeClauses.begin() + (i * negativeFoldSize));
						negativeClausesShuffled = tempNegativeClausesShuffled;
					}
					else
					{
						if(!negativeBodiesShuffled.empty())
							negativeBodiesShuffled.clear();
						if(!negativeHeadsShuffled.empty())
							negativeHeadsShuffled.clear();
						if(!negativeClausesShuffled.empty())
							negativeClausesShuffled.clear();
					}

					unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.begin(), positiveBodiesShuffled.end());
					unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.begin(), negativeBodiesShuffled.end());
					unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.begin(), positiveHeadsShuffled.end());
					unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.begin(), negativeHeadsShuffled.end());
					unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.begin(), positiveClausesShuffled.end());
					unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.begin(), negativeClausesShuffled.end());
				}

				// 3.5. Getting the validation portion of the positive and negative examples after the training part
				if(i != (numberOfFolds - 1))
				{
					if((int)positiveBodies.size() > (int)i)
					{
						vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.begin() + ((i + 1) * positiveFoldSize), positiveBodies.end());
						positiveBodiesShuffled = tempPositiveBodiesShuffled;

						vector<string> tempPositiveHeadsShuffled(positiveHeads.begin() + ((i + 1) * positiveFoldSize), positiveHeads.end());
						positiveHeadsShuffled = tempPositiveHeadsShuffled;

						vector<string> tempPositiveClausesShuffled(positiveClauses.begin() + ((i + 1) * positiveFoldSize), positiveClauses.end());
						positiveClausesShuffled = tempPositiveClausesShuffled;
					}
					else
					{
						if(!positiveBodiesShuffled.empty())
							positiveBodiesShuffled.clear();
						if(!positiveHeadsShuffled.empty())
							positiveHeadsShuffled.clear();
						if(!positiveClausesShuffled.empty())
							positiveClausesShuffled.clear();
					}

					if((int)negativeBodies.size() > i)
					{
						vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.begin() + ((i + 1) * negativeFoldSize), negativeBodies.end());
						negativeBodiesShuffled = tempNegativeBodiesShuffled;

						vector<string> tempNegativeHeadsShuffled(negativeHeads.begin() + ((i + 1) * negativeFoldSize), negativeHeads.end());
						negativeHeadsShuffled = tempNegativeHeadsShuffled;

						vector<string> tempNegativeClausesShuffled(negativeClauses.begin() + ((i + 1) * negativeFoldSize), negativeClauses.end());
						negativeClausesShuffled = tempNegativeClausesShuffled;
					}
					else
					{
						if(!negativeBodiesShuffled.empty())
							negativeBodiesShuffled.clear();
						if(!negativeHeadsShuffled.empty())
							negativeHeadsShuffled.clear();
						if(!negativeClausesShuffled.empty())
							negativeClausesShuffled.clear();
					}

					unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.begin(), positiveBodiesShuffled.end());
					unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.begin(), negativeBodiesShuffled.end());
					unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.begin(), positiveHeadsShuffled.end());
					unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.begin(), negativeHeadsShuffled.end());
					unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.begin(), positiveClausesShuffled.end());
					unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.begin(), negativeClausesShuffled.end());
				}

				// 3.6. Shuffling the data and putting them into the training dataset
				// TODO: Correct this to do incrementally with multiple heads/logs
//				shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
				datasetTrain = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

				// 3.7. Putting the current dataset into the dataset list
				datasets.push_back(make_pair(datasetTrain, datasetValidation));
			}
		}
	}
	else
	{
		// Case of no splitting: in this case, the "train portion" parameter will define the train set size
		if(!unitedBodies.empty())
			unitedBodies.clear();
		if(!unitedHeads.empty())
			unitedHeads.clear();
		if(!unitedClauses.empty())
			unitedClauses.clear();

		// If the train portion is 0.0, a rule of thumb presented in Haykin, 2nd ed. will be used:
		// train_portion ~ 1 - (1 / sqrt(2*s)), where size is the number of connections
		if(util.equals(paramTrainPortion, 0.0))
		{
			tempParamTrainPortion = 1.0 - (double)1.0/sqrt(2.0 * (((neuronsLabels.at(INPUT_LAYER).size() + 1) * neuronsLabels.at(HIDDEN_LAYER).size()) + ((neuronsLabels.at(HIDDEN_LAYER).size() + 1) * neuronsLabels.at(OUTPUT_LAYER).size())));
			if(util.lessOrEqual(floor((positiveFoldSize + negativeFoldSize) * tempParamTrainPortion), 1.0))
				tempParamTrainPortion = (double)1.0/(positiveFoldSize + negativeFoldSize);
		}
		else
			tempParamTrainPortion = paramTrainPortion;

		// 3.1. Getting the validation portion of the positive and negative examples
		vector<vector<string> > tempPositiveBodiesShuffled(positiveBodies.begin() + floor(tempParamTrainPortion * positiveFoldSize), positiveBodies.end());
		positiveBodiesShuffled = tempPositiveBodiesShuffled;
		vector<vector<string> > tempNegativeBodiesShuffled(negativeBodies.begin() + floor(tempParamTrainPortion * negativeFoldSize), negativeBodies.end());
		negativeBodiesShuffled = tempNegativeBodiesShuffled;

		vector<string> tempPositiveHeadsShuffled(positiveHeads.begin() + floor(tempParamTrainPortion * positiveFoldSize), positiveHeads.end());
		positiveHeadsShuffled = tempPositiveHeadsShuffled;
		vector<string> tempNegativeHeadsShuffled(negativeHeads.begin() + floor(tempParamTrainPortion * negativeFoldSize), negativeHeads.end());
		negativeHeadsShuffled = tempNegativeHeadsShuffled;

		vector<string> tempPositiveClausesShuffled(positiveClauses.begin() + floor(tempParamTrainPortion * positiveFoldSize), positiveClauses.end());
		positiveClausesShuffled = tempPositiveClausesShuffled;
		vector<string> tempNegativeClausesShuffled(negativeClauses.begin() + floor(tempParamTrainPortion * negativeFoldSize), negativeClauses.end());
		negativeClausesShuffled = tempNegativeClausesShuffled;

		// 3.2. Joining the data
		unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.begin(), positiveBodiesShuffled.end());
		unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.begin(), negativeBodiesShuffled.end());
		unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.begin(), positiveHeadsShuffled.end());
		unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.begin(), negativeHeadsShuffled.end());
		unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.begin(), positiveClausesShuffled.end());
		unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.begin(), negativeClausesShuffled.end());

		// 3.3. Shuffling the data and putting them into the training dataset
		// TODO: adapt this method to work with multiple heads/logs
//		shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
		datasetValidation = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

		// 3.4. Getting the training portion of the positive and negative examples before the training part
		if(!unitedBodies.empty())
			unitedBodies.clear();
		if(!unitedHeads.empty())
			unitedHeads.clear();
		if(!unitedClauses.empty())
			unitedClauses.clear();

		vector<vector<string> > tempPositiveBodiesShuffled2(positiveBodies.begin(), positiveBodies.begin() + floor(tempParamTrainPortion * positiveFoldSize));
		positiveBodiesShuffled = tempPositiveBodiesShuffled2;
		vector<vector<string> > tempNegativeBodiesShuffled2(negativeBodies.begin(), negativeBodies.begin() + floor(tempParamTrainPortion * negativeFoldSize));
		negativeBodiesShuffled = tempNegativeBodiesShuffled2;

		vector<string> tempPositiveHeadsShuffled2(positiveHeads.begin(), positiveHeads.begin() + floor(tempParamTrainPortion * positiveFoldSize));
		positiveHeadsShuffled = tempPositiveHeadsShuffled2;
		vector<string> tempNegativeHeadsShuffled2(negativeHeads.begin(), negativeHeads.begin() + floor(tempParamTrainPortion * negativeFoldSize));
		negativeHeadsShuffled = tempNegativeHeadsShuffled2;

		vector<string> tempPositiveClausesShuffled2(positiveClauses.begin(), positiveClauses.begin() + floor(tempParamTrainPortion * positiveFoldSize));
		positiveClausesShuffled = tempPositiveClausesShuffled2;
		vector<string> tempNegativeClausesShuffled2(negativeClauses.begin(), negativeClauses.begin() + floor(tempParamTrainPortion * negativeFoldSize));
		negativeClausesShuffled = tempNegativeClausesShuffled2;

		unitedBodies.insert(unitedBodies.begin(), positiveBodiesShuffled.begin(), positiveBodiesShuffled.end());
		unitedBodies.insert(unitedBodies.begin(), negativeBodiesShuffled.begin(), negativeBodiesShuffled.end());
		unitedHeads.insert(unitedHeads.begin(), positiveHeadsShuffled.begin(), positiveHeadsShuffled.end());
		unitedHeads.insert(unitedHeads.begin(), negativeHeadsShuffled.begin(), negativeHeadsShuffled.end());
		unitedClauses.insert(unitedClauses.begin(), positiveClausesShuffled.begin(), positiveClausesShuffled.end());
		unitedClauses.insert(unitedClauses.begin(), negativeClausesShuffled.begin(), negativeClausesShuffled.end());

		// 3.6. Shuffling the data and putting them into the training dataset
		// TODO: adapt this method to work with multiple heads/logs
//		shuffleTrainData(unitedClauses, unitedHeads, unitedBodies);
		datasetTrain = make_pair(unitedClauses, make_pair(unitedBodies, unitedHeads));

		// 3.7. Putting the current dataset into the dataset list
		datasets.push_back(make_pair(datasetTrain, datasetValidation));
	}

	return datasetsIncremental;
//	return datasets;
}

// Method to build an initial model by replicating the found relations between the BCP features and the BK onto new BK rules
void Cilp::buildReplicatedBKNet(vector<string> backgroundKnowledge, vector<string> bcpExamples, bool printLog)
{
	FILE *logFile = NULL;
	int index, index2, currentIndex, total, variablesTotal, size, previousSize, numberOfIterations;
	bool found, hasTarget;
	string literal, content, predicate, newBKClause, newHeadPart, newVariable;
	vector<int> newIndexes, chainedPositions, mappingsIndex;
	vector<string> features, targetFeatures, variables, storedVariables, newStoredVariables, storedHeadVariables, newBK, literalsList, splittedDisjunction, newArtificialVariables;
	vector<vector<int> > foundMapping, foundHeadMappings;
	vector<vector<string> > variableMappingsPerPredicate;
	vector<pair<string, int> > foundHeadVariables;
	vector<vector<vector<int> > > foundMappings;
	vector<pair<int, int> > equalPositions;
	vector<pair<string, vector<int> > > bodyMappings;
	vector<pair<string, vector<string> > > headMappings;
	pair<vector<vector<string> >, vector<vector<string> > > decodedExamples, decodedTargetExamples, decodedBK;

	if(printLog) logFile = fopen("files/main.log", "a");

	// 1. Obtaining the features table of the example set
	decodedExamples = util.decodeRules(bcpExamples);

	for(unsigned int i = 0; i < bcpExamples.size(); i++)
		features.insert(features.begin(), decodedExamples.first.at(i).begin(), decodedExamples.first.at(i).end());

	features = util.getDistinctElements(features);

	// 2. Mapping the background knowledge relations without target predicates in their body and creating a new set of background knowledge rules
	decodedBK = util.decodeRules(backgroundKnowledge);

	newBK.clear();
	numberOfIterations = 0;
	previousSize = -1;
	while((previousSize != (int)newBK.size()) && (numberOfIterations < paramMaxReplicationRecursion))
	{
		numberOfIterations++;
		previousSize = newBK.size();

		// If there is an already created set of replicated background rules, their heads need to be inserted as possible features
		if(newBK.size())
		{
			decodedTargetExamples = util.decodeRules(newBK);

			for(unsigned int i = 0; i < newBK.size(); i++)
				features.insert(features.begin(), decodedTargetExamples.second.at(i).begin(), decodedTargetExamples.second.at(i).end());

			features = util.getDistinctElements(features);
		}

		for(unsigned int i = 0; i < backgroundKnowledge.size(); i++)
		{
			hasTarget = false;

			storedVariables.clear();
			headMappings.clear();
			bodyMappings.clear();

			// Collecting head mappings
			for(unsigned int j = 0; j < decodedBK.second.at(i).size(); j++)
			{
				literal = decodedBK.second.at(i).at(j);

				// Dealing with disjunction
				if(literal.find("|") != string::npos)
				{
					splittedDisjunction = util.splitDelimited(literal, '|');

					for(unsigned int k = 0; k < splittedDisjunction.size(); k++)
					{
						literal   = splittedDisjunction.at(k);
						predicate = (literal.find("(") != string::npos) ? literal.substr(0, literal.find("(")) : literal;
						content   = (literal.find("(") != string::npos) ? literal.substr(literal.find("(")+1, literal.find(")")-literal.find("(")-1) : "";

						variables.clear();
						variables = (content.find(";") != string::npos) ? util.splitDelimited(content, ';') : variables;

						if(!content.empty() && variables.empty())
							variables.push_back(content);

						for(unsigned int l = 0; l < variables.size(); l++)
						{
							if(util.find(storedVariables, variables.at(l)) < 0)
								storedVariables.push_back(variables.at(l));
						}

						headMappings.push_back(make_pair(predicate, variables));
						variables.clear();
						headMappings.push_back(make_pair("|", variables));
					}

					headMappings.erase(headMappings.begin() + headMappings.size() - 1);
				}
				else
				{
					predicate = (literal.find("(") != string::npos) ? literal.substr(0, literal.find("(")) : literal;
					content   = (literal.find("(") != string::npos) ? literal.substr(literal.find("(")+1, literal.find(")")-literal.find("(")-1) : "";

					variables.clear();
					variables = (content.find(";") != string::npos) ? util.splitDelimited(content, ';') : variables;

					if(!content.empty() && variables.empty())
						variables.push_back(content);

					for(unsigned int k = 0; k < variables.size(); k++)
					{
						if(util.find(storedVariables, variables.at(k)) < 0)
							storedVariables.push_back(variables.at(k));
					}

					headMappings.push_back(make_pair(predicate, variables));
				}
			}

			storedHeadVariables = storedVariables;

			// Collecting body mappings
			variablesTotal = 0;
			foundHeadMappings.clear();
			variableMappingsPerPredicate.clear();
			for(unsigned int j = 0; j < decodedBK.first.at(i).size(); j++)
			{
				literal   = decodedBK.first.at(i).at(j);
				predicate = literal.substr(0, literal.find("("));
				content   = literal.substr(literal.find("(")+1, literal.find(")")-literal.find("(")-1);

				variables = (content.find(";") != string::npos) ? util.splitDelimited(content, ';') : variables;

				variableMappingsPerPredicate.push_back(variables);

				for(unsigned int k = 0; k < decodedBK.second.at(i).size(); k++)
				{
					if(!decodedBK.second.at(i).at(k).substr(0, decodedBK.second.at(i).at(k).find("(")).compare(predicate))
					{
						hasTarget = true;
						break;
					}
				}
				if(hasTarget)
					break;

				newIndexes.clear();
				for(unsigned int k = 0; k < variables.size(); k++)
				{
					newIndexes.push_back(util.find(storedHeadVariables, variables.at(k)));

					variablesTotal++;
					if(util.find(storedVariables, variables.at(k)) < 0)
						storedVariables.push_back(variables.at(k));
				}
				foundHeadMappings.push_back(newIndexes);

				newIndexes.clear();
				for(unsigned int k = 0; k < variables.size(); k++)
					newIndexes.push_back(util.find(storedVariables, variables.at(k)));

				bodyMappings.push_back(make_pair(predicate, newIndexes));
			}

			// Converting the mappings to chaining
			index = 0;
			currentIndex = 0;
			chainedPositions.clear();
			for(unsigned int j = 0; j < bodyMappings.size(); j++)
			{
				for(unsigned int k = 0; k < bodyMappings.at(j).second.size(); k++, index++)
				{
					if((int)chainedPositions.size() < index+1)
						chainedPositions.resize(index+1, -1);

					if(chainedPositions.at(index) < 0)
					{
						currentIndex++;
						chainedPositions.at(index) = currentIndex;
						index2 = index;
						for(unsigned int l = j; l < bodyMappings.size(); l++)
						{
							unsigned int m = (l == j) ? (k+1) : 0;
							for(; m < bodyMappings.at(l).second.size(); m++)
							{
								index2++;
								if(bodyMappings.at(j).second.at(k) == bodyMappings.at(l).second.at(m))
								{
									if((int)chainedPositions.size() < index2+1)
										chainedPositions.resize(index2+1, -1);
									chainedPositions.at(index2) = currentIndex;
								}
							}
						}
					}
				}
			}

			/////////////////

			// Printing out all mappings found

//			cout << endl << "BK clause: " << backgroundKnowledge.at(i) << endl;
//			cout << endl << "Mappings preview: " << endl;
//			for(unsigned int m = 0; m < chainedPositions.size(); m++)
//				cout << chainedPositions.at(m) << " ";
//			cout << endl;

			////////////////

			// Finding possible matches between BCP features
			foundMappings.clear();
			newStoredVariables.clear();
			for(unsigned int j = 0; j < bodyMappings.size(); j++)
			{
				for(unsigned int k = 0; k < features.size(); k++)
				{
					literal   = features.at(k);
					predicate = literal.substr(0, literal.find("("));
					content   = literal.substr(literal.find("(")+1, literal.find(")")-literal.find("(")-1);

					variables = (content.find(";") != string::npos) ? util.splitDelimited(content, ';') : variables;

					for(unsigned int l = 0; l < variables.size(); l++)
					{
						if(util.find(newStoredVariables, variables.at(l)) < 0)
							newStoredVariables.push_back(variables.at(l));
					}

					if(!bodyMappings.at(j).first.compare(predicate))
					{
						if(!j)
						{
							newIndexes.clear();
							for(unsigned int l = 0; l < variables.size(); l++)
								newIndexes.push_back(util.find(newStoredVariables, variables.at(l)));

							foundMapping.clear();
							foundMapping.push_back(newIndexes);
							foundMappings.push_back(foundMapping);
						}
						else
						{
	//						// If the found suitable feature is not the first one found for a given body mapping, replicate the entire rule
	//						if(foundMappings.size() > j)
	//						{
	//							foundMapping.clear();
	//							for(unsigned int l = 0; l <= j; l++)
	//							{
	//								foundMapping.push_back(foundMappings.at(l));
	//								foundMappings.push_back(foundMapping);
	//							}
	//
	//							continue
	//						}
	//						else
	//							usedIndex = j;

							// Obtaining the chained positions from original mapping

							size = foundMappings.size();

//							cout << endl << "BK = " << backgroundKnowledge.at(i) << " | size = " << size << endl;

							for(int l = 0; l < size; l++)
							{
								index = 0;
								mappingsIndex.clear();

								for(unsigned int m = 0; m < foundMappings.at(l).size(); m++)
									index += foundMappings.at(l).at(m).size();

								mappingsIndex.clear();
								mappingsIndex.resize(index + variables.size(), -1);
								index = currentIndex = 0;
								for(unsigned int m = 0; (m < foundMappings.at(l).size()) && (m < j); m++)
								{
									for(unsigned int n = 0; n < foundMappings.at(l).at(m).size(); n++, index++)
									{
										if(mappingsIndex.at(index) < 0)
										{
											currentIndex++;
											mappingsIndex.at(index) = currentIndex;
											index2 = index;
											for(unsigned int o = m; o < foundMappings.at(l).size(); o++)
											{
												unsigned int p = (o == m) ? (n+1) : 0;
												for(; p < foundMappings.at(l).at(o).size(); p++)
												{
													index2++;
													if(foundMappings.at(l).at(m).at(n) == foundMappings.at(l).at(o).at(p))
														mappingsIndex.at(index2) = currentIndex;
												}
											}



											for(unsigned int o = 0; o < variables.size(); o++)
											{
												index2++;
												if(util.find(newStoredVariables, variables.at(o)) == foundMappings.at(l).at(m).at(n))
													mappingsIndex.at(index2) = currentIndex;
											}
										}
									}
								}

								// Converting the chaining positions to variable mappings
								for(unsigned int m = 0; m < variables.size(); m++, index++)
								{
									if(mappingsIndex.at(index) < 0)
									{
										currentIndex++;
										mappingsIndex.at(index) = currentIndex;
										index2 = index;

										for(unsigned int n = m+1; n < variables.size(); n++)
										{
											index2++;
											if(variables.at(m) == variables.at(n))
												mappingsIndex.at(index2) = currentIndex;
										}
									}
								}

								// Checking if the current feature, plus the previous ones, follows the current BK clause
								found = true;
								for(unsigned int m = 0; m < mappingsIndex.size(); m++)
								{
									if((mappingsIndex.size() > chainedPositions.size()) || (mappingsIndex.at(m) != chainedPositions.at(m)))
									{
										found = false;
										break;
									}
								}

								if(found)
								{
									// The new considered feature keeps BK relationship: add it into the found mappings
									newIndexes.clear();
									for(unsigned int m = 0; m < variables.size(); m++)
										newIndexes.push_back(util.find(newStoredVariables, variables.at(m)));

									if(foundMappings.at(l).size() > j)
									{
										foundMapping.clear();
										for(unsigned int m = 0; m < j; m++)
											foundMapping.push_back(foundMappings.at(l).at(m));
										foundMapping.push_back(newIndexes);

										foundMappings.push_back(foundMapping);
									}
									else
										foundMappings.at(l).push_back(newIndexes);
								}
							}
						}
					}
				}
			}

			//////////////

			// Printing out all mappings found
//			cout << endl << "BK body mapping found:" << endl;
//			for(unsigned int i2 = 0; i2 < bodyMappings.size(); i2++)
//			{
//				cout << bodyMappings.at(i2).first << ":";
//
//				for(unsigned int j = 0; j < bodyMappings.at(i2).second.size(); j++)
//					cout << " " << bodyMappings.at(i2).second.at(j);
//
//				cout << endl;
//			}
//
//			cout << endl << "Found mappings:" << endl;
//			for(unsigned int h = 0; h < foundMappings.size(); h++)
//			{
//				cout << "Mapping " << h+1 << ":" << endl;
//				for(unsigned int i2 = 0; i2 < foundMappings.at(h).size(); i2++)
//				{
//					for(unsigned int j = 0; j < foundMappings.at(h).at(i2).size(); j++)
//						cout << foundMappings.at(h).at(i2).at(j) << " ";
//
//					cout << endl;
//				}
//			}
//
//			cout << endl;

			/////////////

			// Filtering out the rules without full mapping with BK
			size = foundMappings.size();
			for(int j = size-1; j >= 0; j--)
			{
				total = 0;

				// Checking the case of clauses with 1 literal and internal mapping
				if((foundMappings.at(j).size() == 1) && (foundMappings.at(j).size() == bodyMappings.size()))
				{
					for(unsigned int k = 0; k < foundMappings.at(j).size(); k++)
					{
						equalPositions.clear();

						// Checking the positions where equality happens
						for(unsigned int l = 0; l < bodyMappings.at(k).second.size(); l++)
						{
							for(unsigned int m = l+1; m < bodyMappings.at(k).second.size(); m++)
							{
								if(bodyMappings.at(k).second.at(l) == bodyMappings.at(k).second.at(m))
								{
									if(util.genericPairFind(make_pair(true, true), equalPositions, make_pair((int)l, (int)m)) < 0)
										equalPositions.push_back(make_pair((int)l, (int)m));
								}
							}
						}

						// Checking whether the found pattern previously matches the current mapping
						found = (bodyMappings.at(k).second.size() == foundMappings.at(j).at(k).size());
						for(unsigned int l = 0; l < foundMappings.at(j).at(k).size(); l++)
						{
							if(!found)
								break;

							for(unsigned int m = l+1; m < foundMappings.at(j).at(k).size(); m++)
							{
								if(foundMappings.at(j).at(k).at(l) == foundMappings.at(j).at(k).at(m))
								{
									if(util.genericPairFind(make_pair(true, true), equalPositions, make_pair((int)l, (int)m)) < 0)
									{
										found = false;
										break;
									}
								}
								else if(bodyMappings.at(k).second.at(l) == bodyMappings.at(k).second.at(m))
								{
									found = false;
									break;
								}
							}
						}

						// If the current mapping does not match with the current BK rule being analysed, the mapping is removed
						if(!found)
							foundMappings.erase(foundMappings.begin()+j);
					}
				}

				for(unsigned int k = 0; k < foundMappings.at(j).size(); k++)
					total += foundMappings.at(j).at(k).size();

				if((total != variablesTotal) && (j <= (int)foundMappings.size()))
					foundMappings.erase(foundMappings.begin()+j);
			}

			///////////////

//			cout << endl << "Remaining mappings:" << endl;
//			for(unsigned int j = 0; j < foundMappings.size(); j++)
//			{
//				cout << "Mapping " << j+1 << ":" << endl;
//				for(unsigned int k = 0; k < foundMappings.at(j).size(); k++)
//				{
//					for(unsigned int l = 0; l < foundMappings.at(j).at(k).size(); l++)
//						cout << foundMappings.at(j).at(k).at(l) << " ";
//
//					cout << endl;
//				}
//			}

			///////////////

			// Creating new BK rules with the found patterns
			newArtificialVariables.clear();
			for(unsigned int j = 0; j < foundMappings.size(); j++)
			{
				newBKClause = ":-";
				foundHeadVariables.clear();
				for(unsigned int k = 0; k < foundMappings.at(j).size(); k++)
				{
					newBKClause += bodyMappings.at(k).first + "(";
					for(unsigned int l = 0; l < foundMappings.at(j).at(k).size(); l++)
					{
	//					if(util.genericPairFind(foundHeadVariables, make_pair(newStoredVariables.at(foundMappings.at(j).at(k).at(l)), foundHeadMappings.at(k).at(l))))
	//						foundHeadVariables.push_back(make_pair(newStoredVariables.at(foundMappings.at(j).at(k).at(l)), foundHeadMappings.at(k).at(l)));

						if(util.genericPairFind(make_pair(true, false), foundHeadVariables, make_pair(storedVariables.at(bodyMappings.at(k).second.at(l)), 0)))
							foundHeadVariables.push_back(make_pair(newStoredVariables.at(foundMappings.at(j).at(k).at(l)), bodyMappings.at(k).second.at(l)));

						if(util.isVariable(variableMappingsPerPredicate.at(k).at(l)))
							newBKClause += newStoredVariables.at(foundMappings.at(j).at(k).at(l)) + ";";
						else
							newBKClause += variableMappingsPerPredicate.at(k).at(l) + ";";
					}

					newBKClause.erase(newBKClause.size()-1, 1);
					newBKClause += "),";
				}

				newBKClause.erase(newBKClause.size()-1, 1);
				newHeadPart = "";
				for(unsigned int k = 0; k < headMappings.size(); k++)
				{
					newHeadPart += headMappings.at(k).first;

					if(!headMappings.at(k).second.empty())
						newHeadPart += "(";

					for(unsigned int l = 0; l < headMappings.at(k).second.size(); l++)
					{
						// Finding the body
						found = false;
						for(unsigned int m = 0; m < foundHeadVariables.size(); m++)
						{
							if(foundHeadVariables.at(m).second == util.find(storedVariables, headMappings.at(k).second.at(l)))
							{
								found = true;
								newHeadPart += foundHeadVariables.at(m).first + ";";
								break;
							}
						}

						if(!found)
						{
							newArtificialVariables.push_back(util.idToVariable((int)newArtificialVariables.size()));
							foundHeadVariables.push_back(make_pair("New" + util.convertToString(newArtificialVariables.at(newArtificialVariables.size()-1)), util.find(storedVariables, headMappings.at(k).second.at(l))));
							newHeadPart += "New" + util.convertToString(newArtificialVariables.at(newArtificialVariables.size()-1)) + ";";
						}
					}

					if(!headMappings.at(k).second.empty())
					{
						newHeadPart.erase(newHeadPart.size()-1, 1);
						newHeadPart += ")";
					}

					if(k != (headMappings.size()-1))
						newHeadPart += ",";
				}

				newBK.push_back(newHeadPart + newBKClause);
			}
		}

		// Removing identical rules
		newBK = util.getDistinctElements(newBK);
	}

	// 3. Replacing the "New" variables with '_' (any) where applicable
	decodedBK = util.decodeRules(newBK);
	for(unsigned int j = 0; j < decodedBK.second.size(); j++)
	{
		newStoredVariables.clear();
		for(unsigned int k = 0; k < decodedBK.first.at(j).size() + decodedBK.second.at(j).size(); k++)
		{
			if(k < decodedBK.first.at(j).size())
			{
				index = k;
				literal  = decodedBK.first.at(j).at(index);
			}
			else
			{
				index = k - decodedBK.first.at(j).size();
				literal  = decodedBK.second.at(j).at(index);
			}

			predicate = literal.substr(0, literal.find("("));
			content   = literal.substr(literal.find("(")+1, literal.find(")")-literal.find("(")-1);

			variables = (content.find(";") != string::npos) ? util.splitDelimited(content, ';') : variables;

			for(unsigned int l = 0; l < variables.size(); l++)
			{
				// Checking if it is an artificial variable
				if((variables.at(l).size() >= 3)  && !variables.at(l).substr(0,3).compare("New"))
				{
					newVariable = variables.at(l);

					found = false;

					if(util.find(newStoredVariables, newVariable) >= 0)
						found = true;
					else
						newStoredVariables.push_back(newVariable);

					for(unsigned int m = index; !found && (m < decodedBK.first.at(j).size() + decodedBK.second.at(j).size()); m++)
					{
						if(m < decodedBK.first.at(j).size())
						{
							index2 = m;
							literal  = decodedBK.first.at(j).at(index2);
						}
						else
						{
							index2 = m - decodedBK.first.at(j).size();
							literal  = decodedBK.second.at(j).at(index2);
						}

						predicate = literal.substr(0, literal.find("("));
						content   = literal.substr(literal.find("(")+1, literal.find(")")-literal.find("(")-1);

						vector<string> variables2 = util.splitDelimited(content, ';');

						for(int n = (index2 == index) ? (l+1) : 0; !found && (n < (int)variables2.size()); n++)
						{
							// Checking if it is an artificial variable
							if(!variables2.at(n).compare(newVariable))
								found = true;
						}
					}

					if(!found)
						newBK.at(j).replace(newBK.at(j).find(newVariable), newVariable.size(), "_");
				}
			}
		}
	}

	// 4. Dealing with disjunction in the body
	decodedBK = util.decodeRules(newBK);
	size = newBK.size();
	for(int i = 0; i < size; i++)
	{
		// Collecting body mappings
		for(unsigned int j = 0; j < decodedBK.first.at(i).size(); j++)
		{
			literalsList.clear();
			if(decodedBK.first.at(i).at(j).find("|") != string::npos)
			{
				literalsList = util.splitDelimited(decodedBK.first.at(i).at(j), '|');

				for(unsigned int k = 0; k < literalsList.size(); k++)
				{
					newHeadPart = newBK.at(i).substr(0, newBK.at(i).find(":-")+2);
					newBKClause = newBK.at(i).substr(newBK.at(i).find(":-")+3);

					newBKClause.replace(newBKClause.find(decodedBK.first.at(i).at(j)), decodedBK.first.at(i).at(j).size(), literalsList.at(k));

					newBK.push_back(newHeadPart + newBKClause);
					size++;
				}
			}
		}
	}

	// 5. Removing identical clauses
	newBK = util.getDistinctElements(newBK);

	// 6. Printing the final rules out
	if(printLog)
	{
		fprintf(logFile, "\nReplicated BK:\n");
		for(unsigned int i = 0; i < newBK.size(); i++)
			fprintf(logFile, "%s\n", newBK.at(i).c_str());
	}

	if(printLog)
		fclose(logFile);

	// 7. Creating the initial network
	buildNet(newBK, printLog);
}

bool Cilp::checkBodyEquivalence(string clause1, string clause2)
{
	bool isEquivalent = true;
	string predicate1, content1, predicate2, content2, head1, head2, body;
	vector<string> temp, tempSplit, literals1, literals2, variables1, variables2;

	// Obtaining contents of clause1
	head1 = clause1.substr(0, clause1.find(":-"));
	body = clause1.substr(clause1.find(":-")+2, clause1.size()-clause1.find(":-")-2);

	temp = util.splitDelimited(body, ',');
	for(unsigned int i = 0; i < temp.size(); i++)
	{
		if(temp.at(i).find("|") != string::npos)
		{
			tempSplit = util.splitDelimited(temp.at(i), '|');
			literals1.insert (literals1.begin() + literals1.size(), tempSplit.begin(), tempSplit.end());
		}
		else
			literals1.push_back(temp.at(i));
	}

	// Obtaining contents of clause2
	head2 = clause2.substr(0, clause2.find(":-"));
	body = clause2.substr(clause2.find(":-")+2, clause2.size()-clause2.find(":-")-2);

	temp = util.splitDelimited(body, ',');
	for(unsigned int i = 0; i < temp.size(); i++)
	{
		if(temp.at(i).find("|") != string::npos)
		{
			tempSplit = util.splitDelimited(temp.at(i), '|');
			literals2.insert (literals2.begin() + literals2.size(), tempSplit.begin(), tempSplit.end());
		}
		else
			literals2.push_back(temp.at(i));
	}

	// Comparing the contents of clauses 1 and 2
	for(unsigned int i = 0; i < literals1.size(); i++)
	{
		predicate1 = literals1.at(i).substr(0, literals1.at(i).find("("));
		content1   = literals1.at(i).substr(literals1.at(i).find("(")+1, literals1.at(i).find(")")-literals1.at(i).find("(")-1);
		variables1 = util.splitDelimited(content1, ';');

		for(unsigned int j = 0; j < literals2.size(); j++)
		{
			predicate2 = literals2.at(i).substr(0, literals2.at(i).find("("));
			content2   = literals2.at(i).substr(literals2.at(i).find("(")+1, literals2.at(i).find(")")-literals2.at(i).find("(")-1);
			variables2 = util.splitDelimited(content2, ';');

			if(predicate2.compare(predicate1) || (variables1.size() != variables2.size()))
				isEquivalent = false;
			else
			{
				for(unsigned int k = 0; k < variables1.size(); k++)
				{
					if(variables1.at(k).compare(variables2.at(k)) && variables1.at(k).compare("_") && variables2.at(k).compare("_"))
					{
						isEquivalent = false;
						break;
					}
				}
			}

			if(!isEquivalent)
				break;
		}

		if(!isEquivalent)
			break;
	}

	return isEquivalent;
}

// New BCP 2.0 implementation
void Cilp::bcp(pair<vector<vector<string> >, vector<vector<string> > > dataset)
{
	int pos, pos2;

	bool structural;

	vector<string> globalVariables, localVariables, splittedHeads, splittedBodies;
	string toBeSplitted;

	vector<pair<string, vector<string> > > features;

	for(unsigned int i = 0; i < dataset.first.size(); i++)
	{
		// Iterating through heads, to get all global variables
		for(unsigned j = 0; j < dataset.second.at(i).size(); j++)
		{
			// Iterating through heads, to get all global variables
			toBeSplitted = dataset.second.at(i).at(j);

			pos = toBeSplitted.find("(");
			pos2 = toBeSplitted.find(")");
			if((pos != (int)string::npos) && (pos2 != (int)string::npos) && (pos < pos2))
			{
				toBeSplitted = toBeSplitted.substr(pos+1, pos2-pos);
				splittedHeads = util.split(toBeSplitted, ',');

				globalVariables.insert(globalVariables.begin(), splittedHeads.begin(), splittedHeads.end());
				globalVariables = util.getDistinctElements(globalVariables);
			}
		}

		// Iterating through bodies, to get the macro-operators
		localVariables.clear();
		structural = false;
		for(unsigned j = 0; j < dataset.first.at(i).size(); j++)
		{
			toBeSplitted = dataset.first.at(i).at(j);

			pos = toBeSplitted.find("(");
			pos2 = toBeSplitted.find(")");
			if((pos != (int)string::npos) && (pos2 != (int)string::npos) && (pos < pos2))
			{
				toBeSplitted = toBeSplitted.substr(pos+1, pos2-pos);
				splittedBodies = util.split(toBeSplitted, ',');

				for(unsigned int k = 0; k < splittedBodies.size(); k++)
				{
					if(util.find(globalVariables, splittedBodies.at(k)) >= 0)
					{
						structural = true;
						continue;
					}

					// Collecting local variables
					if((util.find(localVariables, splittedBodies.at(k)) < 0))
						// New local variable
						localVariables.push_back(splittedBodies.at(k));
				}

				// Checking for structural predicates
				if(structural)
				{
//					// Checking for structural predicates
//					if(util.find(globalVariables, splittedBodies.at(k)) >= 0)
//					{
//						for(unsigned int k = 0; k < splittedBodies.size(); k++)
//						{
//							// It's a structural predicate: searching for utilities to consume local variables
//							if(util.find(globalVariables, splittedBodies.at(k)) >= 0)
//							{
//								// Collecting local variables
//								if((util.find(localVariables, splittedBodies.at(k)) < 0) && (util.find(globalVariables, splittedBodies.at(k)) < 0))
//									// New local variable
//									localVariables.push_back(splittedBodies.at(k));
//							}
//						}
//
//						break;
//					}
				}
			}
		}

		// Initializing the set of features
		for(unsigned j = 0; j < dataset.first.at(i).size(); j++)
		{

		}

		// Iterating through bodies, to get the features
		for(unsigned j = 0; j < dataset.first.at(i).size(); j++)
		{
			// Iterating through heads, to get all global variables
			toBeSplitted = dataset.first.at(i).at(j);

			localVariables.clear();

			pos = toBeSplitted.find("(");
			pos2 = toBeSplitted.find(")");
			if((pos != (int)string::npos) && (pos2 != (int)string::npos) && (pos < pos2))
			{
				toBeSplitted = toBeSplitted.substr(pos+1, pos2-pos);
				splittedBodies = util.split(toBeSplitted, ',');

				for(unsigned int k = 0; k < splittedBodies.size(); k++)
				{
					if((util.find(localVariables, splittedBodies.at(k)) >= 0) && (util.find(globalVariables, splittedBodies.at(k)) < 0))
						// New local variable
						localVariables.push_back(splittedBodies.at(k));
				}
			}
		}
	}

//	for(i = 0; i < bcpFeatures.size(); i++)
//		bcpFeatures.at(i).clear();
//	bcpFeatures.clear();
//
//	bcpFeatureLabels.clear();
}
