

/*
	CILP++

	Copyright 2014 Manoel Vitor Macedo França <maneuvitor@gmail.com>

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

//============================================================================
// Name        : hashUnifier.cpp
// Author      : Manoel Vitor Macedo Franca
// Version     : 1.2
// Copyright   : Apache License, Version 2.0
// Description : Program to unify BCP hashes
//============================================================================

#define NUMBER_OF_FOLDS 10
#define VALID_CHARACTERS "abcdefghijklmnopqrstuvxywzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"

//#define THEORY_PATH "C:\\Users\\abdz937\\Research\\Paper Results\\ECML 2013\\Amine\\weka_results\\resultsPerFoldRipper\\"
//#define MAPPING_PATH "C:\\Users\\abdz937\\Research\\Paper Results\\ECML 2013\\Amine\\"
//#define BOTTOM_PATH "C:\\Users\\abdz937\\Research\\Paper Results\\ECML 2013\\Amine\\"

#define MAPPING_PATH "C:\\Users\\abdz937\\Research\\Paper Results\\ECML 2013\\Amine\\"
#define BOTTOM_PATH "C:\\Users\\abdz937\\Research\\Paper Results\\ECML 2013\\Amine\\"

#define TEST_PATH "C:\\Users\\abdz937\\Research\\Datasets\\alzheimers\\amine\\"

#define CONSIDER_ALL_EXAMPLES true
#define CONSIDER_ALL_FEATURES false
#define GROUND_UNCONSTRAINED_VARIABLES true
#define REMOVE_UNCONSTRAINED_VARIABLES false
#define REWRITE_DATA false

#include "utilities.h"

Utilities util;
int currentExample;

// Convert feature id into feature string
string featureConverter(string featureId)
{
	int id;

	util.convertFromString<int>(id, featureId, std::dec);

	char factor 	 = ((id-1)/26) + 48;
	char featureChar = (id-1)%26 + 65;

	string result = "";

	result.append(1, featureChar);
	if(factor != '0')
		result.append(1, factor);

	return result;
}

typedef struct _MappingHash
{
	// Hash skeleton, for maintenance purposes
	vector<string> model;

	// Actual structure
	vector<string> keyMapping;
	vector<vector<string> > content;

	void setExamplesLength(int examplesSize)
	{
		model.resize(examplesSize, "");
	}

	vector<string> getContent(string hashKey)
	{
		int pos = util.find(keyMapping, hashKey);

		if(pos < 0)
		{
			vector<string> emptyContent;
			return emptyContent;
		}
		else
			return content.at(pos);
	};

	void pushContent(string hashKey, int example, string element)
	{
		int pos = util.find(keyMapping, hashKey);

		if(pos < 0)
		{


			keyMapping.push_back(hashKey);

			vector<string> newContent = model;
			newContent.at(example) = element;
			content.push_back(newContent);
		}
		else
			content.at(pos).at(example) = element;
	};

	// Function to get all variable mappings for a fold
	vector<pair<char,vector<char> > > populateHash(string fileName, int hashSize)
	{
		bool beginningOfFile = true, newExampleAlready = true;

		string line, mapping;

		vector<string> lines, mappingContent;
		vector<pair<char,vector<char> > > output;

		lines = util.readLines(fileName);

		setExamplesLength(hashSize);

		for(unsigned int i = 0; i < lines.size(); i++)
		{
			// If it is a mapping
			if(lines.at(i).at(0) == '$')
			{
				newExampleAlready = false;

				if(beginningOfFile)
					beginningOfFile = false;

				// Extracting the mapping content
				unsigned int found = lines.at(i).find("(");
				mapping = lines.at(i).substr(found+1);
				mapping = mapping.erase(mapping.size()-1);

				// Getting all four parameters from the mapping
				mappingContent = util.splitDelimited(mapping, ',');

				// Building the hash
				pushContent(featureConverter(mappingContent.at(0)), currentExample, mappingContent.at(2));
			}
			// If it is a new example
			else
			{
				if(!beginningOfFile && !newExampleAlready)
				{
					newExampleAlready = true;
					currentExample++;
				}
				continue;
			}
		}

		return output;
	}
}MappingHash;

// Smart Ascending Order Comparator
bool smartAscendingOrder(string first, string second)
{
	if(first.size() < second.size()) return true;
	else if(first.size() > second.size()) return false;
	else return first < second;
}

pair<vector<string>, pair<pair<vector<string>, vector<int> >, vector<string> > > extractVariablesFromClause(string clause)
{
	unsigned int pos, pos2;
	string headStr, bodyStr, tempStr, currentLiteral;
	vector<string> headVariables, currentHeadVariables, bodyVariables, currentBodyVariables, bodyLiterals;
	vector<int> arities;

	// This line was generating an error on the algorithm by making clauses to have a different size than bodies and heads
	// if(util.trim(line).size() > 0 && (find(trainedExamples.begin(), trainedExamples.end(), line) == trainedExamples.end()))
	if(util.trim(clause).size() > 0)
	{
		pos = clause.find(":-");

		if (pos != (unsigned int)string::npos)
		{
			tempStr = clause.substr(0, pos);
			headStr = util.trim(tempStr);

			tempStr = clause.substr(pos+2);
			bodyStr = util.trim(tempStr);
		}
		else
		{
			headStr  = clause;
			bodyStr = "true";
		}

		pos2 = bodyStr.find_first_of(VALID_CHARACTERS);
		while(pos2 != (unsigned int)string::npos)
		{
			string substring = bodyStr.substr(pos2);
			tempStr = substring.substr(0, substring.find(")") + 1);

			currentLiteral = util.trim(tempStr);
			bodyLiterals.push_back(currentLiteral);

			// Extracting the variables from the literal
			currentLiteral.erase(currentLiteral.begin(), currentLiteral.begin()+currentLiteral.find("(")+1);
			currentLiteral.erase(currentLiteral.begin()+currentLiteral.find(")"), currentLiteral.end());
			currentBodyVariables = util.splitDelimited(currentLiteral, ',');

			arities.push_back(currentBodyVariables.size());
			bodyVariables.insert(bodyVariables.end(), currentBodyVariables.begin(), currentBodyVariables.end());

			bodyStr.erase(bodyStr.begin(), bodyStr.begin()+bodyStr.find(tempStr)+tempStr.size()+1);

			pos2 = bodyStr.find_first_of(VALID_CHARACTERS);
		}

		pos = headStr.find("),");

		while(pos != (unsigned int)string::npos)
		{
			tempStr = headStr.substr(0, pos);
			currentLiteral = util.trim(tempStr);

			// Extracting the variables from the literal
			currentLiteral.erase(currentLiteral.begin(), currentLiteral.begin()+currentLiteral.find("(")+1);
			currentLiteral.erase(currentLiteral.begin()+currentLiteral.find(")"), currentLiteral.end());
			currentHeadVariables = util.splitDelimited(currentLiteral, ',');
			headVariables.insert(headVariables.end(), currentHeadVariables.begin(), currentHeadVariables.end());

			headStr.erase(headStr.begin(), headStr.begin()+pos+2);
			pos = headStr.find("),");
		}
		currentLiteral = util.trim(headStr);

		// Extracting the variables from the literal
		currentLiteral.erase(currentLiteral.begin(), currentLiteral.begin()+currentLiteral.find("(")+1);
		currentLiteral.erase(currentLiteral.begin()+currentLiteral.find(")"), currentLiteral.end());
		currentHeadVariables = util.splitDelimited(currentLiteral, ',');
		headVariables.insert(headVariables.end(), currentHeadVariables.begin(), currentHeadVariables.end());
	}

	return make_pair(headVariables, make_pair(make_pair(bodyLiterals, arities), bodyVariables));
}

MappingHash unifyHashes(MappingHash inputHash, pair<vector<vector<string> >, vector<string> > decodedBottoms)
{
	int currentExample = 0, index;
	MappingHash unifiedHash;

	for(unsigned int i = 0; i < inputHash.keyMapping.size(); i++)
	{
		inputHash.

		index =

		while(inputHash.keyMapping.at(i) == currentExample)
	}

	return unifiedHash;
}

int main()
{
	bool isConstrained, hadDeletedLiteral;
	int currentArity, accumulatedArity, variableIndex, literalIndex, positiveSplit;
	string fileName, processedClause, literal, temp, targetHead, disjunction, newRule, newAtom, formattedLiteral;
	vector<int> unconstrainedIndexes, literalIndexes;
	vector<string> tempLines, bottomsLines, rulesLines, program, examplesMapping, replacedLiterals, verifiedVariables, ignoredLiterals, linksPerRule;
	pair<vector<vector<string> >, vector<string> > decodedBottoms; // body -- head
	pair<vector<string>, pair<pair<vector<string>, vector<int> >, vector<string> > > clauseMappings; // head -- body

	ofstream outputStream;

	// 1. For each fold: read bottoms, mappings and rules, transform them and print it into an new theory file
	for(unsigned int i = 1; i <= NUMBER_OF_FOLDS; i++)
	{
		MappingHash hash;
		currentExample = 0;
		bottomsLines.clear();
		linksPerRule.clear();

		// 1.1. Reading BCP examples
		fileName = BOTTOM_PATH;
		fileName.append("trainingpositive").append(util.convertToString(i)).append(".data");
		tempLines = util.readLines(fileName);
		bottomsLines.insert(bottomsLines.end(), tempLines.begin(), tempLines.end());
		positiveSplit = bottomsLines.size();

		fileName = BOTTOM_PATH;
		fileName.append("trainingnegative").append(util.convertToString(i)).append(".data");
		tempLines = util.readLines(fileName);
		bottomsLines.insert(bottomsLines.end(), tempLines.begin(), tempLines.end());

		for(unsigned int j = 0; j < bottomsLines.size(); j++)
			replace(bottomsLines.at(j).begin(), bottomsLines.at(j).end(), ';', ',');

		// 1.3. Populating the hash
		fileName = MAPPING_PATH;
		fileName.append("bottoms").append(util.convertToString(i)).append("trainpositiveFeatures.txt");
		hash.populateHash(fileName, bottomsLines.size());

		fileName = MAPPING_PATH;
		fileName.append("bottoms").append(util.convertToString(i)).append("trainnegativeFeatures.txt");
		hash.populateHash(fileName, bottomsLines.size());

		// 1.4. Opening the output stream
		fileName = MAPPING_PATH;
		fileName.append("unifiedHash").append(util.convertToString(i)).append(".txt");
		outputStream.open(fileName.c_str());

		// 1.5. Obtaining all bodies and heads for all rules
		decodedBottoms = util.decodeRules(bottomsLines);

		pair<vector<vector<string> >, vector<string> > decodedRules;

		for(int j = 0; j < (int)rulesLines.size(); j++)
		{
			clauseMappings = extractVariablesFromClause(rulesLines.at(j));

			newRule = rulesLines.at(j);

			hadDeletedLiteral = false;

			bool positiveRule = false;
			if(newRule.substr(0, 3).compare("not"))
				positiveRule = true;

			// Searching for unconstrained variables
			literalIndexes.clear();
			unconstrainedIndexes.clear();
			replacedLiterals.clear();
			verifiedVariables.clear();
			ignoredLiterals.clear();

			for(int k = ((int)clauseMappings.second.second.size()-verifiedVariables.size())-1; (k >= 0) && GROUND_UNCONSTRAINED_VARIABLES && !REWRITE_DATA; k--)
			{
				isConstrained = !util.isVariable(clauseMappings.second.second.at(k));

				for(int l = (k-1); (l >= 0) && !isConstrained; l--)
				{
					if(!clauseMappings.second.second.at(k).compare(clauseMappings.second.second.at(l)))
					{
						isConstrained = true;
						break;
					}
				}

				if(!isConstrained)
				{
					for(unsigned int l = 0; l < clauseMappings.first.size(); l++)
					{
						if(!clauseMappings.second.second.at(k).compare(clauseMappings.first.at(l)))
						{
							isConstrained = true;
							break;
						}
					}
				}

				if(!isConstrained && (util.find(verifiedVariables, clauseMappings.second.second.at(k)) < 0))
				{
					// The variable is, indeed, unconstrained: needs to find more unconstrained variables in the same literal

					// Firstly, the literal of the clause must be found
					unconstrainedIndexes.push_back(k);
					currentArity = clauseMappings.second.second.size() - k;

					verifiedVariables.push_back(clauseMappings.second.second.at(k));

					accumulatedArity = 0;
					int uncLits = 1;

					for(int l = (((int)clauseMappings.second.first.second.size())-1); l >= 0; l--)
					{
						accumulatedArity += clauseMappings.second.first.second.at(l);
						if(accumulatedArity == currentArity)
						{
							literalIndexes.push_back(l);

							if((clauseMappings.second.first.second.at(l) == uncLits) && !CONSIDER_ALL_FEATURES)
								ignoredLiterals.push_back(clauseMappings.second.first.first.at(l));

							break;
						}
						else if(accumulatedArity > currentArity)
						{
							literalIndexes.push_back(l);

							// Searching for other unconstrained variables in the same literal
							for(int m = (k-1); m >= (((int)clauseMappings.second.second.size()) - accumulatedArity); m--)
							{
								bool isConstrained2 = !util.isVariable(clauseMappings.second.second.at(m));

								for(int n = (m-1); (n >= 0) && !isConstrained2; n--)
								{
									if(!clauseMappings.second.second.at(m).compare(clauseMappings.second.second.at(n)))
									{
										isConstrained2 = true;
										break;
									}
								}

								if(!isConstrained2)
								{
									for(unsigned int n = 0; n < clauseMappings.first.size(); n++)
									{
										if(!clauseMappings.second.second.at(m).compare(clauseMappings.first.at(n)))
										{
											isConstrained2 = true;
											break;
										}
									}
								}

								if(!isConstrained2 && (util.find(verifiedVariables, clauseMappings.second.second.at(m)) < 0))
								{
									// More unconstrained variables found: store their index
									unconstrainedIndexes.push_back(m);
									literalIndexes.push_back(l);
									uncLits++;
								}

								verifiedVariables.push_back(clauseMappings.second.second.at(m));
							}

							if((clauseMappings.second.first.second.at(l) == uncLits) && !CONSIDER_ALL_FEATURES)
								ignoredLiterals.push_back(clauseMappings.second.first.first.at(l));

							break;
						}
					}
				}
				else
					verifiedVariables.push_back(clauseMappings.second.second.at(k));
			}

			// Secondly, a literal mapping containing each rule literal which will be replaced is created
			for(unsigned int k = 0; k < literalIndexes.size(); k++)
			{
				if((util.find(replacedLiterals, clauseMappings.second.first.first.at(literalIndexes.at(k))) < 0) && (util.find(ignoredLiterals, clauseMappings.second.first.first.at(literalIndexes.at(k))) < 0))
				{
					if(REMOVE_UNCONSTRAINED_VARIABLES)
						ignoredLiterals.push_back(clauseMappings.second.first.first.at(literalIndexes.at(k)));
					else
						replacedLiterals.push_back(clauseMappings.second.first.first.at(literalIndexes.at(k)));
				}
			}

			// Thirdly, all examples must be queried to build the disjunction, based on which examples contain the unconstrained features
			for(unsigned int k = 0; k < replacedLiterals.size(); k++)
			{
				bool positiveFeature = true;

				int tempIndex = newRule.find(replacedLiterals.at(k));
				while(newRule.at(tempIndex) != ' ')
					tempIndex--;
				if(newRule.at(tempIndex-1) == '+')
					positiveFeature = false;

				int examplesFrom, examplesTo;

				if(CONSIDER_ALL_EXAMPLES)
				{
					examplesFrom = 0;
					examplesTo = decodedBottoms.first.size();
				}
				else
				{
					if(positiveRule)
					{
						examplesFrom = 0;
						examplesTo = positiveSplit;
					}
					else
					{
						examplesFrom = positiveSplit;
						examplesTo = decodedBottoms.first.size();
					}
				}

				disjunction = "(";

				for(int l = examplesFrom; l < examplesTo; l++)
				{
					if(util.findAtom(decodedBottoms.first.at(l), replacedLiterals.at(k)) >= 0)
					{
						newAtom = replacedLiterals.at(k);

						for(unsigned int m = 0; m < literalIndexes.size(); m++)
						{
							if(clauseMappings.second.first.first.at(literalIndexes.at(m)).compare(replacedLiterals.at(k)))
								continue;

							variableIndex = unconstrainedIndexes.at(m);
							literalIndex = literalIndexes.at(m);

							examplesMapping = hash.getContent(clauseMappings.second.second.at(variableIndex));

							if(examplesMapping.at(l).size() > 0)
								newAtom.replace(newAtom.find(clauseMappings.second.second.at(variableIndex)), clauseMappings.second.second.at(variableIndex).size(), examplesMapping.at(l));
						}

						if(newAtom.compare(replacedLiterals.at(k)) && (disjunction.find(newAtom) == string::npos))
						{
							disjunction.append(newAtom);
							disjunction.append(";");
						}
					}
				}

				if(disjunction.find(";") != string::npos)
					disjunction.replace(disjunction.size()-1, 1, ")");
				else
					disjunction = "";

				literalIndex = newRule.find(replacedLiterals.at(k));

				if(disjunction.size() > 0)
					newRule.replace(literalIndex, replacedLiterals.at(k).size(), disjunction);
				else
				{
					hadDeletedLiteral = true;
					if(!positiveFeature)
					{
						while(newRule.at(literalIndex-1) != '+')
							literalIndex--;

						newRule.replace(literalIndex-2, (newRule.find(replacedLiterals.at(k))-literalIndex)+replacedLiterals.at(k).size()+2, disjunction);
					}
					else
						newRule.replace(literalIndex, replacedLiterals.at(k).size(), disjunction);
				}
			}

			// Lastly, all ignored literals are removed from the clause
			for(unsigned int k = 0; k < ignoredLiterals.size(); k++)
			{
				hadDeletedLiteral = true;
				literalIndex = newRule.find(ignoredLiterals.at(k));

				while(newRule.at(literalIndex-1) == ' ')
					literalIndex--;
				if(newRule.at(literalIndex-1) == '+')
					newRule.replace(literalIndex-2, (newRule.find(ignoredLiterals.at(k))-literalIndex)+ignoredLiterals.at(k).size()+2, "");
				else
					newRule.replace(newRule.find(ignoredLiterals.at(k)), ignoredLiterals.at(k).size(), "");
			}

			// Formatting the rule
			util.formatRule(newRule);

			// The new rule is printed in the new theory file, if no literals were removed
			if(!hadDeletedLiteral)
			{
				if(REWRITE_DATA)
					newRule.insert(newRule.size()-2, linksPerRule.at(linksPerRule.size()-1) + ",");
				outputStream << newRule << endl;
			}
			else
			{
				rulesLines.at(j) = newRule;
				j--;
			}
		}

		// 1.7. If re-generating testing data, add extra rules to the generated theory
		if(REWRITE_DATA && !COMBINE_RULES)
		{
			outputStream << endl;

			// Firstly, adding rules to positive testing data
			fileName = BOTTOM_PATH;
			fileName.append("testingpositive").append(util.convertToString(i)).append(".data");
			bottomsLines = util.readLines(fileName);

			fileName = TEST_PATH;
			fileName.append("test").append(util.convertToString(i)).append(".f");
			tempLines = util.readLines(fileName);

			for(unsigned int j = 0; j < linksPerRule.size(); j++)
			{
				for(unsigned int k = 0; k < bottomsLines.size(); k++)
				{
					// Creating one rule for each test example
					newRule = linksPerRule.at(j) + ":-";

					// Formatting the data
					replace(bottomsLines.at(k).begin(), bottomsLines.at(k).end(), ';', ',');
					int index = bottomsLines.at(k).find(":-");

					newRule.append(tempLines.at(k).substr(0, tempLines.at(k).size()-1) + ",!,");
					newRule.append(bottomsLines.at(k).substr(index+2));

					outputStream << newRule << endl;
				}

				outputStream << endl;
			}

			// Secondly, adding rules to negative testing data
			fileName = BOTTOM_PATH;
			fileName.append("testingnegative").append(util.convertToString(i)).append(".data");
			bottomsLines = util.readLines(fileName);

			fileName = TEST_PATH;
			fileName.append("test").append(util.convertToString(i)).append(".n");
			tempLines = util.readLines(fileName);

			for(unsigned int j = 0; j < linksPerRule.size(); j++)
			{
				for(unsigned int k = 0; k < bottomsLines.size(); k++)
				{
					// Creating one rule for each test example
					newRule = linksPerRule.at(j) + ":-";

					// Formatting the data
					replace(bottomsLines.at(k).begin(), bottomsLines.at(k).end(), ';', ',');
					int index = bottomsLines.at(k).find(":-");

					newRule.append(tempLines.at(k).substr(0, tempLines.at(k).size()-1) + ",!,");
					newRule.append(bottomsLines.at(k).substr(index+2));

					outputStream << newRule << endl;
				}

				outputStream << endl;
			}
		}

		// 1.8. Closing the output stream
		outputStream.close();
	}

	return 0;
}
