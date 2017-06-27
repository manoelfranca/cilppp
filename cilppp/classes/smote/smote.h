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

	****************************************************************************

	This is the C++ implementation of the SMOTE over-sampling algorithm:

	Chawla et. al., "SMOTE: Synthetic Minority Over-sampling Technique (2002)",
	Journal of Artificial Intelligence Research
*/

#ifndef SMOTE_H_
#define SMOTE_H_

#include "../variables.h"

class Smote
{
	public:
		Smote(vector<double> t, int n, int k);
		vector<vector<double> > execute();
		virtual ~Smote();

	private:
		vector<double> t;
		int n, k;
};

#endif /* SMOTE_H_ */
