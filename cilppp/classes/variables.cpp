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

#include "variables.h"

bool paramStochastic = STOCHASTIC;
double paramBeta = BETA;
double paramMinimalIncrement = MINIMAL_INCREMENT;
double paramAminThreshold = AMIN_THRESHOLD;
bool paramUnknownInputsAsFalse = UNKNOWN_INPUTS_AS_FALSE;
double paramExamplesAsBKProportion = EXAMPLES_AS_BK_PROPORTION;
bool paramUsesRecursion = USES_RECURSION;
bool paramCompleteIntermediateConcepts = COMPLETE_INTERMEDIATE_CONCEPTS;
bool paramReplicateBk = REPLICATE_BK;
int paramMaxReplicationRecursion = MAX_REPLICATION_RECURSION;
bool paramNewBcp = NEW_BCP;
stoppingCriteria paramTrainingType = (stoppingCriteria)TRAINING_TYPE;
int paramStripSize = STRIP_SIZE;
double paramDefaultLearningRate = DEFAULT_LEARNING_RATE;
double paramDecayFactor = DECAY_FACTOR;
double paramMomentum = MOMENTUM;
int paramBatchSize = BATCH_SIZE;
bool paramMrmrFiltering = MRMR_FILTERING;
double paramFilteringProportion = FILTERING_PROPORTION;
int paramStartingHiddenNeurons = STARTING_HIDDEN_NEURONS;
double paramTrainPortion = TRAIN_PORTION;
double paramTestPortion = TEST_PORTION;
int paramStablizationMaxIterations = STABLIZATION_MAX_ITERATIONS;
bool paramSimpleStopCriterion = SIMPLE_STOP_CRITERION;
double paramEpsilon = EPSILON;
double paramDelta = DELTA;
int paramEpochRuns = EPOCH_RUNS;
double paramCorrectnessRange = CORRECTNESS_RANGE;
double paramCorrectnessProportion = CORRECTNESS_PROPORTION;
double paramHitMargin = HIT_MARGIN;
double paramStabilizationProportion = STABILIZATION_PROPORTION;
int paramStabilizationMaxEpochs = STABILIZATION_MAX_EPOCHS;
double paramWeightInitialRange = WEIGHT_INITIAL_RANGE;
int paramNumberOfValidationFolds = NUMBER_OF_VALIDATION_FOLDS;
int paramNumberOfTestFolds = NUMBER_OF_TEST_FOLDS;
bool paramPreSplittedFolds = PRE_SPLITTED_FOLDS;
double paramInputPruningVariation = INPUT_PRUNING_VARIATION;
int paramPruningRateFactor = PRUNING_RATE_FACTOR;
int paramHiddenNeuronsVariation = HIDDEN_NEURONS_VARIATION;
int paramHiddenNeuronsFactor = HIDDEN_NEURONS_FACTOR;
bool paramPrintTrainError = PRINT_TRAIN_ERROR;
bool paramPrintTestError = PRINT_TEST_ERROR;
bool paramDebugMode = DEBUG_MODE;

// Time variables
time_t setupStart = 0;
time_t setupEnd = 0;
time_t buildStart = 0;
time_t buildEnd = 0;
time_t trainStart = 0;
time_t trainEnd = 0;
time_t testStart = 0;
time_t testEnd = 0;
time_t bcpStart = 0;
time_t bcpEnd = 0;
double setupTotalTime = 0.0;
double buildTotalTime = 0.0;
double trainTotalTime = 0.0;
double testTotalTime = 0.0;
double bcpTotalTime = 0.0;
