#pragma once

/*=======================
*		INPUT 
*=======================*/
#define INPUT_LEN 16

/*=======================
*		TEST
*=======================*/
#define NOT_FOR_TEST	0
#define FOR_TEST	1
#define CHECKPOINT 500000

/*=======================
*		DRAW 
*=======================*/
#define TREE_MAX	INT_MAX
#define TEXT_LEN	15
#define TEXT_PAD	8
#define RADIUS		17

#define X_MAX		2048
#define X_PIVOT		(X_MAX / 3)
#define Y_PIVOT		100
#define Y_GAP		50

/*=======================
*		RETURN	
*=======================*/
#define INSERT_SUCCESS					0
#define INSERT_ERROR_TREE_IS_FULL		1
#define INSERT_ERROR_DUPLICATE_VALUE	2
#define INSERT_ERROR_UNKNOWN			3

#define DELETE_SUCCESS					4
#define DELETE_ERROR_TREE_IS_EMPTY		5
#define DELETE_ERROR_CANT_FIND			6
#define DELETE_ERROR_UNKNOWN			7

#define TEST_SUCCESS					8
#define TEST_ERROR_DOUBLE_RED			9
#define TEST_ERROR_UNBALANCED			10
#define TEST_ERROR_UNKNOWN				11
