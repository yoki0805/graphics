#pragma once
#include <cmath>
#include <vector>
#include "Utils.h"
using namespace std;


struct Neuron
{
	// the number of inputs into this neuron
	int numInputs;
	// the weights for each input
	vector<double> weights;
	// constructor
	Neuron(int numberOfInputs);
};


struct NeuronLayer
{
	// the number of neurons in this layer
	int numNeurons;
	// the layer of neurons
	vector<Neuron> neurons;
	// constructor
	NeuronLayer(int numberOfNeurons, int numberOfInputsPerNeuron);
};


class NeuralNet
{
private:
	int numInputs;
	int numOutputs;
	int numHiddenLayers;
	int NeuronsPerHiddenLayer;

	// storage for each layer of neurons including the output layer
	vector<NeuronLayer> layers;
	void createNet();

public:
	NeuralNet(int numberOfInputs, int numberOfOutputs, 
		int numberOfHiddenLayers, int numberOfNeuronsPerHiddenlayer);
	
	// gets the weights from the ANN
	vector<double> getWeights();
	// returns total number of weights in net
	int getNumberOfWeights();
	// replaces the weights with new ones
	void setWeights(const vector<double>& weights);
	// calculates the outputs from a set of inputs
	vector<double>	getOutputs(const vector<double>& inputs);

	// sigmoid response curve
	inline double sigmoid(double activation, double response) { return (1.0 / (1.0 + exp(-activation / response))); }
};