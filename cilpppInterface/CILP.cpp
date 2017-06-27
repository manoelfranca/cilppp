/*
 * CIL2P.cpp
 *
 *  Created on: 02/05/2011
 *      Author: Manoel
 */

#include "CILP.h"

CILP::CILP(int numeroCamadas, vector<float> biasIniciais, bool ehStochastic)
{
	// 1. Guardando o numero de camadas
	numCamadas = numeroCamadas;

	// 2. Guardando o bias
	if(!biasIniciais.empty())
		biasCamadas = biasIniciais;

	// 3. Alocando os vetores de pesos
	pesosConexoes.resize(numCamadas);

	// 4. Alocando os vetores de rotulos
	rotulosNeuronios.resize(numCamadas);

	// 6. Alocando os thresholds
	thresholds.resize(numCamadas);
}

CILP::~CILP()
{
	unsigned int i, j;

	// 1. Esvaziando numNeuronios
	numNeuronios.clear();

	// 2. Esvaziando biasCamadas
	biasCamadas.clear();

	// 3. Esvaziando os pesos das camadas
	for(i = 0; i < pesosConexoes.size(); i++)
	{
		for(j = 0; j < pesosConexoes.at(i).size(); j++)
			pesosConexoes.at(i).at(j).clear();
	}
}

// Metodo responsavel por construir a rede neural a partir do arquivo em 'caminhoArqProg'. 'fullyConnected' indica se a rede sera parcial
void CILP::constroiRede(string caminhoArqProg, bool fullyConnected)
{
	unsigned int pos;
	string linha, cabecas, cauda;

	float pesoW = 0.0;
	bool existeConexao;

	int maxP_k_m;
	vector<int> k;
	vector<int> m;

	vector<string> clausulas, caudas, temp, vetorIterado;
	vector<vector<string> > cabecasSep;
	vector<pair<int, float> > novoVetorPesos;
	vector<float> novoVetorStatus;

	char linhaCharArray[TAM_LINHA];
	FILE *arquivo, *arqLog;

	time_t qtSegundos;
	struct tm *timestampAtual;

	// 1. Inicializacao
	k.reserve(1);
	m.reserve(1);
	clausulas.reserve(1);
	caudas.reserve(1);
	temp.reserve(1);
	cabecasSep.reserve(1);
	novoVetorPesos.reserve(1);
	novoVetorStatus.reserve(1);

	arqLog = fopen("arquivos/log.txt", "w");

	time(&qtSegundos);
	timestampAtual = localtime(&qtSegundos);

	fprintf(arqLog, "C-IL²P, Versão %0.1f\n", VERSAO);
	fprintf(arqLog, "Autor: Manoel Vitor Macedo França\n\n");
	fprintf(arqLog, "*Log de execução* %s\n", asctime(timestampAtual));

	// 2. Abrindo o arquivo do programa logico, decodificando as clausulas e calculando os parametros da rede
	if ((arquivo = fopen(caminhoArqProg.c_str(), "r")) != NULL)
	{
		fprintf(arqLog, "|-------------------|\n");
		fprintf(arqLog, "|Construindo a rede:|\n");
		fprintf(arqLog, "|-------------------|\n\n");

		// 2.1. Lendo o arquivo do programa e separando-o em cabecas e caudas
		while (fgets(linhaCharArray, TAM_LINHA, arquivo) != NULL)
		{
			string linha(linhaCharArray);

			// Removendo caracteres de formatacao
			if(linha.at(linha.size()-1) == '\n')
				linha.erase(linha.size()-1);

			while(linha.find(" ") != string::npos)
			{
				pos = linha.find(" ");
				linha.erase(pos, 1);
			}

			if(linha.size() > 0 && (find(clausulas.begin(), clausulas.end(), linha) == clausulas.end()))
			{
				// 2.1.1. Separando as clausulas em cabecas e cauda
				clausulas.push_back(linha);
				pos = linha.find("->");

				if (pos != string::npos)
				{
					cabecas = linha.substr(0, pos);
					cauda  = linha.substr(pos+2, linha.size()-pos+1);
				}
				else
				{
					cabecas = "T";
					cauda  = linha;
				}

				// 2.1.2. Separando cada cabeca
				pos = cabecas.find(",");
				temp.clear();
				while(pos != string::npos)
				{
					temp.push_back(cabecas.substr(0, pos));
					cabecas.erase(0, pos+1);
					pos = cabecas.find(",");
				}
				temp.push_back(cabecas);
				cabecasSep.push_back(temp);

				// 2.1.3. Armazenando a cauda
				caudas.push_back(cauda);
			}
		}

		// 2.2. Calculando maxP_k_m
		for(unsigned int i = 0; i < clausulas.size(); i++)
		{
			// 2.2.1. Calculando k
			if(cabecasSep.at(i).at(0).compare("T") != 0)
				k.push_back(cabecasSep.at(i).size());
			else
				k.push_back(0);

			// 2.2.2. Calculando m
			m.push_back(0);
			for(unsigned int j = 0; j < caudas.size(); j++)
			{
				if(!caudas.at(i).compare(caudas.at(j)))
					m.at(i)++;
			}
		}

		// 2.2.3. Obtendo maxP_k_m
		vector<int> kOrd(k.begin(), k.end());
		vector<int> mOrd(m.begin(), m.end());

		sort(kOrd.begin(), kOrd.end());
		sort(mOrd.begin(), mOrd.end());

		maxP_k_m = kOrd.back() > mOrd.back() ? kOrd.back() : mOrd.back();

		// 2.4. Calculando Amin
		// O Amin deve ser maior que (maxP_k_m - 1) / (maxP_k_m + 1) e menor do que 1
		amin = (float) (maxP_k_m - 1) / (maxP_k_m + 1);
		amin += (1.0 - (amin / 2.0)) / 2.0;

		// 2.5. Calculando W
		w = ((float)2/BETA) * ((float)(log(1 + amin) - log(1 - amin)) / ((maxP_k_m * (amin - 1)) + amin + 1));
		w += ACRESCIMO_W;

		fprintf(arqLog, "maxP_k_m = %d; amin = %f; w = %f\n\n", maxP_k_m, amin, w);
	}
	else
	{
		cout << "[Erro] Arquivo de cláusulas não encontrado!" << endl;
		return;
	}

	// 3. Para cada clausula, monta-se a rede a partir dela
	for(unsigned int i = 0; i < clausulas.size(); i++)
	{
		// 3.1. Adicionando o novo neuronio na camada escondida
		rotulosNeuronios.at(ESCONDIDA).push_back(clausulas.at(i));

		fprintf(arqLog, "Neuronio da camada escondida criado: (%d) %s \n", i + 1, clausulas.at(i).c_str());

		// 3.2. Montando a camada de entrada
		for(unsigned int j = 0; j < cabecasSep.at(i).size(); j++)
		{
			// 3.2.1. Verificando se e um literal negativo
			if(cabecasSep.at(i).at(j).at(0) == '!')
			{
				cabecasSep.at(i).at(j).erase(0, 1);
				pesoW = -w;
			}
			else
				pesoW = w;

			if(find(rotulosNeuronios.at(ENTRADA).begin(), rotulosNeuronios.at(ENTRADA).end(), cabecasSep.at(i).at(j)) == rotulosNeuronios.at(ENTRADA).end())
			{
				rotulosNeuronios.at(ENTRADA).push_back(cabecasSep.at(i).at(j));

				fprintf(arqLog, "Neuronio de entrada criado e apontando para %d: %s \n", i + 1, cabecasSep.at(i).at(j).c_str());

				novoVetorPesos.clear();
				novoVetorPesos.push_back(make_pair(rotulosNeuronios.at(ESCONDIDA).size()-1, pesoW));
				pesosConexoes.at(ENTRADA).push_back(novoVetorPesos);
			}
			else
			{
				fprintf(arqLog, "Neuronio de entrada apontando para %d: %s \n", i + 1, cabecasSep.at(i).at(j).c_str());
				pesosConexoes.at(ENTRADA).at(util.find(rotulosNeuronios.at(ENTRADA), cabecasSep.at(i).at(j))).push_back(make_pair(rotulosNeuronios.at(ESCONDIDA).size()-1, pesoW));
			}
		}

		// 3.3. Montando a camada de saida

		// 3.3.1. Conectando a camada de saida com a camada de entrada
		if(find(rotulosNeuronios.at(SAIDA).begin(), rotulosNeuronios.at(SAIDA).end(), caudas.at(i)) == rotulosNeuronios.at(SAIDA).end())
		{
			rotulosNeuronios.at(SAIDA).push_back(caudas.at(i));
			fprintf(arqLog, "Neuronio de saida criado: %s\n", caudas.at(i).c_str());

			novoVetorPesos.clear();
			if(util.find(rotulosNeuronios.at(ENTRADA), caudas.at(i)) != -1)
			{
				fprintf(arqLog, "Neuronio de saida aponta para seu correspondente na entrada: %s\n", caudas.at(i).c_str());
				novoVetorPesos.push_back(make_pair(util.find(rotulosNeuronios.at(ENTRADA), caudas.at(i)), 1.0));
			}
			else
			{
				// 3.3.1.1. Caso nao exista o no de entrada correspondente ao no de saida, ele e criado
				rotulosNeuronios.at(ENTRADA).push_back(caudas.at(i));
				pesosConexoes.at(ENTRADA).push_back(novoVetorPesos);

				fprintf(arqLog, "Neuronio de saida causou criacao de um neuronio %s na entrada\n", caudas.at(i).c_str());

				novoVetorPesos.push_back(make_pair(rotulosNeuronios.at(ENTRADA).size()-1, 1.0));
			}

			pesosConexoes.at(SAIDA).push_back(novoVetorPesos);
		}

		// 3.3.2. Em seguida, faz a relacao da camada escondida com o no da camada de saida
		novoVetorPesos.clear();
		novoVetorPesos.push_back(make_pair(util.find(rotulosNeuronios.at(SAIDA), caudas.at(i)), w));
		pesosConexoes.at(ESCONDIDA).push_back(novoVetorPesos);

		fprintf(arqLog, "Neuronio da camada escondida %d aponta para: %s \n\n", i + 1, caudas.at(i).c_str());
	}

	// 3.4. Caso fullyConnected = true, a rede sera completamente conectada.
	//      Nesse caso, as conexoes restantes receberao peso 0
	if(fullyConnected)
	{
		for(int i = 0; i < numCamadas; i++)
		{
			for(unsigned int j = 0; j < rotulosNeuronios.at(i).size(); j++)
			{
				for(unsigned int k = 0; k < rotulosNeuronios.at((i+1)%3).size(); k++)
				{
					existeConexao = false;
					int k2 = k;

					// 3.4.1. Adicionando as conexoes com peso 0, para as camadas de entrada e escondida
					for(unsigned int l = 0; l < pesosConexoes.at(i).at(j).size(); l++)
					{
						if(pesosConexoes.at(i).at(j).at(l).first == k2)
						{
							existeConexao = true;
							break;
						}
					}
					if(!existeConexao)
						pesosConexoes.at(i).at(j).push_back(make_pair(k, 0.0));
				}
			}
		}
	}

	// 3.5. Inicializando o status da rede
	for(int i = 0; i < numCamadas; i++)
	{
		novoVetorStatus.clear();
		statusRede.push_back(novoVetorStatus);

		for(unsigned int j = 0; j < rotulosNeuronios.at(i).size(); j++)
			statusRede.at(i).push_back(-1.0);
	}

	// 3.6. Por fim, calcula-se os thresholds
	for(unsigned int i = 0; i < rotulosNeuronios.size(); i++)
	{
		vector<string> vetorIterado = rotulosNeuronios.at(i);

		for(unsigned int j = 0; j < vetorIterado.size(); j++)
		{
			/*
			 * Inclusao dos thresholds:
			 *
			 * ENTRADA: threshold = 0
			 * ESCONDIDA: threshold = (1+amin)*('k'-1)*w/2
			 * SAIDA: threshold = (1+amin)*(1-'m')*w/2
			 */
			switch(i)
			{
				case ENTRADA:
					thresholds.at(i).push_back(0.0);
					fprintf(arqLog, "Threshold do neuronio da camada de entrada %s: %f\n", vetorIterado.at(j).c_str(), 0.0);
					break;
				case ESCONDIDA:
					thresholds.at(i).push_back((1+amin)*(k.at(j)-1)*((float)w/2));
					fprintf(arqLog, "Threshold do neuronio da camada escondida %s: %f\n", vetorIterado.at(j).c_str(), (1+amin)*(k.at(j)-1)*((float)w/2));
					break;
				case SAIDA:
					thresholds.at(i).push_back((1+amin)*(1-m.at(j))*((float)w/2));
					fprintf(arqLog, "Threshold do neuronio da camada de saida %s: %f\n", vetorIterado.at(j).c_str(), (1+amin)*(1-m.at(j))*((float)w/2));
					break;
				default:
					break;
			}
		}
	}

	fprintf(arqLog, "\n");
	fclose(arqLog);
}

// Metodo responsavel por processar as entradas ate se encontrar o ponto fixo
void CILP::estabilizaRede(string entrada)
{
	FILE *arqLog;

	unsigned int pos;
	bool arquivoExiste, estabilizou;

	vector<float> somatorioPesosChegando, valoresSaidaNeuronio;
	funcaoAtivacao funcaoUsada;

	time_t qtSegundos;
	struct tm *timestampAtual;

	// Inicializando o arquivo de log
	arquivoExiste = util.arquivoExiste("arquivos/log.txt");
	arqLog = fopen("arquivos/log.txt", "a");

	if(!arquivoExiste)
	{
		time(&qtSegundos);
		timestampAtual = localtime(&qtSegundos);

		fprintf(arqLog, "C-ILP, Versão %0.1f\n", VERSAO);
		fprintf(arqLog, "Autor: Manoel Vitor Macedo França\n\n");
		fprintf(arqLog, "*Log de execução* %s\n\n", asctime(timestampAtual));
	}

	fprintf(arqLog, "|---------------------|\n");
	fprintf(arqLog, "|Estabilizando a rede:|\n");
	fprintf(arqLog, "|---------------------|\n\n");

	// 1. Inicializando a rede, a partir dos neuronios fornecidos como entrada
	for(unsigned int i = 0; i < rotulosNeuronios.at(ENTRADA).size(); i++)
		valoresSaidaNeuronio.push_back(0.0);

	pos = entrada.find(",");
	fprintf(arqLog, "Neuronios de entrada ativados: ");
	while(pos != string::npos)
	{
		if(util.find(rotulosNeuronios.at(ENTRADA), entrada.substr(0, pos)) != -1)
		{
			statusRede.at(ENTRADA).at(util.find(rotulosNeuronios.at(ENTRADA), entrada.substr(0, pos))) = 1.0;
			valoresSaidaNeuronio.at(util.find(rotulosNeuronios.at(ENTRADA), entrada.substr(0, pos))) = 1.0;
			fprintf(arqLog, "%s, ", entrada.substr(0, pos).c_str());
		}

		entrada.erase(0, pos+1);
		pos = entrada.find(",");
	}
	if(entrada.size() > 0)
	{
		statusRede.at(ENTRADA).at(util.find(rotulosNeuronios.at(ENTRADA), entrada)) = 1.0;
		valoresSaidaNeuronio.at(util.find(rotulosNeuronios.at(ENTRADA), entrada)) = 1.0;
		fprintf(arqLog, " %s\n", entrada.c_str());
	}

	// 2. Marcando os neuronios com rotulo "T" como ativados
	for(unsigned int i = 0; i < rotulosNeuronios.at(ENTRADA).size(); i++)
	{
		// Se eh o no com rotulo "T", marca-se ele como ativado
		if(rotulosNeuronios.at(ENTRADA).at(i).compare("T") == 0)
		{
			fprintf(arqLog, "Neuronio 'T' ativado \n\n");
			statusRede.at(ENTRADA).at(i) = 1.0;
		}
	}

	// 3. Loop principal: enquanto os neuronios de entrada e saida nao se coincidirem, os sinais serao propagados pela rede
	estabilizou = false;
	fprintf(arqLog, "Processo de estabilizacao:\n\n");
	for(int iteracao = 0; !estabilizou; iteracao++)
	{
		// 3.1. Propagando a ativacao para a camada seguinte
		for(int i = 0; i < numCamadas; i++)
		{
			fprintf(arqLog, "Camada %d:\n", i + 1);

			// Se a camada a receber os sinais e a de entrada, a funcao e linear; caso contrario, bipolar
			funcaoUsada = (i == SAIDA ? linear : bipolar);

			// 3.1.1. Resetando o vetor contendo o somatorio dos pesos chegando em um no. Se o no for um 'T', ele recebera somatorio 1
			somatorioPesosChegando.clear();
			for(unsigned int j = 0; j < rotulosNeuronios.at((i+1)%3).size(); j++)
			{
				if(rotulosNeuronios.at((i+1)%3).at(j).compare("T") == 0)
					somatorioPesosChegando.push_back(1.0);
				else
					somatorioPesosChegando.push_back(0.0);
			}

			// 3.1.2. Para cada neuronio da camada seguinte, soma-se os pesos de suas conexoes com os neuronios da camada anterior
			//        multiplicados pelos vetores de saida
			for(unsigned int j = 0; j < valoresSaidaNeuronio.size(); j++)
			{
				for(unsigned int k = 0; k < pesosConexoes.at(i).at(j).size(); k++)
					somatorioPesosChegando.at(pesosConexoes.at(i).at(j).at(k).first) += (pesosConexoes.at(i).at(j).at(k).second * valoresSaidaNeuronio.at(j));
			}

			// 3.1.3. Aplica-se a funcao de ativacao a cada somatorio para verificar se o neuronio sera ativado
			valoresSaidaNeuronio.clear();
			for(unsigned int j = 0; j < somatorioPesosChegando.size(); j++)
			{
				fprintf(arqLog, "Somatorio chegando no neuronio %s da camada %d: %.6f\n", rotulosNeuronios.at((i+1)%3).at(j).c_str(), (i + 1)%3 + 1, somatorioPesosChegando.at(j));
				switch(funcaoUsada)
				{
					case linear:
						fprintf(arqLog, "Funcao de ativacao: linear\n");
						break;
					case binaria:
						fprintf(arqLog, "Funcao de ativacao: binaria\n");
						break;
					case hiperbolica:
						fprintf(arqLog, "Funcao de ativacao: hiperbolica\n");
						break;
					case bipolar:
						fprintf(arqLog, "Funcao de ativacao: bipolar\n");
						break;
					default:
						break;
				}
				fprintf(arqLog, "Resultado da funcao de ativacao: %f\n", calculaFuncaoAtivacao(somatorioPesosChegando.at(j), funcaoUsada));
				fprintf(arqLog, "Threshold: %f\n", thresholds.at((i+1)%3).at(j));

				// 3.1.3.1. Calculando o vetor de saida da proxima camada e atualizando seu status
				valoresSaidaNeuronio.push_back(calculaFuncaoAtivacao(somatorioPesosChegando.at(j) - thresholds.at((i+1)%3).at(j), funcaoUsada));
				statusRede.at((i+1)%3).at(j) = calculaFuncaoAtivacao(somatorioPesosChegando.at(j) - thresholds.at((i+1)%3).at(j), funcaoUsada);

				if(valoresSaidaNeuronio.at(j) >= amin)
				{
					if(valoresSaidaNeuronio.at(j) <= 1.0)
						fprintf(arqLog, "Neuronio ativado: %s\n", rotulosNeuronios.at((i+1)%3).at(j).c_str());
					else
						fprintf(arqLog, "Neuronio %s obteve ativacao fora dos limites\n", rotulosNeuronios.at((i+1)%3).at(j).c_str());
				}
				else
				{
					if(valoresSaidaNeuronio.at(j) <= -amin)
					{
						if(valoresSaidaNeuronio.at(j) >= -1.0)
							fprintf(arqLog, "Neuronio desativado: %s\n", rotulosNeuronios.at((i+1)%3).at(j).c_str());
						else
							fprintf(arqLog, "Neuronio %s obteve desativacao fora dos limites\n", rotulosNeuronios.at((i+1)%3).at(j).c_str());
					}
					else
						fprintf(arqLog, "Neuronio %s obteve ativacao desconhecida\n", rotulosNeuronios.at((i+1)%3).at(j).c_str());
				}
			}

			fprintf(arqLog, "\n");

			// 3.1.4. Verificando se o sistema estabilizou (se a camada de saida contem ativacao identica a de entrada)
			if(i == ESCONDIDA)
				estabilizou = redeEstabilizou();
		}
	}

	fprintf(arqLog, "*Sistema estabilizou*\n\n");
}

// Metodo de aplicacao da funcao de ativacao passada aos pesos somados
float CILP::calculaFuncaoAtivacao(float somatorioPesos, funcaoAtivacao funcao)
{
	switch(funcao)
	{
		case linear:
			return somatorioPesos;
			break;
		case binaria:
			if(somatorioPesos < 0)
				return 0.0;
			else
				return 1.0;
			break;
		case hiperbolica:
			return tanh(somatorioPesos);
			break;
		case bipolar:
			return ((float) 2 / (1 + exp(-1 * somatorioPesos * BETA))) - 1;
			break;
		default:
			cout << "[Erro] Tipo de função de ativação escolhida não está implementada!" << endl;
			break;
	}

	return 0.0;
}

// Metodo responsavel por treinar a rede com arquivos de exemplo
void CILP::treinaExemplos(string caminhoArqExemplos)
{
	int pos;
	bool arquivoExiste;

	char linhaCharArray[TAM_LINHA];
	FILE *arquivo, *arqLog;

	time_t qtSegundos;
	struct tm *timestampAtual;

	string headStr, bodyStr;

	vector<string> trainedExamples;
	vector<string> heads;
	vector<vector<string>> bodies;

	// Initialization


	// 1. Inicializando o arquivo de log
	arquivoExiste = util.arquivoExiste("arquivos/log.txt");
	arqLog = fopen("arquivos/log.txt", "a");

	if(!arquivoExiste)
	{
		time(&qtSegundos);
		timestampAtual = localtime(&qtSegundos);

		fprintf(arqLog, "C-ILP, Versão %0.1f\n", VERSAO);
		fprintf(arqLog, "Autor: Manoel Vitor Macedo França\n\n");
		fprintf(arqLog, "*Log de execução* %s\n\n", asctime(timestampAtual));
	}

	// 2. Abrindo e analisando o arquivo de exemplos de treino
	if ((arquivo = fopen(caminhoArqProg.c_str(), "r")) != NULL)
	{
		fprintf(arqLog, "|-----------------|\n");
		fprintf(arqLog, "|Treinando a rede:|\n");
		fprintf(arqLog, "|-----------------|\n\n");

		// 2.1. Lendo o arquivo do programa e separando-o em cabecas e caudas
		// Example format: T <- L1,L2,L3,...,Ln, in which T represents the target to classify.
		while (fgets(linhaCharArray, TAM_LINHA, arquivo) != NULL)
		{
			string linha(linhaCharArray);

			// Removendo caracteres de formatacao
			if(linha.at(linha.size()-1) == '\n')
				linha.erase(linha.size()-1);

			while(linha.find(" ") != string::npos)
			{
				pos = linha.find(" ");
				linha.erase(pos, 1);
			}

			// 2.1.1. Splitting the example into body literals and head literal
			if(linha.size() > 0 && (find(trainedExamples.begin(), trainedExamples.end(), linha) == trainedExamples.end()))
			{
				trainedExamples.push_back(linha);
				pos = linha.find("->");

				if (pos != string::npos)
				{
					bodyStr = linha.substr(0, pos);
					headStr = linha.substr(pos+2, linha.size()-pos+1);
				}
				else
				{
					bodyStr = "T";
					headStr  = linha;
				}

				// 2.1.2. Separando cada cabeca
				pos = bodyStr.find(",");
				temp.clear();
				while(pos != string::npos)
				{
					temp.push_back(bodyStr.substr(0, pos));
					bodyStr.erase(0, pos+1);
					pos = bodyStr.find(",");
				}
				temp.push_back(bodyStr);
				bodies.push_back(temp);

				// 2.1.3. Armazenando a cauda
				heads.push_back(headStr);
			}

		}
	}
}

// Metodo para calcular o numero de clausulas que possuem o no passado como cauda
int CILP::calculaConexoesNaSaida(string rotuloNeuronio)
{
	int total = 0;
	pair<int, float> conexao;

	for(unsigned int i = 0; i < pesosConexoes.at(ESCONDIDA).size(); i++)
	{
		for(unsigned int j = 0; j < pesosConexoes.at(ESCONDIDA).at(i).size(); j++)
		{
			conexao = pesosConexoes.at(ESCONDIDA).at(i).at(j);

			if(util.find(rotulosNeuronios.at(SAIDA), rotuloNeuronio) == conexao.first)
			{
				total++;
				break;
			}
		}
	}

	return total;
}

// Metodo para desativar todos os neuronios (para realizar outros experimentos com a mesma rede)
void CILP::desativaRede()
{
	for(unsigned int i = 0; i < statusRede.size(); i++)
	{
		for(unsigned int j = 0; statusRede.at(i).size(); j++)
			statusRede.at(i).at(j) = -1.0;
	}
}

// Metodo para retornar o status de um neuronio como um enum
statusNeuronio CILP::retornaStatusNeuronio(int camada, string rotulo)
{
	if(util.find(rotulosNeuronios.at(camada), rotulo) == -1)
		return inexistente;

	float nivelAtivacaoNeuronio = statusRede.at(camada).at(util.find(rotulosNeuronios.at(camada), rotulo));

	if((nivelAtivacaoNeuronio <= 1.0) && (nivelAtivacaoNeuronio >= -1.0))
	{
		if(nivelAtivacaoNeuronio >= amin)
			return ativado;
		else if(nivelAtivacaoNeuronio <= -amin)
			return desativado;
		else
			return desconhecido;
	}
	else
		return foraDosLimites;
}

// Metodo para retornar o status de um neuronio como uma string
string CILP::retornaStatusNeuronioString(int camada, string rotulo)
{
	if(util.find(rotulosNeuronios.at(camada), rotulo) == -1)
		return "inexistente";

	float nivelAtivacaoNeuronio = statusRede.at(camada).at(util.find(rotulosNeuronios.at(camada), rotulo));

	if((nivelAtivacaoNeuronio <= 1.0) && (nivelAtivacaoNeuronio >= -1.0))
	{
		if(nivelAtivacaoNeuronio >= amin)
			return "ativado";
		else if(nivelAtivacaoNeuronio <= -amin)
			return "desativado";
		else
			return "desconhecido";
	}
	else
		return "foraDosLimites";
}

// Metodo para verificar se a rede estabilizou
bool CILP::redeEstabilizou()
{
	// Se o status dos nos de entrada e de saida com mesmo rotulo forem iguais, o sistema estabilizou
	for(unsigned int i = 0; i < rotulosNeuronios.at(SAIDA).size(); i++)
	{
		// Se o neuronio de saida 'i' estiver com status diferente do que a entrada, nao estabilizou
		if(retornaStatusNeuronio(SAIDA, rotulosNeuronios.at(SAIDA).at(i)) == retornaStatusNeuronio(ENTRADA, rotulosNeuronios.at(SAIDA).at(i)))
		{
			if(retornaStatusNeuronio(ENTRADA, rotulosNeuronios.at(SAIDA).at(i)) == foraDosLimites)
				cout << "[Erro] Existem neuronios na rede fora dos limites de ativacao!" << endl;

			continue;
		}
		else if(retornaStatusNeuronio(ENTRADA, rotulosNeuronios.at(SAIDA).at(i)) == inexistente)
			continue;

		return false;
	}

	return true;
}

void CILP::imprimeStatusRede()
{
	cout << endl << "Status atual dos neuronios do sistema: " << endl << endl;

	for(unsigned int i = 0; i < statusRede.size(); i++)
	{
		cout << "Camada: " << i + 1 << endl;

		for(unsigned int j = 0; j < statusRede.at(i).size(); j++)
			cout << rotulosNeuronios.at(i).at(j) << " - (" << retornaStatusNeuronioString(i, rotulosNeuronios.at(i).at(j)) << ")" << statusRede.at(i).at(j) << endl;

		cout << endl;
	}
}
