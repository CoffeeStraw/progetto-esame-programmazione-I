/******************************************************************************/
/*!
 * @file   gamelib.h
 * @author Antonio Strippoli
 * @date   December, 2017
 * @brief  Header file of gamelib.c
 */
/******************************************************************************/

// Using an #include guard to prevent double definitions
#ifndef GAMELIB_H_INCLUDED
#define GAMELIB_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FALSE 0
#define TRUE  !(FALSE)

// Enums used to improve the code readability
typedef enum {DEAD, INJURED, ALIVE}                                         PlayerState;
typedef enum {KITCHEN, LIVING_ROOM, SHED, STREET, ALONG_LAKE, EXIT_CAMPING} TypeZone;
typedef enum {JUNK, BANDAGE, KNIFE, GUN, GASOLINE, ADRENALINE, NOTHING}     ObjType;

typedef struct zone {
    unsigned char ID;
    TypeZone      type;
    ObjType       object;
    struct zone*  next_zone;
} Zone;

typedef struct player {
    PlayerState    state;
    Zone*          pos;
    unsigned short backpack[6];
    int            obj_count;
    unsigned char  searched;
} Player;

// Main menu functions
void newGame();
void loadGame();
void closeGame();

char  g_ans;  /**<Global variable used to take an answer s/n from the user. */
int   g_menu; /**<Global variable used to navigate through the menues. */

char* concat   (const char*, const char*);
int   getValue (int, int);
char  getAns   ();
void  waitEnter();

void  textFramed(const char* text);
void  textFramedSub(const char* text);

#endif
