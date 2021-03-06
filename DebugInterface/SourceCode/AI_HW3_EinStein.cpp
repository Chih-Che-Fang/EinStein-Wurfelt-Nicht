//Import library that will be used
#include "stdafx.h"
#include <unordered_map>
#include <tuple>
#include <limits.h>
#include <stdio.h>
#include <iostream>
#include<string>
#include<sstream>
#include<windows.h>
#include <fstream>
using namespace std;

#define BLUE 0 //Blue Chess ID
#define RED 1  //Red Chess ID
#define CHESS_NUM 6 //Number of Chess, that is, Chess 1 ~ 6
#define DEPTH_LIMIT 5 //Max search depth for expect-minimax algorithm
#define OUTPUT_FILE "log.txt" //File Name of debug log
#define DEBUG_MODE 1 //If program is in debug mode 0:debug 1:non-debug
#define BOARD_INPUT 14 //Number of board state's input
#define BOARD_SIZE 25 //Define 1-D board size

//Move Class is used to represent a valid chess move
class Move {
public:
	int from; //start position
	int to; //End position
	Move(int _from, int _to) { //Move constructor
		from = _from;
		to = _to;
	}
};
//Board Class is used to represent the board and store current state of chess board
class Board
{
public:
	int dice; // Number of dice
	bool color; //color(1:Red  0:Blue)
	int ChessPos[12] = {}; //Chess Position for each chess, Ex. ChessPos[0] = i means chess 0 is in position i
	int * BChessPos = ChessPos; //Chess Position for blue chess
	int * RChessPos = ChessPos + 6; //Chess Position for red chess
	unsigned long BChess = 0; //bit borad for blue chess
	unsigned long RChess = 0; //bit borad for red chess
	int pBoard[25]; //1-D Representation of board

	Board() {//Constrouctor of Board

	}

	Board(int * state) {//Constrouctor of Board
						//Read board information form board state
		dice = state[1] - 1;
		color = state[0];
		RChess = 0;
		BChess = 0;
		for (int i = 0; i < 25; i++) pBoard[i] = -1; //init pBoard values
		for (int i = 2; i <= 7; ++i) {//Store blue chess positions and board chess
			ChessPos[i - 2] = state[i] - 1;
			if (ChessPos[i - 2] >= 0) {//if chess i -2 exists, store its position and information
				pBoard[ChessPos[i - 2]] = i - 2;
				BChess |= (1 << ChessPos[i - 2]);
			}
		}
		for (int i = 8; i <= 13; ++i) {//Store red chess positions and board chess
			ChessPos[i - 2] = state[i] - 1;
			if (ChessPos[i - 2] >= 0) {//if chess i -2 exists, , store its position and information
				pBoard[ChessPos[i - 2]] = i - 2;
				RChess |= (1 << ChessPos[i - 2]);
			}
		}
	}
	void fillBoard(int * state) {//Renew Board information
								 //Read board information form board state
		dice = state[1] - 1;
		color = state[0];
		RChess = BChess = 0;
		for (int i = 0; i < 25; i++) pBoard[i] = -1;
		for (int i = 2; i <= 7; ++i) {//Store blue chess positions and board chess
			ChessPos[i - 2] = state[i] - 1;
			if (ChessPos[i - 2] >= 0) {//if chess i -2 exists, , store its position and information
				pBoard[ChessPos[i - 2]] = i - 2;
				BChess |= (1 << ChessPos[i - 2]);
			}
		}
		for (int i = 8; i <= 13; ++i) {//Store blue chess positions and board chess
			ChessPos[i - 2] = state[i] - 1;
			if (ChessPos[i - 2] >= 0) {//if chess i -2 exists, , store its position and information
				pBoard[ChessPos[i - 2]] = i - 2;
				RChess |= (1 << ChessPos[i - 2]);
			}
		}
	}
};

//Define hash of each node 的計算, 用來計算與儲存重複盤面
typedef std::tuple<unsigned long, unsigned long, bool> key_t;
struct tuple_hash : public std::unary_function<key_t, std::size_t> {
	std::size_t operator()(const key_t& k) const
	{
		return std::get<0>(k) ^ std::get<1>(k) ^ (unsigned long)std::get<2>(k);
	}
};
//chess[0]: color(1:Red 0:Blue) chess[1]: dice chess[2~7]: Red Chess chess[8~13]
//int testBoardState[14] = { 0, 3, 4, 23, 20, 0, 0, 3, 0, 0, 23, 13, 8, 0 };
int boardState[14] = { 0, 2, 25, 24, 14, 12, 0, 15, 1, 8, 3, 6, 19, 23 };

//Heuristic value for Blue Chess
float BEvalValue[25] = { 16, 10, 5, 2.5, 1,
10, 8, 4, 2, 1,
5, 4, 4, 2, 1,
2.5, 2, 2, 2, 1,
1, 1, 1, 1, 0 };
//Heuristic value for Blue Chess
float REvalValue[25] = { 0, 1, 1, 1, 1,
1, 2, 2, 2, 2.5,
1, 2, 4, 4, 5,
1, 2, 4, 8, 10,
1, 2.5, 5, 10, 16 };
//direction[0] possible shift for blue chess => -1:左 -6:左上 -5: 往上 
//direction[1] possible shift for red chess => 1:右 6:右下上 5: 往下
int direction[2][3] = { { -5, -6, -1 },{ 5, 6, 1 } };

Board board(boardState); //Board information
unordered_map<tuple<unsigned long, unsigned long, bool>, float, tuple_hash> nodeMap; // tuple[0]: BChess bit board tuple[1]: RChess bit board tuple[2]: color
Move bestMove(-1, -1); //Best Move information
char boardMap[25]; //Printing board, Only for debug printing board

				   //Do initialization
void init() {
	//nodeMap.clear();
}

//Calculate estimated game scores
float Evaluate() {
	float RValue = 0.0f;
	float BValue = 0.0f;

	for (int i = 0; i < CHESS_NUM; i++) {
		if (board.BChessPos[i] >= 0) BValue += BEvalValue[board.BChessPos[i]];//Calculate estimated game scores for blue chess
		if (board.RChessPos[i] >= 0) RValue += REvalValue[board.RChessPos[i]];//Calculate estimated game scores for red chess
	}

	return (board.color == BLUE) ? BValue - RValue : RValue - BValue; //Derive the final estimated game scores
}

//Cut-Off Test to determin wheather we should cut game tree at current depth
bool CutOff_Test(int depth) {
	return (board.BChess & 1) || (board.RChess & (1 << 24)) || (depth == 0) || (board.RChess == 0) || (board.BChess == 0);
}

//Check if a move is valid or not
bool isValidMove(int nextPost, int dir, int origPos) {
	if (board.color == BLUE) {
		if ((dir >= 1) && (origPos % 5 == 0)) return false; //if blue chess is on boarder, it can only move toward up
	}
	else {
		if ((dir >= 1) && (origPos % 5 == 4)) return false; //if red chess is on boarder, it can only move toward down
	}
	return nextPost >= 0 && nextPost <= 24; //Check if chess move out of board
}

//According to dice number, determin which chess can move
void findPossibleChess(int * posChess) {
	int dice = (board.color == BLUE) ? board.dice : board.dice + CHESS_NUM;
	if (board.ChessPos[dice] >= 0) { //The Chess of dice no. exists, choose it directly
		posChess[0] = dice;
	}
	else {//The Chess of dice no. do not exist, choose the existing chess that most closed from both sides
		int lowerBound = (board.color == BLUE) ? 0 : CHESS_NUM;
		int upperBound = (board.color == BLUE) ? CHESS_NUM : 2 * CHESS_NUM;

		for (int i = dice + 1; i < upperBound; ++i) {//find the existing chess from right side
			if (board.ChessPos[i] >= 0) {
				posChess[0] = i;
				break;
			}
		}
		for (int i = dice - 1; i >= lowerBound; --i) {//find the existing chess from left side
			if (board.ChessPos[i] >= 0) {
				posChess[1] = i;
				break;
			}
		}
	}
}

//Make each chess move and update its relating information
int makeMove(int * origPos, int * nextPos) {
	int eatenChess = -1;
	int chessId = board.pBoard[*origPos]; //Get chosen chess


	board.ChessPos[chessId] = *nextPos; //Update chosen chess's position
	//Record and Update eaten chess
	eatenChess = board.pBoard[*nextPos];
	if (board.pBoard[*nextPos] >= 0) board.ChessPos[board.pBoard[*nextPos]] = -1;
	//Update  red/blue chesses' bit board
	if (board.color == BLUE) {
		board.BChess &= ~(1 << *origPos); //Clear previous position
		board.BChess |= (1 << *nextPos); //Update new position
		board.RChess &= ~(1 << *nextPos); //Update new position for adversarial side
	}
	else {
		board.RChess &= ~(1 << *origPos);  //Clear previous position
		board.RChess |= (1 << *nextPos); //Update new position
		board.BChess &= ~(1 << *nextPos); //Update new position for adversarial side
	}
	//Update chess board
	board.pBoard[*nextPos] = chessId;
	board.pBoard[*origPos] = -1;
	board.color = !board.color; //Change blue/red to red/blue
	return eatenChess;
}
//Revert board state to the state before making each move
void unmakeMove(int * origPos, int * nextPos, int eatenChessId, unsigned long * tempChess, bool tempColor) {
	int chessId = board.pBoard[*nextPos];
	//Revert chosen chess's information
	board.ChessPos[chessId] = *origPos;
	//Revert chess board state
	board.pBoard[*origPos] = chessId;
	board.pBoard[*nextPos] = -1;

	if (eatenChessId >= 0) {//Revert information of eaten chess
		board.pBoard[*nextPos] = eatenChessId;
		board.ChessPos[eatenChessId] = *nextPos;
	}
	//Revert color
	board.color = tempColor;
	//Revert blue/red bit board
	board.BChess = tempChess[0];
	board.RChess = tempChess[1];
}

//Check if current board state is visited or not, if Yes: get previous recored scores directly
//Time complexity: O(1)
bool getVisitedNode(pair<tuple<unsigned long, unsigned long, bool>, float> *next_state) {
	//Get current board state
	get<0>(next_state->first) = board.BChess;
	get<1>(next_state->first) = board.RChess;
	get<2>(next_state->first) = board.color;
	//Find if there is a visited node that is the same as current board state
	unordered_map<tuple<unsigned long, unsigned long, bool>, float>::iterator it;
	if ((it = nodeMap.find(next_state->first)) != nodeMap.end()) {
		next_state->second = it->second; // get previous recored scores directly
		return true;
	}
	return false;
}
//Perform expected minimax to find optimal move
float expected_minmax(int depth, bool chanceNode) {
	float bestVal = 0;
	if (chanceNode) {//If current node is a chance node
					 //Cut-Off test
		if (CutOff_Test(depth)) {
			return Evaluate(); //Game end or cut-off, return esitmated game scores
		}
		else {//Game is still going on, claculate the expected game scores for each dice possibability 
			tuple<unsigned long, unsigned long, bool> node =
				tuple<unsigned long, unsigned long, bool>(board.BChess, board.RChess, board.color);
			int origDice = board.dice;
			for (int d = 0; d < CHESS_NUM; d++) {// For each possible dice
				board.dice = d;
				bestVal += expected_minmax(depth - 1, false); //Calculate its expected game scores
				board.dice = origDice;
			}
			bestVal /= 6;
			//Stored current board state to hashmap for future use
			pair<tuple<unsigned long, unsigned long, bool>, float> state =
				pair<tuple<unsigned long, unsigned long, bool>, float>(node, bestVal);
			nodeMap.insert(state);
			return bestVal; //Return expected value
		}
	}
	else {
		//Derive possible move chess
		int posChess[2] = { -1, -1 };
		findPossibleChess(posChess);
		bestVal = INT_MIN;

		//Declare next node for storing current board state
		tuple<unsigned long, unsigned long, bool> next_node =
			tuple<unsigned long, unsigned long, bool>(-1, -1, -1);
		pair<tuple<unsigned long, unsigned long, bool>, float> next_state =
			pair<tuple<unsigned long, unsigned long, bool>, float>(next_node, -1);

		//Get max expected value
		for (int chessId = 0; chessId < 2; ++chessId) { //For each possible chess that can be moved
			if (posChess[chessId] < 0) continue;
			for (int dir = 0; dir < 3; ++dir) { //For each direction
				int origPos = board.ChessPos[posChess[chessId]];
				int nextPos = origPos + direction[board.color][dir];
				if (!isValidMove(nextPos, dir, origPos)) continue; //Check if a move is valid or not

																   //Store current node's information
				unsigned long tempChess[2] = { board.BChess, board.RChess };
				bool tempColor = board.color;

				int eatenChessId = makeMove(&origPos, &nextPos); //Make each chess move and update its relating information

				float val = getVisitedNode(&next_state) ? -next_state.second : -expected_minmax(depth, true); //Calculate estimated game scores for each possible move

				if (DEBUG_MODE && depth == DEPTH_LIMIT) {//Only for debug
					cout << val << ":Move(" << origPos << "," << nextPos << ")" << endl;
				}
				unmakeMove(&origPos, &nextPos, eatenChessId, tempChess, tempColor);
				if (val > bestVal) {//If etimated games for this move is larger, update best move to current move
					bestVal = val;
					if (depth == DEPTH_LIMIT) {
						bestMove.from = origPos; //Record start point for this move
						bestMove.to = nextPos; //Record end point for this move
					}
				}
			}
		}
		return bestVal;
	}
}

//Print board only for debug
void printBoard() {
	cout << "Board Graph:(a~d: Red chess, 0~5:Blue chess)" << endl;
	for (int i = 0; i < 25; ++i) {//Initialize chess board
		boardMap[i] = '.';
	}
	for (int i = 0; i < CHESS_NUM; ++i) {
		if (board.BChessPos[i] >= 0) boardMap[board.BChessPos[i]] = i + '0';//write bule chess
		if (board.RChessPos[i] >= 0) boardMap[board.RChessPos[i]] = i + 'a';//write red chess
	}
	for (int i = 0; i < BOARD_SIZE; ++i) {//output board
		cout << boardMap[i] << "  ";
		if (i % 5 == 4) cout << "\n";
	}
}

#if DEBUG_MODE
int main()
{
	init(); //Do initialization
	cout << "please input board state according to the following order: \n[color, dice, 6 position of blue chess, 6 position of red chess]"<<endl;
	for (int i = 0; i < BOARD_INPUT; ++i) { //Read board state from user interface
		cin >> boardState[i];
	}
	printBoard();
	cout << "Estimated game scores for each direction:" << endl;
	expected_minmax(DEPTH_LIMIT, false);//Perform expected minimax to find optimal move
	cout << "\nBest Move:" << bestMove.from << " " << bestMove.to;//Output best move
	system("pause");
	return 0;
}
#else
//Stored log only for debug
void outputLog() {
	std::ofstream out(OUTPUT_FILE); //For debug log
	out << "ChessPos:";
	for (int i = 0; i < CHESS_NUM * 2; i++) {
		out << board.ChessPos[i] << " ";
	}
	out << "\nColor:" << board.color;
	out << "\nDice:" << board.dice;
	out << "\n" << (bestMove.from + 1) << " " << (bestMove.to + 1) << endl;
	out.close();
}

//Chessing AI main function
int main()
{
	//Decare variables for reading input
	string line;
	stringstream ss;
	char Color;
	int Dice;
	int pos;
	while (true) {//Chessing routine, return best move for each board state
		init();//Do Initialization
		getline(cin, line);
		ss.clear();
		ss.str("");
		ss << line;
		ss >> line;

		if (line == "exit") {//Game end, finish the routine
			return 0;
		}
		else if (line == "ini") {//Set initialized board state
			ss >> Color;
			if (Color == 'B') cout << "23 19 15 25 24 20\n";
			else cout << "6 7 11 1 2 3\n";
		}
		else if (line == "get") {//AI's turn, return best move	
								 //Get current color and dice information
			ss >> Color;
			ss >> Dice;

			boardState[1] = Dice;
			boardState[0] = (Color == 'B') ? 0 : 1;
			for (int i = 0; i < 2 * CHESS_NUM; ++i) {//Get each chess's position
				ss >> pos;
				boardState[i + 2] = pos;
			}
			board.fillBoard(boardState);//Fill the read data to our own board data structure
			outputLog(); //Oputput log only for debug
			expected_minmax(DEPTH_LIMIT, false);//Perform expected minimax to find optimal move
			cout << (bestMove.from + 1) << " " << (bestMove.to + 1) << endl; //Output best move calculated by chessing AI
		}
	}
}
#endif

