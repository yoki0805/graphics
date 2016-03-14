#include "AutoTurret.h"


// turret info
const float TurretInfo::halfSize = 2.5f;
const float TurretInfo::maxSpeed = 0.4f;
const float TurretInfo::maxBaseRotSpeed = 4.0f;
const float TurretInfo::maxBodyRotSpeed = 8.0f;
const float TurretInfo::maxRaderRotSpeed = 12.0f;
const int TurretInfo::maxHP = 100;
const int TurretInfo::maxReloadSteps = 45;


// turret AI
const int TurretAI::numInputs = 17;
const int TurretAI::numOutputs = 5;
const int TurretAI::numHiddenLayers = 1;
const int TurretAI::numNeuronsPerHiddenLayer = 6;


// bullet
const float Bullet::halfSize = 0.3f;
const float Bullet::speed = 3.0f * TurretInfo::maxSpeed;

const int Explosion::maxExplosionSteps = 45;

void Bullet::init(const glm::vec2& pos, float angle, AutoTurret* turretPtr)
{
	position = pos;
	theta = angle;
	belong = turretPtr;
	valid = true;

	// get direction
	glm::vec3 temp = glm::rotateY(glm::vec3(0, 0, 1), theta);
	direction = glm::vec2(temp.x, temp.z);
}

void Bullet::update(int mapHalfSize, std::vector<AutoTurret>& turrets, std::list<Explosion>& booms)
{
	// check collision
	for (int i = 0; i < turrets.size(); ++i)
	{
		if (!turrets[i].isAlive() || valid == false)
			continue;
		if (&turrets[i] == belong)
			continue;
		if (turrets[i].collide(position, Bullet::halfSize))
		{
			turrets[i].addHealth(-RandInt(20, 30));
			turrets[i].getAI().addFitness(-50);
			belong->getAI().addFitness(25);
			valid = false;

			// create the explosion
			if (!turrets[i].isAlive()) {
				Explosion e;
				e.turret = &turrets[i];
				e.steps = 0;
				booms.push_back(e);
			}
		}

		// make fitness > 0
		if (turrets[i].getAI().getFitness() < 0)
			turrets[i].getAI().setFitness(0);
	}

	if (valid == true)
		position += Bullet::speed * direction;

	// bounds
	static float left = (float)-mapHalfSize - Bullet::halfSize;
	static float right = (float)mapHalfSize + Bullet::halfSize;
	static float top = (float)-mapHalfSize - Bullet::halfSize;
	static float bottom = (float)mapHalfSize + Bullet::halfSize;

	// check position
	if (position[0] < left) valid = false;
	if (position[0] > right) valid = false;
	if (position[1] < top) valid = false;
	if (position[1] > bottom) valid = false;
}


float AutoTurret::angleClamped(float theta)
{
	theta = fmod(theta, 360.f);
	return theta >= 0 ? theta : (theta + 360);
}

AutoTurret* AutoTurret::getNearestTurret(std::vector<AutoTurret>& turrets)
{
	AutoTurret* near = NULL;
	float far = 9999999;

	for (int i = 0; i < turrets.size(); ++i)
	{
		if (!turrets[i].isAlive() || &turrets[i] == this)
			continue;

		glm::vec2 dir = turrets[i].getPosition() - position;
		float dotLen = glm::dot(dir, dir);
		if (dotLen < far) {
			near = &turrets[i];
			far = dotLen;
		}
	}

	return near;
}

Bullet* AutoTurret::getNearestBullet(std::list<Bullet>& bullets)
{
	Bullet* near = NULL;
	float far = 9999999;

	std::list<Bullet>::iterator it;
	for (it = bullets.begin(); it != bullets.end(); ++it) {
		if (it->valid == false)
			continue;

		glm::vec2 dir = it->position - position;
		float dotLen = glm::dot(dir, dir);
		if (dotLen < far) {
			near = &(*it);
			far = dotLen;
		}
	}

	return near;
}

glm::vec2 AutoTurret::getAvoidVector(const glm::vec2& obj)
{
	glm::vec2 dir = position - obj;
	return glm::normalize(dir);
}

void AutoTurret::respawn(int mapHalfSize)
{
	thetaBase = (float)RandFloat() * 360;
	thetaBody = (float)RandFloat() * 360;
	thetaRader = (float)RandFloat() * 360;

	reloading = RandInt(0, TurretInfo::maxReloadSteps);
	health = TurretInfo::maxHP;

	position[0] = (float)RandomClamped() * (mapHalfSize - TurretInfo::halfSize);
	position[1] = (float)RandomClamped() * (mapHalfSize - TurretInfo::halfSize);

	glm::vec3 tempVec = glm::rotateY(glm::vec3(0, 0, 1), thetaBase);
	direction = glm::vec2(tempVec.x, tempVec.z);
}

bool AutoTurret::collide(const glm::vec2& obj, float r)
{
	float dotLen = (r + TurretInfo::halfSize) * (r + TurretInfo::halfSize);
	glm::vec2 dir = position - obj;
	return (glm::dot(dir, dir) <= dotLen);
}

void AutoTurret::update(int mapHalfSize, std::vector<AutoTurret>& turrets, std::list<Bullet>& bullets)
{
	std::vector<double> inputs, outputs;

	AutoTurret* nearTurret = getNearestTurret(turrets);
	Bullet* nearBullet = getNearestBullet(bullets);

	// get the target vector & the nearest turret forward vector
	float targetLen;
	glm::vec2 target, targetVec;
	if (nearTurret == NULL) {
		target = glm::vec2(0);
		targetVec = glm::vec2(0);
		targetLen = 0;
	}
	else {
		target = -getAvoidVector(nearTurret->getPosition());
		targetVec = nearTurret->getDirection();
		targetLen = glm::length(nearTurret->getPosition() - position);
	}

	// get the danger vector & the nearest bullet forward vector
	float dangerLen;
	glm::vec2 danger, dangerVec;
	if (nearBullet == NULL) {
		danger = glm::vec2(0);
		dangerVec = glm::vec2(0);
		dangerLen = 0;
	}
	else {
		danger = -getAvoidVector(nearBullet->position);
		dangerVec = nearBullet->direction;
		dangerLen = glm::length(nearBullet->position - direction);
	}
	
	// get the self moving vector
	glm::vec3 tempVec = glm::rotateY(glm::vec3(0, 0, 1), thetaBase);
	glm::vec2 baseVec = glm::vec2(tempVec.x, tempVec.z);
	direction = baseVec;

	//get the self turret vector
	tempVec = glm::rotateY(glm::vec3(0, 0, 1), thetaBody);
	glm::vec2 BodyVec = glm::vec2(tempVec.x, tempVec.z);

	// puts the inputs
	inputs.reserve(TurretAI::numInputs);
	inputs.push_back(target[0]);
	inputs.push_back(target[1]);
	inputs.push_back(targetVec[0]);
	inputs.push_back(targetVec[1]);
	inputs.push_back(targetLen / TurretInfo::halfSize * 2);
	inputs.push_back(danger[0]);
	inputs.push_back(danger[1]);
	inputs.push_back(dangerVec[0]);
	inputs.push_back(dangerVec[1]);
	inputs.push_back(dangerLen / TurretInfo::halfSize * 2);
	inputs.push_back(baseVec[0]);
	inputs.push_back(baseVec[1]);
	inputs.push_back(BodyVec[0]);
	inputs.push_back(BodyVec[1]);
	inputs.push_back(position[0] / mapHalfSize);
	inputs.push_back(position[1] / mapHalfSize);
	inputs.push_back((float)reloading / TurretInfo::maxReloadSteps);

	// get the outputs
	outputs = ai.getNeuralNet().getOutputs(inputs);

	double speed = (outputs[0] + outputs[1]) * TurretInfo::maxSpeed;
	double rotBaseForce = (outputs[0] - outputs[1]) * TurretInfo::maxBaseRotSpeed;
	double rotBodyForce = (outputs[2] - outputs[3]) * TurretInfo::maxBodyRotSpeed;
	double shooting = outputs[4];

	thetaBase += (float)rotBaseForce;
	thetaBody += (float)rotBodyForce;
	thetaRader += TurretInfo::maxRaderRotSpeed;

	// corret theta in 0 to 360
	thetaBase = angleClamped(thetaBase);
	thetaBody = angleClamped(thetaBody);
	thetaRader = angleClamped(thetaRader);

	// shooting
	if (shooting > 0.5 && reloading == TurretInfo::maxReloadSteps) {
		// fitness of aim
		float aim = glm::dot(BodyVec, target);
		if (aim > 0.996f) { // -5 ~ 5
			if (aim > 0.999)  // -2 ~ 2
				ai.addFitness(75);
			else
				ai.addFitness(25);

			Bullet b;
			glm::vec2 bpos = position + TurretInfo::halfSize * BodyVec;
			b.init(bpos, thetaBody, this);
			bullets.push_back(b);
			reloading = 0;
		}
	}

	// controll the Body's rotation
	ai.addFitness(-fabs(rotBodyForce / TurretInfo::maxBodyRotSpeed));

	// fitness > 0
	if (ai.getFitness() < 0)
		ai.setFitness(0);

	// reloading
	if (reloading < TurretInfo::maxReloadSteps)
		reloading++;

	// move turret
	position += (float)speed * baseVec;

	// collision avoidance
	if (nearTurret != NULL && collide(nearTurret->getPosition(), TurretInfo::halfSize)) {
		// move back and avoidance
		position -= (float)speed * baseVec;
		position += 0.1f * getAvoidVector(nearTurret->getPosition());
	}
	
	// bounds
	static float left = (float)-mapHalfSize + TurretInfo::halfSize;
	static float right = (float)mapHalfSize - TurretInfo::halfSize;
	static float top = (float)-mapHalfSize + TurretInfo::halfSize;
	static float bottom = (float)mapHalfSize - TurretInfo::halfSize;

	// check position
	if (position[0] < left) position[0] = left;
	if (position[0] > right) position[0] = right;
	if (position[1] < top) position[1] = top;
	if (position[1] > bottom) position[1] = bottom;
}