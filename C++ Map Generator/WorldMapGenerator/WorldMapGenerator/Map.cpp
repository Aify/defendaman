#include "Map.h"

int Map::getMapWidth () {
	return mapWidth;
};

int Map::getMapHeight () {
	return mapHeight;
};

int** Map::getMapBase () {
	return mapBase;
}

int** Map::getMapTemp () {
	return tempLayer;
}

int Map::getMapSeed () {
	return mapSeed;
}

int Map::randomizeSeed () {
	srand (time (NULL));
	mapSeed = rand ();
	return mapSeed;
}

void Map::smoothMap () {
	int thisNeighbours;
	for (int x = 0; x < mapWidth; x++)
		for (int y = 0; y < mapHeight; y++) {
			thisNeighbours = getNeighbourConditional (x, y, &Map::baseWallConditional);
			if (thisNeighbours < 4)
				mapBase[x][y] = baseSceneryDefault;
			else if (thisNeighbours > 4)
				mapBase[x][y] = baseWallDefault;
		}
}



void Map::buildMapBase () {
	createRandomBaseWalls ();

	//increase this to increase smoothing amount, 5 looks good though.
	for (int i = 0; i < 5; i++)
		smoothMap ();
}

int Map::getNeighbourConditional (int xPos, int yPos, int (Map::*condition)(int, int, int, int)) {
	int neighbourCount = 0;
	for (int x = xPos - 1; x <= xPos + 1; x++)
		for (int y = yPos - 1; y <= yPos + 1; y++)
			neighbourCount += (this->*condition) (x, y, xPos, yPos);
	return neighbourCount;
}

/*
Creates a 2D grid based on the int mapSize.
It assigns indexes to each spot on the array, which will be
used to create actual map tiles.
*/
void Map::createRandomBaseWalls () {
	for (int x = 0; x < mapWidth; x++)
		for (int y = 0; y < mapHeight; y++) {
			if (x == 0 || x == mapWidth - 1 || y == 0 || y == mapHeight - 1) {
				mapBase[x][y] = baseWallDefault;
				continue;
			}
			mapBase[x][y] = (rand() % 100 <= baseWallChance) ? baseWallDefault : baseSceneryDefault;
		}
}


/*
Function tells us if the object id at this position is a collidable wall,
or if its a visual asset.
*/
bool Map::isBaseWall (int xPos, int yPos) {
	return mapBase[xPos][yPos] <= baseWallMax;
}

void Map::resetMap (int ** map, const int value) {
	for (int x = 0; x < mapWidth; x++)
		for (int y = 0; y < mapHeight; y++)
			map[x][y] = value;
}

/*
This will create a set amount if points to originate grass, and a size of
origination. then fan out with a random diffusion level until it meets its
same value.
(Imagine an elevation map).
*/
void Map::createBaseScenery () {
	//how many origins of grass there will be
	int origins = 3;

	//size of the initial dot 
	int size = (rand () % 4) + 1;

	//amount of random paths to walk out from origin to make origin point
	int walks = 0;

	int tmpSize = size;
	while (tmpSize > 0)
		walks += 4 * tmpSize--;

	//every n cycles, the value of the grass will decrease
	int diffusion = 2;

	//this 2d array is a result of one cycle, it is saved to the tmp array after each process
	int ** localTemp = new int*[mapWidth];
	for (int i = 0; i < mapWidth; ++i)
		localTemp[i] = new int[mapHeight];
	resetMap (localTemp, 0);

	//When a -1 is read as a value anywhere on the map, it isnt done elevation calculations
	bool flagIsNull = true;

	for (int i = 0; i < origins; i++) {
		//pick a random point to place the grass origin
		int x = rand () % mapWidth;
		int y = rand () % mapHeight;
		tempLayer[x][y] = 1;

	}

	while (flagIsNull) {
		flagIsNull = false;
		for (int x = 0; x < mapWidth; x++)
			for (int y = 0; y < mapHeight; y++)
				if (getNeighbourConditional (x, y, &Map::surroundingHeightsCount) >= localTemp[x][y]) {
					localTemp[x][y] += (rand () % 10 <= getNeighbourConditional (x, y, &Map::surroundingHeightsCount)) ? 1 : 0;
				}

		for (int x = 0; x < mapWidth; x++)
			for (int y = 0; y < mapHeight; y++) {
				tempLayer[x][y] += localTemp[x][y];
				if (tempLayer[x][y] == -1)
					flagIsNull = true;
			}
		//drawMap (tempLayer);
		resetMap (localTemp, 0);
	}



	delete[] localTemp;
	

}

int Map::surroundingHeightsCount (int neighXPos, int neighYPos, int myXPos, int myYPos) {
	//neighbour testing within the bounds of the map
	if (neighXPos >= 0 && neighXPos < mapWidth && neighYPos >= 0 && neighYPos < mapHeight) {
		//make sure were not selecting the spot that was passed to us
		if (neighXPos != myXPos || neighYPos != myYPos) {
			if (tempLayer[neighXPos][neighYPos] >= tempLayer[myXPos][myYPos])
				return 1;
		}
	}
	return 0;
}

int Map::baseWallConditional (int neighXPos, int neighYPos, int myXPos, int myYPos) {
	//neighbour testing within the bounds of the map
	if (neighXPos >= 0 && neighXPos < mapWidth && neighYPos >= 0 && neighYPos < mapHeight) {
		//make sure were not selecting the spot that was passed to us
		if (neighXPos != myXPos || neighYPos != myYPos) {
			if (isBaseWall (neighXPos, neighYPos))
				return 1;
		}
	} else {
		return 1;
	}
	return 0;
}


void Map::drawMap (int ** mapArray) {
	printf ("\n\n -------------------------------------------------------------- \n\n");
	for (size_t x = 0; x < getMapWidth (); x++) {
		printf ("\n ");
		for (size_t y = 0; y < getMapHeight (); y++)
			printf ("%d ", mapArray[x][y]);
		/*if (map.isBaseWall(x,y))
		printf ("W ");
		else
		printf ("  ");*/
	}
}

void Map::joinMaps (int ** baseMap, int ** topMap) {
	for (int x = 0; x < mapWidth; x++)
		for (int y = 0; y < mapHeight; y++)
			if (baseMap[x][y] >= baseSceneryDefault)
				baseMap[x][y] += topMap[x][y];

}

//... requests a list of 0 or more parameters of pairs, which contain <string, void*>
std::string Map::ConvertToJSONString (int ** map) {
	std::string JSONString ("{ ");
	JSONString.append ("\"mapWidth\" : " + intToString (mapWidth) + ", ");
	JSONString.append ("\"mapHeight\" : " + intToString (mapHeight) + ", ");
	JSONString.append ("\"mapIDs\" : [");
	for (int x = 0; x < mapWidth; x++) {
		JSONString += "[";
		for (int y = 0; y < mapHeight; y++) {
			JSONString += intToString(map[x][y]);
			if (y < mapWidth - 1)
				JSONString += ", ";
		}
		JSONString += "]";
		if (x < mapHeight - 1)
			JSONString += ", ";
	}
	JSONString += "]";
	JSONString += "}";
	return JSONString;
}

void Map::createSpawnPoints (int ** map, int teams) {
	int ** spawnPoints = new int*[2];
	spawnPoints[0] = new int[teams];
	spawnPoints[1] = new int[teams];
	for (int i = 0; i < teams; i++) {
		//pick a place to place the generation spawn
		int x = rand () % mapWidth;
		int y = rand () % mapHeight;
		if (map[x][y] >= baseSceneryDefault && map[x][y] <= baseSceneryMax) {
			map[x][y] = objectIDList[0];
			spawnPoints[0][i] = x;
			spawnPoints[1][i] = y;
		} else {
			//placing a spot has failed, retry
			i--;
			continue;
		}
	}
	//validateSpawns (map, spawnPoints, teams);
}

void Map::validateSpawns (int ** map, int ** spawnPoints, int teams) {
	const int moveCost = 10;
	const int diagonalMoveCost = 14;
	// create the hValues map for finding distance to the goal.
	int ** hValues;
	hValues = new int*[mapWidth];
	for (int i = 0; i < mapWidth; i++)
		hValues[i] = new int[mapHeight];

	//for each team that exists
	for (int i = 0; i < teams; i++) {
		//the x and y position of the team currently path finding from
		int thisTeamX = spawnPoints[0][i];
		int thisTeamY = spawnPoints[1][i];
		//for the team that is testing, find a path to all of the other teams
		for (int j = 0; j < teams; j++) {
			//skip pathfinding for ourselves
			if (j == i)
				continue;
			//the x and y position of the team currently path finding to
			int goalTeamX = spawnPoints[0][j];
			int goalTeamY = spawnPoints[1][j];

			//find the heuristic values for all points on the map to the goal
			for (int x = 0; x < mapWidth; x++) {
				for (int y = 0; y < mapHeight; y++) {
					hValues[x][y] = std::abs (x - goalTeamX) + std::abs(y - goalTeamY);
				}
			}


			drawMap (hValues);
		}
	}
}

std::string Map::intToString (int n) {
	char temp[16];
	sprintf (temp, "%d", n);
	return std::string (temp);
}