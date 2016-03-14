#pragma once
#include <list>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "NeuralNetwork.h"
#include "Utils.h"


class TurretInfo
{
public:
	const static float halfSize;
	const static float maxSpeed;
	const static float maxBaseRotSpeed;
	const static float maxBodyRotSpeed;
	const static float maxRaderRotSpeed;
	const static int maxHP;
	const static int maxReloadSteps;
};


class TurretAI
{
private:
	NeuralNet brain;
	double fitness;

public:
	const static int numInputs;
	const static int numOutputs;
	const static int numHiddenLayers;
	const static int numNeuronsPerHiddenLayer;

	// constructor
	TurretAI() : fitness(0), 
		brain(numInputs, numOutputs, numHiddenLayers, numNeuronsPerHiddenLayer) {}

	// inline functions
	inline NeuralNet& getNeuralNet() { return brain; }
	inline double getFitness() { return fitness; }
	inline void setFitness(double f) { fitness = f; }
	inline void addFitness(double f) { fitness += f; }
};

struct Explosion;
class AutoTurret;
class Bullet
{
public:
	glm::vec2 position;
	glm::vec2 direction;
	AutoTurret* belong;
	float theta;
	bool valid;

	Bullet() : belong(NULL), valid(false) {}

	void init(const glm::vec2& pos, float angle, AutoTurret* turretPtr);
	void update(int mapHalfSize, std::vector<AutoTurret>& turrets, std::list<Explosion>& booms);

	const static float halfSize;
	const static float speed;
};


class AutoTurret
{
private:
	// rendering info
	float thetaBase;
	float thetaBody;
	float thetaRader;

	// auto turret info
	int health;
	int reloading;
	glm::vec2 position;
	glm::vec2 direction;
	TurretAI ai;

	// helper functions
	float angleClamped(float theta);
	AutoTurret* getNearestTurret(std::vector<AutoTurret>& turrets);
	Bullet* getNearestBullet(std::list<Bullet>& bullets);
	glm::vec2 getAvoidVector(const glm::vec2& obj);

public:
	// constructor
	AutoTurret() : ai(), health(0) {}

	// member functions
	void respawn(int mapHalfSize);
	bool collide(const glm::vec2& obj, float r);
	void update(int mapHalfSize, std::vector<AutoTurret>& turrets, std::list<Bullet>& bullets);

	// inline functions
	inline float getThetaBase() { return thetaBase; }
	inline float getThetaBody() { return thetaBody; }
	inline float getThetaRader() { return thetaRader; }
	inline int getHealth() { return health; }
	inline void addHealth(int hp) { health += hp; }
	inline bool isAlive() { return (health > 0 ? true : false); }
	inline glm::vec2 getPosition() { return position; }
	inline glm::vec2 getDirection() { return direction; }
	inline TurretAI& getAI() { return ai; }
};


struct Explosion
{
	AutoTurret* turret;
	int steps;

	const static int maxExplosionSteps;
};