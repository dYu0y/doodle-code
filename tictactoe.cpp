#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <conio.h>
#include <windows.h>
using namespace std;


const int MainMenu = 0, DifficultySelection = 1, Playing = 2, Exit = 3;
int Section;

const int Mode_PvP = 0, Mode_PvC = 1;
int GameMode;

const int Tie = 0, Win = 1, Lose = 2, Continue = -1;
int GameStatus;

const int  Easy = 0, Normal = 1, Hard = 2;
int AIDifficulty; 

//  WinTimes  : 玩家2(PvP) 或 電腦(PvC) 贏的次數
//  LoseTimes : 玩家1(PvP) 或 玩家(PvC) 贏的次數
int WinTimes, TieTimes, LoseTimes;

char TictactoeTable[9];

int Round;//  從0開始算 
int Tmp[9];//  記錄每一回合下在哪個點 

/*  線的編號: 
//  0|1|2  3|3|3  6| |    | |7
//  0|1|2  4|4|4   |6|    |7| 
//  0|1|2  5|5|5   | |6  7| | 
*/
int Score[8+1];//  只有 8 條線卻紀錄 9 個分數是因為 PosToLineTable 的關係
int LineCount[8+1];//  紀錄 9 個的原因同上 
const int PosToLineTable[9][4] = {//  多的用 8 來表示，方便 Score[] 和 LineCount[] 的紀錄
								{0, 3, 6, 8},
								{1, 3, 8, 8},
								{2, 3, 7, 8},
								{0, 4, 8, 8},
								{1, 4, 6, 7},
								{2, 4, 8, 8},
								{0, 5, 7, 8},
								{1, 5, 8, 8},
								{2 ,5 ,6 ,8}	
												};

const int LineToPosTable[8][3] = {
								{0, 3, 6},
								{1, 4, 7},
								{2, 5, 8},
								{0, 1, 2},
								{3, 4, 5},
								{6, 7, 8},
								{0, 4, 8},
								{2, 4, 6}
												};

const int SpecialTable[8][4] = {//  詳細說明請看第NNNNNNNNN行 
								{ 5,  7,  1,  3},
								{ 6,  8,  3,  5},	
								{ 3,  7,  1,  5},
								{ 8, -1,  7, -1},
								{-1, -1, -1, -1},
								{ 6, -1,  7, -1},
								{-1, -1, -1, -1},
								{-1, -1, -1, -1}
									};

/* 	角|邊|角
//	--------
//	邊|  |邊
//	--------
//	角|邊|角
*/
int RestNum, Rest[9+1], RestCornerNum, RestCorner[4+1], RestSideNum, RestSide[4+1];
const int Mid = 4, Corner[4] = {0, 2, 6, 8}, Side[4] = {1, 3, 5, 7};


char Player1_Mark, Player2_Mark;//  在 玩家 vs 電腦 模式下，玩家固定使用 Player1_Mark
                                //					電腦AI 固定使用 Player2_Mark


int turn;//  當  回合數 % 2 == turn  時，那個回合就是 玩家2(PvP) 或 電腦(PvC) 的回合 

int timer;

int Selection;




void Menu();

void Game();
char NextGameInit();

int GetPlayerPosition();
void PrintRestPosition();

int CheckWin();

void pop(int*, int&, int);

void gotoxy(int, int);
void draw();

void AIMove_Easy();
void AIMove_Normal();
void AIMove_HardMode();




int main()
{
	srand((unsigned)time(NULL));
	
	Section = MainMenu;
	
	while(true)
	{
		WinTimes = 0;
		TieTimes = 0;
		LoseTimes = 0;
		Selection = 0;
		
		if(Section == MainMenu)
			Menu();
		
		if(Section == Playing)
			Game();
			
		if(Section == Exit)
			break;
	}
	
	system("cls");
	gotoxy(30, 12);
	printf("感謝遊玩~~  m(_ _)m");
	
	gotoxy(0, 23);
	system("pause");
	
	return 0;
}




void Menu()
{
	timer = -1;
	while(true)
	{
		if(!kbhit())
		{
			timer = timer == 119 ? 0 : timer + 1;
			
			if(timer % 60 == 0)
				draw();
			
			Sleep(10);
		}
		else
		{
			timer = 0;
			
			int c = getch();//  用 int 不用 char 是為了要讀取方向鍵 ↑←→↓，宣告為 char 的話讀取會溢位 
							//  備註: 1 個方向鍵會有 2 個輸入 
			
			if(c == 224)//  方向鍵 
			{
				c = getch();
				
				//  72 是上， 75 是左 
				if(c == 72 || c == 75)
					Selection--;
				//  77 是右， 80 是下 
				if(c == 77 || c == 80)
					Selection++;
			}
			else
			{ 
				if(c == 'w' || c == 'W' || c == 'a' || c == 'A')
					Selection--;
				else if(c == 's' || c == 'S' || c == 'd' || c == 'D')
					Selection++;
				else if(c != 13)
					continue;
			}
			
			if(Selection < 0)
				Selection = 2;
			if(Selection > 2)
				Selection = 0;
			
			if(c == 13)
			{
				if(Section == MainMenu)
				{
					GameMode = Selection;
					
					if(GameMode == Mode_PvC)
						Section = DifficultySelection;
					else if(GameMode == Mode_PvP)
						Section = Playing;
					else
						Section = Exit;
				}
				else if(Section == DifficultySelection)
				{
					AIDifficulty = Selection;
					Section = Playing;
				}
				
				Selection = 0;
				
				if(Section == Playing || Section == Exit)
					break;
					
			}
			
			//  在這邊加入這個 draw() 是為了要立即刷新畫面，
			//  避免按下 Enter 後還要等個 0.6 秒畫面才更新 
			system("cls");
			draw();
		}
	}
}




void Game()
{
	
	while(true)
	{
		
		Player1_Mark = NextGameInit();
		
		if(Player1_Mark == 0)
		{
			system("cls");
			Section = MainMenu;
			return;
		}
		if(Player1_Mark == 'O')
		{
			turn = 1;
			Player2_Mark = 'X';
		}
		else
		{
			turn = 0;
			Player2_Mark = 'O';
		}
		
		
		bool end = false;
		
		while(!end)
		{
			draw();
			
			//Computer's turn
			if(Round % 2 == turn)
			{
				if(GameMode == Mode_PvP)
					Tmp[Round] = GetPlayerPosition();
					
				if(GameMode == Mode_PvC)
				{
					gotoxy( 0, 16);
					printf("電腦的回合...");
					
					Sleep(600);
					
					if(AIDifficulty == Hard)
						AIMove_HardMode();
					if(AIDifficulty == Normal)
						AIMove_Normal();
					if(AIDifficulty == Easy)
						AIMove_Easy();
				}
				
				for(int i = 0; i < 4; i++)
					Score[PosToLineTable[Tmp[Round]][i]]++;
				
				TictactoeTable[Tmp[Round]] = Player2_Mark;
			}
			else
			{
				//plaryer1's turn
				Tmp[Round] = GetPlayerPosition();
				
				for(int i = 0; i < 4; i++)
					Score[PosToLineTable[Tmp[Round]][i]]--;
				
				TictactoeTable[Tmp[Round]] = Player1_Mark;
			}
			
			
			for(int i = 0; i < 4; i++)
				LineCount[PosToLineTable[Tmp[Round]][i]]++;
			
			if(Tmp[Round] % 2 == 1)
				pop(RestSide, RestSideNum, Tmp[Round]);
			else if(Tmp[Round] != 4)
				pop(RestCorner, RestCornerNum, Tmp[Round]);
			
			pop(Rest, RestNum, Tmp[Round]);
			
			end = (GameStatus = CheckWin()) != Continue ? true : false;
			
			Round++;
		}
		
		
		switch(GameStatus)
		{
			case Win:
				WinTimes++;
				
				draw();
				
				gotoxy( 0, 18);
				if(GameMode == Mode_PvP)
					printf("玩家2贏了!\n");
					
				if(GameMode == Mode_PvC)
					printf("你輸了!!\n");
					
				break;
				
			case Tie:
				TieTimes++;
				
				draw();
				
				gotoxy( 0, 18);
				printf("平手!\n");
				
				break;
				
			case Lose:
				LoseTimes++;
				
				draw();
				
				gotoxy( 0, 18);
				if(GameMode == Mode_PvP)
					printf("玩家1贏了!\n");
					
				if(GameMode == Mode_PvC)
					printf("你贏了!!!\n");
					
				break;
		}
		
		gotoxy( 0, 19);
		system("pause");
	}
	
}




char NextGameInit()
{
	
	Round = 0;
	
	RestNum = 9;
	for(int i = 0; i < 9; i++)
		Rest[i] = i;
	
	RestCornerNum = 4;
	for(int i = 0; i < 4; i++)
		RestCorner[i] = Corner[i];
	
	RestSideNum = 4;
	for(int i = 0; i < 4; i++)
		RestSide[i] = Side[i];
	
	for(int i = 0; i < 9; i++)
		TictactoeTable[i] = ' ';
	
	memset(LineCount, 0, sizeof(LineCount));
	memset(Score, 0, sizeof(Score));
	
	
	char tmp = 0;
	while(true)
	{
		system("cls");
		draw();
		
		gotoxy( 0, 15);
		printf("請輸入你(玩家1)想當的一方(O或X，如果不想玩請輸入0):");
		
		fflush(stdin);
		scanf("%[OoXx0]c", &tmp);
		
		if(tmp == 'o' || tmp == 'O' || tmp == 'x' || tmp == 'X' || tmp == '0')
			break;
			
		printf("%c", 7);
	}
	
	if(tmp == 'O' || tmp == 'o')
		return 'O';
	
	if(tmp == 'X' || tmp == 'x')
		return 'X';
	
	return 0;
}


int GetPlayerPosition()
{
	char pos;
	
	while(true)
	{
		if(Round != 0)
		{
			
			gotoxy( 0, 15);
			if(Round % 2 != turn)
			{
				if(GameMode == Mode_PvP)
					printf("剛剛玩家2下在...");
				if(GameMode == Mode_PvC)
					printf("剛剛電腦下在...");
			}
			else
			{
				if(GameMode == Mode_PvP)
					printf("剛剛玩家1下在...");
				if(GameMode == Mode_PvC)
					printf("剛剛你下在...");
			}
			printf("%d\n", Tmp[Round-1]);
		}
		
		
		gotoxy( 0, 16);
		if(Round % 2 == turn)
			printf("玩家2的回合!");
		else
		{
			if(GameMode == Mode_PvP)
				printf("玩家1的回合!");
			if(GameMode == Mode_PvC)
				printf("你的回合!");
		}
		
		
		gotoxy( 0, 17);
		printf("請選擇你要下在哪裡(");
		PrintRestPosition();
		printf("):");
		
		
		fflush(stdin);
		scanf(" %[0-8]c", &pos);
		
		
		pos -= '0';
		
		
		system("cls");
		draw();
		
		
		if(pos < 0 || pos > 8)
		{
			gotoxy( 29, 15);
			printf("%c請輸入0~8之間的數字!", 7);
			continue;
		}
		
		if(TictactoeTable[pos] == ' ')
			break;
		
		gotoxy( 29, 15);
		printf("%c你不可以下在這裡!", 7);
	}
	return (pos);
}


void PrintRestPosition()
{
	bool continuous = false;
	for(int i = 0; i < RestNum; i++)
	{
		if(Rest[i] + 1 == Rest[i + 1])
		{
			if(continuous)
				continue;
			
			if(i != 0)
				printf(", ");
			
			printf("%d~", Rest[i]);
			continuous = true;
		}
		else if(continuous)
		{
			printf("%d", Rest[i]);
			continuous = false;
		}
		else
		{
			if(i != 0)
				printf(", ");
			printf("%d", Rest[i]);
		}
	}
}


int CheckWin()
{
	for(int i = 0; i < 8; i++)
	{
		if(Score[i] == 3)
			return Win;
	}
	
	for(int i = 0; i < 8; i++)
	{
		if(Score[i] == -3)
			return Lose;
	}
	
	if(Round == 8)
		return Tie;
	
	return Continue;
}


void pop(int *arr, int &restn, int n)
{
	int tmp = 9;
	for(int i = 0; i < restn; i++)
	{
		if(i >= tmp)
		{
			arr[i] = arr[i+1];
			continue;
		}
		if(arr[i] == n)
			tmp = i--;
	}
	--restn;
}


void gotoxy(int xpos, int ypos)
{
	COORD scrn;
	HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
	scrn.X = xpos;
	scrn.Y = ypos;
	SetConsoleCursorPosition(hOuput,scrn);
}


void draw()
{
	int AIColor[4] = {FOREGROUND_GREEN, FOREGROUND_GREEN | FOREGROUND_BLUE, FOREGROUND_RED, FOREGROUND_RED};
	char selection[2][4][20] = {{"玩家 vs 玩家\0", "玩家 vs 電腦\0", "結束遊戲\0", "\0"}, {"簡單\0", "普通\0", "困難\0", "困難\0"}};
	if(Section == MainMenu || Section == DifficultySelection)
	{
		int Selection_pos_y[2][3] = {{12, 15, 18}, { 6, 12, 18}};
		int Selection_len[2][3] = {{6, 6, 4}, {2, 2, 2}};
		
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		if(Section == MainMenu)
		{
			gotoxy( 0, 3);
			printf("          *****  ***  *****  *****    *    *****  *****   ***   *****  \n");
			printf("            *     *   *        *     * *   *        *    *   *  *      \n");
			printf("            *     *   *        *    *****  *        *    *   *  *****  \n");
			printf("            *     *   *        *    *   *  *        *    *   *  *      \n");
			printf("            *    ***  *****    *    *   *  *****    *     ***   *****  \n");
		}
		
		for(int i = 0; i < 3; i++)
		{
			gotoxy(0, Selection_pos_y[Section][i]);
			printf("                                                                              ");
		}
		
		for(int i = 0; i < 3; i++)
		{
			if(Section == MainMenu)
				if(Selection == i)
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
					gotoxy(39-Selection_len[Section][i]-4+timer/60, Selection_pos_y[Section][i]);
					printf("%c", 26);
					gotoxy(39+Selection_len[Section][i]+3-timer/60, Selection_pos_y[Section][i]);
					printf("%c", 27);
				}
				else
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			
			if(Section == DifficultySelection)
			{
				if(Selection == i)
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | AIColor[i]);
					gotoxy(39-Selection_len[Section][i]-4+timer/60, Selection_pos_y[Section][i]);
					printf("%c", 26);
					gotoxy(39+Selection_len[Section][i]+3-timer/60, Selection_pos_y[Section][i]);
					printf("%c", 27);
				}
				else
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), AIColor[i]);	
			}
				
			gotoxy(39-Selection_len[Section][i], Selection_pos_y[Section][i]);
			printf("%s", selection[Section][i]);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		gotoxy( 34, 22);
		printf("請使用W,S,A,D或方向鍵來選擇模式 按Enter鍵確認\n");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	else if(Section == Playing)
	{
		system("cls");
		
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		
		gotoxy(11, 6);
		printf("對應位置:");
		gotoxy(13, 8);
		printf("0 1 2");
		gotoxy(13, 10);
		printf("3 4 5");
		gotoxy(13, 12);
		printf("6 7 8");
		
		gotoxy(34, 8);
		printf(" | | ");
		gotoxy(34, 9);
		printf("-----");
		gotoxy(34, 10);
		printf(" | | ");
		gotoxy(34, 11);
		printf("-----");
		gotoxy(34, 12);
		printf(" | | ");
		
		for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 3; j++)
			{
				gotoxy(34 + j * 2, 8 + i * 2);
				if(TictactoeTable[i*3+j] == 'O')
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
					printf("O");
				}
				else if(TictactoeTable[i*3+j] == 'X')
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
					printf("X");
				}
			}
		}
		
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		if(GameMode == Mode_PvP)
		{
			gotoxy( 51, 8);
			printf("玩家1贏的次數:%4d", LoseTimes);
			gotoxy( 51, 10);
			printf("玩家2贏的次數:%4d", WinTimes);
			gotoxy( 51, 12);
			printf("平手次數:%4d", TieTimes);
			gotoxy(31, 4);
			printf("玩家 vs 玩家");
		}
		else if(GameMode == Mode_PvC)
		{
			gotoxy( 54, 8);
			printf("你贏的次數:%4d", LoseTimes);
			gotoxy( 54, 10);
			printf("你輸的次數:%4d", WinTimes);
			gotoxy( 54, 12);
			printf("平手次數:%4d", TieTimes);
			gotoxy(29, 5);
			printf("玩家 vs 電腦(    )");
			gotoxy(42, 5);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | AIColor[AIDifficulty]);
			printf("%s", selection[1][AIDifficulty]);
		}
		
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}


void AIMove_Easy()
{
	Tmp[Round] = Rest[rand() % RestNum];
}


void AIMove_Normal()
{
	for(int i = 0; i < 8; i++)
	{
		if(Score[i] == 2)
			for(int j = 0; j < 3; j++)
				if(TictactoeTable[LineToPosTable[i][j]] != 'O' && TictactoeTable[LineToPosTable[i][j]] != 'X')
				{
					Tmp[Round] = LineToPosTable[i][j];
					return;
				}
	}
	
	for(int i = 0; i < 8; i++)
	{
		if(Score[i] == -2)
			for(int j = 0; j < 3; j++)
				if(TictactoeTable[LineToPosTable[i][j]] != 'O' && TictactoeTable[LineToPosTable[i][j]] != 'X')
				{
					Tmp[Round] = LineToPosTable[i][j];
					return;
				}
	}
	
	if(AIDifficulty == Hard)
	{
		for(int i = 0; i < RestCornerNum; i++)
			if(LineCount[PosToLineTable[i][0]] == 1 && LineCount[PosToLineTable[i][1]] == 1 && Score[PosToLineTable[i][0]] == -1 && Score[PosToLineTable[i][1]] == -1)
			{
				Tmp[Round] = RestCorner[i];
				return;
			}
	}
	
	AIMove_Easy();
}


void AIMove_HardMode()
{
	switch(Round)
	{
		//  AI 先 
		case 0:
			if(rand() % 10 < 6)
				Tmp[0] = Mid;
			else
				Tmp[0] = RestCorner[rand() % 4];
			
			return;
			
		case 2:
			if(Tmp[0] != Mid)
			{
				//  O . .
				//  . x .
				//  . . 1
				if(Tmp[1] == Mid)
					Tmp[Round] = 8 - Tmp[0];
				
				//  O x .      O . x
				//  . . .  or  . . .
				//  1 . .      1 . .
				else if(Tmp[1] != Mid)
					for(int i = 0; i < 2; i++)
					{
						if(LineCount[PosToLineTable[Tmp[0]][i]] == 1)
							for(int j = 0; j < RestCornerNum; j++)
								if(PosToLineTable[Tmp[0]][i] == PosToLineTable[RestCorner[j]][i])
								{
									Tmp[Round] = RestCorner[j];
									return;
								}
					}
			}
			else if(Tmp[0] == Mid)
			{ 
				//  x . .
				//  . O .
				//  . . 1
				if(Tmp[1] % 2 == 0)
					Tmp[Round] = 8 - Tmp[1];
				
				//  . x .
				//  . O .
				//  1 . 2
				else if(Tmp[1] % 2 != 0)
					for(int i = 0; i < RestCornerNum; i++)
					{
						if(LineCount[PosToLineTable[RestCorner[i]][0]] == LineCount[PosToLineTable[RestCorner[i]][1]])
						{
							Tmp[Round] = RestCorner[i];
							return;
						}
					}
			}
			
			return;
			
		case 4:
			if(Tmp[0] != Mid)
			{
				if(Tmp[1] != Mid)
				{ 
					//  O x .      O . x
					//  1 . x  or  1 . .
					//  O . .      O . x
					if(TictactoeTable[(Tmp[0] + Tmp[2]) / 2] != 'X')
						Tmp[Round] = (Tmp[0] + Tmp[2]) / 2;
					//  O . x
					//  x . .
					//  O . 1
					else if(RestCornerNum == 1)
						Tmp[Round] = RestCorner[0];
					//  O x .
					//  x 1 .
					//  O . .
					else
						Tmp[Round] = Mid;
					
					return;
				} 
				//  O . x      O x .
				//  . x .  or  . x .
				//  1 . O      . 1 O
				else
				{
					AIMove_Normal();
					return;
				}
			}
			//  
			//  
			//  
			else if(Tmp[1] * Tmp[3] % 2 == 1)
			{
				for(int i = 0; i < 8; i++)
					if(Score[i] == 2)
						for(int j = 0; j < 3; j++)
							if(TictactoeTable[LineToPosTable[i][j]] != 'O' && TictactoeTable[LineToPosTable[i][j]] != 'X')
							{
								Tmp[Round] = LineToPosTable[i][j];
								return;
							}
			}
			else
			{
				//  x a a
				//  a O b
				//  a b O
				//
				//  a :
				for(int i = 0; i < 8; i++)
				{ 
					if(Score[i] == -2)
						for(int j = 0; j < 3; j++)
							if(TictactoeTable[LineToPosTable[i][j]] != 'O' && TictactoeTable[LineToPosTable[i][j]] != 'X')
							{
								Tmp[Round] = LineToPosTable[i][j];
								return;
							}
				} 
				//  b :
				for(int i = 0; i < 2; i++)
				{ 
					if(LineCount[PosToLineTable[Tmp[2]][i]] == 1)
						for(int j = 0; j < RestCornerNum; j++)
							if(PosToLineTable[Tmp[2]][i] == PosToLineTable[RestCorner[j]][i])
							{
								Tmp[Round] = RestCorner[j];
								return;
							}
				} 
			}
			return;
		//  AI 後 
		case 1:
			//  1 . 2
			//  . o .
			//  3 . 4
			if(Tmp[0] == Mid)
				Tmp[Round] = RestCorner[rand() % RestCornerNum];
			//  o . .
			//  . 1 .
			//  . . .
			else if(Tmp[0] != Mid) 
				Tmp[Round] = Mid;
				
			return;
			
		case 3:
			//	o . .		. . o
			//	. X .  or	. X .
			//	. . o		o . .
			//--------
			//	o 1 .
			//	2 X 3
			//	. 4 o
			if(Tmp[0] % 2 != 1 && Tmp[0] + Tmp[2] == 8)
			{
				Tmp[Round] = RestSide[rand() % RestSideNum];
				return;
			}
			//	X . .       . . X        o . .        . . o
			//	. o .   or  . o .   or   . o .   or   . o .
			//	. . o       o . .        . . X        X . .
			//--------
			//	X . 1
			//	. o .
			//	2 . o
			else if(Tmp[0] == Mid && Tmp[1] + Tmp[2] == 8)
			{
				Tmp[Round] = RestCorner[rand() % RestCornerNum];
				return;
			}
			//  To avoid   ==>   o o .
			//  	               o X .
			//  	               . . X   ...
			else if(Tmp[0] * Tmp[2] % 2 == 1 && Tmp[0] + Tmp[2]  != 8)
			{
				Tmp[Round] = Tmp[0] + Tmp[2] - 4;
				return;
			}
			//  To avoid   ==>   o o .
			//                   . X .
			//                   o . X   ...
			else
			{
				int tmp1, tmp2;
				if(Tmp[0] > Tmp[2])
				{
					tmp1 = Tmp[2];
					tmp2 = Tmp[0];
				}
				else
				{
					tmp1 = Tmp[0];
					tmp2 = Tmp[2];
				}
				for(int i = 0; i < 2; i++)
				{ 
					if(SpecialTable[tmp1][i] == -1)
						break;
					else if(SpecialTable[tmp1][i] == tmp2)
					{
						Tmp[Round] = SpecialTable[tmp1][i + 2];
						return;
					}
				}
			}
			AIMove_Normal();
			return;
			
		default:
			AIMove_Normal();
			return;
	}
}

