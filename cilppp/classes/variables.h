/*
	CILP++

	Copyright 2012 Manoel Vitor Macedo Fran�a <maneuvitor@gmail.com>

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

#ifndef CILPPP_CLASSES_VARIABLES_H_
#define CILPPP_CLASSES_VARIABLES_H_

#include <cstddef>
#include <cstdlib>
#include <vector>
#include <map>
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
#include <exception>
#include <csignal>
#include <sys/types.h>
#include <omp.h>
#include <dirent.h>

using namespace std;

/* Header defines */
#define AUTHOR "Manoel Vitor Macedo Franca"
#define VERSION 1.0
/* End of header defines */

/* Structural defines */
#define INPUT_LAYER 0
#define HIDDEN_LAYER 1
#define OUTPUT_LAYER 2
/* End of structural defines */

/* Auxiliary defines */
#define MAX_SIZE INT_MAX
#define NO_CONNECTION -DBL_MAX
#define NUMBER_OF_LAYERS 3
/* End of auxiliary defines */

/* Primary enumerations */
enum stoppingCriteria {standard, earlyStopping};
/* End of Primary enumerations */

/* Fixed #defines */
#define DEFAULT_TRANING_STABILIZATION regularStable
#define DEFAULT_TESTING_STABILIZATION regularStable
#define NORMALIZE_WEIGHTS_ONLY true
#define FOLD_FILES_FORMAT 'a' //s = standard; a = aleph
/* End of fixed #defines */

/* Default values for the system */

// STOCHASTIC == false => standard neuron activation (output within range [amin, 1])
// STOCHASTIC == true  => stochastic neuron activation (has a possibility of activating/deactivating unknown neuron activations)

#define STOCHASTIC false
#define BETA 1.0
#define MINIMAL_INCREMENT 0.01
#define AMIN_THRESHOLD 0.9
#define UNKNOWN_INPUTS_AS_FALSE false
#define EXAMPLES_AS_BK_PROPORTION 0.0
#define USES_RECURSION false
#define COMPLETE_INTERMEDIATE_CONCEPTS false
#define REPLICATE_BK false
#define MAX_REPLICATION_RECURSION 5
#define NEW_BCP false
#define TRAINING_TYPE 1
#define STRIP_SIZE 0
#define DEFAULT_LEARNING_RATE 0.1
#define DECAY_FACTOR 0.995
#define MOMENTUM 0.0
#define BATCH_SIZE 10
#define MRMR_FILTERING false
#define FILTERING_PROPORTION 0.5
#define STARTING_HIDDEN_NEURONS 2
#define TRAIN_PORTION 1.0
#define TEST_PORTION 1.0
#define STABLIZATION_MAX_ITERATIONS 100
#define EPOCH_RUNS 100
#define CORRECTNESS_RANGE 0.25
#define CORRECTNESS_PROPORTION 0.99
#define SIMPLE_STOP_CRITERION true
#define EPSILON 0.001
#define DELTA 0.0001
#define HIT_MARGIN 0.1
#define STABILIZATION_PROPORTION 0.9
#define STABILIZATION_MAX_EPOCHS 5
#define WEIGHT_INITIAL_RANGE 0.0001
#define NUMBER_OF_VALIDATION_FOLDS 3
#define NUMBER_OF_TEST_FOLDS 10
#define PRE_SPLITTED_FOLDS true
#define INPUT_PRUNING_VARIATION 0
#define PRUNING_RATE_FACTOR 3
#define HIDDEN_NEURONS_VARIATION 5
#define HIDDEN_NEURONS_FACTOR 3
#define PRINT_TRAIN_ERROR false
#define PRINT_TEST_ERROR false
#define DEBUG_MODE false

// Deprecated
#define MINIMUM_CHANGE -FLT_MAX
#define MINIMUM_ERROR 0.001

/* End of system #defines */

// Global variables
extern bool paramStochastic;
extern double paramBeta;
extern double paramMinimalIncrement;
extern double paramAminThreshold;
extern bool paramUnknownInputsAsFalse;
extern double paramExamplesAsBKProportion;
extern bool paramUsesRecursion;
extern bool paramCompleteIntermediateConcepts;
extern bool paramReplicateBk;
extern int paramMaxReplicationRecursion;
extern bool paramNewBcp;
extern stoppingCriteria paramTrainingType;
extern int paramStripSize;
extern double paramDefaultLearningRate;
extern double paramDecayFactor;
extern double paramMomentum;
extern int paramBatchSize;
extern bool paramMrmrFiltering;
extern double paramFilteringProportion;
extern int paramStartingHiddenNeurons;
extern double paramTrainPortion;
extern double paramTestPortion;
extern int paramStablizationMaxIterations;
extern bool paramSimpleStopCriterion;
extern double paramEpsilon;
extern double paramDelta;
extern int paramEpochRuns;
extern double paramCorrectnessRange;
extern double paramCorrectnessProportion;
extern double paramHitMargin;
extern double paramStabilizationProportion;
extern int paramStabilizationMaxEpochs;
extern double paramWeightInitialRange;
extern int paramNumberOfValidationFolds;
extern int paramNumberOfTestFolds;
extern bool paramPreSplittedFolds;
extern double paramInputPruningVariation;
extern int paramPruningRateFactor;
extern int paramHiddenNeuronsVariation;
extern int paramHiddenNeuronsFactor;
extern bool paramPrintTrainError;
extern bool paramPrintTestError;
extern bool paramDebugMode;

// Time global variables
extern time_t setupStart, setupEnd, buildStart, bcpStart, buildEnd, trainStart, trainEnd, testStart, testEnd, bcpEnd;
extern double setupTotalTime, buildTotalTime, trainTotalTime, testTotalTime, bcpTotalTime;

#endif /* CILPPP_CLASSES_VARIABLES_H_ */
