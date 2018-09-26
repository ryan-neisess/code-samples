#ifndef DIGITAL_MUSIC_MANAGER_H
#define DIGITAL_MUSIC_MANAGER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

typedef struct length {
	int minutes;
	int seconds;
} Length;

typedef struct record {
	char artist[100];
	char album[100];
	char title[100];
	char genre[40];
	Length duration;
	int plays;
	int rating;
	struct record *pNext;
	struct record *pPrev;
} Record;

void loadData(Record **pHead);

void storeData(Record **pHead);

void printData(Record **pHead);

int determineSearchValue(int choice, Record pTemp, Record pCurr);

void sortData(Record **pHead);

void insertData(Record **pHead);

Record *searchList(Record **pHead);

void deleteData(Record **pHead);

void editData(Record **pHead);

void rateSong(Record **pHead);

void exitDMM(Record **pHead);

void openMenu(void);

#endif