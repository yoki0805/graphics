#include "Battlefield.h"


void Battlefield::setInfo(int mapHalfSize, int numTurrets)
{
	this->mapHalfSize = mapHalfSize;
	this->numTurrets = numTurrets;

	// create the turrets
	for (int i = 0; i < numTurrets; ++i) {
		turrets.push_back(AutoTurret());
		turrets[i].respawn(mapHalfSize);
		// check collision
		for (int j = i-1; j >= 0; --j)
		{
			if (turrets[i].collide(turrets[j].getPosition(), TurretInfo::halfSize)) {
				turrets[i].respawn(mapHalfSize);
				j = i;
			}
		}
	}

	// set up the genetic algorithm
	pool.create(numTurrets, 0.1, 0.7, turrets[0].getAI().getNeuralNet().getNumberOfWeights());

	// set up the display lists
	static int firstTime = 1;
	if (firstTime) {
		SetBaseDisplayList();
		SetTurretDisplayList();
		SetBulletDisplayList();
		firstTime = 0;
	}
}

void Battlefield::drawBullets()
{
	// bullets
	std::list<Bullet>::iterator it;
	for (it = bullets.begin(); it != bullets.end(); ++it) {
		if (it->valid == false)
			continue;
		glPushMatrix();
		glTranslatef(it->position[0], TurretInfo::halfSize, it->position[1]);
		DrawBullet(Bullet::halfSize);
		glPopMatrix();
	}
}

void Battlefield::drawExplosions()
{
	// explosions
	std::list<Explosion>::iterator eit;
	for (eit = booms.begin(); eit != booms.end(); ++eit) {
		if (eit->steps > Explosion::maxExplosionSteps) {
			continue;
		}
		
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		glEnable(GL_NORMALIZE);
		glEnable(GL_BLEND);
		glm::vec2 pos = eit->turret->getPosition();
		float s = 7 * TurretInfo::halfSize * ((float)eit->steps / Explosion::maxExplosionSteps);
		float a = 1 - ((float)eit->steps / Explosion::maxExplosionSteps);
		glTranslatef(pos[0], 0, pos[1]);
		glColor4f(1, 0.2, 0.2, a);
		DrawTurretShadow(TurretInfo::halfSize, eit->turret->getThetaBase(), eit->turret->getThetaBody(), eit->turret->getThetaRader());
		glTranslatef(0, TurretInfo::halfSize, 0); 
		glRotatef(90, -1, 0, 0);
		glColor4f(1, 1, 0, a);
		glPushMatrix();
		glScalef(s, s, 2);
		glutSolidTorus(0.05, 0.45, 8, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, 0, -TurretInfo::halfSize * 0.7);
		glScalef(s, s, s);
		glutSolidCylinder(0.09, 0.25, 16, 8);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, 0, 2.5 * TurretInfo::halfSize * (-a + 1));
		glScalef(s, s, s);
		glutSolidSphere(0.25, 16, 8);
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_NORMALIZE);
		glPopMatrix();
		glPopAttrib();
	}
}

void Battlefield::drawTurrets()
{
	// turrets
	for (int i = 0; i < numTurrets; ++i) {
		// pass the dead turrets
		if (!turrets[i].isAlive())
			continue;
		// rendering
		glm::vec2 pos = turrets[i].getPosition();
		glPushMatrix();
		glTranslatef(pos[0], 0, pos[1]);
		DrawTurret(TurretInfo::halfSize, turrets[i].getThetaBase(),
			turrets[i].getThetaBody(), turrets[i].getThetaRader());
		glPopMatrix();
	}
}

void Battlefield::drawBulletsShadow()
{
	// bullets
	std::list<Bullet>::iterator it;
	for (it = bullets.begin(); it != bullets.end(); ++it) {
		if (it->valid == false)
			continue;
		glPushMatrix();
		glTranslatef(it->position[0], TurretInfo::halfSize, it->position[1]);
		DrawBulletShadow(Bullet::halfSize);
		glPopMatrix();
	}
}

void Battlefield::drawTurretsShadow()
{
	// turrets
	for (int i = 0; i < numTurrets; ++i) {
		// pass the dead turrets
		if (!turrets[i].isAlive())
			continue;
		// rendering
		glm::vec2 pos = turrets[i].getPosition();
		glPushMatrix();
		glTranslatef(pos[0], 0, pos[1]);
		DrawTurretShadow(TurretInfo::halfSize, turrets[i].getThetaBase(),
			turrets[i].getThetaBody(), turrets[i].getThetaRader());
		glPopMatrix();
	}
}

void Battlefield::drawLabels()
{
	// best genome ever
	if (turrets[0].isAlive()) {
		glm::vec2 pos = turrets[0].getPosition();
		glColor4f(1, 1, 0, 1);
		glPushMatrix();
		glTranslatef(pos[0], (TurretInfo::halfSize * 2) + 3, pos[1]);
		glRotatef(90, 1, 0, 0);
		glutSolidCone(1, 2, 4, 4);
		glPopMatrix();
	}

	// best genome in the previous population
	if (turrets[1].isAlive()) {
		glm::vec2 pos = turrets[1].getPosition();
		glColor4f(0, 1, 0, 1);
		glPushMatrix();
		glTranslatef(pos[0], (TurretInfo::halfSize * 2) + 3, pos[1]);
		glRotatef(90, 1, 0, 0);
		glutSolidCone(1, 2, 4, 4);
		glPopMatrix();
	}
}

void Battlefield::drawScene()
{
	GLfloat lightPos[] = { -50, 50, -50, 1.0 };
	SetLighting(lightPos);

	glm::mat4 shadowMatrix;
	glm::vec4 lpos = glm::vec4(lightPos[0], lightPos[1], lightPos[2], lightPos[3]);
	SetShadowMatrix(shadowMatrix, lpos, glm::vec4(0, 1, 0, 0));

	// attribs of drawScene
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// get stencil value
	glColorMask(0, 0, 0, 0);    // don't write to rgb buffer
	glDepthMask(0);    // close depth test
	glEnable(GL_STENCIL_TEST);    // opengl stencil test
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	// stencil floor
	glPushMatrix();
	glBegin(GL_QUADS);
	glVertex3i(-mapHalfSize, 0, -mapHalfSize); glVertex3i(-mapHalfSize, 0, mapHalfSize);
	glVertex3i(mapHalfSize, 0, mapHalfSize); glVertex3i(mapHalfSize, 0, -mapHalfSize);
	glEnd();
	glPopMatrix();
	glColorMask(1, 1, 1, 1);
	glDepthMask(1);

	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// draw reflection
	glPushMatrix();
	glScalef(1, -1, 1);    // upside down
	glFrontFace(GL_CW);    // need to change front face 
	glEnable(GL_LIGHTING);

	drawBullets();
	drawTurrets();
	drawExplosions();

	glFrontFace(GL_CCW);
	glPopMatrix();
	glDisable(GL_STENCIL_TEST);

	// blending the reflection & floor
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);    // for lighting
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	DrawBase(mapHalfSize);
	glPopAttrib();

	// draw the shadow
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	glPushMatrix();
	glMultMatrixf(glm::value_ptr(shadowMatrix));    // using the shadow matrix

	glColor4f(0, 0, 0, 0.5);
	drawBulletsShadow();
	drawTurretsShadow();

	glPopMatrix();
	glPopAttrib();
	glDisable(GL_STENCIL_TEST);

	drawBullets();
	drawTurrets();
	drawExplosions();
	drawLabels();

	// draw fonts for GA
	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	fonts.printf(1, 0, "Generation: %d", pool.getGeneration());
	fonts.printf(1, 1, "Average Fitness: %.1f", pool.getAverageFitness());
	fonts.printf(1, 2, "Best Fitness (Green): %.0f", pool.getBestFitness());
	fonts.printf(1, 3, "Best Fitness Ever (Yellow): %.0f", pool.getBestFitnessEver());
	glPopMatrix();
	
	// end of drawScene
	glPopAttrib();
}

void Battlefield::update()
{
	static int steps = 0;

	// the new epoch
	if (steps >= 1800)
	{
		// get all chromos from turrets
		vector<Genome> oldPop;
		oldPop.reserve(numTurrets);
		for (int i = 0; i < numTurrets; ++i) {
			TurretAI& temp = turrets[i].getAI();
			oldPop.push_back(Genome(temp.getNeuralNet().getWeights(),
				temp.getFitness()));
		}

		// GA looping
		pool.epoch(oldPop);

		// feedback to the turrets
		vector<Genome>& newPop = pool.getChromos();
		for (int i = 0; i < numTurrets; ++i) {
			TurretAI& temp = turrets[i].getAI();
			temp.getNeuralNet().setWeights(newPop[i].weights);
			temp.setFitness(0);

			// need to respwan
			turrets[i].respawn(mapHalfSize);
			// check collision
			for (int j = i - 1; j >= 0; --j)
			{
				if (turrets[i].collide(turrets[j].getPosition(), TurretInfo::halfSize)) {
					turrets[i].respawn(mapHalfSize);
					j = i;
				}
			}
		}
		bullets.clear();
		booms.clear();
		steps = 0;
	}
	steps++;

	// bullets
	std::list<Bullet>::iterator it;
	for (it = bullets.begin(); it != bullets.end(); ++it) {
		if (it->valid == false) {
			it = bullets.erase(it);
			continue;
		}
		it->update(mapHalfSize, turrets, booms);
	}

	// explosions
	std::list<Explosion>::iterator eit;
	for (eit = booms.begin(); eit != booms.end(); ++eit) {
		if (eit->steps > Explosion::maxExplosionSteps) {
			eit = booms.erase(eit);
			continue;
		}
		eit->steps++;
	}

	// turrets
	for (int i = 0; i < numTurrets; ++i) {
		// pass the dead turrets
		if (!turrets[i].isAlive()) {
			continue;
		}
		// update turret
		turrets[i].update(mapHalfSize, turrets, bullets);
	}
}