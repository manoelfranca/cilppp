//=========================================================
//
//A C++ program to implement the mRMR selection using mutual information
// written by Hanchuan Peng.
//
//Disclaimer: The author of program is Hanchuan Peng
//      at <penghanchuan@yahoo.com>.
//
//The CopyRight is reserved by the author.
//
//Last modification: Oct/24/2005
//
//
// make -f mrmr.makefile
//
// by Hanchuan Peng
// 2005-08-01
// 2005-10-17
// 2005-10-20
// 2005-10-22
// 2005-10-24: finalize the computing parts of the whole program
// 2005-10-25: add non-discretization for the classification variable. Also slightly change some output info for the web application
// 2005-11-01: add control to the user-defined max variable number and sample number
// 2006-01-26: add gnu_getline.c to convert the code to be compilable under Max OS.
// 2007-01-25: change the address info

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

//#define _GNU_SOURCE
#include "mrmr.h"

int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;

	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) puts("allocation failure in ivector()");
	return v-nl+NR_END;
}


void free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with ivector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}


void sort2(unsigned long n, float arr[], float brr[])
{
	unsigned long i,ir=n,j,k,l=1;
	int *istack,jstack=0;
	float a,b,temp;

	istack = ivector(1,NSTACK);
	for (;;) {
		if (ir-l < M) {
			for (j=l+1;j<=ir;j++) {
				a=arr[j];
				b=brr[j];
				for (i=j-1;i>=1;i--) {
					if (arr[i] <= a) break;
					arr[i+1]=arr[i];
					brr[i+1]=brr[i];
				}
				arr[i+1]=a;
				brr[i+1]=b;
			}
			if (!jstack) {
				free_ivector(istack,1,NSTACK);
				return;
			}
			ir=istack[jstack];
			l=istack[jstack-1];
			jstack -= 2;
		} else {
			k=(l+ir) >> 1;
			SWAP(arr[k],arr[l+1])
			SWAP(brr[k],brr[l+1])
			if (arr[l+1] > arr[ir]) {
				SWAP(arr[l+1],arr[ir])
				SWAP(brr[l+1],brr[ir])
			}
			if (arr[l] > arr[ir]) {
				SWAP(arr[l],arr[ir])
				SWAP(brr[l],brr[ir])
			}
			if (arr[l+1] > arr[l]) {
				SWAP(arr[l+1],arr[l])
				SWAP(brr[l+1],brr[l])
			}
			i=l+1;
			j=ir;
			a=arr[l];
			b=brr[l];
			for (;;) {
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				SWAP(arr[i],arr[j])
				SWAP(brr[i],brr[j])
			}
			arr[l]=arr[j];
			arr[j]=a;
			brr[l]=brr[j];
			brr[j]=b;
			jstack += 2;
			if (ir-i+1 >= j-l) {
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
			} else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
			}
		}
	}
}


void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
  fprintf(stderr,"run-time error...%s\n",error_text);
  fprintf(stderr,"...now exiting to system...\n");
  exit(1);
}


float gammln(float xx)
{
  double x,y,tmp,ser;
  static double cof[6]={76.18009172947146,-86.50532032941677,
			24.01409824083091,-1.231739572450155,
			0.1208650973866179e-2,-0.5395239384953e-5};
  int j;

  y=x=xx;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser=1.000000000190015;
  for (j=0;j<=5;j++) ser += cof[j]/++y;
  return -tmp+log(2.5066282746310005*ser/x);
}


#define MAXIT 100
#define EPS 3.0e-7
#define FPMIN 1.0e-30

float betacf(float a, float b, float x)
{
  int m,m2;
  float aa,c,d,del,h,qab,qam,qap;

  qab=a+b;
  qap=a+1.0;
  qam=a-1.0;
  c=1.0;
  d=1.0-qab*x/qap;
  if (fabs(d) < FPMIN) d=FPMIN;
  d=1.0/d;
  h=d;
  for (m=1;m<=MAXIT;m++) {
    m2=2*m;
    aa=m*(b-m)*x/((qam+m2)*(a+m2));
    d=1.0+aa*d;
    if (fabs(d) < FPMIN) d=FPMIN;
    c=1.0+aa/c;
    if (fabs(c) < FPMIN) c=FPMIN;
    d=1.0/d;
    h *= d*c;
    aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
    d=1.0+aa*d;
    if (fabs(d) < FPMIN) d=FPMIN;
    c=1.0+aa/c;
    if (fabs(c) < FPMIN) c=FPMIN;
    d=1.0/d;
    del=d*c;
    h *= del;
    if (fabs(del-1.0) < EPS) break;
  }

  if (m > MAXIT) puts("a or b too big, or MAXIT too small in betacf");

  return h;
}
#undef MAXIT
#undef EPS
#undef FPMIN


float betai(float a, float b, float x)
{
  float bt;

  if (x < 0.0 || x > 1.0) puts("Bad x in routine betai");
  if (x == 0.0 || x == 1.0) bt=0.0;
  else
    bt=exp(gammln(a+b)-gammln(a)-gammln(b)+a*log(x)+b*log(1.0-x));
  if (x < (a+1.0)/(a+b+2.0))
    return bt*betacf(a,b,x)/a;
  else
    return 1.0-bt*betacf(b,a,1.0-x)/b;
}


DataTable::DataTable()
{
    data = NULL;
    data2d = NULL;
    nsample = 0;
    nvar = 0;
    classLabel = NULL;
    sampleNo = NULL;
    variableName = NULL;
    b_zscore = 0;		// initialze the data as not being z-scored
    b_discetize = 0;	// initialze the data as continous
    return;
}

DataTable::~DataTable ()
{
    if(data)
    {
		delete[]data;
		data = NULL;
    }
    if(classLabel)
    {
		delete[]classLabel;
		classLabel = NULL;
    }
    if(sampleNo)
    {
		delete[]sampleNo;
		sampleNo = NULL;
    }
    if(variableName)
    {
    	for(long i = 0; i < nvar; i++)
    	{
    		if(variableName[i])
			  delete[] variableName[i];
    	}
		delete[] variableName;
		variableName = NULL;
    }
    if(data2d)
    	destroyData2d();
    nsample = nvar = 0;
    return;
}

int DataTable::buildData2d()
{
	if(!data)
		fprintf(stderr, "The data is not ready yet: data point is NULL");

	if(data2d)
		destroyData2d();

	if(nsample <= 0 || nvar <= 0)
	{
		fprintf(stderr, "The data is not ready yet: nsample=%d nvar=%d", (int)nsample, (int)nvar);
		return 0;
	}

	data2d = new float *[nsample];
	if (!data2d)
	{
		fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
		return 0;
	}

	else
	{
		for(long i = 0; i < nsample; i++)
			data2d[i] = data + i * nvar;
	}

	return 1;
}

void DataTable::destroyData2d()
{
	if (data2d)
    {
		delete[]data2d;
		data2d = NULL;
    }
}

void DataTable::printData(long nsample_start, long nsample_end, long nvar_start, long nvar_end)
{}

void DataTable::printData()
{
	printData(1, nsample, 1, nvar);
}

void DataTable::zscore(long indExcludeColumn, int b_discretize)
{
	if(!data2d)
		buildData2d();

    if(!b_discretize)
    	return; // in this case, just generate the 2D data array

    long i, j;
    for(j = 0; j < nvar; j++)
    {
    	if(j == indExcludeColumn)
    		continue; //this is useful to exclude the first column, which will be the target classification variable

		double cursum = 0;
		double curmean = 0;
		double curstd = 0;

		for(i = 0; i < nsample; i++)
			cursum += data2d[i][j];

		curmean = cursum / nsample;
		cursum = 0;
		register double tmpf;

		for(i = 0; i < nsample; i++)
		{
			tmpf = data2d[i][j] - curmean;
			cursum += tmpf * tmpf;
		}

		curstd = (nsample == 1) ? 0 : sqrt(cursum / (nsample - 1));	//nsample -1 is an unbiased version for Gaussian
		for(i = 0; i < nsample; i++)
			data2d[i][j] = (data2d[i][j] - curmean) / curstd;
    }

    b_zscore = 1;
}

void DataTable::discretize(double threshold, int b_discretize)
{
	long indExcludeColumn = 0; //exclude the first column
    if(b_zscore == 0)
    	zscore(indExcludeColumn, b_discretize); //exclude the first column

    if(!b_discretize)
      return; // in this case, just generate the 2D array

    long i, j;
    for(j = 0; j < nvar; j++)
    {
    	if(j == indExcludeColumn)
    		continue;

		register double tmpf;
		for(i = 0; i < nsample; i++)
		{
			tmpf = data2d[i][j];
			if(tmpf > threshold)
				tmpf = 1;
			else
			{
				if(tmpf < -threshold)
					tmpf = -1;
				else
					tmpf = 0;
			}
			data2d[i][j] = tmpf;
		}
    }

    b_discetize = 1;
}

template < class T > T putInRange(T val, T minval, T maxval)
{
	T myval = val;
	if(minval > maxval)
	{
		fprintf(stderr, "The input parameters of putInRange() have error. The result is wrong!\n");
		return myval;
	}
	if(myval < minval)
		myval = minval;
	else
	{
		if(myval > maxval)
			myval = maxval;
	}

	return myval;
}

void printPaperInfo()
{
	printf("\n\n *** This program and the respective minimum Redundancy Maximum Relevance (mRMR) \n");
	printf( "     algorithm were developed by Hanchuan Peng <hanchuan.peng@gmail.com>for\n");
	printf( "     the paper \n");
	printf( "     \"Feature selection based on mutual information: criteria of \n");
	printf( "      max-dependency, max-relevance, and min-redundancy,\"\n");
	printf( "      Hanchuan Peng, Fuhui Long, and Chris Ding, \n");
	printf( "      IEEE Transactions on Pattern Analysis and Machine Intelligence,\n");
	printf( "      Vol. 27, No. 8, pp.1226-1238, 2005.\n\n");

	return;
}

void printHelp()
{
	printf("\nUsage: mrmr_osx -i <dataset> -t <threshold> [optional arguments]\n");
	printf("\t -i <dataset>    .CSV file containing M rows and N columns, row - sample, column - variable/attribute.\n");
	printf("\t -t <threshold> a float number of the discretization threshold; non-specifying this parameter means no discretizaton (i.e. data is already integer); 0 to make binarization.\n");
	printf("\t -n <number of features>   a natural number, default is 50.\n");
	printf("\t -m <selection method>    either \"MID\" or \"MIQ\" (Capital case), default is MID.\n");
	printf("\t -s <MAX number of samples>   a natural number, default is 1000. Note that if you don't have or don't need big memory, set this value small, as this program will use this value to pre-allocate memory in data file reading.\n");
	printf("\t -v <MAX number of variables/attibutes in data>   a natural number, default is 10000. Note that if you don't have or don't need big memory, set this value small, as this program will use this value to pre-allocate memory in data file reading.\n");
	printf("\t [-h] print this message.\n");

	printPaperInfo();

	return;
}

DataTable *readData(vector<string> data, char sep, int b_readName, long nmaxvar, long nmaxsample)
{
	DataTable *myData = new DataTable;
	if(!myData)
    {
		fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
		exit(EXIT_FAILURE);
		return 0;
    }

	long MAXLINELEN = nmaxsample+1; //+1 for the first row which is the variable names
	long MAXVARNUM = nmaxvar+1; //+1 for the first column which is the classification variable
//  unsigned char *row[MAXVARNUM];

	myData->data = new float[MAXLINELEN * MAXVARNUM];
	if(!myData->data)
    {
		fprintf(stderr, "Line %d: Fail to allocate memory of %d bytes, -- if you don't have or don't need big memory, better set the #MaxVariables and #MaxSamples to a smaller value .\n", __LINE__, (int)(MAXLINELEN * MAXVARNUM));
		exit(EXIT_FAILURE);

		return myData;
    }

	long *posfield = new long[MAXVARNUM + 1];	// plus to get the correct number of separation intervals
	if(!posfield)
    {
		fprintf(stderr, "Line %d: Fail to allocate memory .\n", __LINE__);
		exit(EXIT_FAILURE);

		return 0;
    }

	char *line;
	size_t len = 0;
	int nbyteread;

	myData->nsample = 0;
	myData->nvar = 0;
	float *pdata = myData->data;

//  printf ("\ninitial input len=%d\n", len);

//	while((nbyteread = gnu_getline(&line, &len, fid)) != -1) //change on 01/26/2006 for the Mac OS
//    {
	for(unsigned int i = 0; i < data.size(); i++)
	{
		len = strlen(data.at(i).c_str());
		nbyteread = len;
		line = new char[len];

		for(unsigned int j = 0; j < len; j++)
			line[j] = data.at(i).at(j);

		if(myData->nsample > MAXLINELEN)
			break;

      //    printf ("Retrieved line of length %u :\n", nbyteread);
      //    printf ("\n%s input len=%d real len=%d\n", line, len, nbyteread);

      // read a line and find all partition intervals

		long ncurfield = 1;	//at least one field in one line
		posfield[0] = -1;		// later one, the start positon will always plus 1, so to get the correct starting location at 0

		long j = 0;
		while(j < nbyteread)
		{
			if(line[j] == sep)
			{
				ncurfield++;
				if(ncurfield > MAXVARNUM)
				{
					fprintf (stderr, "Maximum number of variable %d is attained!\n", (int)MAXVARNUM);
					break;
				}

				posfield[ncurfield - 1] = j;
//            	printf("%d ", j);
			}

			j++;
		}

		posfield[ncurfield] = nbyteread - 1;	// the end of a line is a '\n' char will wouldn't do any harm if removed

		if(myData->nsample == 0)
		{
			myData->nvar = ncurfield;
//        	printf ("nvar=%d\n", myData->nvar);
		}
		else
		{
			if(myData->nvar != ncurfield)
			{
				fprintf(stderr, "Line %d: Read a different number of field: The DATA has error!!!\n", __LINE__);
				fprintf(stderr, "Previous line: %s\n", data.at(i-1).c_str());
				fprintf(stderr, "Current line: %s\n", data.at(i).c_str());
				exit(EXIT_FAILURE);

				return 0;
			}
//        	printf ("ncurfield=%d\n", ncurfield);
		}

      // convert the intervals data in char type as numbers

		const int buflen = 128;
		int intervalLen = 0;
		char tmpbuf[buflen + 1];	// at most 10 char long, thus should be very safe. Add 1 to put '\0'

		if (b_readName)
		{
			if (myData->nsample == 0)
			{
				myData->variableName = new char *[ncurfield];
				if(!myData->variableName)
				{
					fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
					exit(EXIT_FAILURE);

					return 0;
				}

				for(j = 0; j < ncurfield; j++)
				{
					intervalLen = posfield[j + 1] - posfield[j] - 1;
					intervalLen = (intervalLen > buflen) ? buflen : intervalLen;

					myData->variableName[j] = new char[intervalLen + 1];
					if(!myData->variableName[j])
					{
						fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
						exit(EXIT_FAILURE);
						return 0;
					}

					int tmpj = 0;
					for(tmpj = 0; tmpj < intervalLen; tmpj++)
						myData->variableName[j][tmpj] = line[posfield[j] + 1 + tmpj];

					myData->variableName[j][tmpj] = '\0';
//                	printf ("%s\n", myData->variableName[j]);
				}
			}
			else
			{
				for(j = 0; j < ncurfield; j++)
				{
					intervalLen = posfield[j + 1] - posfield[j] - 1;
					intervalLen = (intervalLen > buflen) ? buflen : intervalLen;
					int tmpj = 0;

					for(tmpj = 0; tmpj < intervalLen; tmpj++)
						tmpbuf[tmpj] = line[posfield[j] + 1 + tmpj];

					tmpbuf[tmpj] = '\0';	//even for the input like ",,,", this is correct to generate the val to be 0
					double val = atof(tmpbuf);
//                	printf("%s\n", tmpbuf);

					*pdata++ = val;	//assign the read data
				}
			}
		}
		else
		{
			for(j = 0; j < ncurfield; j++)
			{
				intervalLen = posfield[j + 1] - posfield[j] - 1;
				intervalLen = (intervalLen > buflen) ? buflen : intervalLen;

				int tmpj = 0;
				for(tmpj = 0; tmpj < intervalLen; tmpj++)
					tmpbuf[tmpj] = line[posfield[j] + 1 + tmpj];

				tmpbuf[tmpj] = '\0';
				double val = atof(tmpbuf);
//     	        printf("%s\n", tmpbuf);

				*pdata++ = val;	//assign the read data
			}
		}
// increase the count of the rows

		myData->nsample++;
    }

	if(b_readName)
		myData->nsample--;	//when the variable name is present, then the total number of samples has only n-1 rows

	if(posfield)
    {
		delete[]posfield;
		posfield = 0;
    }

	return myData;		// if succeed
}

template < class T > void copyvecdata(T * srcdata, long len, int *desdata, int &nstate)
{
	long i;

  //note: originally I added 0.5 before rounding, however seems the negative numbers and 
  //      positive numbers are all rounded towarded 0; hence int(-1+0.5)=0 and int(1+0.5)=1;
  //      This is unwanted because I need the above to be -1 and 1.
  // for this reason I just round with 0.5 adjustment for positive and negative differently

	//copy data
	int minn, maxx;
	if (srcdata[0] > 0)
		maxx = minn = int (srcdata[0] + 0.5);
	else
		maxx = minn = int (srcdata[0] - 0.5);

	int tmp;
	double tmp1;
	for(i = 0; i < len; i++)
    {
		tmp1 = double(srcdata[i]);
		tmp = (tmp1 > 0) ? (int) (tmp1 + 0.5) : (int) (tmp1 - 0.5);	//round to integers
		minn = (minn < tmp) ? minn : tmp;
		maxx = (maxx > tmp) ? maxx : tmp;
		desdata[i] = tmp;
  //    printf("%i ",desdata[i]);
    }
  //printf("\n");

  //make the vector data begin from 0 (i.e. 1st state)
	for(i = 0; i < len; i++)
		desdata[i] -= minn;

  //return the #state
	nstate = (maxx - minn + 1);

	return;
}


template < class T > double * compute_jointprob(T * img1, T * img2, long len, long maxstatenum, int &nstate1, int &nstate2)
{
	//get and check size information

	long i, j;
//  int nstate1 = 0, nstate2 = 0;

	int b_returnprob = 1;

  //copy data into new INT type array (hence quantization) and then reange them begin from 0 (i.e. state1)

	int *vec1 = new int[len];
	int *vec2 = new int[len];

	int nrealstate1 = 0, nrealstate2 = 0;

	copyvecdata(img1, len, vec1, nrealstate1);
	copyvecdata(img2, len, vec2, nrealstate2);

  //update the #state when necessary
	nstate1 = (nstate1 < nrealstate1) ? nrealstate1 : nstate1;
  //printf("First vector #state = %i\n",nrealstate1);
	nstate2 = (nstate2 < nrealstate2) ? nrealstate2 : nstate2;
  //printf("Second vector #state = %i\n",nrealstate2);

//  if (nstate1!=maxstatenum || nstate2!=maxstatenum)
//    printf("find nstate1=%d nstate2=%d\n", nstate1, nstate2);

  //generate the joint-distribution table

	double *hab = new double[nstate1 * nstate2];
	double **hab2d = new double *[nstate2];

	for(j = 0; j < nstate2; j++)
		hab2d[j] = hab + (long) j *nstate1;

	for(i = 0; i < nstate1; i++)
	{
		for(j = 0; j < nstate2; j++)
			hab2d[j][i] = 0;
	}

	for(i = 0; i < len; i++)
    {
		//old method -- slow
		//     indx = (long)(vec2[i]) * nstate1 + vec1[i];
		//     hab[indx] += 1;

		//new method -- fast
		hab2d[vec2[i]][vec1[i]] += 1;
		//printf("vec2[%d]=%d vec1[%d]=%d\n", i, vec2[i], i, vec1[i]);
    }

	//return the probabilities, otherwise return count numbers
	if(b_returnprob)
    {
		for(i = 0; i < nstate1; i++)
		{
			for(j = 0; j < nstate2; j++)
				hab2d[j][i] /= len;
		}
    }

	//finish
	if(hab2d)
    {
		delete[] hab2d;
		hab2d = 0;
    }
	if(vec1)
    {
		delete[] vec1;
		vec1 = 0;
    }

	if(vec2)
    {
		delete[] vec2;
		vec2 = 0;
    }

	return hab;
}

double compute_mutualinfo (double *pab, long pabhei, long pabwid)
{
	//check if parameters are correct

	if(!pab)
    {
		fprintf(stderr, "Got illeagal parameter in compute_mutualinfo().\n");
		exit(1);
    }

	long i, j;

	double **pab2d = new double *[pabwid];
	for(j = 0; j < pabwid; j++)
		pab2d[j] = pab + (long) j *pabhei;


	double *p1 = 0, *p2 = 0;
	long p1len = 0, p2len = 0;
	int b_findmarginalprob = 1;

	//generate marginal probability arrays
	if(b_findmarginalprob != 0)
    {
		p1len = pabhei;
		p2len = pabwid;
		p1 = new double[p1len];
		p2 = new double[p2len];

		for(i = 0; i < p1len; i++)
			p1[i] = 0;
		for(j = 0; j < p2len; j++)
			p2[j] = 0;

		for(i = 0; i < p1len; i++)	//p1len = pabhei
		{
			for(j = 0; j < p2len; j++)	//p2len = panwid
			{
//          	printf("%f ",pab2d[j][i]);
				p1[i] += pab2d[j][i];
				p2[j] += pab2d[j][i];
			}
		}
    }

	//calculate the mutual information

	double muInf = 0;

	muInf = 0.0;
	for(j = 0; j < pabwid; j++)	// should for p2
    {
		for(i = 0; i < pabhei; i++)	// should for p1
		{
			if(pab2d[j][i] != 0 && p1[i] != 0 && p2[j] != 0)
			{
				muInf += pab2d[j][i] * log(pab2d[j][i] / p1[i] / p2[j]);
	      //	printf("%f %fab %fa %fb\n",muInf,pab2d[j][i]/p1[i]/p2[j],p1[i],p2[j]);
			}
		}
    }

	muInf /= (double)log(2.0);

	//free memory
	if(pab2d)
		delete[]pab2d;

	if(b_findmarginalprob != 0)
    {
		if(p1)
			delete[] p1;
		if(p2)
			delete[] p2;
    }

	return muInf;
}

double calMutualInfo(DataTable * myData, long v1, long v2)
{
	double mi = -1;		//initialized as an illegal value

	if(!myData || !myData->data || !myData->data2d)
    {
		fprintf(stderr, "Line %d: The input data is NULL.\n", __LINE__);
		exit(1);
		return mi;
    }

	long nsample = myData->nsample;
	long nvar = myData->nvar;

	if(v1 >= nvar || v2 >= nvar || v1 < 0 || v2 < 0)
    {
		fprintf(stderr, "Line %d: The input variable indexes are invalid (out of range).\n", __LINE__);
		exit(1);
		return mi;
    }

	long i;

	//copy data

	int *v1data = new int[nsample];
	int *v2data = new int[nsample];

	if(!v1data || !v2data)
    {
		fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
		exit(1);
		return mi;
    }

	for(i = 0; i < nsample; i++)
    {
		v1data[i] = int(myData->data2d[i][v1]);	//the float already been discretized, should be safe now
		v2data[i] = int(myData->data2d[i][v2]);
    }

	//compute mutual info

	long nstate = 3;		//always true for DataTable, which was discretized as three states

	int nstate1 = 0, nstate2 = 0;
	double *pab =
    compute_jointprob(v1data, v2data, nsample, nstate, nstate1, nstate2);
	mi = compute_mutualinfo(pab, nstate1, nstate2);
  //printf("pab=%d nstate1=%d nstate2=%d mi=%5.3f\n", long(pab), nstate1, nstate2, mi);

  //free memory and return
  if(v1data)
  {
      delete[] v1data;
      v1data = 0;
  }
  if(v2data)
  {
      delete[] v2data;
      v2data = 0;
  }
  if(pab)
  {
      delete[] pab;
      pab = 0;
  }

  return mi;
}

long * mRMR_selection (DataTable * myData, long nfea, FeaSelectionMethod mymethod)
{
	long *feaInd = 0;

	if(!myData || !myData->data || !myData->data2d)
    {
		fprintf(stderr, "Line %d: The input data is NULL.\n", __LINE__);
		exit(1);
		return feaInd;
    }

	if(nfea < 0)
    {
		fprintf(stderr, "Line %d: The input data nfea is negative.\n", __LINE__);
		exit(1);
		return feaInd;
    }

	long poolUseFeaLen = myData->nvar - 1; //500;
//	long poolUseFeaLen = 500;
//	if(poolUseFeaLen > myData->nvar - 1)	// there is a target variable (the first one), that is why must remove one
//		poolUseFeaLen = myData->nvar - 1;

	if(nfea > poolUseFeaLen)
		nfea = poolUseFeaLen;

	feaInd = new long[nfea];
	if(!feaInd)
    {
		fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
		exit(1);
		return feaInd;
    }

	long i, j;

  //determine the pool

	float *mival = new float[myData->nvar];
	float *poolInd = new float[myData->nvar];
	char *poolIndMask = new char[myData->nvar];
	if(!mival || !poolInd || !poolIndMask)
    {
		fprintf(stderr, "Line %d: Fail to allocate memory.\n", __LINE__);
		exit(1);
		return feaInd;
    }

	for(i = 0; i < myData->nvar; i++)	//the mival[0] is the entropy of target classification variable
    {
		mival[i] = -calMutualInfo(myData, 0, i);	//set as negative for sorting purpose
		poolInd[i] = i;
		poolIndMask[i] = 1;	//initialized to be everything in pool would be considered
//      if(i < nfea)   printf("poolInd[%d]=%d\t%5.3f\n", i, int (poolInd[i]), mival[i]);
    }

//  printf ("\n{%d}\n", myData->nvar - 1);

	float *NR_mival = mival;	//vector_phc(1,myData->nvar-1);
	float *NR_poolInd = poolInd;	//vector_phc(1,myData->nvar-1);

	sort2(myData->nvar - 1, NR_mival, NR_poolInd);	// note that poolIndMask is not needed to be sorted, as everything in it is 1 up to this point

	mival[0] = -mival[0];

	for(i = 1; i < myData->nvar - 1; i++)
		mival[i] = -mival[i];

  //mRMR selection

	long poolFeaIndMin = 1;
	long poolFeaIndMax = poolFeaIndMin + poolUseFeaLen - 1;

	feaInd[0] = long(poolInd[1]);
	poolIndMask[feaInd[0]] = 0;	//after selection, no longer consider this feature

	poolIndMask[0] = 0;		// of course the first one, which corresponds to the classification variable, should not be considered. Just set the mask as 0 for safety.

	long k;
	for(k = 1; k < nfea; k++)	//the first one, feaInd[0] has been determined already
    {
		double relevanceVal, redundancyVal, tmpscore, selectscore = 0.0;
		long selectind = 0;
		int b_firstSelected = 0;

		for(i = poolFeaIndMin; i <= poolFeaIndMax; i++)
		{
			if(poolIndMask[long (poolInd[i])] == 0)
				continue;		//skip this feature as it was selected already

			relevanceVal = calMutualInfo (myData, 0, long(poolInd[i]));	//actually no necessary to re-compute it, this value can be retrieved from mival vector
			redundancyVal = 0;

			for(j = 0; j < k; j++)
				redundancyVal += calMutualInfo (myData, feaInd[j], long (poolInd[i]));

			redundancyVal /= k;

			switch(mymethod)
			{
				case MID:
					tmpscore = relevanceVal - redundancyVal;
					break;
				case MIQ:
					tmpscore = relevanceVal / (redundancyVal + 0.0001);
					break;
				default:
					fprintf(stderr, "Undefined selection method=%d. Use MID instead.\n", mymethod);
					tmpscore = relevanceVal - redundancyVal;
					break;
			}

			if(b_firstSelected == 0)
			{
				selectscore = tmpscore;
				selectind = long(poolInd[i]);
				b_firstSelected = 1;
			}
			else
			{
				if(tmpscore > selectscore)
				{		//update the best feature found and the score
					selectscore = tmpscore;
					selectind = long(poolInd[i]);
				}
			}
		}

		feaInd[k] = selectind;
		poolIndMask[selectind] = 0;
   	}

	//return
	if(mival)
    {
		delete[]mival;
		mival = 0;
    }
	if(poolInd)
    {
		delete[]poolInd;
		poolInd = 0;
    }
	if(poolIndMask)
    {
		delete[]poolIndMask;
		poolIndMask = 0;
    }

	return feaInd;
}

vector<int> mrmr(vector<string> data, int numberOfFeatures)
{
	vector<int> output;
	double t_discretize = 9999;
	int nfea = ceil(numberOfFeatures * paramFilteringProportion);
	int select_method = MID;
	long nmaxvar = numberOfFeatures;  //default maximum number of variables
	long nmaxsample = data.size();

	// Read data from CSV files
	DataTable *myData = readData (data, ',', 1, nmaxvar, nmaxsample);
	//  myData->printData (1, 10, 1, 10);

	//  printf ("nsample read=%d\nn_var read=%d\n", myData->nsample, myData->nvar);

	// Data preprocessing

	if (long (t_discretize) == 9999)
		myData->discretize (t_discretize, 0);	//do not do discretization
	else
		myData->discretize (t_discretize, 1);	//do discretization

	//  myData->printData (1, 10, 1, 10);
	//  myData->printData ();

	// mutual information test

	//  double curmi = calMutualInfo(myData, 0, 1);
	//  printf("mi(1,2) = %5.3f\n", curmi);

	// mRMR selecton
	long *feaInd = mRMR_selection (myData, nfea, FeaSelectionMethod (select_method));

	for(int i = 0; i < nfea; i++)
		output.push_back(feaInd[i]);

	delete[]feaInd;

	delete myData;

	return output;
}
