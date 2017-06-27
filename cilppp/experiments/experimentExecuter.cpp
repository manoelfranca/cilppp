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

#include "../classes/cilpExperiment.h"

int main (int argc, char *argv[])
{
	printf("\n***********************");
	printf("\n* CILP++, Version %1.1f *", VERSION);
	printf("\n***********************\n\n");

	string backgroundKnowledgeFilePath = "files/backgroundKnowledge.data";
	string trainingDataFilePath = "files/training.data";
	string testingDataFilePath = "files/testing.data";

	CilpExperiment *experimentClass = new CilpExperiment(backgroundKnowledgeFilePath, trainingDataFilePath, testingDataFilePath);

	experimentClass->setVariables();

	if(paramReplicateBk)
		experimentClass->replicatedBkExperiment();
	else
		experimentClass->basicExperiment();

	delete(experimentClass);

	printf("\nCILP++ successfully finished running. Press <Enter> to exit...\n");
		while (getchar() < 0);

	return 0;
}
