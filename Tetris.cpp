#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <conio.h>
#include <vector>
#include <string>
#include <ctime>

const int WIDTH = 10;
const int HEIGHT = 20;
const int MOVE_UPDATE = 3; // Lower = faster block movement
const int WAIT = 60;  // Time in between new frames

const std::vector<std::string> COLORS = {
	"\u001b[31m",
	"\u001b[32m",
	"\u001b[33m",
	"\u001b[34m",
	"\u001b[35m",
	"\u001b[36m"
};

// Coordinates of all the blocks' spawning positions
const std::vector <std::vector<std::vector<int>>> BLOCK_TYPES = {
	{ { WIDTH / 2 - 1, 0 }, { WIDTH / 2, 0 }, { WIDTH / 2 + 1, 0 }, { WIDTH / 2, 1 } },  // T-block
	{ { WIDTH / 2 - 2, 0 }, { WIDTH / 2 - 1, 0 }, { WIDTH / 2, 0 }, { WIDTH / 2 + 1, 0 } },  // I-block
	{ { WIDTH / 2 - 1, 0 }, { WIDTH / 2 - 1, 1 }, { WIDTH / 2, 1 }, { WIDTH / 2 + 1, 1 } },  // L-block 1
	{ { WIDTH / 2 + 1, 0 }, { WIDTH / 2 - 1, 1 }, { WIDTH / 2, 1 }, { WIDTH / 2 + 1, 1 } }  // L-block 2
};

#define RESETCOLOR "\u001b[0m"


class Block
{
public:
	std::vector<std::vector<int>> coordinates = BLOCK_TYPES[rand() % BLOCK_TYPES.size()];
	std::vector<int> centerPoint = { WIDTH / 2, 0 };
	std::string color = COLORS[rand() % COLORS.size()];

	void fallCenter()
	{
		centerPoint[1] += 1;
	}

	std::vector<std::vector<int>> fall()
	{
		std::vector<std::vector<int>> newCoordinates = coordinates;
		for (int i = 0; i < coordinates.size(); i++)
		{
			newCoordinates[i][1] += 1;
		}
		return newCoordinates;
	}

	std::vector<std::vector<int>> rotate()
	{
		std::vector<std::vector<int>> rotatedCoordinates = coordinates;

		// Subtract the center point from each coordinate
		for (int i = 0; i < coordinates.size(); i++)
		{
			rotatedCoordinates[i][0] -= centerPoint[0];
			rotatedCoordinates[i][1] -= centerPoint[1];
		}

		// Rotate the coordinates 90 degrees clockwise
		for (int i = 0; i < coordinates.size(); i++)
		{
			// (x, y) = (y, -x)
			rotatedCoordinates[i] = { rotatedCoordinates[i][1],-rotatedCoordinates[i][0] };
		}

		// Add the center point back to each coordinate
		for (int i = 0; i < coordinates.size(); i++)
		{
			rotatedCoordinates[i][0] += centerPoint[0];
			rotatedCoordinates[i][1] += centerPoint[1];
		}

		return rotatedCoordinates;
	}

	std::vector<std::vector<int>> moveSideways(int jump)
	{
		std::vector<std::vector<int>> newCoords = { };

		for (auto coord : coordinates)
		{
			newCoords.push_back({ coord[0] + jump, coord[1] });
		}

		return newCoords;
	}
};


class Game
{
	std::vector<Block> blocks = { Block {} };

	std::vector<std::vector<int>> blockAtCoordinates(int x, int y)
	{
		for (Block block : blocks)
		{
			for (std::vector<int> coordinate : block.coordinates)
			{
				if (coordinate[0] == x && coordinate[1] == y)
				{
					return block.coordinates;
				}
			}
		}

		return { };
	}

	std::string blockStringAtCoordinate(int x, int y)
	{
		// Used by the draw method to find 
		// the ascii character of a given coordinte, color included
		for (Block block : blocks)
		{
			for (std::vector<int> coordinate : block.coordinates)
			{
				if (coordinate[0] == x && coordinate[1] == y)
				{
					return block.color + "O " + RESETCOLOR;
				}
			}
		}
		return "  ";
	}

	void draw()
	{
		std::string board = "";

		// Add the top border
		for (int i = 0; i <= WIDTH; i++)
		{
			board += "##";
		}

		// Draw the main board
		for (int y = 0; y < HEIGHT; y++)
		{
			board += "\n#";
			for (int x = 0; x < WIDTH; x++)
			{
				board += blockStringAtCoordinate(x, y);
			}
			board += "#";
		}

		// Add the bottom border
		board += "\n";
		for (int i = 0; i <= WIDTH; i++)
		{
			board += "##";
		}

		printf("\033[%d;%dH", 1, 1);  // Move the cursor to the top left, may only work on windows
		std::cout << board;
	}

	bool validMove(std::vector<std::vector<int>> originalCoords, std::vector<std::vector<int>> newCoords)
	{	
		for (auto newCoord : newCoords)
		{
			// Check horizontal and vertical movement
			if (0 > newCoord[0] || newCoord[0] >= WIDTH || newCoord[1] >= HEIGHT)
			{
				return false;
			}
		}

		for (Block block : blocks)
		{
			if (block.coordinates == originalCoords)
			{
				continue;
			}

			for (auto newCoord : newCoords)
			{
				for (Block block : blocks)
				{
					if (block.coordinates == originalCoords)
					{
						continue;
					}

					for (auto coordinate : block.coordinates)
					{
						if (coordinate[0] == newCoord[0] && coordinate[1] == newCoord[1])
						{
							return false;
						}
					}
				}
			}
		}
	}

	void fallAllBlocks()
	{
		for (int i = 0; i < blocks.size(); i++)
		{
			if (validMove(blocks[i].coordinates, blocks[i].fall()))
			{
				blocks[i].coordinates = blocks[i].fall();
				blocks[i].fallCenter();
			}
		}
	}

	int detectLineClear()
	{
		for (int y = HEIGHT - 1; y >= 0; y--)
		{
			bool lineClear = true;
			for (int x = 0; x < WIDTH; x++)
			{
				std::vector<std::vector<int>> checkBlock = blockAtCoordinates(x, y);
				if (checkBlock.size() == 0 || checkBlock == blocks[blocks.size() - 1].coordinates)
				{
					lineClear = false;
					break;
				}
			}

			if (lineClear)
			{
				return y;
			}
		}

		return -1;
	}

	void clearLine(int y)
	{
		for (int i = 0; i < blocks.size(); i++)
		{
			for (int j = blocks[i].coordinates.size() - 1; j >= 0; j--)
			{
				if (blocks[i].coordinates[j][1] == y)
				{
					blocks[i].coordinates.erase(blocks[i].coordinates.begin() + j);
				}
			}
		}
	}

	void rotateBlock()
	{
		auto rotatedCoordinates = blocks[blocks.size() - 1].rotate();

		if (validMove(blocks[blocks.size() - 1].coordinates, rotatedCoordinates))
		{
			blocks[blocks.size() - 1].coordinates = rotatedCoordinates;
		}
	}

	void moveHorizontal(int jump)
	{
		auto sidewaysCoords = blocks[blocks.size() - 1].moveSideways(jump);

		if (validMove(blocks[blocks.size() - 1].coordinates, sidewaysCoords))
		{
			blocks[blocks.size() - 1].coordinates = sidewaysCoords;
			blocks[blocks.size() - 1].centerPoint[0] += jump;
		}
	}

	void getInput()
	{
		if (!_kbhit())
		{
			return;
		}

		switch (_getch())
		{
		case 'w':
			rotateBlock();
			break;

		case 'a':
			moveHorizontal(-1);
			break;

		case 's':
			fallAllBlocks();
			break;

		case 'd':
			moveHorizontal(1);
			break;
		}
	}

	void clearEmptyBlocks()
	{
		for (int i = blocks.size() - 1; i >= 0; i--)
		{
			if (blocks[i].coordinates.empty())
			{
				blocks.erase(blocks.begin() + i);
			}
		}
	}

	bool loseCheck()
	{
		for (auto coordinate : blocks[blocks.size() - 1].coordinates)
		{
			if (blockAtCoordinates(coordinate[0], coordinate[1]) != blocks[blocks.size() - 1].coordinates)
			{
				return true;
			}
		}
		
		return false;
	}

public:
	void runGame()
	{
		int frameNumber = 0;

		while (true)
		{
			for (int i = 0; i < HEIGHT * MOVE_UPDATE; i++)
			{
				frameNumber += 1;
				getInput();

				if (frameNumber == MOVE_UPDATE)
				{
					if (!validMove(blocks[blocks.size() - 1].coordinates, blocks[blocks.size() - 1].fall()))
					{
						blocks.push_back(Block{});
						if (loseCheck())
						{
							return;
						}
					}

					int lineClear = detectLineClear();

					if (lineClear != -1)
					{
						clearLine(lineClear);
					}

					fallAllBlocks();

					frameNumber = 0;
				}

				draw();
				Sleep(WAIT);
			}
		}
	}
};


int main()
{
	srand(time(0));

	while (true)
	{
		Game game;
		game.runGame();

		std::cout << "Play again? (y/n) ";
		char answer;
		std::cin >> answer;

		if (answer == 'n')
		{
			return 0;
		}

		system("cls");
	}
}
