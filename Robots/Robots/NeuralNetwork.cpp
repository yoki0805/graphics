#include "NeuralNetwork.h"


// methods for Neuron
Neuron::Neuron(int numberOfInputs) : numInputs(numberOfInputs + 1)
{
	// we need an additional weight for the bias hence the +1
	weights.reserve(numberOfInputs + 1);
	for (int i = 0; i < numberOfInputs + 1; ++i) {
		// set up the weights with an initial random value
		weights.push_back(RandomClamped());
	}
}


// methods for NeuronLayer
NeuronLayer::NeuronLayer(int numberOfNeurons, int numberOfInputsPerNeuron) : numNeurons(numberOfNeurons)
{
	// set up the neurons in this layer
	neurons.reserve(numberOfNeurons);
	for (int i = 0; i < numberOfNeurons; ++i) {
		neurons.push_back(Neuron(numberOfInputsPerNeuron));
	}
}


// methods for NeuralNet
NeuralNet::NeuralNet(int numberOfInputs, int numberOfOutputs,
	int numberOfHiddenLayers, int numberOfNeuronsPerHiddenlayer)
{
	numInputs = numberOfInputs;
	numOutputs = numberOfOutputs;
	numHiddenLayers = numberOfHiddenLayers;
	NeuronsPerHiddenLayer = numberOfNeuronsPerHiddenlayer;

	// including the output layer
	layers.reserve(numberOfHiddenLayers + 1);
	createNet();
}

void NeuralNet::createNet()
{
	// create the layers of the network
	if (numHiddenLayers >= 1)
	{
		// create first hidden layer
		layers.push_back(NeuronLayer(NeuronsPerHiddenLayer, numInputs));

		// other hidden layers
		for (int i = 0; i < numHiddenLayers - 1; ++i) {
			layers.push_back(NeuronLayer(NeuronsPerHiddenLayer, NeuronsPerHiddenLayer));
		}

		// create output layer
		layers.push_back(NeuronLayer(numOutputs, NeuronsPerHiddenLayer));
	}
	else
	{
		// create output layer
		layers.push_back(NeuronLayer(numOutputs, numInputs));
	}
}

vector<double> NeuralNet::getWeights()
{
	vector<double> weights;
	weights.reserve(getNumberOfWeights());

	// for each layer
	for (int i = 0; i < layers.size(); ++i) {
		// for each neuron
		for (int j = 0; j < layers[i].numNeurons; ++j) {
			// for each weight
			for (int k = 0; k < layers[i].neurons[j].numInputs; ++k) {
				weights.push_back(layers[i].neurons[j].weights[k]);
			}
		}
	}

	return weights;
}


int NeuralNet::getNumberOfWeights()
{
	if (numHiddenLayers >= 1) {
		int total = (numInputs + 1) * NeuronsPerHiddenLayer +
			(NeuronsPerHiddenLayer + 1) * NeuronsPerHiddenLayer * (numHiddenLayers - 1) +
			(NeuronsPerHiddenLayer + 1) * numOutputs;
		return total;
	}
	else
		return (numInputs + 1) * numOutputs;
}

void NeuralNet::setWeights(const vector<double>& weights)
{
	int cnt = 0;

	// for each layer
	for (int i = 0; i < layers.size(); ++i) {
		// for each neuron
		for (int j = 0; j < layers[i].numNeurons; ++j) {
			// for each weight
			for (int k = 0; k < layers[i].neurons[j].numInputs; ++k) {
				layers[i].neurons[j].weights[k] = weights[cnt++];
			}
		}
	}
}

vector<double> NeuralNet::getOutputs(const vector<double>& inputs)
{
	vector<double> outputs;
	vector<double> tempInputs;

	// first check that we have the correct amount of inputs
	if (inputs.size() != numInputs)
		return outputs;

	tempInputs = inputs;

	// for each layer
	for (int i = 0; i < layers.size(); ++i)
	{
		// pass the values to the next layer
		if (i >= 1)
			tempInputs = outputs;

		// clear up for this layer
		outputs.clear();

		// for each neuron
		for (int j = 0; j < layers[i].numNeurons; ++j)
		{
			double actValue = 0;
			int tempNumInputs = layers[i].neurons[j].numInputs;

			// for each weight
			for (int k = 0; k < tempNumInputs - 1; ++k)
			{
				// sum the weights x inputs
				actValue += layers[i].neurons[j].weights[k] * tempInputs[k];
			}

			// add in the bias(-1)
			actValue -= layers[i].neurons[j].weights[tempNumInputs - 1];

			// we can store the outputs from each layer as we generate them. 
			// the combined activation is first filtered through the sigmoid function
			outputs.push_back(sigmoid(actValue, 1));
		}
	}

	return outputs;
}