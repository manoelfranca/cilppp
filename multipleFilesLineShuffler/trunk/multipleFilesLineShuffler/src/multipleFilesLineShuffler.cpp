//============================================================================
// Name        : multipleFilesLineShuffler.cpp
// Author      : Manoel Vitor Macedo França
// Version     : 1.0
// Copyright   : Apache 2.0 License
// Description : Program to shuffle multiple files using a same random seed
//============================================================================

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

#include "utilities.h"

#define FILE_NAME_1 "test5.n"
#define FILE_NAME_2 "testingnegative5.data"

using namespace std;

int main()
{
	Utilities u;
	int currentSize;
	string baseInputPath1, baseInputPath2;
	vector<string> lines1, lines2;
	ofstream outputStream1, outputStream2;

	baseInputPath1 = "files/";
	baseInputPath1 += FILE_NAME_1;

	baseInputPath2 = "files/";
	baseInputPath2 += FILE_NAME_2;

	lines1 = u.readLines(baseInputPath1);
	lines2 = u.readLines(baseInputPath2);

	srand (time(NULL));

	outputStream1.open(baseInputPath1.c_str(), ios::out | ios::trunc);
	outputStream2.open(baseInputPath2.c_str(), ios::out | ios::trunc);

	currentSize = lines1.size();

	for(; currentSize > 0; currentSize--)
	{
		int randomIndex = (int)floor(((float)rand()/RAND_MAX) * currentSize);
		if (randomIndex == currentSize)
			randomIndex--;

		outputStream1 << lines1.at(randomIndex) << endl;
		outputStream2 << lines2.at(randomIndex) << endl;

		lines1.erase(lines1.begin()+randomIndex);
		lines2.erase(lines2.begin()+randomIndex);
	}

	outputStream1.close();
	outputStream2.close();

	return 0;
}
