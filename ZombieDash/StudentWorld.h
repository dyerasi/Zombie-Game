#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Level.h"
#include "GameWorld.h"
#include "Actor.h" 
#include <string>
#include <vector>
using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp


class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath);
	virtual int init();
    virtual int move(); //use gameWorld functions pg24
    virtual void cleanUp();
	//bool useFlameThrower() {} //see pg 27, create 3 new flame objects and use for loop to place onto graphics
	//bool useLandmine(); //should these be void or bool?
	//bool useVaccine();
	void addActor(Actor* a) { actors.push_back(a); }
	void finishLevel() {
		m_levelFinished = true;
		playSound(SOUND_LEVEL_FINISHED);
	}
	bool levelFinished() const { return m_levelFinished; }
	bool noBlockingOfMovement(double m_x, double m_y, Actor* a); //passes is updated Actor a coordinates and a itself 
	bool checkForOverlaps(double x1, double y1, Actor* a); //checks if ANY object will overlap at x, y coord
	int citizensLeft() { return m_citizensLeft; }
	void decreasecitizens() { m_citizensLeft--; }
	void addZombie(double x, double y);
	double distance(double x1, double y1, double x2, double y2) { return sqrt((x1 - x2)*(x1-x2) + (y1 - y2)*(y1 - y2)); }
	double distanceToMC(Actor* a);
	double distanceToNearestZ(double x, double y);
	void findNewCoords(double(&coords)[2], int dir, int pixelX, int pixelY); //updates array of two doubles, updated x and y coordinates
	Penelope* getMC() const { return mc; }
	bool checkMovementTowards(Actor* a, Actor* b, int pixels);
	void updateGameText();
	bool locateNearestVomitTrigger(double x1, double y1, double& x2, double& y2, double& dist);
	bool zombieTryAndThrowUp(Actor* a);
	bool overlap(double x1, double y1, double x2, double y2);
	virtual ~StudentWorld(); 

private:
	bool m_levelFinished;
	void generateObjects(Level lev);
	string generateLevelFile(int levelN); //uses sstream to generate assetPath + "/level" + levelN
	Penelope* mc; //special pointer to main character
	vector<Actor*> actors; //list of pointers to actors(objects in game)
	bool m_destructorCalled;
	int m_citizensLeft, m_zombiesLeft;
};

#endif // STUDENTWORLD_H_
