// LIBRARIES --------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

// MACROS -----------------------------------------------------------------------------------------
#define OR ||
#define AND &&
#define NUM_PLAYERS 4

// GLOBAL STRUCTURES ------------------------------------------------------------------------------
struct Dice {
    int die1;
    int die2;
};
struct Mutex {
    pthread_mutex_t dealer_deadbolt;
    pthread_mutex_t player_deadbolt;
};
struct Condition {
    pthread_cond_t dice_available;
    pthread_cond_t win;
};

// INSTANCES OF GLOBAL STRUCTURES -----------------------------------------------------------------
struct Dice dice;
struct Mutex mutex;
struct Condition condition;

// GLOBAL VARIABLES -------------------------------------------------------------------------------
FILE *logFile;
int seed = 0;
int turn = 0;
bool t1win = false;
bool t2win = false;
int t1Sum = 10000;
int t2Sum = 10000;
pthread_t player_threads[NUM_PLAYERS];
pthread_t dealer_thread;

// FUNCTION PROTOTYPES ----------------------------------------------------------------------------
void launch_game();
void *dealer_routine(void *arg);
void *player_routine(void *player_id);
void play_turn(long);
int roll_die();
void end_game();

// FUNCTIONS --------------------------------------------------------------------------------------
/*
 * MAIN FUNCTION:
 *     Open the file in append mode.
 *     Set the command line argument as the seed.
 *     Seed the random number generator.
 *     Launch the Matching Dice game.
 *     End the Matching Dice game.
 *     Close the log file.
 *     Exit the program.
 */
int main(int argc, char *argv[]) {
    logFile = fopen("log.txt", "a");
    seed = atoi(argv[1]);
    srand(seed);
    launch_game();
    end_game();
    fclose(logFile);
    exit(EXIT_SUCCESS);
}

/*
 * LAUNCH GAME FUNCTION:
 *     Initializes the mutex object attributes.
 *     Initializes the condition object attributes.
 *     Launches the dealer thread for the Matching Dice game.
 *     Launches the player threads for the Matching Dice game.
 */
void launch_game() {
    // initializes the mutex object attributes
    pthread_mutex_init(&mutex.dealer_deadbolt, NULL);
    pthread_mutex_init(&mutex.player_deadbolt, NULL);

    // initializes the condition object attributes
    pthread_cond_init(&condition.dice_available, NULL);
    pthread_cond_init(&condition.win, NULL);

    // create new threads, dealer_thread and the three player_threads
    for (long id_num = 0; id_num <= NUM_PLAYERS; id_num++) {
        // if the id_num is the dealer's
        if (id_num == 0)
            pthread_create(&dealer_thread, NULL, &dealer_routine, (void *)id_num);
        // if the id_num is one of the player's
        else
            pthread_create(&player_threads[id_num], NULL, &player_routine, (void *)id_num);
    }

    // suspends the execution of calling thread until the target thread terminates
    for (int id_num = 0; id_num <= NUM_PLAYERS; id_num++) {
        // if the id_num is the dealer's
        if (id_num == 0)
            pthread_join(dealer_thread, NULL);
        // if the id_num is one of the player's
        else
            pthread_join(player_threads[id_num], NULL);
    }
}

/*
 * DEALER ROUTINE:
 *     Dealer randomly selects a player.
 *     Dealer gives the dice to the initial player.
 *     Dealer waits for a win.
 *     Dealer announces the winner in the log.
 */
void *dealer_routine(void *id_num) {
    // variable in the scope of the dealer routine function
    long player_id = (long)id_num;

    // dealer plays to randomly define the initial player
    play_turn(player_id);

    // locks the mutex object attribute, dealer_deadbolt, for thread synchronization
    pthread_mutex_lock(&mutex.dealer_deadbolt);

    // while neither team has won the Matching Dice game, dealer waits
    while (t1win == false AND t2win == false)
        pthread_cond_wait(&condition.win, &mutex.dealer_deadbolt);

    // unlocks the mutex object attribute, dealer_deadbolt, for thread synchronization
    pthread_mutex_unlock(&mutex.dealer_deadbolt);

    // if team A and C win the game, the dealer announces it in the file
    if (t1win)
        fprintf(logFile, "DEALER: The winning team is A and C\n\n");
    // if team B and D win the game, the dealer announces it in the file
    else if (t2win)
        fprintf(logFile, "DEALER: The winning team is B and D\n\n");

    // terminates the calling thread
    pthread_exit(NULL);
}

/*
 * PLAYER ROUTINE:
 *     Players play the Matching Dice game.
 *     Players play when it's their turn.
 *     Players wait when it's not their turn.
 *     Players continue to play while neither team has won.
 */
void *player_routine(void *id_num) {
    // variables in the scope of the player_routine function
    long player_id = (long)id_num;

    // loops as long as neither team has won
    while (t1win == false AND t2win == false) {
        // locks the mutex object mutex_player_deadbolt for thread synchronization
        pthread_mutex_lock(&mutex.player_deadbolt);

        // while neither team has won, but it's not a player's turn, the player waits
        while (player_id != turn AND (t1win == false AND t2win == false))
            pthread_cond_wait(&condition.dice_available, &mutex.player_deadbolt);

        // if neither team has won, the player plays their turn of the Matching Dice game
        if (t1win == false AND t2win == false)
            play_turn(player_id);

        // unlocks the mutex object mutex_player_deadbolt for thread synchronization
        pthread_mutex_unlock(&mutex.player_deadbolt);
    }

    // terminates the calling thread
    pthread_exit(NULL);
}

/*
 * PLAY TURN FUNCTION:
 *     Current player rolls the dice.
 *     If the dealer is playing, the following will occur:
 *         The dealer uses their die roll to define the initial player.
 *     If a player is playing, the following will occur:
 *         Add the sum of the die rolls.
 *         Compare the sum of the rolls to that of their teammates' last sum.
 *         A team wins if the sum of their rolls is equal to that of their teammates last sum.
 *         If player doesn't get a winning roll, we move on to the next player's turn.
 */
void play_turn(long id_num) {
    // variables in the scope of the play turn function
    char id_char;
    int sum;

    // initialize the dice by rolling each one, then set the sum according to the rolls
    dice.die1 = roll_die();
    dice.die2 = roll_die();
    sum = dice.die1 + dice.die2;

    // makes sure the player is defined correctly in the log and terminal
    if (id_num == 1) id_char = 'A';
    else if (id_num == 2) id_char = 'B';
    else if (id_num == 3) id_char = 'C';
    else if (id_num == 4) id_char = 'D';

    // if the current player is the dealer
    if (id_num == 0) {
        // dealer rolls the die until it rolls in the range of 1 through 4
        while (dice.die1 > 4 OR dice.die1 < 0)
            dice.die1 = roll_die();

        // dealer defines the initial player based off of their die roll
        if (dice.die1 == 1) turn = 1;
        else if (dice.die1 == 2) turn = 2;
        else if (dice.die1 == 3) turn = 3;
        else if (dice.die1 == 4) turn = 4;
    }

    // if the current player isn't the dealer
    else if (id_num > 0) {
        // prints data to the log and terminal
        printf("Player %c: %d %d\n", id_char, dice.die1, dice.die2);
        fprintf(logFile, "Player %c: gets %d and %d with a sum %d\n", id_char, dice.die1, dice.die2, sum);

        // if the current player is A or C
        if ((int)id_num % 2 != 0) {
            // if the sum of the current player's die rolls is not equal to the sum of their last player's
            if (t1Sum != sum) {
                // set the sum of the team, for comparison to their next player's rolls
                t1Sum = sum;
            }
            // if the sum of the current player's die rolls is equal to the sum of their last player's
            else if (t1Sum == sum){
                // set a win for the team to true
                t1win = true;
                // signals that we have a win and tells the the dealer that it's time to declare the winner
                pthread_cond_signal(&condition.win);
            }
        }

        // if the current player is B or D
        else if ((int)id_num % 2 == 0) {
            // if the sum of the current player's die rolls is not equal to the sum of their last player's
            if (t2Sum != sum) {
                // set the sum of the team, for comparison to their next player's rolls
                t2Sum = sum;
            }
            // if the sum of the current player's die rolls is equal to the sum of their last player's
            else if (t2Sum == sum){
                // set a win for the team to true
                t2win = true;
                // signals that we have a win and tells the the dealer that it's time to declare the winner
                pthread_cond_signal(&condition.win);
            }
        }

        // increments the turn for the next player
        turn++;
        // if the turn increments past player D, we're going to change the turn to player A's
        if (turn > NUM_PLAYERS)
            turn = 1;
    }

    // broadcasts that the dice are available for the next player
    pthread_cond_broadcast(&condition.dice_available);
}

/*
 * ROLL DIE FUNCTION:
 *     Rolls a six sided die.
 *     Returns die roll value.
 */
int roll_die() {
    return (rand() % 6) + 1;
}

/*
 * END GAME FUNCTION:
 *     Destroys mutex object attributes.
 *     Destroys condition object attributes.
 */
void end_game() {
    // destroys the mutex object attributes
    pthread_mutex_destroy(&mutex.dealer_deadbolt);
    pthread_mutex_destroy(&mutex.player_deadbolt);

    // destroys the condition object attributes
    pthread_cond_destroy(&condition.dice_available);
    pthread_cond_destroy(&condition.win);
}