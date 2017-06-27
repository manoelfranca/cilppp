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

#ifndef NEURALNETBP_H_
#define NEURALNETBP_H_

#include "utilities.h"

/* Enumerations */
enum activationFunction {linear, binary, hyperbolic, bipolar};
enum neuronStatus {activated, unknown, desactivated, outOfBounds, doesNotExist};

// If changing the size of this enumeration, make sure to change its associated #define as well!
enum stabilizationType {regularSimple, regularStable, stochasticSimple, stochasticStable};
#define STABILIZATION_TYPE_SIZE 4
/* End of enumerations */

using namespace std;

typedef struct _confusionTable
{	int truePositives;
	int falsePositives;
	int trueNegatives;
	int falseNegatives;
	int trueMixed;
	int falseMixed;
}confusionTable;

class Cilp
{
	public:
		Cilp(bool);
		virtual ~Cilp();

		bool noBackgroundKnowledge, haveIntermediateConcepts;

		void buildNet(vector<string>, bool);
		void buildReplicatedBKNet(vector<string>, vector<string>, bool);
		void addNewHiddenNeurons(int);
		void addNewInputNeuron(string);
		void addNewOutputNeuron(string);
		int stabilizeNet(stabilizationType, vector<string>, bool, FILE*);
		void trainExamples(pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > >, bool, bool, vector<FILE*>, vector<FILE*>);
		void trainNet(vector<string>, bool);
		void clearNetStatus();
		void printNetStatus(bool);
		void printNetWeights(bool);
		void trepanExporter();
		void printDatasetInfo(vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > >, bool, string);
		void pruneInputNeurons(int);
		double evaluateActivation(double, int, int, double, bool, double&);
		vector<pair<pair<double, double>, confusionTable> > testNet(vector<string>, bool);
		neuronStatus evaluateRange(int, double);
		void shuffleTrainData(vector<string>&, vector<vector<string> >&, vector<vector<string> >&);
		vector<vector<double> > evaluateData(vector<vector<string> >);
		pair<vector<vector<string> >, vector<vector<string> > > decodeData(vector<string>);
		vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > splitDataByTarget(vector<string>, vector<vector<string> >, vector<vector<string> >, int, double);
		vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > >, pair<vector<string>, pair<vector<vector<string> >, vector<vector<string> > > > > > buildDatasetFromFolds();
		pair<vector<vector<string> >, vector<string> > filterInputData(pair<vector<vector<string> >, vector<string> >);
		vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > buildDatasetFromIncrementalFolds(int);
		vector<vector<pair<pair<vector<string>, pair<vector<vector<string> >, vector<string> > >, pair<vector<string>, pair<vector<vector<string> >, vector<string> > > > > > splitDataIncrementallyByTarget(int, vector<string>, vector<vector<string> >, vector<string>, int);
		bool checkBodyEquivalence(string, string);
		void bcp(pair<vector<vector<string> >, vector<vector<string> > >);

		vector<vector<double> > netStatus;
		vector<vector<string> > neuronsLabels;
		vector<vector<string> > bcpFeatures;
		vector<string> bcpFeatureLabels;

	private:
		int numberOfLayers;
		double w, amin, learningRate;
//		double weightsMean, weightsStd;
		bool stochastic;

		Utilities util;

		vector<int> numberOfNeurons;

		vector<vector<double> > normalizedAmins;
		vector<vector<double> > layersBias;
		vector<vector<double> > thresholds;
		vector<vector<double> > normalizationFactors;
		vector<vector<vector<pair<int, double> > > > connectionsWeights;

		void normalizeWeights(bool);
		void revertWeights(bool);
		double calculateActivationFunction(double, activationFunction);
		double calculateTotalInput(int, int, bool);
		int calculateOutputConnections(string);
		neuronStatus returnNeuronStatus(int, string, bool, double&, double&);
		string returnNeuronStatusString(int, string, bool, double&, double&);
		bool isStabilized(stabilizationType);
		vector<double> convertData(int, vector<string>);
		vector<double> convertInputData(int, vector<string>, string);
		double calculateActivationDerivative(double, activationFunction);
		bool checkStopCriterion(vector<vector<double> >, vector<vector<double> >, vector<vector<double> >, int, int, vector<int>&, vector<int>&);
		bool checkStopCriterionSimple(stoppingCriteria, vector<vector<double> >, vector<double>, double, int, int);
		string returnNonnegatedLiteral(string);
};

#endif /* NEURALNETBP_H_ */
