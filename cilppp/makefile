# makefile for the CILP++ project
# Author: Manoel Vitor Macedo Franca

##### General commands: run, debug, clean #####

run: rvariables.o rmrmr.o rcilp.o rcilpExperiment.o rexperimentExecuter.o
	g++ -fopenmp -o cilp variables.o mrmr.o cilp.o cilpExperiment.o experimentExecuter.o -Wall	

debug: dvariables.o dmrmr.o dcilp.o dcilpExperiment.o dexperimentExecuter.o
	g++ -g -o cilp variables.o mrmr.o cilp.o cilpExperiment.o experimentExecuter.o -Wall		

clean:
	rm -f cilp cilp.exe variables.o cilp.o cilpExperiment.o experimentExecuter.o mrmr.o
	rm -f classes/cilpExperiment.h.gch classes/cilp.h.gch classes/utilities.h.gch classes/variables.h.gch
	rm -f classes/mrmr/mrmr.h.gch	

##### Internal commands #####
	
dmrmr.o: classes/mrmr/mrmr.cpp classes/mrmr/mrmr.h
	g++ -g -O0 -c classes/mrmr/mrmr.cpp classes/mrmr/mrmr.h -Wall

dvariables.o: classes/variables.cpp classes/variables.h
	g++ -g -O0 -c classes/variables.cpp classes/variables.h -Wall

dcilp.o: classes/cilp.cpp classes/cilp.h
	g++ -g -O0 -c classes/cilp.cpp classes/cilp.h -Wall

dcilpExperiment.o: classes/cilpExperiment.cpp classes/cilpExperiment.h
	g++ -g -O0 -c classes/cilpExperiment.cpp classes/cilp.h -Wall

dexperimentExecuter.o: experiments/experimentExecuter.cpp classes/cilpExperiment.h
	g++ -g -O0 -c experiments/experimentExecuter.cpp classes/cilpExperiment.h -Wall
	
rmrmr.o: classes/mrmr/mrmr.cpp classes/mrmr/mrmr.h
	g++ -fopenmp -O3 -c classes/mrmr/mrmr.cpp classes/mrmr/mrmr.h -Wall

rvariables.o: classes/variables.cpp classes/variables.h
	g++ -fopenmp -O3 -c classes/variables.cpp classes/variables.h -Wall

rcilp.o: classes/cilp.cpp classes/cilp.h
	g++ -fopenmp -O3 -c classes/cilp.cpp classes/cilp.h -Wall

rcilpExperiment.o: classes/cilpExperiment.cpp classes/cilpExperiment.h
	g++ -fopenmp -O3 -c classes/cilpExperiment.cpp classes/cilp.h -Wall

rexperimentExecuter.o: experiments/experimentExecuter.cpp classes/cilpExperiment.h
	g++ -fopenmp -O3 -c experiments/experimentExecuter.cpp classes/cilpExperiment.h -Wall
	