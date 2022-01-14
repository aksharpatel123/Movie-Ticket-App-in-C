// Akshar Patel
// Movie Ticket selling app made in C.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "QueueLib.h"
#include "BSTLib.h"
#include "ListLib.h"
#include "StackLib.h"

#define ROWALPHA 26
#define ROWNUM 20
#define MAXCOLS 20
#define MAXROWS 26
#define CUSTNAME 30

void get_command_line_parameter(char *argv[], char ParamName[], char ParamValue[]);
int ReadFileIntoQueue(FILE *QueueFile, QNODE **QH, QNODE **QT);
int PrintMenu();
int ReadFileIntoBST(FILE *BSTFile, BNODE **BSTnode);
BNODE *PickAndDisplayTheater(BNODE *BSTRoot, char MovieTheaterMap[][ROWNUM], int *MapRow, int *MapCol);
int ReadMovieTheaterFile(char MovieTheaterMap[][MAXCOLS], char FileName[], int row, int col);
void PrintSeatMap(char MovieTheaterMap[][MAXCOLS], int row, int col);
void WriteMap(BNODE *MyTheater, char MovieTheaterMap[][MAXCOLS], int MapRow, int MapCol);
void PrintReceipts(SNODE **StackTop);

int main(int argc, char *argv[])
{
	int i, j, k;
	FILE *queueFile = NULL;
	FILE *zipFile = NULL;
	char arg_value[20];
	char queuefilename[20];
	char zipfilename[20];
	int ReceiptNumber = 0;
	int choice = 0;
	int SeatNumber;
	char Row;
	char Ticket[5];
	int ArrayRow, ArrayCol;
	int MapRow, MapCol;
	char MovieTheaterMap[MAXROWS][MAXCOLS] = {};
	LNODE *TicketLinkedListHead = NULL;
	QNODE *QueueHead = NULL;
	QNODE *QueueTail = NULL;
	BNODE *BSTRoot = NULL;
	BNODE *MyTheater = NULL;
	SNODE *StackTop = NULL;
	int NumberOfTickets = 0;


	if (argc != 4)
	{
		printf("%s QUEUE=xxxxxx ZIPFILE=xxxxx RECEIPTNUMBER=xxxxx\n", argv[0]);
		exit(0);
	}
	get_command_line_parameter(argv, "QUEUE=", queuefilename);
	get_command_line_parameter(argv, "ZIPFILE=", zipfilename);
	get_command_line_parameter(argv, "RECEIPTNUMBER=", arg_value);

	ReceiptNumber = atoi(arg_value);
	//printf("\n\t ReceiptNumber = %d", ReceiptNumber);

	/* open queuefilename - if it does not open, print message and exit */	


	queueFile = fopen(queuefilename,"r");
	if(queueFile == NULL)
	{
		printf("The file /%s/ could not be opened, exiting...\n", queuefilename);
		exit(0);
	}

	int customerCnt=0;
	customerCnt = ReadFileIntoQueue(queueFile, &QueueHead, &QueueTail);

	// open zipfilename - if it does not open, print message and exit


	zipFile = fopen(zipfilename,"r");
	if(zipFile == NULL)
	{
		printf("The file /%s/ could not be opened, exiting...\n", zipfilename);
		exit(0);
	}
	int BST_Cnt;
	BST_Cnt = ReadFileIntoBST(zipFile, &BSTRoot);


	while (QueueHead != NULL)
	{
		choice = PrintMenu();

		switch (choice)
		{
			case 1 :
				printf("\n\nHello %s\n", QueueHead->name);				
				MyTheater = PickAndDisplayTheater(BSTRoot, MovieTheaterMap, &MapRow, &MapCol);
				if (MyTheater == NULL)
					break;
				printf("\n\nHow many movie tickets do you want to buy? ");
				scanf("%d", &NumberOfTickets);
				for (i = 0; i < NumberOfTickets; i++)
				{
					do
					{
						printf("\nPick a seat (Row Seat) ");
						scanf(" %c%d", &Row, &SeatNumber);
						Row = toupper(Row);
						ArrayRow = (int)Row - 65;
						ArrayCol = SeatNumber - 1;
					
						if ((ArrayRow < 0 || ArrayRow >= MapRow) ||
							(ArrayCol < 0 || ArrayCol >= MapCol))
						{
							printf("\nThat is not a valid seat.  Please choose again\n\n");
						}		
					}
					while ((ArrayRow < 0 || ArrayRow >= MapRow) ||
						   (ArrayCol < 0 || ArrayCol >= MapCol));
					
					if (MovieTheaterMap[ArrayRow][ArrayCol] == 'O')
					{	
						MovieTheaterMap[ArrayRow][ArrayCol] = 'X';
						sprintf(Ticket, "%c%d", Row, SeatNumber); 
						InsertNode(&TicketLinkedListHead, Ticket);
					}
					else
					{
						printf("\nSeat %c%d is not available.\n\n", Row, SeatNumber);
						i--;
					}
					PrintSeatMap(MovieTheaterMap, MapRow, MapCol);
				}
				
				WriteMap(MyTheater, MovieTheaterMap, MapRow, MapCol);
				push(&StackTop, TicketLinkedListHead, ReceiptNumber, MyTheater->MovieTheaterName);
				TicketLinkedListHead = NULL;
				ReceiptNumber++;
				printf("\nThank you %s - enjoy your movie!\n", QueueHead->name);
				deQueue(&QueueHead);
				
				break;
			case 2 : 
				printf("\n\nCustomer Queue\n\n");
				DisplayQueue(QueueHead);
				printf("\n\n");
				break;
			case 3 :
				PickAndDisplayTheater(BSTRoot, MovieTheaterMap, &MapRow, &MapCol);
				break;
			case 4 : 
				PrintReceipts(&StackTop);
				break;
			default :
				printf("Bad menu choice");
		}
	}

	printf("Good job - you sold tickets to all of the customers in line.\n");
	PrintReceipts(&StackTop);
	return 0;
}

void get_command_line_parameter(char *argv[], char ParamName[], char ParamValue[])
{
	int i = 0;
	
	while (argv[++i] != NULL)
	{
		if (!strncmp(argv[i], ParamName, strlen(ParamName)))
		{
			strcpy(ParamValue, strchr(argv[i], '=') + 1);
		}
	}
	
	return;
}

int ReadFileIntoQueue(FILE *QueueFile, QNODE **QH, QNODE **QT)
{
	//	read the passed in file and calls enQueue for each name in the file
	char customerName[CUSTNAME]={};
	int j=0;
	
	while (fgets(customerName, sizeof(customerName)-1, QueueFile))
	{
		enQueue(customerName, QH, QT);
		j++;
	}
	return j;
}


int ReadFileIntoBST(FILE *BSTFile, BNODE **BSTnode)
{
	//	read the passed in file and tokenize each line and pass the information to AddBSTNode
	char MTN[40];
	char ZC[5];
	char FN[100];
	char DIM[5];
	int cnt = 0;

	char *token;
	char info[150];

    while(fgets(info , sizeof(info)-1, BSTFile))
    {

        token = strtok(info, "|");
        strcpy(MTN, token);

        token = strtok(NULL, "|");
        strcpy(ZC, token);

        token = strtok(NULL, "|");
        strcpy(FN, token);

        token = strtok(NULL, "|");
        strcpy(DIM, token);
        
        AddBSTNode(BSTnode, MTN, ZC, FN, DIM);
        cnt++;
    }
    return cnt;
}


int PrintMenu()
{
	int choice = 0;
	
	printf("\n\n1.	Sell tickets to next customer\n\n");
	printf("2.	See who's in line\n\n");
	printf("3.	See the seat map for a given theater\n\n");
	printf("4.	Print today's receipts\n\n");
	printf("Choice : ");
	scanf("%d", &choice);
	
	while (choice < 1 || choice > 4)
	{
		printf("Invalid choice.  Please reenter. ");
		scanf("%d", &choice);
	}
	return choice;
}


BNODE *PickAndDisplayTheater(BNODE *BSTRoot, char MovieTheaterMap[][ROWNUM], int *MapRow, int *MapCol)
{
	BNODE *MyTheater = NULL;
	char zip[6] = {};
	char MyDims[6] = {};	
	
	printf("\n\nPick a theater by entering the zipcode\n\n");
				
	// Traverse the BST in order and print out the theaters in zip code order - use InOrder()
	
	InOrder(BSTRoot); // prints theatre name with its Zipcode

	// Prompt for a zip

	printf("\nEnter zip ");
	scanf(" %s",zip);

	// Call SearchForBNODE()

	MyTheater = SearchForBNODE(BSTRoot, zip);

	// If theater is not found, then print message

	// If theater is found, then tokenize the theater's dimensions and
	// pass those dimensions and the MovieTheaterMap and the theater's filename
	// to ReadMovieTheaterFile()
	if(MyTheater == NULL)
	{
		printf("Theatre not found!!!\n");
		exit(0);
	}
	else
	{
		char *token;
		
		token = strtok(MyTheater->Dimensions, "x");
		strcpy(MyDims,token);
        *MapRow = atoi(MyDims);
        //printf("%d\n", *MapRow);

        token = strtok(NULL, "x");
        strcpy(MyDims,token);
        *MapCol = atoi(MyDims);
        //printf("%d\n", *MapCol);
	}

	// If ReadMovieTheaterFile() returns FALSE, then print
	// "Unable to print that seat map at this time.  Check back later."

	// else call PrintSeatMap()
	int tp;
	tp = ReadMovieTheaterFile(MovieTheaterMap, MyTheater->FileName, *MapRow, *MapCol);
	if(tp == 0)
	{
		printf("Unable to print that seat map at this time.  Check back later.\n");
	}
	else
	{
		PrintSeatMap(MovieTheaterMap, *MapRow, *MapCol);
	}

	// return the found theater node
	return MyTheater;
}

int ReadMovieTheaterFile(char MovieTheaterMap[][MAXCOLS], char FileName[], int row, int col)
{
	FILE *MapFile;
	char MapLine[MAXROWS*MAXCOLS+10]={};
	int i, j, k = 0;
	
	MapFile = fopen(FileName, "r+");
	if (MapFile == NULL)
	{
		printf("\nFile \"%s\" did not open\n\n", FileName);
		return 0;
	}
	
	fgets(MapLine, sizeof(MapLine)-1, MapFile);
	
	if (strlen(MapLine) != row*col)
		return 0;
	
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			MovieTheaterMap[i][j] = MapLine[k++];
		}
	}
	
	fclose(MapFile);
	
	return 1;	
}


void PrintSeatMap(char MovieTheaterMap[][MAXCOLS], int row, int col)
{
	int i, j;

	printf("\n\n\t");
	for (i = 1; i <= col; i++)
	{
		printf("Seat%-3d", i);		
	}
	printf("\n");
	
	for (i = 0; i < row; i++)
	{
		printf("Row %c\t", i+65);
		for (j = 0; j < col; j++)
		{
			printf("%-7c", MovieTheaterMap[i][j]);
		}
		printf("\n");
	}
}



void WriteMap(BNODE *MyTheater, char MovieTheaterMap[][MAXCOLS], int MapRow, int MapCol)
{
	FILE *MapFile;
	int i, j, k = 0;
	char MovieTheaterLine[MapRow*MapCol];
	
	MapFile = fopen(MyTheater->FileName, "w+");
	if (MapFile == NULL)
	{
		printf("\nFile \"%s\" did not open\n\n", MyTheater->FileName);
	}
	
	for (i = 0; i < MapRow; i++)
	{
		for (j = 0; j < MapCol; j++)
		{
			MovieTheaterLine[k++] = MovieTheaterMap[i][j];
		}
	}		
	MovieTheaterLine[k] = '\0';
	
	fputs(MovieTheaterLine, MapFile);
	fclose(MapFile);
}


void PrintReceipts(SNODE **StackTop)
{
	char Ticket[100] = {};
	if(*StackTop == NULL)
	{
		printf("There are no receipts to print!!\n");
	}
	else 
	{
		while((*StackTop)->TicketList != NULL)
		{ 
			printf("Receipt #%d \n\n\t%s\n", (*StackTop)->ReceiptNumber, (*StackTop)->MovieTheaterName);

			while((*StackTop)->TicketList != NULL)
			{
				ReturnAndFreeLinkedListNode(&(*StackTop)->TicketList, Ticket);
				printf("\t%s", Ticket);
			}
			
		}
		pop(StackTop);
	}
}

