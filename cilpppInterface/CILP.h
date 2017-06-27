/*
 * CIL2P.h
 *
 *  Created on: 02/05/2011
 *      Author: Manoel
 */

#ifndef REDENEURALBP_H_
#define REDENEURALBP_H_

// Defines de parametros
#define VERSAO 1.0
#define BETA 1
#define ACRESCIMO_W 0.01

// Defines indicadores da camada da rede
#define ENTRADA 0
#define ESCONDIDA 1
#define SAIDA 2

// Definicoes auxiliares
#define TAM_LINHA 256
#define SEM_CONEXAO -FLT_MAX

#include <cmath>
#include <climits>
#include <cfloat>
#include <ctime>
#include <iostream>
#include <algorithm>

#include "Utilidades.h"

enum funcaoAtivacao {linear, binaria, hiperbolica, bipolar};
enum statusNeuronio {ativado, desconhecido, desativado, foraDosLimites, inexistente};

using namespace std;

class CILP
{
	public:
		CILP(int, vector<float>, bool);
		virtual ~CIL2P();

		void constroiRede(string, bool);
		void estabilizaRede(string);
		void treinaExemplos(string);
		void desativaRede();
		void imprimeStatusRede();

	private:
		int numCamadas;
		float w;
		float amin;
		bool stochastic;

		Utilidades util;

		vector<int> numNeuronios;
		vector<float> biasCamadas;
		vector<vector<float> > thresholds;
		vector< vector <float> > statusRede;
		vector< vector <string> > rotulosNeuronios;
		vector< vector < vector <pair<int, float> > > > pesosConexoes;

		float calculaFuncaoAtivacao(float, funcaoAtivacao);
		int calculaConexoesNaSaida(string);
		statusNeuronio retornaStatusNeuronio(int, string);
		string retornaStatusNeuronioString(int, string);
		bool redeEstabilizou();
};

#endif /* REDENEURALBP_H_ */
