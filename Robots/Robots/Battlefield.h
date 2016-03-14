#pragma once
#include <list>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/freeglut.h>
#include "GeneticAlgorithm.h"
#include "SceneRendering.h"
#include "AutoTurret.h"
#include "Font.h"


class Battlefield
{
private:
	int mapHalfSize;
	int numTurrets;
	std::vector<AutoTurret> turrets;
	std::list<Explosion> booms;
	std::list<Bullet> bullets;
	GeneticAlgo pool;
	Font fonts;

	void drawBullets();
	void drawTurrets();
	void drawExplosions();
	void drawBulletsShadow();
	void drawTurretsShadow();
	void drawLabels();

public:
	void setInfo(int mapHalfSize, int numTurrets);
	void drawScene();
	void update();
};