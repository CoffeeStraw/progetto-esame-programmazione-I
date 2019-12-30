/******************************************************************************/
 /*!
  * @file   main.c
  * @author Antonio Strippoli
  * @date   December, 2017
  * @brief  Main file of the project
  */
/******************************************************************************/
#include "gamelib.h"

int main(int argc, char const *argv[])
{
    srand(time(NULL)); // Starting my random generator, generating the seed
    do {
        system("clear");

        printf("   ___ _                         ___           _ _            \n"
               "  / _ (_) ___  ___  ___  _ __   / __\\_ _ _   _| | |_         \n"
               " / /_\\/ |/ _ \\/ __|/ _ \\| '_ \\ / _\\/ _` | | | | | __|    \n"
               "/ /_\\\\| |  __/\\__ \\ (_) | | | / / | (_| | |_| | | |_      \n"
               "\\____/|_|\\___||___/\\___/|_| |_\\/   \\__,_|\\__,_|_|\\__|\n\n");

        printf("1) Nuova Partita     \n"
               "2) Carica Partita    \n"
               "0) Esci dal gioco\n\n");

        printf("La tua scelta: ");
        g_menu = getValue(0,2);

        switch(g_menu)
        {
            case 1:
                newGame();
                break;
            case 2:
                loadGame();
                break;
        }
    } while(g_menu != 0);
    closeGame();
    return 0;
}
