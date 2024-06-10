    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #define SIZE 3 //set the size of the matrix

    void print_error() {
        printf("Error\n"); //func to print error
        exit(1);
    }

    void print_board(char board[SIZE][SIZE]) {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                printf("%c ", board[i][j]); //print the value for each box
            }
            printf("\n");
        }
    }

    int is_winner(char board[SIZE][SIZE], char player) {
        for (int i = 0; i < SIZE; i++) {
            if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
                //check for each row
                return 1;
            if (board[0][i] == player && board[1][i] == player && board[2][i] == player)
                //check each col
                return 1;
        }
        if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
            //check alahson
            return 1;
        if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
            //check alahson
            return 1;
        return 0;
    }

    int is_draw(char board[SIZE][SIZE]) {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == ' ')
                    return 0; // if we have empty box there is still a way to win
            }
        }
        return 1;// if there is no empty box there is a draw(first we chack if there is a winner)
    }

    void make_move(char board[SIZE][SIZE], char move, char player) {
         

        int pos = move - '1'; //set the postion
        int row = pos / SIZE; //row calc
        int col = pos % SIZE; //col cacl

        if (move < '1' || move > '9' || board[row][col] != ' ') {
                //if the move is not between 1 to 9 or the position is the bboard is taken
                print_error();
        }

        board[row][col] = player; //put the sign in the right box
        
        print_board(board);
        
    }


    char next_move(char *strategy, char board[SIZE][SIZE]) {
        for (int i = 0; i < 9; i++) {
            //iterating for each char in strategy to chack if the box is empty
            char move = strategy[i]; //set the move for strategy[i]
            int pos = move - '1'; // calc pos = move -1 (in ascii table)
            int row = pos / SIZE; // row calc
            int col = pos % SIZE; //col calc
            if (board[row][col] == ' '){
                //if there is an epmty box  return the move
                return move;
            }
        }
        return strategy[8];//if all strategy is taken except the last one return him
    }


    int main(int argc, char *argv[]) {
        if (argc != 2 || strlen(argv[1]) != 9) {
            //check right amount of args and if the length of the input (aka strategy) is 9
            print_error();
        }
        
        char *strategy = argv[1]; //the strategy
        int digits[10] = {0}; //array to check if we already have the number
        for (int i = 0; i < 9; i++) {
            //check if we didnt get num between 1 to 9 || we get the same num again
            if (strategy[i] < '1' || strategy[i] > '9' || digits[strategy[i] - '0']) {
                print_error();
            }
            digits[strategy[i] - '0'] = 1;//set that we got the number
        }
        //set the board (empty board)
        char board[SIZE][SIZE] = {
            {' ', ' ', ' '},
            {' ', ' ', ' '},
            {' ', ' ', ' '}
        };

        char computer = 'X'; //set the sign for computer
        char us = 'O'; //set the sign for us


        //loop for starting game
        while (1) {
            //computer move
            char move = next_move(strategy, board);//get the next move from the strategy input
            make_move(board, move, computer);//do the move above

            if (is_winner(board, computer)) {
                //check if computer won
                printf("I win\n");
                exit(1);
                break;
            }
            if (is_draw(board)) {
                ////check if after computer move there is a draw
                printf("DRAW\n");
                exit(1);
                break;
            }
            //user move
            char us_move;
            scanf(" %c", &us_move);//take the input from the user
            make_move(board, us_move, us); //do the user move

            if (is_winner(board, us)) {
                //check if user won
                printf("I lost\n");
                exit(1);
                break;
            }
            if (is_draw(board)) {
                //check if after user move there is a draw
                printf("DRAW\n");
                exit(1);
                break;
            }
        }

        return 0;
    }

