/******************************************************************************/
  /*!
   * @file   gamelib.c
   * @author Antonio Strippoli
   * @date   December, 2017
   * @brief  Core of the project
   */
/******************************************************************************/
#include "gamelib.h"

// ------------------------------SETTING VARIABLES------------------------------
static Zone* first_zone = NULL;
static Zone* last_zone  = NULL;

#define MAX_LANDS 7

static int const object_prop [6][6] = {
    {30,20,40, 0, 0,10},
    {20,10,10,30, 0,30},
    {20,10,30, 0,30,10},
    {80, 0,10, 0,10, 0},
    {70, 0,10, 0,20, 0},
    {90, 0,10, 0, 0, 0}
};

static Player P1, P2;

#define BACKPACK_SIZE 4
static unsigned int gasoline_turns = 0;
static unsigned int turn_check     = 0;

// TAGS USED FOR PRINTING ENUMS
static const char* tags_state[3] = {
    "Morto",
    "Ferito",
    "Vivo"
};
static const char* tags_zone[6] = {
    "Cucina",
    "Soggiorno",
    "Rimessa",
    "Strada",
    "Lungo lago",
    "Uscita campeggio"
};
static const char* tags_obj[7] = {
    "Cianfrusaglia",
    "Bende",
    "Coltello",
    "Pistola",
    "Benzina",
    "Adrenalina",
    "Nessuno"
};

// PROTOTYPES OF FUNCTIONS
static void    createMap     ();
static ObjType randomObject  (TypeZone);
static void    addZone       (TypeZone, ObjType);
static void    deleteLastZone();
static void    printZone     (Zone*, unsigned char);
static void    printMap      ();
static void    closeMap      ();
static void    deleteMap     ();

static void    shiftManager  ();
static void    doTurn        (Player*);
static void    progressZone  (Player*);
static void    rummage       (Player*, int*);
static void    takeItem      (Player*, int*);
static void    heal          (Player*, int*);
static void    useAdrenaline (Player*, int*);
static void    craft         (Player*, int*);
static ObjType chooseItem    (unsigned short*);
static void    callGieson    (Player*, int*);
static void    victory       (Player*, int*);
static void    gameOver      (Player*, int*);

static void    setValues     (Player*, Player*, unsigned int, unsigned int);
static void    saveGame      ();
static void    deleteSave    ();

// ---------------------------MAP BUILDING FUNCTIONS----------------------------
/**
 * Manages the creation of the map, informing the player whether he can play the game or not
 * @see addZone
 * @see deleteLastZone
 * @see printMap
 * @see closeMap
 * @see deleteMap
 */
void createMap()
{
    do
    {
        system("clear");

        textFramed("Menù Creazione Mappa");

        if (last_zone != NULL && last_zone->ID >= MAX_LANDS)
            printf("Numero minimo di terre raggiunto!\n");

        printMap(first_zone);

        printf("1) Inserisci una nuova zona\n"
               "2) Rimuovi l'ultima zona\n"
               "3) Termina creazione mappa\n"
               "0) Torna al menù principale\n\n");

        printf("La tua scelta: ");

        g_menu = getValue(0,3);
        switch(g_menu)
        {
            case 1: // New zone
                addZone(-1,-1);
                break;
            case 2: // Deletes zone
                deleteLastZone();
                break;
            case 3: // Closes the route and start the game
                closeMap();
                break;
            case 0: // Returns to the main menu
                g_menu = -1;
        }

    } while(g_menu != -1);
    deleteMap();
}

/**
 * Generates a random object for the zone
 * @param  i Type of the zone for which we have to generate a random object
 * @return   An int value indicating the object choosen
 *
 * <b>Example usage:</b>
 * @code
 *      randomObject(KITCHEN); // Generates a random object for the kitchen
 * @endcode
 */
ObjType randomObject(TypeZone i)
{
    int row       = (sizeof(object_prop)/sizeof(object_prop[0][0])) / (sizeof(object_prop)/sizeof(object_prop[0]));
    int rand_prop = rand()%100 + 1;
    int sum       = 0;

    for(int j = 0; j < row; j++)
    {
        sum += object_prop[i][j];
        if (rand_prop > sum-object_prop[i][j] && rand_prop <= sum && object_prop[i][j] != 0)
            return j;
    }
    // If the function ever reaches this point, there is an error
    fprintf(stderr, "\nSi è verificato un errore imprevisto all'interno della funzione randomObject().\n");
    waitEnter();
    return NOTHING;
}

/**
 * Adds a new zone to the map (linked list)
 * @param type_zone   An enum indicating the type of the zone. If -1, it will ask to the user the type of the land
 * @param object_type An enum indicating the ID of the object. If -1, the randomObject function will be called to generate it
 *
 * <b>Example usage:</b>
 * @code
 *      addZone(KITCHEN, -1); // Appends a zone with type "kitchen" and object generated randomly to the linked list
 * @endcode
 *
 * @see randomObject
 */
void addZone(TypeZone type_zone, ObjType object_type)
{
    Zone* new_zone = (Zone*)malloc(sizeof(Zone));

    if(new_zone == NULL)
    {
        fprintf(stderr, "\nImpossibile allocare ulteriore memoria per la nuova zona.\n");
        exit(-1);
    }

    // Filling new_zone->type
    if(type_zone == -1) // If we want to take the type of the zone from the user
    {
        printf("\n");
        textFramedSub("Creazione Nuova Zona");

        printf("1) Cucina    \n"
               "2) Soggiorno \n"
               "3) Rimessa   \n"
               "4) Strada    \n"
               "5) Lungo lago\n\n");

        printf("La tua scelta: ");
        new_zone->type = getValue(1,5)-1;
    }
    else
        new_zone->type = type_zone;

    // Filling new_zone->object
    if(object_type == -1) // If we want to generate the object randomly
        new_zone->object = randomObject(new_zone->type);
    else
        new_zone->object = object_type;

    new_zone->next_zone = NULL;

    if(first_zone == NULL) // Adding first zone
    {
        new_zone->ID = 1;
        first_zone = new_zone;
        last_zone  = first_zone;
    }
    else                   // Tail insertion
    {
        new_zone->ID = last_zone->ID + 1;
        last_zone->next_zone = new_zone;
        last_zone = last_zone->next_zone;
    }
}

/**
 * Deletes the last zone of the linked list, printing an error message in case of no zone detected
 */
void deleteLastZone()
{
    if (first_zone == NULL)
    {
        printf("\nPrima di poter eliminare una terra devi crearne almeno una.\nPremi INVIO.");
        waitEnter();
    }
    else if (first_zone->next_zone == NULL)
    {
        free(first_zone);
        first_zone  = last_zone = NULL;
    }
    else
    {
        Zone* current = first_zone;

        while (current->next_zone != last_zone)
            current = current->next_zone;

        free(current->next_zone);
        last_zone = current;
        last_zone->next_zone = NULL;
    }
}

/**
 * Closes the map, adding the exit_camping zone, then sets the initial values for two players and the global variables and starts the game
 * @see addZone
 * @see setValues
 * @see saveGame
 * @see shiftManager
 */
void closeMap()
{
    if(last_zone == NULL)
    {
        printf("\nDevi inserire delle zone prima di poter chiudere la mappa.\nPremi INVIO.");
        waitEnter();
    }
    else if (last_zone->ID >= MAX_LANDS)
    {
        printf("__________________________________________________________________________________________________\n\n"
               "Ti piace la mappa che hai creato? Verrà aggiunta automaticamente l'uscita del campeggio in coda.\n\n"
               "NOTA BENE: Il gioco dispone di una funzione di auto-salvataggio, che verrà effettuato al termine\n"
               "di ogni turno di uno dei due giocatori. Al termine della partita il salvataggio verrà rimosso.\n\n"
               "Vuoi cominciare la tua avventura? (s/n): ");
        g_ans = getAns();

        if(g_ans == 's')
        {
            // Closing the map inserting the exit
            addZone(EXIT_CAMPING,-1);
            // Setting the players initial pos and the global variables for the game
            setValues(NULL, NULL, 0, 0);
            // First save of the game
            saveGame();
            // Starting the shift manager
            shiftManager(P1, P2);
        }
    }
    else
    {
        printf("__________________________________________________________________________________________________\n\n"
               "La mappa deve contenere almeno 8 zone. Ricorda che verrà aggiunta automaticamente come ultima zona (da un'eventuale settima in poi) l'uscita del campeggio.\nNe devi inserire almeno altre %d.\nPremi INVIO.", MAX_LANDS-last_zone->ID);
        waitEnter();
    }
}

// -----------------------------PRINTING FUNCTIONS------------------------------
/**
 * Prints the desidered zone, allowing the player to know if there's an object or not
 * @param zone    The zone that we have to print
 * @param obj_vis A boolean value indicating whether we have to visualize the object or not
 */
void printZone(Zone* zone, unsigned char obj_vis)
{
    printf("-> TIPO: %-16s ", tags_zone[zone->type]);

    if(obj_vis)
        printf("| OGGETTO: %s\n", tags_obj[zone->object]);
    else
        printf("| OGGETTO: ???\n");
}

/**
 * Prints a graphical visualization of the linked list, starting from first_zone and calling printZone to print each zone
 * @see printZone
 */
void printMap()
{
    Zone* current = first_zone;

    printf("\nINIZIO-----------------------------------------------\n");
    while(current != NULL)
    {
        printf("%-2d", current->ID);
        printZone(current, TRUE);
        current = current->next_zone;
    }
    printf("FINE-------------------------------------------------\n\n");
}

/**
 * Does the free() for each zone of the map which was previously allocated with malloc()
 */
void deleteMap()
{
    Zone* temp = first_zone;

    while (first_zone != NULL)
    {
        temp       = first_zone;
        first_zone = first_zone->next_zone;
        free(temp);
    }
    first_zone = last_zone = NULL;
}

// --------------------------------GAME FUNCTIONS-------------------------------
/**
 * Manages the turns of the two players, calling myTurn() when a player has to make some choices
 * @see doTurn
 * @see saveGame
 * @see deleteSave
 */
static void shiftManager()
{
    do
    {
        if(turn_check == 0 && P1.pos != NULL && P2.pos != NULL)
        {
            int rand_turn = rand()%100 + 1;

            if (rand_turn > 50)
            {
                doTurn(&P1);
                turn_check = 1;
            }
            else
            {
                doTurn(&P2);
                turn_check = 2;
            }
        }
        else if (turn_check == 2 || P2.pos == NULL)
        {
            doTurn(&P1);
            turn_check = 0;
        }
        else if (turn_check == 1 || P1.pos == NULL)
        {
            doTurn(&P2);
            turn_check = 0;
        }
        saveGame();
    } while(P1.pos != NULL || P2.pos != NULL);

    g_menu = -1;
    deleteSave();
}

/**
 * Prints some useful info for the game and manages the actions that the players can do, calling the respective functions
 * @param myP The player who is currently playing
 *
 * @see progressZone
 * @see rummage
 * @see takeItem
 * @see heal
 * @see useAdrenaline
 * @see craft
 * @see callGieson
 * @see victory
 * @see gameOver
 */
void doTurn(Player* myP)
{
    int moves   = 1;
    int p_moves = 1;
    while (moves > 0)
    {
        p_moves = moves;
        system("clear");

        // Printing stats and inventory
        char gas_info [50], player_name[10];
        if(gasoline_turns)
            sprintf(gas_info, "Turni rimanenti al sicuro da Gieson: %d", gasoline_turns);
        else
            strcpy(gas_info, " ");

        if (myP == &P1)
            strcpy(player_name, "Giacomo");
        else
            strcpy(player_name, "Marzia");

        printf("─────────────────────┤ I N V E N T A R I O ├─────────────────────────────┐      \n"
               " Turno di %-10s │ Cianfrusaglia = %-2d    Bende = %-2d    Coltello = %-2d │     \n"
               "─────────────────────┤       Pistola = %-2d  Benzina = %-2d  Adrenalina = %-2d │\n"
               " STATO: %-10s   ├───────────────────────────────────────────────────┘           \n"
               " MOSSE RIMANENTI: %-2d │ %s\n"
               "─────────────────────┘                                                          \n\n",
                player_name, myP->backpack[0], myP->backpack[1], myP->backpack[2],
                myP->backpack[3], myP->backpack[4], myP->backpack[5],
                tags_state[myP->state],
                moves, gas_info);

        // Printing zone
        printf("ZONA CORRENTE--------------------------------------\n");
        printZone(myP->pos, myP->searched);
        printf("---------------------------------------------------\n\n");

        printf("1) Avanza alla prossima zona           \n"
               "2) Scopri l'oggetto                    \n"
               "3) Raccogli l'oggetto                  \n"
               "4) Curati con le bende                 \n"
               "5) Usa una scarica di adrenalina       \n"
               "6) Tenta di utilizzare le cianfrusaglie\n\n");

        printf("La tua scelta: ");

        g_menu = getValue(1,6);
        printf("__________________________________________________________________________________________________\n\n");
        switch(g_menu)
        {
            case 1:
                progressZone(myP);
                break;
            case 2:
                rummage(myP, &moves);
                break;
            case 3:
                takeItem(myP, &moves);
                break;
            case 4:
                heal(myP, &moves);
                break;
            case 5:
                useAdrenaline(myP, &moves);
                break;
            case 6:
                craft(myP, &moves);
                break;
        }
        moves--;

        // If the player selects an action that he can't do, Gieson will not appear
        if(p_moves != moves)
            callGieson(myP, &moves);

        if (myP->pos == NULL && myP->state != DEAD)
            victory(myP, &moves);
        else if (myP->state == DEAD)
            gameOver(myP, &moves);
    }
}

/**
 * Sets the pointer <b>pos</b> to the next zone, checking if the player reaches the EXIT_CAMPING
 * @param myP   The player who is currently playing
 */
void progressZone(Player* myP)
{
    if (myP->pos->next_zone != NULL)
    {
        myP->pos = myP->pos->next_zone;
        myP->searched = FALSE;
        printf("Avanzi di una zona, recandoti in %s.\nPremi INVIO.", tags_zone[myP->pos->type]);
        waitEnter();
    }
    else
    {
        if (myP == &P1 )
            printf("Giacomo, stai per uscire dal campeggio! Ancora uno sforzo e sarai salvo!\nPremi INVIO.");
        else
            printf("Marzia, stai per uscire dal campeggio! Ancora uno sforzo e sarai salva!\nPremi INVIO.");

        waitEnter();
        myP->pos = NULL; // Setting the current pos to null since the player isn't playing anymore
    }
}

/**
 * Allows the player to rummage in the zone where he currently is
 * @param myP   The player who is currently playing
 * @param moves Avaiable moves for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void rummage(Player* myP, int* moves)
{
    if(myP->pos->object != NOTHING && myP->searched == TRUE)
    {
        printf("Trovi %s in bella vista, ma ti limiti ad osservare, senza concludere nulla.\n", tags_obj[myP->pos->object]);
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");

        printf("Premi INVIO.");
        waitEnter();
    }
    else if(myP->pos->object != NOTHING && myP->searched == FALSE)
    {
        printf("Hai trovato: %s\nMa per il momento non puoi prenderlo.\nPremi INVIO.", tags_obj[myP->pos->object]);
        waitEnter();

        myP->searched = TRUE;
    }
    else if(myP->searched == FALSE)
    {
        printf("Cerchi disperatamente un oggetto che possa tornarti utile, senza successo.\nPremi INVIO.");
        waitEnter();

        myP->searched = TRUE;
    }
    else
    {
        printf("Continui a rovistare in giro, pur consapevole che non troverai mai nulla di utile.\n");
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");

        printf("Premi INVIO.");
        waitEnter();
    }
}

/**
 * Takes the item of the zone incrementing by 1 the number of that item in the player's bag
 * @param myP   The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void takeItem(Player* myP, int* moves)
{
    if(myP->pos->object != NOTHING && myP->searched == TRUE)
    {
        if (myP->obj_count > BACKPACK_SIZE)
        {
            printf("Il tuo zaino è pieno. Devi consumare qualche oggetto prima di raccoglierne un altro.\n");
            (*moves)++;
            textFramedSub("Puoi scegliere una nuova azione da fare");
        }
        else
        {
            printf("Guardi in giro con aria furtiva, dopodiché inserisci quanto avevi cercato prima nello zaino.\n");
            char inv_modified [50];
            strcpy(inv_modified, tags_obj[myP->pos->object]);
            strcat(inv_modified, " +1");
            textFramedSub(inv_modified);

            myP->backpack[myP->pos->object]++;
            myP->obj_count++;
            myP->pos->object = NOTHING;
        }
    }
    else if(myP->pos->object == NOTHING && myP->searched == TRUE)
    {
        printf("Provi a prendere qualcosa di utile ma probabilmente qualcuno ci ha pensato prima di te.\n");
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");
    }
    else
    {
        printf("Non sai cosa prendere, sarebbe meglio cercare prima.\n");
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");
    }
    printf("Premi INVIO.");
    waitEnter();
}

/**
 * Alters the status of the current player if he has a bandage
 * @param myP   The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void heal(Player* myP, int* moves)
{
    if (myP->backpack[BANDAGE] > 0)
    {
        if(myP->state == ALIVE)
        {
            printf("Nonostante tu abbia delle bende con te, ti accordgi di non essere ferito e quindi decidi di non sprecarle.\n");
            (*moves)++;
            textFramedSub("Puoi scegliere una nuova azione da fare");
        }
        else
        {
            printf("Utilizzi una benda per curarti completamente.\n");
            textFramedSub("Bende -1");
            textFramedSub("Le tue ferite sono state guarite!");

            myP->state = ALIVE;
            myP->backpack[BANDAGE]--;
            myP->obj_count--;
        }
        printf("Premi INVIO.");
        waitEnter();
    }
    else
    {
        printf("Non disponi di alcuna benda per curarti.\n");
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");

        printf("Premi INVIO.");
        waitEnter();
    }
}

/**
 * Modifies the param moves, letting the player choose an extra option during his turn.
 * @param myP   The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void useAdrenaline(Player* myP, int* moves)
{
    if (myP->backpack[ADRENALINE] > 0)
    {
        printf("Usi una scarica di adrenalina, che ti permette di effettuare altre 2 azioni.\n");
        textFramedSub("Adrenalina -1");
        printf("Premi INVIO.");
        waitEnter();

        myP->backpack[ADRENALINE]--;
        myP->obj_count--;
        if (*moves == 1)
            *moves+=2;
        else
            *moves+=3;
    }
    else
    {
        printf("Il tuo corpo non reagisce e ti senti più fiacco del solito. (Non hai adrenaline con te)\n");
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");

        printf("Premi INVIO.");
        waitEnter();
    }
}

/**
 * A random craft system. Based on the number of junk possessed by the player, if the player is lucky this function will give him an useful object
 * @param myP   The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void craft(Player* myP, int* moves)
{
    if (myP->backpack[JUNK] > 0)
    {
        if(rand()%100 + 1 >= 30)
        {
            int random_craft;
            switch(myP->backpack[JUNK])
            {
                case 1:
                    random_craft = rand()%3 + 1;
                    break;
                case 2:
                    random_craft = rand()%2 + 2;
                    break;
                default: // If we have more than 3 junks
                    random_craft = 3;
                    break;
            }

            switch(random_craft)
            {
                case 1:
                    printf("Riesci a trovare parte di una lama ormai poco affilata ed un legnetto, creandoti un coltello.\n");
                    myP->backpack[KNIFE]++;
                    break;
                case 2:
                    printf("Riassembli una pistola caricandoci l'unico proiettile che hai trovato.\n");
                    myP->backpack[GUN]++;
                    break;
                case 3:
                    printf("Noti che tra le numerose cianfrusaglie in tuo possesso non avevi notato prima una tanica di benzina, seppur non proprio piena.\n");
                    myP->backpack[GASOLINE]++;
                    break;
                default:
                    printf("An error has occurred. Please check the craft() function in gamelib.c\n");
            }
            textFramedSub("Cianfrusaglia = 0");
            myP->obj_count -= myP->backpack[JUNK];
            myP->backpack[JUNK] = 0;
        }
        else
        {
            printf("Finisci col realizzare che hai in mano un oggetto completamente inutile, buttandolo via.\n");
            textFramedSub("Cianfrusaglia -1");
            myP->backpack[JUNK]--;
            myP->obj_count--;
        }
    }
    else
    {
        printf("A quanto pare non hai alcuna cianfrusaglia da poter riutilizzare...\n");
        (*moves)++;
        textFramedSub("Puoi scegliere una nuova azione da fare");
    }
    printf("Premi INVIO.");
    waitEnter();
}

/**
 * This function returns the object that the player can use against Gieson, letting him choose if he has more than one object
 * @param  backpack The backpack of the current player
 * @return          The object that the can player can use (returns nothing, an enum, in case of no useful object detected)
 */
ObjType chooseItem(unsigned short backpack[6])
{
    if((backpack[GASOLINE]>0 && backpack[GUN]>0) || (backpack[GASOLINE]>0 && backpack[KNIFE]>0) || (backpack[KNIFE]>0 && backpack[GUN]>0))
    {
        printf("\nNon ti fai prendere dalla paura ed hai la prontezza di scegliere al volo qualcosa con cui difenderti.\n\nChe cosa vuoi utilizzare?\n");
        int count = 1;
        ObjType item_choice [4];

        for (ObjType i = 0; i < 6; i++)
        {
            if(backpack[i] > 0 && (i==KNIFE || i==GUN || i==GASOLINE)) // Check if I have any useful object
            {
                printf("%d) %s\n", count, tags_obj[i]);
                item_choice[count] = i;
                count++;
            }
        }
        printf("\nLa tua scelta: ");
        return item_choice[getValue(1,count-1)];
    }
    else if(backpack[GASOLINE] > 0)
        return GASOLINE;
    else if(backpack[GUN]      > 0)
        return GUN;
    else if(backpack[KNIFE]    > 0)
        return KNIFE;
    else
        return NOTHING;
}

/**
 * Manages the encounter of Gieson and the player
 * @param myP   The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 * @see chooseItem
 */
void callGieson(Player* myP, int* moves)
{
    unsigned int        rand_arrival   = rand()%100 + 1;
    unsigned char       gieson_has_to_appear;

    // Checking if Gieson has to appear
    if(gasoline_turns > 0)
    {
        gasoline_turns--;
        gieson_has_to_appear = FALSE;
    }
    else if ( (P1.state != DEAD && P2.state != DEAD && myP->pos == NULL && rand_arrival <= 75) ||
            ( (P1.state == DEAD || P2.state == DEAD) && rand_arrival <= 50) ||
              (rand_arrival <= 30) )
        gieson_has_to_appear = TRUE;
    else
        gieson_has_to_appear = FALSE;

    if(gieson_has_to_appear)
    {
        printf("\nSenti i pesanti passi di Gieson farsi sempre più vicini finché non lo vedi. Lui è qui.");
        ObjType choice = chooseItem(myP->backpack);

        switch(choice)
        {
            case GASOLINE:
                printf("\nAfferri con rapidità la tua tanica di benzina e la svuoti su Gieson, per poi dargli fuoco.\nHai come l'impressione che per un po' non si farà vivo.\n");
                textFramedSub("4 Turni al sicuro da Gieson");
                textFramedSub("Benzina -1");
                printf("Premi INVIO.");
                myP->backpack[GASOLINE]--;
                myP->obj_count--;
                gasoline_turns = 4;
                break;

            case GUN:
                printf("\nImpugni la tua pistola e spari un colpo contro di lui. Sai che non basterà a fermarlo, ma riesci a scappare via completamente illeso.\n");
                textFramedSub("Pistola -1");
                printf("Premi INVIO.");
                myP->backpack[GUN]--;
                myP->obj_count--;
                break;

            case KNIFE:
                if(myP->state > INJURED)
                {
                    printf("\nTi aggredisce provocandoti un'importante ferita. Cominci a sanguinare, ma riesci a contrattaccare estraendo\n"
                           "un coltello dallo zaino e piantandoglielo nel corpo. Riesci così a rallentarlo e ad allontanarti.\n");
                    textFramedSub("Coltello -1");
                    textFramedSub("Sei ferito!");
                    printf("Premi INVIO.");
                    myP->state = INJURED;
                    myP->backpack[KNIFE]--;
                    myP->obj_count--;
                }
                else
                {
                    printf("\nLe tue ferite purtroppo sono molto gravi e non riesci a trovare le forze neppure per tentare\n"
                           "di difenderti con quel coltello rimasto nel tuo zaino. Per te è Game Over.\nPremi INVIO.");
                    myP->state = DEAD;
                    myP->pos   = NULL;
                    *moves     = 0;
                }
                break;

            default:
                printf("\nBen presto ti rendi conto che non hai modo di affrontarlo né di scappare. Per te è Game Over.\nPremi INVIO.");
                myP->state = DEAD;
                myP->pos   = NULL;
                *moves     = 0;
        }
        waitEnter();
    }
    else if (rand_arrival <= 40 && gasoline_turns == 0) // Small percentage to get a little surprise from Gieson
    {
        printf("\nSenti un fruscio vicino a te e cominci a correre. Dopodiché ti giri indietro ma non vedi niente.\nPremi INVIO.");
        waitEnter();
    }
}

/**
 * Prints a victory message to the current player
 * @param myP The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void victory(Player* myP, int* moves)
{
    system("clear");
    *moves = 0;

    char end_game [70];
    if (myP == &P1 && (P2.pos != NULL || P2.state == DEAD) )
        strcpy(end_game, "Ma che fine ha fatto la tua compagna Marzia?!");
    else if(myP == &P2 && (P1.pos != NULL || P1.state == DEAD) )
        strcpy(end_game, "Ma che fine ha fatto il tuo compagno Giacomo?!");
    else
        strcpy(end_game, "Miglior finale raggiunto! Entrambi i giocatori si sono salvati!");

    printf("      hhd             \n"
           "    dssssy            \n"
           "     yssyd            \n"
           "       sshhhsssssd    \n"
           "     hsssssssyhhysy   \n"
           "   dysyysssssy   ysy  \n"
           "sssssh  ysssssh   hyd \n"
           "d        ysssssd      \n"
           "         ysssssy      \n"
           "        hssydsss      \n"
           "       hssy  sss      \n"
           "      hssy   yssyhhhhh\n"
           "     hssy     yyyyyyyy\n"
           "    hsss              \n"
           "   dssy               \n"
           "    dss    VITTORIA!! \n"
           "     yssy             \n"
           "       yssy           \n"
           "\n"
           "Fuggi più veloce che puoi dal campeggio e sei finalmente in salvo!\n%s\nPremi INVIO.", end_game);

    waitEnter();
}

/**
 * Prints a game over message to the player
 * @param myP The player who is currently playing
 * @param moves Moves available for the player. We take this as a parameter and use it in the function in case of bad use of this action
 */
void gameOver(Player* myP, int* moves)
{
    system("clear");
    *moves = 0;

    printf("                                   _----..................___            \n"
           " __,,..,-====>       _,.--''------'' |   _____  ______________`''--._    \n"
           " \\      `\\   __..--''                |  /::: / /::::::::::::::\\      `\\  \n"
           "  \\       `''                        | /____/ /___ ____ _____::|    .  \\ \n"
           "   \\         SEI MORTO.        ,.... |            `    `     \\_|   ( )  |\n"
           "    `.      Premi INVIO.     /`     `.\\ ,,''`'- ,.----------.._     `   |\n"
           "      `.                     |        ,'       `               `-.      |\n"
           "        `-._                 \\                                    ``.. / \n"
           "            `---..............>                                          \n");
    waitEnter();
}

// ------------------------------SYSTEM FUNCTIONS-------------------------------
/**
 * Sets the values for the game
 * @param t_P1             Pointer to first  player to set
 * @param t_P2             Pointer to second player to set
 * @param t_gasoline_turns Obtaining an integer to set gasoline_turns
 * @param t_turn_check     Obtaining an integer to set turn_check
 */
void setValues(Player* t_P1, Player* t_P2, unsigned int t_gasoline_turns, unsigned int t_turn_check)
{
    if (t_P1 == NULL && t_P2 == NULL)
    {
        P1.pos   = P2.pos   = first_zone;
        P1.state = P2.state = ALIVE;

        #ifdef DEBUG
            P1.backpack[JUNK]  = P1.backpack[BANDAGE]  = P1.backpack[KNIFE] =
            P1.backpack[GUN]   = P1.backpack[GASOLINE] = P1.backpack[ADRENALINE] = 99;
            P1.obj_count = -100;
            P1.searched  = FALSE;

            P2.backpack[JUNK]  = P2.backpack[BANDAGE]  = P2.backpack[KNIFE] =
            P2.backpack[GUN]   = P2.backpack[GASOLINE] = P2.backpack[ADRENALINE] = 99;
            P2.obj_count = -100;
            P2.searched  = FALSE;
        #else
            P1.backpack[JUNK]       = 0;
            P1.backpack[BANDAGE]    = 0;
            P1.backpack[KNIFE]      = 1;
            P1.backpack[GUN]        = 0;
            P1.backpack[GASOLINE]   = 0;
            P1.backpack[ADRENALINE] = 0;
            P1.obj_count = 1;
            P1.searched  = FALSE;

            P2.backpack[JUNK]       = 0;
            P2.backpack[BANDAGE]    = 0;
            P2.backpack[KNIFE]      = 0;
            P2.backpack[GUN]        = 0;
            P2.backpack[GASOLINE]   = 0;
            P2.backpack[ADRENALINE] = 2;
            P2.obj_count = 2;
            P2.searched  = FALSE;
        #endif
    }
    else
    {
        P1 = *t_P1;
        P2 = *t_P2;
    }
    gasoline_turns = t_gasoline_turns;
    turn_check     = t_turn_check;
}

/**
 * Saves the current game printing all the important variables into GameSave.save
 */
void saveGame()
{
    if(first_zone != NULL)
    {
        FILE* fptr;
        fptr = fopen("GameSave.save", "w");
        if(fptr == NULL)
        {
            fprintf(stderr, "Errore nell'apertura del file di salvataggio automatico.\n");
            exit(-1);
        }

        fprintf(fptr, "LINKED LIST:\n");
        Zone* current = first_zone;
        while(current != NULL)
        {
            fprintf(fptr, "%d-%d", current->type, current->object);
            if(current->next_zone != NULL)
                fprintf(fptr, ",");
            else
                fprintf(fptr, "#");

            current = current->next_zone;
        }

        fprintf(fptr, "\nPLAYERS:\n");
        fprintf(fptr, "P1-%d-%4d-|%4d-%4d-%4d-%4d-%4d-%4d|-%4d-%d\n",
                P1.state, P1.pos == NULL ? 0 : P1.pos->ID,
                P1.backpack[0], P1.backpack[1], P1.backpack[2], P1.backpack[3], P1.backpack[4], P1.backpack[5],
                P1.obj_count, P1.searched);
        fprintf(fptr, "P2-%d-%4d-|%4d-%4d-%4d-%4d-%4d-%4d|-%4d-%d\n",
                P2.state, P2.pos == NULL ? 0 : P2.pos->ID,
                P2.backpack[0], P2.backpack[1], P2.backpack[2], P2.backpack[3], P2.backpack[4], P2.backpack[5],
                P2.obj_count, P2.searched);

        fprintf(fptr, "GAME VARIABLES:\n");
        fprintf(fptr, "%d, %d", turn_check, gasoline_turns);

        fclose(fptr);
    }
    else
        printf("Non è possibile salvare in questo momento.");
}

/**
 * Moves the player to the position of the map specified
 * @param myP         The player of which we have to assign the position
 * @param my_cur_zone A very small number representing the current position of the player
 */
static void assignPosition(Player* myP, unsigned char my_cur_zone)
{
    Zone* current = first_zone;

    if (my_cur_zone == 0)
        current = NULL;
    else
        for(int i = 1; i<my_cur_zone; i++)
            current = current->next_zone;

    myP->pos = current;
}

/**
 * Reads the GameSave.save file, reallocates the memory for the linked list and starts a new game.
 * It also call assignPosition to set the pos of the players given the ID of the zone where they currently are
 * @see assignPosition
 * @see setValues
 * @see shiftManager
 */
void loadGame()
{
    deleteMap(); // Just to prevent some errors I do another clear of the map

    Player t_P1, t_P2;
    unsigned char t_cur_zone;
    unsigned int  t_gasoline_turns, t_turn_check;
    FILE* fptr;

    fptr = fopen("GameSave.save", "r");
    if(fptr == NULL)
    {
        fprintf(stderr, "\nAttualmente non è presente alcun salvataggio.\nPremi INVIO.");
        waitEnter();
        return;
    }

    // READING LINKED LIST
    fscanf(fptr, "LINKED LIST:");
    while(getc(fptr) != '#')
    {
        int z_type,z_obj;
        fscanf(fptr, "%d-%d", &z_type, &z_obj);
        addZone(z_type, z_obj);
    }

    // READING PLAYERS
    while(getc(fptr) != ':');
    while(getc(fptr) != '\n');

    fscanf(fptr, "P1-%u-%hhd-|%4hd-%4hd-%4hd-%4hd-%4hd-%4hd|-%4d-%hhd\n",
        &t_P1.state, &t_cur_zone,
        &t_P1.backpack[0], &t_P1.backpack[1], &t_P1.backpack[2], &t_P1.backpack[3], &t_P1.backpack[4], &t_P1.backpack[5],
        &t_P1.obj_count, &t_P1.searched);
    assignPosition(&t_P1, t_cur_zone);

    fscanf(fptr, "P2-%u-%hhd-|%4hd-%4hd-%4hd-%4hd-%4hd-%4hd|-%4d-%hhd\n",
        &t_P2.state, &t_cur_zone,
        &t_P2.backpack[0], &t_P2.backpack[1], &t_P2.backpack[2], &t_P2.backpack[3], &t_P2.backpack[4], &t_P2.backpack[5],
        &t_P2.obj_count, &t_P2.searched);
    assignPosition(&t_P2, t_cur_zone);

    // READING GAME VARIABLES
    while(getc(fptr) != ':');
    while(getc(fptr) != '\n');
    fscanf(fptr, "%d, %d", &t_turn_check, &t_gasoline_turns);

    fclose(fptr);

    // Setting values and starting the game
    setValues(&t_P1, &t_P2, t_gasoline_turns, t_turn_check);
    shiftManager();
}

/**
 * Simple alias to the remove function. It deletes the GameSave.save file
 */
void deleteSave()
{
    remove("GameSave.save");
}

// -----------------------------MAIN MENU FUNCTIONS-----------------------------
/**
 * Prints the story of the game and let the player choose if he wants to start the creation of the map or not
 * @see createMap
 */
void newGame()
{
    printf("___________________________________________________________________________________________________________\n\n"
           "È una notte buia e tempestosa al campeggio \"Lake Trasymeno\".\n\n"

           "Data la situazione, Giacomo e Marzia hanno deciso di accamparsi in riva al lago con la loro tenda...\n"
           "... sono però ignari di quanto successe in quello stesso campeggio 22 anni prima.\n\n"

           "Il caso volle che un giovane studente della facoltà di Informatica di nome Gieson perdesse la vita a causa di un \"segmentation fault\",\n"
           "causato dall'inesperienza del programmatore che scrisse il software per il noleggio delle barche,\n"
           "consentendogli di noleggiare una barca che poi si rivelò difettosa... L'incidente lo portò ad annegare nel lago.\n\n"

           "La sua sete di vendetta fu inarrestabile, a tal punto che permise alla sua anima di continuare a vagare nei dintorni\n"
           "alla ricerca di altri studenti di Informatica come lui, per mettere fine alla loro carriera e impedirgli in futuro di mietere nuove vittime.\n\n"

           "Gieson fa quindi la sua apparizione ai due colleghi informatici, che non perdono un secondo per darsela a gambe.\n"
           "Giacomo e Marzia sono in grave pericolo ed hanno bisogno di un aiuto per scappare!\n\n");

    printf("Vuoi aiutarli a scappare vivi dal campeggio? (s/n): ");
    g_ans = getAns();

    if(g_ans == 's')
        createMap();
}

/**
 * Simply closes the game by clearing the screen and printing a message.
 */
void closeGame()
{
    system("clear");
    printf("Chiusura del programma...\n\n");
}

// ------------------------------UTILITY FUNCTIONS------------------------------
/**
 * Writes a text inside a frame
 * @param text The text that we want to write
 */
void textFramed(const char* text)
{
    int text_len = strlen(text)%2 == 0 ? strlen(text) : strlen(text)+1;

    for(int i=0; i<text_len; i++)
    {
        if (i == text_len/2)
            printf("^");
        else
            printf("─");
    }
    printf("\n %s\n", text);

    for(int i=0; i<text_len; i++)
        printf("─");

    printf("\n");
}

/**
 * Writes a text inside a different frame from the one of textFramed. This function will be mostly used for game's notification
 * @param text The text that we want to write
 */
void textFramedSub(const char* text)
{
    int text_len = strlen(text)+2;

    printf("╔");

    for(int i=0; i<text_len; i++)
            printf("─");

    printf("┐\n│ %s │\n└", text);

    for(int i=0; i<text_len; i++)
        printf("─");

    printf("┘\n");
}

/**
 * Captures everything inside the stdin and simply delete all this things by exiting from the function
 */
static void clear_stdin()
{
   char unwanted[50];
   fgets(unwanted, sizeof(unwanted), stdin);
}

/**
 * Utility function to check if the value taken by the user is correct or not, also checking if it is included between inf_l and sup_l
 * @param  inf_l Inferior limit
 * @param  sup_l Superior limit
 * @return       The value taken by the user
 */
int getValue(int inf_l, int sup_l)
{
    int opt;

    while( (scanf("%d", &opt) != 1)
          || (opt < inf_l)
          || (opt > sup_l) )
    {
        clear_stdin();
        printf("Il valore inserito non corrisponde a nessuna delle scelte proposte.\n\n");
        printf("La tua scelta: ");
    }
    clear_stdin();

    return opt;
}

/**
 * Utility function to check if the char taken by the user is correct or not
 * @return The char taken by the user
 */
char getAns()
{
    char opt;

    scanf("%c", &opt);
    clear_stdin();

    return opt;
}

/**
 * A kind of alias for getchar. This function will be used to wait for the pressing of enter by the user
 */
void waitEnter()
{
    while( getchar() != '\n' );
}
