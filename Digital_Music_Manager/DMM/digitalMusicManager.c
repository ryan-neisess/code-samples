#include "digitalMusicManager.h"

// Includes specialized makeNode functionality; creates Records based on the number of data sets stored
void loadData(Record **pHead) {
	FILE *infile = NULL;
	Record *pMem = NULL, *pTemp = NULL;
	char newLine = '\0';
	int i = 0;
	
	infile = fopen("savedList.txt", "r");
	for (i = 0; !feof(infile); i++) {
		// allocates memory and initializes pointers
		pMem = (Record *)malloc(sizeof(Record));
		pMem->pNext = NULL;
		pMem->pPrev = NULL;

		// initializing of data fields for the new node
		fgets(pMem->artist, 100, infile);
		fgets(pMem->album, 100, infile);
		fgets(pMem->title, 100, infile);
		fgets(pMem->genre, 40, infile);
		fscanf(infile, "%d", &(pMem->duration.minutes));
		fscanf(infile, "%d", &(pMem->duration.seconds));
		fscanf(infile, "%d", &(pMem->plays));
		fscanf(infile, "%d", &(pMem->rating));
		fscanf(infile, "%c", &newLine);
		fscanf(infile, "%c", &newLine);

		if (i == 0) { // list is empty, Record is the first item to go in the list
			*pHead = pMem;
			pTemp = *pHead; // need to do this for the second and later reads from the file to work
			pMem->pNext = *pHead;
			pMem->pPrev = *pHead;
		}
		else { // list already contains one item, Record is n > 1, inserts at end; pTemp must be set
			pTemp->pNext = pMem;
			(*pHead)->pPrev = pMem;
			pMem->pNext = *pHead;
			pMem->pPrev = pTemp;
			pTemp = pMem; //advances pTemp to current Record (pMem) after pMem's pointers are assigned
		}
	} // concludes for loop
	fclose(infile);
}

void storeData(Record **pHead) {
	FILE *infile = NULL;
	Record *pCurr = NULL;

	infile = fopen("currentList.txt", "w");
	pCurr = *pHead; // sets pMem at the front of the list
	do {
		fputs(pCurr->artist, infile);
		fputs(pCurr->album, infile);
		fputs(pCurr->title, infile);
		fputs(pCurr->genre, infile);
		fprintf(infile, "%d:%d\n", pCurr->duration.minutes, pCurr->duration.seconds);
		fprintf(infile, "%d\n", pCurr->plays);
		fprintf(infile, "%d\n\n", pCurr->rating);
		pCurr = pCurr->pNext;
	} while (pCurr->pNext != (*pHead)->pNext);
	fclose(infile);
}

void printData(Record **pHead) {
	Record *pCurr = NULL;

	putchar('\n');
	putchar('\n');
	printf("Music information is listed in the following format:\n"
		"\tArtist\n"
		"\tAlbum\n"
		"\tTitle\n"
		"\tGenre\n"
		"\tSong Length\n"
		"\tNumber of Plays\n"
		"\tRating\n\n");
	system("pause");
	putchar('\n');
	pCurr = *pHead;
	// for (pMem = *pHead; pMem->pNext != (*pHead)->pNext; pMem = pMem->pNext) {
	do{
		/* initializes pMem to the start of the list; runs until pMem's next pointer is the same as the 
		next pointer of the node pHead points to (meaning the nodes are the same and the program is back 
		at the "beginning" of the circular doubly linked list; advances pMem to the next Record */
		printf("\t%s", pCurr->artist);
		Sleep(25);
		printf("\t%s", pCurr->album);
		Sleep(25);
		printf("\t%s", pCurr->title);
		Sleep(25);
		printf("\t%s", pCurr->genre);
		Sleep(25);
		printf("\t%d:%d\n", pCurr->duration.minutes, pCurr->duration.seconds);
		Sleep(25);
		printf("\t%d\n", pCurr->plays);
		Sleep(25);
		printf("\t%d\n\n", pCurr->rating);
		Sleep(25);
		pCurr = pCurr->pNext;
	} while (pCurr->pNext != (*pHead)->pNext);
}

// Determines which item in a list is greater or lesser based on rating, artist, album, title, or genre
int determineSearchValue(int choice, Record pTemp, Record pCurr) {
	int comp = 0;

	switch (choice) {
	case 4: // sorting ascending based on rating integer value
		if (pTemp.rating > pCurr.rating) {
			comp = 1;
		}
		else if (pTemp.rating < pCurr.rating) {
			comp = -1;
		}
		// if the ratings are equal, comparison will be based on artist, code below
	case 0: // sorting ascending based on ASCII values for alphanumerics, artist
	default:
		comp = strcmp(pTemp.artist, pCurr.artist);
		break;
	case 1: // sorting ascending based on ASCII values for alphanumerics, album
		comp = strcmp(pTemp.album, pCurr.album);
		break;
	case 2: // sorting ascending based on ASCII values for alphanumerics, title
		comp = strcmp(pTemp.title, pCurr.title);
		break;
	case 3: // sorting ascending based on ASCII values for alphanumerics, genre
		comp = strcmp(pTemp.genre, pCurr.genre);
		break;
	}
	return comp;
}

// Sorts the data 
void sortData(Record **pHead) {
	Record *pCurr = NULL, *pTemp = NULL, *pSub = NULL;
	int choice = 0, i = 0, comp = 0, left = 0, count = 0;

	printf("\nEnter a number corresponding to the criteria to sort by:\n"
		"(Note that sorting of strings is performed based on ASCII value)\n"
		"\t0 - Artist, 1 - Album, 2 - Title, 3 - Genre, 4 - Rating\n\n");
	scanf(" %d", &choice);

	// counts the number of items in the list
	pCurr = *pHead;
	i = 1;
	do {
		pCurr = pCurr->pNext;
		i++;
	} while (pCurr->pNext != (*pHead)->pNext);

	do {
		// returns to the front of the list, establishes first two items to be compared
		pCurr = *pHead;
		pTemp = pCurr;
		pCurr = pCurr->pNext;

		// compares two items at a time until the end of the list is reached
		count = 1;
		while ((pCurr->pNext != (*pHead)->pNext) && (count < i)) {
			comp = determineSearchValue(choice, *pTemp, *pCurr);
			if (comp > 0) { // swaps the items if left > right
				// swaps markers
				pSub = pTemp;
				pTemp = pCurr;
				pCurr = pSub;

				// swaps pointers
				pCurr->pPrev->pNext = pTemp;
				pTemp->pNext->pPrev = pCurr;

				pTemp->pPrev = pCurr->pPrev;
				pCurr->pPrev = pTemp;

				pCurr->pNext = pTemp->pNext;
				pTemp->pNext = pCurr;

				if (count == 1) { // sets pHead to the new "start" of the list, only on first comparison
					*pHead = pTemp;
				}
			}
			// advances the markers to the next items to be compared
			pTemp = pCurr;
			pCurr = pCurr->pNext;
			count++;
		} // concludes while loop
		i--;
		if (count > left) {
			// executes only the first time after running through the list; establishes lenth of list
			left = count;
		}
		left--;
	} while (left > 1);

	/* Attempted code to perform bubble sort on a doubly linked list without passing through once first
	
	do {
		// returns to the front of the list, establishes first two items to be compared
		pCurr = *pHead;
		pTemp = pCurr;
		pCurr = pCurr->pNext;

		// compares two items at a time until the end of the list is reached
		count = 1;
		while(pCurr->pNext != (*pHead)->pNext){
			switch (choice) {
				case 0: // sorting ascending based on ASCII values for alphanumerics
					comp = strcmp(pTemp->artist, pCurr->artist);
					break;
			}
			if (comp > 0) { // swaps the items if left > right
				// swaps markers
				pSub = pTemp;
				pTemp = pCurr;
				pCurr = pSub;

				// swaps pointers
				pCurr->pPrev->pNext = pTemp;
				pTemp->pNext->pPrev = pCurr;

				pTemp->pPrev = pCurr->pPrev;
				pCurr->pPrev = pTemp;

				pCurr->pNext = pTemp->pNext;
				pTemp->pNext = pCurr;
				
				if (count == 1) { // sets pHead to the new "start" of the list, only on first comparison
					*pHead = pTemp;
				}
			}			
			// advances the markers to the next items to be compared
			pTemp = pCurr;
			pCurr = pCurr->pNext;
			count++;
		} // concludes while loop
		if (count > left) {
			// executes only the first time after running through the list; establishes lenth of list
			left = count;
		}
		left--;
	} while (left > 1);
	*/
}

// Performs insert at front w/ custom makeNode functionality then sorts the data
void insertData(Record **pHead) {
	Record *pMem = NULL, *pTemp = NULL;
	char newLine = '\0';

	// allocates memory and initializes pointers
	pMem = (Record *)malloc(sizeof(Record));
	pMem->pNext = NULL;
	pMem->pPrev = NULL;

	printf("Enter the new information in the following order:\n"
		"\tArtist\n"
		"\tAlbum\n"
		"\tTitle\n"
		"\tGenre\n"
		"\tSong Length - Minutes\n"
		"\tSong Length - Seconds\n"
		"\tNumber of Plays\n"
		"\tRating\n\n");

	// initializing of data fields for the new node
	//fflush(stdin); --> why doesn't this work?
	scanf("%c", &newLine);
	fgets(pMem->artist, 100, stdin);
	fgets(pMem->album, 100, stdin);
	fgets(pMem->title, 100, stdin);
	fgets(pMem->genre, 40, stdin);
	scanf(" %d", &(pMem->duration.minutes));
	scanf(" %d", &(pMem->duration.seconds));
	scanf(" %d", &(pMem->plays));
	scanf(" %d", &(pMem->rating));

	// insertion of node at front
	pMem->pNext = *pHead;
	pMem->pPrev = (*pHead)->pPrev;
	(*pHead)->pPrev->pNext = pMem;
	(*pHead)->pPrev = pMem;
	*pHead = pMem;

	sortData(pHead);
} // --> Consider instead insert in order


Record *searchList(Record **pHead) {
	Record *pCurr = NULL;
	int choice = 0, num1 = 0, num2 = 0, found = 0;
	char string[100] = {0}, newLine = '\0';

	//scanf("%c", &newLine);
	// Assumes user enters valid input
	printf("What parameter do you want to use to search? Provide a numerical selection.\n"
		"\t0 - Artist, 1 - Album, 2 - Title, 3 - Genre, 4 - Rating\n"
		"\t5 - Number of Plays, 6 - Duration\n\n");
	scanf(" %d", &choice);
	printf("What are you searching for? Enter a string or integer, or for duration, \n"
		"enter an integer for the minutes followed by an integer for the seconds.\n\n\t");
	if (choice < 4) {
		scanf("%c", &newLine);
		fgets(string, 100, stdin);
	}
	else if ((choice == 4) || (choice == 5)) {
		scanf("%d", &num1);
	}
	else if (choice == 6) {
		scanf("%d%d", &num1, &num2);
	}

	// compares items based on the given criteria
	pCurr = *pHead;
	do {
		switch (choice) {
			case 0:
				if (strcmp(string, pCurr->artist) == 0) {
					found = 1;
					return pCurr;
				}
				break;
			case 1:
				if (strcmp(string, pCurr->album) == 0) {
					found = 1;
					return pCurr;
				}
				break;
			case 2:
				if (strcmp(string, pCurr->title) == 0) {
					found = 1;
					return pCurr;
				}
				break;
			case 3:
				if (strcmp(string, pCurr->genre) == 0) {
					found = 1;
					return pCurr;
				}
				break;
			case 4:
				if (num1 == pCurr->rating) {
					found = 1;
					return pCurr;
				}
				break;
			case 5:
				if (num1 == pCurr->plays) {
					found = 1;
					return pCurr;
				}
				break;
			case 6:
				if ((num1 == pCurr->duration.minutes) && (num2 == pCurr->duration.seconds)) {
					found = 1;
					return pCurr;
				}
				break;
		}
		pCurr = pCurr->pNext;
	} while (pCurr->pNext != (*pHead)->pNext); // runs until back at the beginning of the circular list
	
	return NULL; // executes if data not found
}


void deleteData(Record **pHead) {
	Record * pMem = NULL;

	pMem = searchList(pHead);
	if (pMem != NULL) { // if the desired node was found
		pMem->pPrev->pNext = pMem->pNext;
		pMem->pNext->pPrev = pMem->pPrev;
		if (pMem == (*pHead)) { // in case the node being deleted is the 'front' of the list
			*pHead = pMem->pNext;
		}
		free(pMem);
		printf("Node successfully found and deleted.\n\n");
	}
	else {
		printf("The node was not found.\n\n");
	}
}

// Allows the user to edit any piece of data
void editData(Record **pHead) {
	Record * pMem = NULL;
	int choice = 0, num1 = 0, num2 = 0, found = 0;
	char string[100] = { 0 }, newLine = '\0';

	pMem = searchList(pHead);

	if (pMem == NULL) {
		printf("Data not found.\n\n");
		return;
	}

	// Assumes user enters valid input
	printf("What parameter do you want to use to change? Provide a numerical selection.\n"
		"\t0 - Artist, 1 - Album, 2 - Title, 3 - Genre, 4 - Rating\n"
		"\t5 - Number of Plays, 6 - Duration\n\n");
	scanf(" %d", &choice);
	printf("What is the new data? Enter a string or integer, or for duration, \n"
		"enter an integer for the minutes followed by an integer for the seconds.\n\n\t");
	if (choice < 4) {
		scanf("%c", &newLine);
		fgets(string, 100, stdin);
	}
	else if ((choice == 4) || (choice == 5)) {
		scanf("%d", &num1);
	}
	else if (choice == 6) {
		scanf("%d%d", &num1, &num2);
	}
	// repetitive code, could simplify in the future

	switch (choice) {
		case 0: strcpy(pMem->artist, string);
			break;
		case 1: strcpy(pMem->album, string);
			break;
		case 2: strcpy(pMem->title, string);
			break;
		case 3: strcpy(pMem->genre, string);
			break;
		case 4: pMem->rating = num1;
			break;
		case 5: pMem->plays = num1;
			break;
		case 6: pMem->duration.minutes = num1;
			pMem->duration.seconds = num2;
			break;
	}
}

// Allows the user to edit only the rating
void rateSong(Record **pHead) {
	Record * pMem = NULL;
	int num = 0;

	pMem = searchList(pHead);

	if (pMem == NULL) {
		printf("Data not found.\n\n");
		return;
	}

	printf("Enter the new rating: ");
	scanf("%d", &num);
	pMem->rating = num;
}

// Exits the Digital Music Manager, storing the data to the file before exiting
void exitDMM(Record **pHead) {
	storeData(pHead);
}

// Loads the menu
void openMenu(void) {
	int menuSel = 0;
	Record *pHead = NULL;

	/* Notes: tried to use a recursive call, however pHead was initialized each time as a result; 
	utilized a do while loop instead */
	do {
		printf("Welcome! Please make a selection based on the following menu:\n\n"
			"\t1. Load\n"
			"\t2. Store\n"
			"\t3. Display\n"
			"\t4. Insert\n"
			"\t5. Delete\n"
			"\t6. Edit\n"
			"\t7. Sort\n"
			"\t8. Rate\n"
			"\t9. Exit\n\n"
			"Number of your selection: ");
		scanf("%d", &menuSel);

		if (menuSel == 9) {
			printf("\n Bai bai.\n\n");
			exitDMM(&pHead);
		}
		else {
			switch (menuSel) {
			case 1: loadData(&pHead);
				break;
			case 2: storeData(&pHead);
				break;
			case 3: printData(&pHead);
				break;
			case 4: insertData(&pHead);
				break;
			case 5: deleteData(&pHead);
				break;
			case 6: editData(&pHead);
				break;
			case 7: sortData(&pHead);
				break;
			case 8: rateSong(&pHead);
				break;
			default: printf("\nInvalid selection. Please make another selection.\n\n");
			} // concludes switch
			putchar('\n');
			system("pause");
			system("cls");
			Sleep(500);
		} // concludes else
	} while (menuSel != 9);
}