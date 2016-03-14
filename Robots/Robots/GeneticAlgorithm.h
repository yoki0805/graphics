#pragma once
#include <vector>
#include <algorithm>
#include "Utils.h"
using namespace std;


// create a structure to hold each genome
struct Genome
{
	vector<double> weights;
	double fitness;

	Genome() : fitness(0) {}
	Genome(const vector<double>& w, double f): weights(w), fitness(f) {}

	// overload '<' used for sorting
	friend bool operator<(const Genome& a, const Genome& b) { return (a.fitness < b.fitness); }
};


// genetic algorithm
class GeneticAlgo
{
private:
	// holds the entire population of chromosomes
	vector<Genome> genomes;
	// population of genomes
	int population;
	// amount of weights per chromo
	int chromoLength;
	// total fitness of population
	double totalFitness;
	// best fitness
	double bestFitness;
	// average fitness
	double averageFitness;
	// keeps track of the best genome
	int bestGenomeIndex;

	// store the best genome in history
	Genome bestGenomeEver;

	// mutation rate
	double mutationRate;
	// crossover rate
	double crossoverRate;
	// generation counter
	int generation;

	void crossover(const vector<double>& mama,
				   const vector<double>& papa,
				   vector<double>& baby1, 
				   vector<double>& baby2);

	void mutate(vector<double>& chromo);

	Genome getChromoRoulette();

	void elitism(int nBest, int numCopiese, vector<Genome>& pop);

public:
	GeneticAlgo() : totalFitness(0), bestFitness(0),
		averageFitness(0), bestGenomeIndex(0), generation(0) {}

	// create a randomly pool
	void create(int popSize, double mutRate, double crossRate, int numWeights);
	
	// this runs the GA for one generation
	vector<Genome> epoch(const vector<Genome>& oldGenomes);

	// inline functions
	inline vector<Genome> getChromos() const { return this->genomes; }
	inline double getAverageFitness() const { return averageFitness; }
	inline double getBestFitness() const { return bestFitness; }
	inline double getBestFitnessEver() const { return bestGenomeEver.fitness; }
	inline int getGeneration() const { return generation; }
};