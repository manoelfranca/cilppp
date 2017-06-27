
#include "Utilities.h"

Utilities::Utilities(wxTextCtrl *memo)
{
	bool fileExists = (logFile = fopen("log.txt", "r"));

    memoInput = memo;

	if(fileExists)
	{
        fclose(logFile);
        logFile = fopen("log.txt", "a");        
    }
	else
    {
        logFile = fopen("log.txt", "a");

		//time(&qtSegundos);
		//timestampAtual = localtime(&qtSegundos);

		//fprintf(arqLog, "C-ILP, Versão %0.1f\n", VERSAO);
		fprintf(logFile, "Autor: Manoel Vitor Macedo França\n\n");
		//fprintf(arqLog, "*Log de execução* %s\n\n", asctime(timestampAtual));
	}
}

Utilities::Utilities(){}
Utilities::~Utilities(){}
    
int Utilities::find(vector<string> vetor, string elemento)
{
	unsigned int i;

	for(i = 0; i < vetor.size(); i++)
	{
		if(vetor.at(i).compare(elemento) == 0)
			return i;
	}
	return -1;
}

int find(vector<int> vetor, int elemento)
{
	unsigned int i;

	for(i = 0; i < vetor.size(); i++)
	{
		if(vetor.at(i) == elemento)
			return i;
	}
	return -1;
}

bool Utilities::fileExists(string fileName)
{
	if(FILE *file = fopen(fileName.c_str(), "r"))
	{
		fclose(file);
		return true;
	}

	return false;
}

void Utilities::printLog(string message, bool newLine)
{
    fprintf(logFile, "%s", message.c_str());
    *memoInput << message.c_str();

    if(newLine)
    {
        fprintf(logFile, "\n");
        *memoInput << "\n";
    }
}

