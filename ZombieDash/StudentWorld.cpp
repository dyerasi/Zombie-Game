#include "StudentWorld.h" //includes level.h, gameworld.h, and actor.h
#include "GameConstants.h"
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <iomanip>
#include "Actor.h"
using namespace std;


GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath): GameWorld(assetPath),m_destructorCalled(false), m_levelFinished(false)
{
	m_citizensLeft = m_zombiesLeft = 0;
} 


//takes in coordinates and updates them to move n pixels  in direction dir
void StudentWorld::findNewCoords(double(&coords)[2], int dir, int pixelX, int pixelY) {
	switch (dir) {
	case(0)://right
		coords[0] += pixelX;
		break;
	case(90): //up
		coords[1] += pixelY;
		break;
	case(180): //left
		coords[0] -= pixelX;
		break;
	case(270): //down
		coords[1] -= pixelY;
		break;
	}
}


string StudentWorld::generateLevelFile(int levelN) {
	
	string x = "level";
	if (levelN < 10)
		x += "0";
	ostringstream oss;
	oss << x << levelN << ".txt";
	x = oss.str();
	return x;
}
int StudentWorld::init()
{
	//reset member variables for each new level
	m_levelFinished = m_destructorCalled =false;
	m_citizensLeft = m_zombiesLeft = 0;

	Level currentLev(assetPath()); //initialize new level
	int level = getLevel(); //get current level number
	string levelFile = generateLevelFile(level);
	Level::LoadResult result = currentLev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found)
		return GWSTATUS_PLAYER_WON; //finished all available levels
	else if (result == Level::load_fail_bad_format)
		return GWSTATUS_LEVEL_ERROR;
	else if (result == Level::load_success)
		generateObjects(currentLev);
	
 
	return GWSTATUS_CONTINUE_GAME;
}

//make sure to increment citzensLeft + zombiesLeft!!!
void StudentWorld::generateObjects(Level lev) {
	//go through each char in level and generate proper object for it
	for (int x = 0; x != (VIEW_WIDTH/SPRITE_WIDTH); x++) {
		for (int y = 0; y != (VIEW_HEIGHT/SPRITE_HEIGHT); y++) {
			Level::MazeEntry ge = lev.getContentsOf(x, y);
			double xCoord = SPRITE_WIDTH * x; //pixel coordinates(16*whatever it is in level.txt file)
			double yCoord = SPRITE_HEIGHT * y;
			switch (ge) {
			case Level::empty:
				break;
			case Level::smart_zombie:
				actors.push_back(new SmartZombie(this, xCoord, yCoord));
				m_zombiesLeft++;
				break;
			case Level::dumb_zombie:
				actors.push_back(new DumbZombie(this, xCoord, yCoord));
				m_zombiesLeft++;
				break;
			case Level::player:
				mc = new Penelope(this, xCoord, yCoord);
				actors.push_back(mc);
				break;
			case Level::exit:
				actors.push_back(new Exit(this, xCoord, yCoord));
				break;
			case Level::wall:
				actors.push_back(new Wall(this, xCoord, yCoord));
				break;
			case Level::pit:
				actors.push_back(new Pit(this, xCoord, yCoord));
				break;
			case Level::citizen:
				actors.push_back(new Citizen(this, xCoord, yCoord));
				m_citizensLeft++;
				break;
			case Level::vaccine_goodie:
				actors.push_back(new VaccineGoodie(this, xCoord, yCoord));
				break;
			case Level::gas_can_goodie:
				actors.push_back(new GasCanGoodie(this, xCoord, yCoord));
				break;
			case Level::landmine_goodie:
				actors.push_back(new LandmineGoodie(this, xCoord, yCoord));
				break;
			}
		}
	}
}

int StudentWorld::move()
{

	//MAKE SURE that live actors dont interact with actors who have dies in the same tick
	for (vector<Actor*>::iterator it1 = actors.begin(); it1 != actors.end() && !mc->isDead(); it1++) { //1st iterator through actor objects
		if (((*it1)->isDead()))
			continue;
		(*it1)->doSomething();
		for (vector<Actor*>::iterator it2 = actors.begin(); it2 != actors.end() && !mc->isDead(); it2++) { 
			//if both actors arent dead, arent the same, overlap, and it1 doesnt block movement
			//if it1 doesnt block movement, then it is an activating object(flames, pits, vomit, etc)
			if( !((*it1)->blocksMovement()) && !((*it2)->isDead()) && !((*it1)->isDead()) && (*it2)!=(*it1) && overlap((*it1)->getX(), (*it1)->getY(), (*it2)->getX(), (*it2)->getY())){
				(*it1)->activateIfAppropriate((*it2)); //have activating object activate with all other overlapping actors
				if (levelFinished() && !(mc->isDead()))
					return GWSTATUS_FINISHED_LEVEL;
			}
		}
	}
	if (mc->isDead()) {
		decLives();
		playSound(SOUND_PLAYER_DIE);
		return GWSTATUS_PLAYER_DIED;
	}
	updateGameText();

	//remove dead actors
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
		Actor* ptr = (*it);
		if ((ptr)->isDead()){ //if the actor is dead
			delete ptr; //delete the dead actor object
			(*it) = nullptr; //then set it to nullptr to indicate it is no longer valid
		}
	}
	actors.erase(remove(actors.begin(), actors.end(), nullptr), actors.end()); //remove the nullptrs from actors
	return GWSTATUS_CONTINUE_GAME;
}



void StudentWorld::cleanUp()
{	
	if (!m_destructorCalled) {
		//delete mc;
		for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
			delete (*it);
		actors.clear();
	}
	m_destructorCalled = true;
	//make sure two consecutive calls to cleanUp doesnt result in undefined behavior
	//no dangling pointers + etc
}

void StudentWorld::addZombie(double x, double y) {
	int dOrs = randInt(1, 10);
	if (dOrs < 8)//create dumb zombie
		actors.push_back(new DumbZombie(this, x, y));
	else
		actors.push_back(new SmartZombie(this, x, y));
}


//returns true if any other actor overlaps with a's possible new coordinates
bool StudentWorld::checkForOverlaps(double x1, double y1, Actor* a) {
	double x2, y2;
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++) {
		if (a == (*it))
			continue;
		 x2 = (*it)->getX();
		 y2 = (*it)->getY();
		 if (overlap(x1, y1, x2, y2))
			 return true;
	}
	return false;
}

bool StudentWorld::noBlockingOfMovement(double m_x, double m_y, Actor* a) {
	//l1 is coords for top left of rectangle, r1 is coords for bottom right of rectangle
	double l1x, l1y, r1x, r1y, l2x, l2y, r2x, r2y;
	l1x = m_x;
	r1x = l1x + SPRITE_WIDTH - 1;
	r1y = m_y;
	l1y = r1y + SPRITE_HEIGHT - 1;
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++) {
		
		if (a == nullptr) { //special case to check for creation of flame object at given coordinate
			if ((*it)->blocksFlame() && !(*it)->isDead()) { //if its a wall or exit and not dead
				if (overlap(m_x, m_y, (*it)->getX(), (*it)->getY())) //if the flame would overlap with a wall, return false and dont make flame
					return false;
			}
			else //if not an alive wall or exit, ignore it
				continue;
		}
		else {
			if ( !(*it)->blocksMovement() && !(*it)->blocksFlame()) //if the object represented by it doesnt block movement or isnt an exit
				continue;
			if (!((*it)->isDead()) && (*it) != a) {//if the object is stil active and not the same object being passed in
				//create bottom left and top right coordinates for a rectangle representing the area actor is in
				//check if rectangles would overlap, and if they do return false to not allow movement
				if ((*it)->blocksFlame() && !(*it)->blocksMovement() && a->usesExit()) //if its checking between the exit and a human or vomit
					continue;
				l2x = (*it)->getX();
				r2x = l2x + SPRITE_WIDTH - 1;
				r2y = (*it)->getY();
				l2y = r2y + SPRITE_HEIGHT - 1;
				if (l1x > r2x || l2x > r1x) //if one area on left side of other
					continue;
				else if (l1y < r2y || l2y < r1y) //if one are above other
					continue;
				else
					return false; //they are overlapping rectangles
			}
		}
		
	}
	
	return true;
}

bool StudentWorld::overlap(double x1, double y1, double x2, double y2) {
	double x = (x1 - x2)*(x1-x2);
	double y = (y1 - y2)*(y1 - y2);
	return (x + y <= 100);
}

StudentWorld::~StudentWorld() {
	if(!m_destructorCalled)
		cleanUp();
}

void StudentWorld::updateGameText() {
	ostringstream oss;
	oss << "Score: "; // << getScore() << "  ";
	oss.fill('0'); //fill score with 0's till its 6 digits
	if (getScore() < 0) {
		oss << "-" << setw(6) << (-1 * getScore());
		oss << "  ";
	}
	else {
		oss << setw(6) << getScore();
		oss << "  ";
	}
	oss << "Level: " << getLevel() << "  ";
	oss << "Lives: " << getLives() << "  ";
	oss << "Vaccines: " << mc->getNumVaccines() << "  ";
	oss << "Flames: " << mc->getNumFlameCharges() << "  ";
	oss << "Mines: " << mc->getNumLandmines() << "  ";
	oss << "Infected: " << mc->getInfectionCount() << "  " << endl;
	setGameStatText(oss.str());
	
}

double StudentWorld::distanceToMC(Actor* a) {
	return distance(a->getX(), a->getY(), mc->getX(), mc->getY());
}

double StudentWorld::distanceToNearestZ(double x, double y) {
	if (m_zombiesLeft == 0)//if no zombies left
		return -1;
	double min = 999;
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++) {
		if (!((*it)->threatensCitizens()) || (*it)->isDead())//if its a dead or nonzombie actor
			continue;
		double d = distance(x, y, (*it)->getX(), (*it)->getY());
		if (d < min)
			min = d;
	}
	return min;
 }

//for a citizen 
bool StudentWorld::checkMovementTowards(Actor* a, Actor* b, int pixels) { //checks movement from a to b
	//x1==x2 && y1==y2 is impossible as the objects would block each other
	double x1 = a->getX();
	double y1 = a->getY();
	double x2 = b->getX();
	double y2 = b->getY();
	double original_x1 = x1;
	double original_y1 = y1;

	int dir; //direction from a to b
	
	int dirHorizontal, dirVertical;
	if (x1 < x2)
		dirHorizontal = 0; //right
	if (x1 > x2)
		dirHorizontal = 180; //left
	if (y1 < y2)
		dirVertical = 90; //up
	if (y1 > y2)
		dirVertical = 270; //down
	int chooseD = randInt(1, 2); //creates a rand into to choose between moving vertically or horizontally closer to actor b
	if (y1 == y2) //if a and b are on same y, then move horizontally to get closer as dirVertical hanst been set
		chooseD = 1;
	if (x1 == x2) //id a and b on same x, move vertically to get closer and dirHorizontal hasnt been set
		chooseD = 2;
	if (chooseD == 1) //choose dirHorizontal
		dir = dirHorizontal;
	else  //choose dirVertical
		dir = dirVertical;
	switch (dir) {
	case(0):
		x1 += pixels;
		break;
	case(90):
		y1 += pixels;
		break;
	case(180):
		x1 -= pixels;
		break;
	case(270):
		y1 -= pixels;
		break;
	}
	if (noBlockingOfMovement(x1, y1, a)) {
		a->setDirection(dir);
		a->moveTo(x1, y1);
		return true;
	}
	else if (!noBlockingOfMovement(x1, y1, a) && original_x1 != x2 && original_y1 != y2) { //if that movement was blocked and there is another movement path to try
		if (chooseD == 1) 
			dir = dirVertical; //change directions from horizontal to vertical
		if (chooseD == 2)
			dir = dirHorizontal; //change direction from vertical to horizontal
		x1 = original_x1;
		y1 = original_y1;
		switch (dir) {
		case(0):
			x1 += pixels;
			break;
		case(90):
			y1 += pixels;
			break;
		case(180):
			x1 -= pixels;
			break;
		case(270):
			y1 -= pixels;
			break;
		}
		if (noBlockingOfMovement(x1, y1, a)) {
			a->setDirection(dir);
			a->moveTo(x1, y1);
			return true;
		}
		else {
			return false; //no movement made
		}

	}
	
	return false; //no movement made 
}




// Return true if there is a living human, otherwise false.  If true,
	 // otherX, otherY, and distance will be set to the location and distance
	 // of the human nearest to (x,y).
bool StudentWorld::locateNearestVomitTrigger(double x1, double y1, double& x2, double& y2, double& dist) {
	bool humansLeft = false;
	if (mc->isDead())
		return humansLeft;
	double min = 99999;
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++) {
		if (!(*it)->usesExit() || !(*it)->canBeInfected() || (*it)->isDead()) //if it doesnt use an exit, cant be infected, or is a dead object
			continue; //cycle through all nonactive/human objects
		else { //it is penelope or citizen 
			humansLeft = true;
			if (distance(x1, y1, (*it)->getX(), (*it)->getY()) < min) {
				min = distance(x1, y1, (*it)->getX(), (*it)->getY());
				x2 = (*it)->getX(); //set x2/y2 to location of closest human
				y2 = (*it)->getY();
			}
		}
	}
	dist = min; //set to distance to overall min
	return humansLeft;
}

//bullimic function
bool StudentWorld::zombieTryAndThrowUp(Actor* a) {
	double x, y;
	x = a->getX();
	y = a->getY();
	switch (a->getDirection()) {
	case(0):
		x += SPRITE_WIDTH;
		break;
	case(90):
		y += SPRITE_HEIGHT;
		break;
	case(180):
		x -= SPRITE_WIDTH;
		break;
	case(270):
		y -= SPRITE_HEIGHT;
		break;
	}
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++) {
		if ((*it)->isDead() || !(*it)->canBeInfected() || !(*it)->usesExit() || a == (*it))
			continue;
		else { //is human
			if (distance(x, y, (*it)->getX(), (*it)->getY()) <= 10) {
				int r = randInt(1, 3);
				if (r == 1) {
					actors.push_back(new Vomit(this, x, y, a->getDirection()));
					playSound(SOUND_ZOMBIE_VOMIT);
					return true;
				}
			}
		
		}
	

	}
	return false; //no vomit made
}

