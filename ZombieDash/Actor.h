#ifndef ACTOR_INCLUDED
#define ACTOR_INCLUDED
#include "GraphObject.h"
#include <iostream>
using namespace std;


class StudentWorld;

class Actor : public GraphObject
{
public:
	Actor(StudentWorld* w, int imageID, double x, double y, int dir, int depth, bool usesExit, bool canDieFallBurn, bool canBeInfected, bool collectGoodie, bool blocksMovement, bool blocksFlame,
		bool triggersL, bool triggersZV,bool threatensC, bool triggersC)
		:GraphObject(imageID, x, y, dir, depth), m_isDead(false), m_world(w), m_canUseExit(usesExit), m_canDieByFallOrBurn(canDieFallBurn), m_canBeInfected(canBeInfected),
		m_canPickUpGoodie(collectGoodie), m_blocksMovement(blocksMovement), m_blocksFlame(blocksFlame), m_triggersLandmine(triggersL), m_triggersZombieVomit(triggersZV),
		m_threatensCitizen(threatensC), m_triggersCitizen(triggersC){}

	// Action to perform for each tick.
	virtual void doSomething() = 0;


	// Is this actor dead?
	bool isDead() const { return m_isDead; }

	// Mark this actor as dead.
	virtual void setDead() { m_isDead = true; }

	// Get this actor's world
	StudentWorld* world() const { return m_world; }

	// If this is an activated object, perform its effect on a (e.g., for an
	// Exit have a use the exit).
	virtual void activateIfAppropriate(Actor* a) { return; } //make pure virtual and implement for activiating objects
	/*
	// If this object uses exits, use the exit.
	virtual void useExitIfAppropriate();

	// If this object can die by falling into a pit or burning, die.
	virtual void dieByFallOrBurnIfAppropriate();

	// If this object can be infected by vomit, get infected.
	virtual void beVomitedOnIfAppropriate();

	// If this object can pick up goodies, pick up g
	virtual void pickUpGoodieIfAppropriate(Goodie* g);
	*/
	// Does this object block agent movement?
	virtual bool blocksMovement() const { return m_blocksMovement; }
	
	// Does this object block flames?
	virtual bool blocksFlame() const { return m_blocksFlame; }

	// Does this object trigger landmines only when they're active?
	virtual bool triggersLandmines() const { return m_triggersLandmine; }

	// Can this object cause a zombie to vomit?
	virtual bool triggersZombieVomit() const { return m_triggersZombieVomit; }

	// Is this object a threat to citizens?
	virtual bool threatensCitizens() const { return m_threatensCitizen; }

	// Does this object trigger citizens to follow it or flee it?
	virtual bool triggersCitizens() const { return m_triggersCitizen; }

	virtual bool canFallOrBurn() { return m_canDieByFallOrBurn; }
	//virtual void fallOrBurn();

	bool usesExit() const { return m_canUseExit; }
	bool canBeInfected() const { return m_canBeInfected; }
	bool triggersCitizen() const { return m_triggersCitizen; }

private:
	bool m_isDead;
	bool m_canUseExit;
	bool m_canDieByFallOrBurn;
	bool m_canBeInfected;
	bool m_canPickUpGoodie;
	bool m_blocksMovement;
	bool m_blocksFlame;
	bool m_triggersLandmine;
	bool m_triggersZombieVomit;
	bool m_threatensCitizen;
	bool m_triggersCitizen;
	StudentWorld* m_world;
};



//WALL
class Wall : public Actor
{
public:
	Wall(StudentWorld* w, double x, double y) : Actor(w, IID_WALL, x, y, right, 0, false, false, false, false, true, true, false, false, false, false){}
	virtual void doSomething() { ; } //do walls do anything?
	//virtual bool blocksMovement() const;
	//virtual bool blocksFlame() const;
};




//ACTIVATING OBJECTS
class ActivatingObject : public Actor
{
public:
	ActivatingObject(StudentWorld* w, int imageID, double x, double y, int dir, int depth, bool usesExit, bool canDieFallBurn, bool canBeInfected, bool collectGoodie, bool blocksMovement, bool blocksFlame,
		bool triggersL, bool triggersZV, bool threatensC, bool triggersC): Actor( w, imageID, x,  y, dir,  depth, usesExit, canDieFallBurn, canBeInfected, collectGoodie, blocksMovement, blocksFlame,
		triggersL, triggersZV, threatensC, triggersC){}
private:
};

class Exit : public ActivatingObject
{
public:
	Exit(StudentWorld* w, double x, double y):ActivatingObject(w, IID_EXIT, x, y, right, 1, false, false, false, false, false, true, false, false, false, true){}
	virtual void doSomething() { return;  }
	
	virtual void activateIfAppropriate(Actor* a);
	/*
	virtual bool blocksFlame() const;
	*/
};
class Pit : public ActivatingObject
{
public:
	Pit(StudentWorld* w, double x, double y):ActivatingObject(w, IID_PIT, x, y, right, 0, false, false, false, false, false, false, false, false, false, true){}
	virtual void doSomething(){}
	virtual void activateIfAppropriate(Actor* a);
};




//TIMED OBJECTS
class TimedObject : public ActivatingObject
{
public:
	TimedObject(StudentWorld* w, int imageID, double x, double y, int dir, int depth, bool usesExit,  bool canDieFallBurn,bool triggersL, int lifespan)
		:ActivatingObject(w, imageID, x, y, dir, depth, usesExit, canDieFallBurn, false, false, false, false, triggersL, false, false, true), m_lifespan(lifespan){}
	virtual void doSomething();
	virtual bool updateLifeSpan(); //if it returns true, setDead()
	virtual void activateIfAppropriate(Actor* a) {
		if (a->canFallOrBurn())
			a->setDead();
	}

private:
	int m_lifespan;
};
class Flame : public TimedObject
{
public:
	Flame(StudentWorld* w, double x, double y, int dir): TimedObject(w, IID_FLAME, x, y, dir, 0, false, false, true, 2){}
	//virtual void doSomething(); //decrement lifespan
};

class Vomit : public TimedObject
{
public:
	Vomit(StudentWorld* w, double x, double y, int dir): TimedObject(w, IID_VOMIT, x, y, dir, 0, true, false, false, 2){} //in this case lifespan is ticks till active, not till it should be set dead
	//virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
};

class Landmine : public TimedObject
{
public:
	Landmine(StudentWorld* w, double x, double y) : TimedObject(w, IID_LANDMINE, x, y, 0, 1, false, true, false, 30), m_active(false){}
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual void setDead();
	//virtual void dieByFallOrBurnIfAppropriate();
private:
	bool m_active;
};



//GOODIES
class Penelope; //let goodies know their is a class called penelope that will b implemented later and it is okay to use pointers of its type //pickUp(Penelope* p);

class Goodie : public ActivatingObject
{
public:
	Goodie(StudentWorld* w, int imageID, double x, double y): ActivatingObject(w, imageID, x, y, right, 1, false, true, false, false, false, false, false, false, false, false){}
	virtual void activateIfAppropriate(Actor* a);
	//virtual void dieByFallOrBurnIfAppropriate();

	virtual void doSomething(){}
	virtual void pickUp(Penelope* p);
};

class VaccineGoodie : public Goodie
{
public:
	VaccineGoodie(StudentWorld* w, double x, double y):Goodie(w, IID_VACCINE_GOODIE, x, y){}
	virtual void pickUp(Penelope* p);
};

class GasCanGoodie : public Goodie
{
public:
	GasCanGoodie(StudentWorld* w, double x, double y):Goodie(w, IID_GAS_CAN_GOODIE, x, y){}
	virtual void pickUp(Penelope* p);
};

class LandmineGoodie : public Goodie
{
public:
	LandmineGoodie(StudentWorld* w, double x, double y):Goodie(w, IID_LANDMINE_GOODIE, x, y){}
	virtual void pickUp(Penelope* p);
};





//AGENT
class Agent : public Actor
{
public:
	Agent(StudentWorld* w, int imageID, double x, double y, int dir, int depth, bool usesExit, bool canDieFallBurn, bool canBeInfected, bool collectGoodie, bool blocksMovement, bool blocksFlame,
		bool triggersL, bool triggersZV, bool threatensC, bool triggersC) : Actor(w, imageID, x, y, dir, depth, usesExit, canDieFallBurn, canBeInfected, collectGoodie, blocksMovement, blocksFlame,
			triggersL, triggersZV, threatensC, triggersC), m_evenTick(false) {}
	/*
	virtual bool blocksMovement() const;
	virtual bool triggersOnlyActiveLandmines() const;
	*/
	virtual void doSomething() { m_evenTick = (!m_evenTick); } //most agents dont move every other tick
	virtual bool evenTick() { return m_evenTick; }
private:
	bool m_evenTick;

};


//HUMAN
class Human : public Agent
{
public:
	Human(StudentWorld* w, int imageID, double x, double y, int dir, int depth, bool usesExit, bool canDieFallBurn, bool canBeInfected, bool collectGoodie, bool blocksMovement, bool blocksFlame,
		bool triggersL, bool triggersZV, bool threatensC, bool triggersC) : Agent(w, imageID, x, y, dir, depth, usesExit, canDieFallBurn, canBeInfected, collectGoodie, blocksMovement, blocksFlame,
			triggersL, triggersZV, threatensC, triggersC), m_infectionCount(0){}
	/*
	virtual void beVomitedOnIfAppropriate();
	virtual bool triggersZombieVomit() const;
	*/
	virtual bool incrementInfectionCount();

	// Make this human uninfected by vomit.
	void clearInfection() { m_infectionCount = 0; }

	// How many ticks since this human was infected by vomit?
	//int getInfectionDuration() const;
	int getInfectionCount() { return m_infectionCount; }
	bool isInfected() { return m_infectionCount != 0; }
private:
	int m_infectionCount;
};

class Penelope : public Human
{
public:
	Penelope(StudentWorld* w, double x, double y) : Human(w, IID_PLAYER, x, y, right, 0, true, true, true, true, true, false, true, true, false, true),
		m_landmines(0), m_flameCharges(0), m_vaccines(0){}
	virtual void doSomething();
	void move(int dir);
	void useVaccine(){
		if (m_vaccines > 0) {
			clearInfection();
			m_vaccines--;
		}
	}
	void useFlameThrower();
	void useLandMine();
	virtual void setDead() { 
		Actor::setDead(); 
		m_flameCharges = m_landmines = m_vaccines = 0;
	}
	
	// Increase the number of vaccines the object has.
	void increaseVaccines() { m_vaccines++; }
	void decreaseVaccines() { m_vaccines--; }

	// Increase the number of flame charges the object has.
	void increaseFlameCharges() { m_flameCharges = m_flameCharges + 5; }
	void decreaseFlameCharges() { m_flameCharges--; }

	// Increase the number of landmines the object has.
	void increaseLandmines() { m_landmines+=2; }
	void decreaseLandmine() { m_landmines--; }

	// How many vaccines does the object have?
	int getNumVaccines() const { return m_vaccines; }

	// How many flame charges does the object have?
	int getNumFlameCharges() const { return m_flameCharges; }

	// How many landmines does the object have?
	int getNumLandmines() const { return m_landmines; }
private:
	int m_landmines;
	int m_vaccines;
	int m_flameCharges;
};


class Citizen : public Human
{
public:
	Citizen(StudentWorld* w, double x, double y):Human(w, IID_CITIZEN, x, y, right, 0, true, true, true, false, true, false, true, true, false, false){}
	virtual void doSomething();
	virtual bool incrementInfectionCount();
	virtual void setDead();
	//virtual void useExitIfAppropriate();
	//virtual void dieByFallOrBurnIfAppropriate();
private:
};




//ZOMBIES
class Zombie : public Agent
{
public:
	Zombie(StudentWorld* w, double x, double y) :Agent(w, IID_ZOMBIE, x, y, right, 0, false, true, false, false, true, false, true, false, true, false), 
		m_movementPlanDistance(0), m_return(false){}
	virtual void doSomething();
	virtual void setDead();
	bool shouldReturn() { return m_return; } //returns whether or not this base class completed an action that should finish dumb/smart zombies doSomething() call
	int getMovemenetPlanDistance() { return m_movementPlanDistance; }
	bool decrementMovementPlanDistance() { return (m_movementPlanDistance--) <= 0; }
	void resetMovementPlanDistance(int n) { m_movementPlanDistance = n; }
private:
	int m_movementPlanDistance;
	bool m_return;
};

class DumbZombie : public Zombie
{
public:
	DumbZombie(StudentWorld* w, double x, double y): Zombie(w, x, y){}
	virtual void doSomething();
	virtual void setDead();
	//virtual void dieByFallOrBurnIfAppropriate();
};

class SmartZombie : public Zombie
{
public:
	SmartZombie(StudentWorld* w, double x, double y):Zombie(w, x, y){}
	virtual void setDead();
	virtual void doSomething();
	//virtual void dieByFallOrBurnIfAppropriate();
};

#endif // ACTOR_INCLUDED




