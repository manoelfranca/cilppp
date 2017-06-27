/*
	CILP++

	Copyright 2012 Manoel Vitor Macedo Franca <maneuvitor@gmail.com>

	The Minimum Redundancy Maximum Relevance Feature Selection used herein is based on:

		"Feature selection based on mutual information: criteria of max-dependency, max-relevance, and min-redundancy,"

		Hanchuan Peng, Fuhui Long, and Chris Ding
		IEEE Transactions on Pattern Analysis and Machine Intelligence,
		Vol. 27, No. 8, pp.1226-1238, 2005. [PDF]

		"Minimum redundancy feature selection from microarray gene expression data,"

		Chris Ding, and Hanchuan Peng,
		Journal of Bioinformatics and Computational Biology,
		Vol. 3, No. 2, pp.185-205, 2005. [PDF]

	See the "Credits" file for additional copyright requirements

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

#ifndef MRMR_H_
#define MRMR_H_

#include "../variables.h"

extern char *optarg;
extern int optind, opterr;

enum FeaSelectionMethod { MID, MIQ };

#define NR_END 1
#define FREE_ARG char*
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

extern int *ivector(long nl, long nh);
extern void free_ivector(int *v, long nl, long nh);

void sort2(unsigned long, float[], float[]);

#undef M
#undef NSTACK
#undef SWAP

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
#define M 7
#define NSTACK 50

extern void nrerror(char[]);
extern float gammln(float);
extern float betacf(float a, float b, float x);
extern float betai(float a, float b, float x);

class DataTable
{
public:
  float *data;
  float **data2d;
  long nsample;
  long nvar;
  int *classLabel;
  int *sampleNo;
  char **variableName;
  int b_zscore;
  int b_discetize;
    DataTable ();
   ~DataTable ();
   int buildData2d ();
  void destroyData2d ();
  void printData (long, long, long, long);
  void printData ();
  void zscore (long, int);
  void discretize (double, int);
};

DataTable *readData (char[], char, int, long, long);
double calMutualInfo (DataTable*, long, long);
template < class T > T putInRange (T, T, T);
void printPaperInfo();
void printHelp ();
DataTable * readData (vector<string>, char, int, long, long);
template < class T > void copyvecdata (T*, long, int*, int&);
template < class T > double * compute_jointprob (T*, T*, long, long, int&, int&);
double compute_mutualinfo (double*, long, long);
double calMutualInfo (DataTable*, long, long);
long * mRMR_selection (DataTable*, long, FeaSelectionMethod);
void printHelp ();
vector<int> mrmr(vector<string>, int);

#endif /* MRMR_H_ */
