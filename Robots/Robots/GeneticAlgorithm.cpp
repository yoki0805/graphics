#include "GeneticAlgorithm.h"


void GeneticAlgo::create(int popSize, double mutRate, double crossRate, int numWeights)
{
	population = popSize;
	mutationRate = mutRate;
	crossoverRate = crossRate;
	chromoLength = numWeights;

	// initialise population with chromosomes consisting of random
	// weights and all fitnesses set to zero
	genomes.reserve(popSize);
	for (int i = 0; i < popSize; ++i)
	{
		genomes.push_back(Genome());
		genomes[i].weights.reserve(numWeights);
		for (int j = 0; j < numWeights; ++j)
		{
			genomes[i].weights.push_back(RandomClamped());
		}
	}
}

void GeneticAlgo::mutate(vector<double>& chromo)
{
	static const double maxPerturbation = 0.2;
	for (int i = 0; i < chromo.size(); ++i)
	{
		// do we perturb this weight?
		if (RandFloat() < mutationRate) {
			// add or subtract a small value to the weight
			chromo[i] += (RandomClamped() * maxPerturbation);
		}
	}
}

Genome GeneticAlgo::getChromoRoulette()
{
	// generate a random number between 0 & total fitness count
	double slice = (double)(RandFloat() * totalFitness);

	if (totalFitness == 0)
		slice = 0;

	// this will be set to the chosen chromosome
	Genome selected;

	// go through the chromosones adding up the fitness so far
	double fitnessSoFar = 0;

	for (int i = 0; i < population; ++i)
	{
		fitnessSoFar += genomes[i].fitness;

		// if the fitness so far > random number return the chromo at this point
		if (fitnessSoFar >= slice)
		{
			selected = genomes[i];
			break;
		}
	}

	return selected;
}

void GeneticAlgo::crossover(const vector<double>& mama,
							const vector<double>& papa,
							vector<double>& baby1,
							vector<double>& baby2)
{
	// just return parents as offspring dependent on the rate
	// or if parents are the same
	if ((RandFloat() > crossoverRate) || (mama == papa))
	{
		baby1 = mama;
		baby2 = papa;
		return;
	}

	// determine a crossover point
	int breakPoint = RandInt(0, chromoLength - 1);

	// create the offspring
	for (int i = 0; i < breakPoint; ++i)
	{
		baby1.push_back(mama[i]);
		baby2.push_back(papa[i]);
	}

	for (int i = breakPoint; i < chromoLength; ++i)
	{
		baby1.push_back(papa[i]);
		baby2.push_back(mama[i]);
	}
}

void GeneticAlgo::elitism(int nBest, int numCopies, vector<Genome>& pop)
{
	int offset = 0;
	while (nBest--)
	{
		for (int i = 0; i < numCopies; ++i) {
			pop.push_back(genomes[(population - 1) - offset]);
		}
		offset++;
	}
}

vector<Genome> GeneticAlgo::epoch(const vector<Genome>& oldGenomes)
{
	// assign the given population to the classes population
	genomes = oldGenomes;

	// reset 
	totalFitness = 0;
	bestFitness = 0;
	averageFitness = 0;

	// sort the population
	sort(genomes.begin(), genomes.end());

	// get total fitness
	for (int i = 0; i < genomes.size(); ++i)
	{
		totalFitness += genomes[i].fitness;
	}
	// get average fitness
	averageFitness = totalFitness / population;
	// get best fitness
	bestFitness = genomes[population - 1].fitness;
	// get the best genome ever
	if (bestFitness >= bestGenomeEver.fitness) {
		bestGenomeEver.fitness = bestFitness;
		bestGenomeEver.weights = genomes[population - 1].weights;
	}

	vector<Genome> newGenomes;
	newGenomes.reserve(population);

	// keep n best chromos in new population
	newGenomes.push_back(bestGenomeEver);
	elitism(1, 1, newGenomes);

	// GA loop
	while (newGenomes.size() < population)
	{
		// grab two chromosones
		Genome mama = getChromoRoulette();
		Genome papa = getChromoRoulette();

		// create some offspring via crossover
		vector<double> baby1, baby2;
		crossover(mama.weights, papa.weights, baby1, baby2);

		// now we mutate
		mutate(baby1);
		mutate(baby2);

		// copy into new population
		newGenomes.push_back(Genome(baby1, 0));
		newGenomes.push_back(Genome(baby2, 0));
	}

	// finished so assign new pop back into genomes
	bestGenomeIndex = 1;
	genomes = newGenomes;
	generation++;

	return genomes;
}