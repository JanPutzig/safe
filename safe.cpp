#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <vector>

using std::string;
using std::vector;

// The states of spaces in a MirrorGrid.
// These values are important to determine how a ray of light behaves after entering this space.
// Furthermore, a space can only be a solution if it contains no mirror.
//
enum Mirror {
	//space contains no mirror
	EMPTY, 
	//space contains a mirror leaning right (/)
	SLASH,
	// space contains a mirror leaning left (\)
	BACKSLASH
};

//(row,column)-coordinates of a space in a 2D-grid
typedef std::pair<int, int> Position;
// 2D-grid of mirrors, the lock of a safe
typedef vector<vector<Mirror>> MirrorGrid;

class Safe
{
	//the locking mechanism
	MirrorGrid grid;

	//the dimensions of the grid
	int rows, columns;

	//follows the path a beam of light from starting position in direction would take
	//start is updated to the endpoint, direction to the final direction
	//returns a list of all EMPTY spaces the path contains
	//CAUTION: Designed for starting positions on the edge of the grid
	// and starting direction facing inward. No safeguards included.
	std::list<Position> markRay(Position& start, Position& direction);

public:
	
	//constructor to build grid from dimensions and mirror positions
	Safe(int r, int c,vector<Position> slashes, vector<Position> backSlashes);

	//calculates whether or not the safe can be opened or is by default open
	//if not open by default, calculates the lexicographically smallest solution
	//as well as the total number of solutions 
	std::string testSecurity();
};

//generates list of test cases from file
std::list<Safe> readFile(std::string file)
{
	//initialize output-list and file_reader
	std::list<Safe> result = std::list<Safe>();
	std::ifstream inputFile;
	inputFile.open(file);

	
	int r, c, m, n;
	//if available, read first line of new Safe
	while (inputFile >> r >> c >> m >> n)
	{
		int y, x;
		vector<Position> slashes(m);
		vector<Position> bSlashes(n);

		//read m lines, the /-mirrors
		for (int i = 0; i < m; i++)
		{
			inputFile >> y >> x;
			slashes[i] = Position(y-1,x-1);
		}
		
		//read n lines, the \-mirrors
		for (int i = 0; i < n; i++)
		{
			inputFile >> y >> x;
			bSlashes[i] = Position(y-1, x-1);
		}
		//for every test case, add the new safe to the end of the list
		result.push_back(Safe(r,c,slashes,bSlashes));
	}

	//cleanup and return
	inputFile.close();
	return result;
}

int main()
{
	//input prompt
	std::cout << "Please designate the input file: \n";

	//get filename
	std::string file;
	std::cin >> file;

	//generate safes from file
	std::list<Safe> testCases = readFile(file);
	
	//initialize output file stream
	std::ofstream outputFile;
	outputFile.open(file+".out");

	//run tests for all safes
	std::list<Safe>::iterator it;
	int i = 1;

	for (it = testCases.begin(); it != testCases.end(); it++)
	{
		//write result of this safe to file
		outputFile << "Case " << i << ": " << it->testSecurity() << std::endl;
		i++;
	}

	//cleanup
	outputFile.close();
	
	return 0;
}

string Safe::testSecurity()
{
	//start at emitter
	Position location(0, -1);
	Position direction(0, 1);

	//find all possible positions where a mirror can be inserted and is hit by the ray from the emitter
	std::list<Position> defaultPath = markRay(location, direction);
	
	//check if ray already reaches the detector (safe is open)
	if (location.first == rows-1 && location.second == columns)
	{
		return "0";
	}
	
	//start from detector inwards
	location = { rows - 1,columns };
	direction = { 0, -1};

	//find all possible positions where a mirror can be inserted and might reflect a ray towards the detector
	std::list<Position> targetPath = markRay(location, direction);
	

	//the intersection of targetPath and defaultPath is the list of solutions
	int intersections = 0;
	Position solution(-1, -1);

	//iterate over both sorted lists to find their intersections
	std::list<Position>::const_iterator defaultIt = defaultPath.cbegin();
	std::list<Position>::const_iterator targetIt = targetPath.cbegin();
	while (defaultIt != defaultPath.cend() && targetIt != targetPath.cend())
	{
		if (*defaultIt < *targetIt)
		{
			defaultIt++;
		}
		else if(*defaultIt == *targetIt)
		{
			intersections++;
			if (solution.first == -1)
			{
				solution = *defaultIt;
			}
			defaultIt++;
			targetIt++;
		}
		else
		{
			targetIt++;
		}
	}

	//no intersection implies unsolvable
	if (intersections == 0)
	{
		return "impossible";
	}
	//otherwise construct the output string as per specifications
	else {
		return std::to_string(intersections) + " " + std::to_string(solution.first+1) + " " + std::to_string(solution.second+1);
	}
}

//comparison function for lexicographical order
bool lexiComp(const Position& a, const Position& b)
{
	if (a.first != b.first) {
		return a < b;
	}
	else
	{
		return a.second < b.second;
	}
}

std::list<Position> Safe::markRay(Position& start, Position& direction)
{
	//initialize the list of empty spaces in the path
	std::list<Position> path = std::list<Position>();

	//take first step
	start.first += direction.first;
	start.second += direction.second;

	//while the last step didn't take us over the edge of the grid
	while (0 <= start.second && start.second < columns && 0 <= start.first && start.first < rows)
	{
		switch (grid[start.first][start.second])
		{
			//empty spaces are added to the list
			case Mirror::EMPTY:
				path.push_back(start);
				break;
			//mirrors reflect, thereby chaning the direction of the ray
			case Mirror::SLASH:
			{
				int tmp = direction.first;
				direction.first = -direction.second;
				direction.second = -tmp;
				break;
			}
			case Mirror::BACKSLASH:
			{
				int tmp = direction.second;
				direction.second = direction.first;
				direction.first = tmp;
				break;
			}
		}
		//take the next step
		start.first += direction.first;
		start.second += direction.second;
	}
	
	//sort the list and remove duplicates
	path.sort(lexiComp);
	path.unique();
	return path;
}

Safe::Safe(int r, int c, vector<Position> slashes, vector<Position> backslashes):rows(r),columns(c)
{
	//initialize empty grid according to dimensions
	grid = MirrorGrid(rows);
	for (int i = 0; i < rows; i++)
	{
		grid[i] = vector<Mirror>(columns,Mirror::EMPTY);
	}

	//add /-mirrors
	for (int i = 0; i < slashes.size(); i++)
	{
		grid[slashes[i].first][slashes[i].second] = Mirror::SLASH;
	}

	//add \-mirrors
	for (int i = 0; i < backslashes.size(); i++)
	{
		grid[backslashes[i].first][backslashes[i].second] = Mirror::BACKSLASH;
	}
}
