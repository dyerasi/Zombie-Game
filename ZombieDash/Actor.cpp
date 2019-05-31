
#include "Actor.h"
#include "StudentWorld.h"
#include <string>
#include <iostream>
using namespace std;



void Exit::activateIfAppropriate(Actor* a) {
	if (a->usesExit() && a->canBeInfected()) {
		if (!(a->triggersCitizen())) { //if it is a citizen
			world()->increaseScore(1500); //add 1500 instead of 500 to counter 1000 lost from setting citizen to dead
			static_cast<Citizen*>(a)->setDead();
			world()->playSound(SOUND_CITIZEN_SAVED);
		}
		else if (a->triggersCitizen() && world()->citizensLeft() == 0) { //is penelope and no citizens left to save
			world()->finishLevel(); //let studentworld know level has been finsished
		}
	}
}
void Vomit::activateIfAppropriate(Actor* a) {
	if (a->canBeInfected() && a->blocksMovement()){ //if its a human
		if (!static_cast<Human*>(a)->isInfected()) { //who hasnt been infected before
			static_cast<Human*>(a)->incrementInfectionCount();
			if (a != world()->getMC())//if its a citizen
				world()->playSound(SOUND_CITIZEN_INFECTED);
		}
	}	
}

void Pit::activateIfAppropriate(Actor* a) {
	if (a->canFallOrBurn())
		a->setDead();
}
void TimedObject::doSomething() {
	if (isDead())
		return;
	if(updateLifeSpan()) //if the lifespan is now <= 0
		setDead();
}
bool TimedObject::updateLifeSpan() {
	if (m_lifespan <= 0) {
		return true; //true indicated this object has died
	}
	m_lifespan--;
	return false;
}
void Landmine::doSomething() {
	if (isDead())
		return;
	if (updateLifeSpan()) {
		m_active = true;
		return;
	}
}
void Landmine::activateIfAppropriate(Actor* a) {
	if (a->triggersLandmines() && a->blocksMovement() && m_active) {//if it is a citizen, penelope,or zombie, and the landmine is active
		this->setDead();
	}
	else if (a->triggersLandmines() && !a->blocksMovement()) { //if its a flame
		this->setDead();
	}
}
void Landmine::setDead() {
	Actor::setDead();
	world()->playSound(SOUND_LANDMINE_EXPLODE);
	double x = getX();
	double y = getY();
	world()->addActor(new Flame(world(), x, y, up));
	world()->addActor(new Flame(world(), x + SPRITE_WIDTH, y, up)); //Flame 1
	world()->addActor(new Flame(world(), x, y + SPRITE_HEIGHT, up)); //Flame 2
	world()->addActor(new Flame(world(), x + SPRITE_WIDTH, y + SPRITE_HEIGHT, up)); //Flame 3
	world()->addActor(new Flame(world(), x + SPRITE_WIDTH, y - SPRITE_HEIGHT, up)); //Flame 4
	world()->addActor(new Flame(world(), x, y - SPRITE_HEIGHT, up)); //Flame 5
	world()->addActor(new Flame(world(), x - SPRITE_WIDTH, y - SPRITE_HEIGHT, up)); //Flame 6
	world()->addActor(new Flame(world(), x - SPRITE_WIDTH, y, up)); //Flame 7
	world()->addActor(new Flame(world(), x - SPRITE_WIDTH, y + SPRITE_HEIGHT, up)); //Flame 8
	world()->addActor(new Pit(world(), x, y)); //pit
}


//Humans(human, penelope, citizen)
void Citizen::setDead() {
	Actor::setDead();
	world()->increaseScore(-1000); //setting citizen to dead automatically loses 1000 points
	world()->decreasecitizens();
}

//check to make sure no intersection with other blockable objects before updating with moveTo(...)
void Penelope::move(int dir) {
	switch (dir)
	{
	case(KEY_PRESS_RIGHT):
		setDirection(right);
		if(world()->noBlockingOfMovement(getX()+4, getY(), this))
			moveTo(getX() + 4, getY());
		break;
	case(KEY_PRESS_LEFT):
		setDirection(left);
		if (world()->noBlockingOfMovement(getX() - 4, getY(), this))
			moveTo(getX() - 4, getY());
		break;
	case(KEY_PRESS_UP):
		setDirection(up);
		if (world()->noBlockingOfMovement(getX(), getY() + 4, this))
			moveTo(getX(), getY() + 4);
		break;
	case(KEY_PRESS_DOWN):
		setDirection(down);
		if (world()->noBlockingOfMovement(getX(), getY() - 4, this))
			moveTo(getX(), getY() - 4);
		break;
	}
}
bool Human::incrementInfectionCount() {
	m_infectionCount++;
	if (m_infectionCount >= 500) {
		setDead();
		return true;
	}
	return false;
}


void Penelope::doSomething() {
	if (isDead())
		return;
	if(isInfected())
		if (incrementInfectionCount()) {
			return;
		}
	//implement flaethrowe, landmine, vaccine later!!!!!!! pg 27-28!!!
	int key;
	if (world()->getKey(key)) {
		switch (key)
		{
		case KEY_PRESS_ENTER: //use vaccine
			useVaccine();
			break;
		case KEY_PRESS_SPACE://flamethrower
			useFlameThrower();
			break;
		case KEY_PRESS_TAB://landmines
			useLandMine();
			break;
		case KEY_PRESS_DOWN:
		case KEY_PRESS_UP:
		case KEY_PRESS_LEFT:
		case KEY_PRESS_RIGHT:
			move(key);
			break;
		}
	}

}
void Penelope::useFlameThrower() {
	if (m_flameCharges <= 0)
		return;
	m_flameCharges--;
	world()->playSound(SOUND_PLAYER_FIRE);
	double x;
	double y;
	int dir = getDirection();
	for (int i = 1; i < 4; i++) {
		 x = getX();
		 y = getY();
		switch (dir) {
		case(right):
			x = x + (i * SPRITE_WIDTH);
			break;
		case(up):
			y = y + (i * SPRITE_HEIGHT);
			break;
		case(left):
			x = x - (i * SPRITE_HEIGHT);
			break;
		case(down):
			y = y - (i * SPRITE_HEIGHT);
			break;
		}
		if (world()->noBlockingOfMovement(x, y, nullptr)) { //passing in nullptr checks for the creation of flames at given coords
			world()->addActor(new Flame(world(), x, y, dir));
		}
		else {
			break; //hit a wall or exit so must stop loop
		}
	}
}
void Penelope::useLandMine() {
	if (m_landmines <= 0)
		return;
	m_landmines--;
	world()->addActor(new Landmine(world(), getX(), getY()));
}
bool Citizen::incrementInfectionCount() {
	if (Human::incrementInfectionCount()) { //if its now dead
		world()->playSound(SOUND_ZOMBIE_BORN);
		world()->addZombie(getX(), getY()); 
		return true;
	}
	return false; //citizen has not died yet
}

void Citizen::doSomething() {
	if (isDead())
		return;
	
	Agent::doSomething(); //keep track of even tick
	if (evenTick())
		return; //citizen is paralyzed every other tick and does nothing


	if (isInfected()) 
		if (incrementInfectionCount()) //increment infection count and returns immediately if citizen is dead
			return;
	//check where if anywhere citizen should move 
	int pixels = 2; //the number of pixels citizen moves if it can
	double dist_p = world()->distanceToMC(this);
	double dist_z = world()->distanceToNearestZ(getX(), getY()); //-1 if no zombies left
	//check movement towards penelope
	if ((dist_p < dist_z || dist_z == -1) && dist_p <= 80) { //if its closer to penelope than a zombie and is withing 80 pixels of the main character
		if (world()->checkMovementTowards(this, world()->getMC(), pixels)) {
			return; //citizen has moved 2 pixels closer to mc, and shouldnt do anything else
		}
	}
	//check movement away from zombies
	else if (dist_z <= 80) { //if true, citizen will try and move away from zombie unless it brings him closer to other zombies
		bool direction[4] = { false }; //direction[] contains bool values for each direction that determines if citizen can move there //(index *90) = dir: 0, 90, 180, 270 (right, up, left, down)
		double distanceToZ[4] = { -1 }; //companion array to direction[] filled with distance to nearest zombie given each possible directional move. -1 represents an invalid direction
		for (int i = 0; i != 4; i++) {
			double x = getX();
			double y = getY();
			double coords[] = { x, y };
			world()->findNewCoords(coords, (i * 90), pixels, pixels);
			x = coords[0];
			y = coords[1];
			if (world()->noBlockingOfMovement(x, y, this)) { //if this was a valid directional movement
				direction[i] = true; //indicate it is a valid direction
				distanceToZ[i] = world()->distanceToNearestZ(x, y); //find distance to nearest zombie at possible new location
			}
		}
		int chosenDir = -1; //represents index of best move for citizen, -1 if best move is to do nothing
		for (int i = 0; i != 4; i++) {
			if (direction[i] && distanceToZ[i] > dist_z) { //if its a valid direction and the distance to the closest zombie there is more than old spot
				dist_z = distanceToZ[i]; //mark the new distance to the closest zombie as the result of moving towards i
				chosenDir = i;
			}
		}
		if (chosenDir == -1) // if no better direction found, return and dont move
			return;
		setDirection((chosenDir * 90)); 
		double coords[2] = { getX(), getY() };
		world()->findNewCoords(coords, chosenDir * 90, pixels, pixels);
		moveTo(coords[0], coords[1]);
		return;
	}
	//if doSomething() reaches this point, the citizen does nothing
	
	return; //this is just here for aesthetic purposes and sleep deprivation :)
}






//ZOMBIES
void Zombie::setDead() {
	Actor::setDead();
	world()->increaseScore(1000);
	world()->playSound(SOUND_ZOMBIE_DIE);
}

void DumbZombie::setDead() {
	Zombie::setDead();
	if (randInt(1, 10) == 1) { //this zombie will try and fling a vaccine
		int dir = randInt(0, 3) * 90;
		int pixels;
		if ((dir / 90) % 2 == 0) //0(right) or 180(left)
			pixels = SPRITE_WIDTH;
		else //up or down
			pixels = SPRITE_HEIGHT;
		double coords[] = { getX(), getY() };
		world()->findNewCoords(coords, dir, pixels, pixels);
		if (!world()->checkForOverlaps(coords[0], coords[1], this)) //if no overlaps, add a vaccine goodie there
			world()->addActor(new VaccineGoodie(world(), coords[0], coords[1]));
	}

}

void SmartZombie::setDead() {
	Zombie::setDead(); //increases score by 1000 and sets to dead
	world()->increaseScore(1000); //and by another 1000
}

void Zombie::doSomething() {
	if (isDead())
		return;
	Agent::doSomething(); //toggles bool value m_evenTick; true = even
	if (evenTick()) //should do nothing every even tick
		return;
	m_return = false;
	if (world()->zombieTryAndThrowUp(this)) {
		m_return = true; //if the zombie throws up, it shouldnt do anything else this tick but return
		return;
	}
	if (m_movementPlanDistance <= 0) {
		m_movementPlanDistance = randInt(3, 10);
	}
	//here is where dumbZombie and smartZombie do different things that will be implemented in their own doSomething()
}

void DumbZombie::doSomething() {
	if (isDead())
		return;
	Zombie::doSomething();
	if (evenTick() || shouldReturn()) //if its and even tick or zombie's do something caused zombie to vomit
		return;
	setDirection(90 * randInt(0, 3)); //0=right, 90 = up, 180 = left, 270 = down;
	double x1;
	double y1;
	double arr[2] = { getX(), getY() };
	world()->findNewCoords(arr, getDirection(), 1, 1);
	x1 = arr[0];
	y1 = arr[1];
	if (world()->noBlockingOfMovement(x1, y1, this)) {
		moveTo(x1, y1);
		decrementMovementPlanDistance();
	}
	else //if it couldnt make a move
		resetMovementPlanDistance(0);


}

void SmartZombie::doSomething() {
	if (isDead())
		return;
	Zombie::doSomething(); //toggles m_evenTick 
	if (evenTick() || shouldReturn()) //if its an even tick zombie does nothing
		return;
	double x, y, d;
	double &dist = d;
	double &x2 = x;
	double &y2 = y;
	double xnew;
	double ynew;
	if (!world()->locateNearestVomitTrigger(getX(), getY(), x2, y2, d))
		return; //mc is dead and level must end asap
	if (dist <= 80) {
		if (getX() == x2 || getY() == y2) {
			if (getX() == x2) {
				if (getY() > y2) {
					setDirection(down);
				}
				else //y1 < y2
					setDirection(up);
		
			}
			else { //y1 == y2
				if (getX() > x2)
					setDirection(left);
				else
					setDirection(right);
			}
		}
		else {
			int verticalOrHorizontal = randInt(0, 1);
			if (verticalOrHorizontal == 1) { //choose horizontal
				if (getX() > x2)
					setDirection(left);
				else //x1 < x2
					setDirection(right);
			}
			else { //verticalOrHorizontal == 0; choose vertical
				if (getY() > y2)
					setDirection(down);
				else //y1 < y2
					setDirection(up);
			}

		}
	}
	else
		setDirection(90 * randInt(0, 3)); //not close to a human to choose a random direction
	
	int pixels = 1;
	 xnew = -1;
	 ynew = -1;
	switch (getDirection()) {
		xnew = getX();
		ynew = getY();
	case(0):
		xnew += pixels;
		break;
	case(90):
		ynew += pixels;
		break;
	case(180):
		xnew -= pixels;
		break;
	case(270):
		ynew -= pixels;
		break;
	}
	if (world()->noBlockingOfMovement(xnew, ynew, this)) {
		moveTo(xnew, ynew);
		decrementMovementPlanDistance();
	}
	else //smart zombie couldnt move as it wouldve been blocked
		resetMovementPlanDistance(0);
}


//GOODIES

void Goodie::activateIfAppropriate(Actor* a) { //doSomething used for all goodies, then most derived version of PickUp(Penelope* p) called
	if (isDead())
		return;
	if(!world()->getMC()->isDead()) //if mc isnt dead
		pickUp(world()->getMC());
}
void Goodie::pickUp(Penelope* p) {
	if (world()->overlap(p->getX(), p->getY(), this->getX(), this->getY())) { //if penelope over laps with this goodie
		world()->increaseScore(50);
		this->setDead(); //make this goodie disapear
		world()->playSound(SOUND_GOT_GOODIE);
	}
}

void VaccineGoodie::pickUp(Penelope* p) {
	Goodie::pickUp(p);
	if(isDead())
		p->increaseVaccines();
}

void GasCanGoodie::pickUp(Penelope* p) {
	Goodie::pickUp(p);
	if(isDead())
		p->increaseFlameCharges();
}

void LandmineGoodie::pickUp(Penelope* p) {
	Goodie::pickUp(p);
	if(isDead())
		p->increaseLandmines();
}

