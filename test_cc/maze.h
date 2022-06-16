#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int **stack;
int stackIndex = 0;

int **neighbors_matrix;

int step_debug = 0;
int maze_number_debug = 0;
int maze_plan_debug = 0;
int maze_dim = 0;


void push(int row, int col) {
    if (stackIndex >= 0) {
        stack[stackIndex][0] = row;
        stack[stackIndex][1] = col;

    }
    stackIndex++;

}

void pop(int *row, int *col) {
    stackIndex--;
    if (stackIndex >= 0) {

        *row = stack[stackIndex][0];
        *col = stack[stackIndex][1];
    }
}

char **create_empty_maze() {
    char **maze_plan = (char**)calloc(maze_dim, sizeof(char*));
    int i, j;

    for (i = 0; i < maze_dim; i++) {
        maze_plan[i] = (char*)calloc(maze_dim, sizeof(char));
        char c;
        if (i % 2 == 0) {
            c = '+';
        } else {
            c = '|';
        }

        for (j = 0; j < maze_dim; j++) {
            if (i % 2 == 0) {
                if (j % 2 == 0) {
                    maze_plan[i][j] = '+';
                } else {
                    maze_plan[i][j] = '-';
                }
            } else {
                if (j % 2 == 0) {
                    maze_plan[i][j] = '|';
                } else {
                    maze_plan[i][j] = ' ';
                }
            }
        }
    }
    return maze_plan;
}

char** generate_maze(int **maze, int start_row, int start_col, int **visited, int matrix_dim) {

    push(start_row, start_col);         
    visited[start_row][start_col] = 1;

    char **maze_plan = create_empty_maze(); 

    int counter0 = 0;
    int temp_row = start_row; 
    int temp_col = start_col; 
    int temp_row1 = start_row;
    int temp_col1 = start_col;

    maze_plan[start_row][start_col] = '*'; 


    while (stackIndex != 0) {
        int row, col;
        int n_index;

        pop(&row, &col);
        start_row = row;
        start_col = col;
        visited[row][col] = 1;

        int k = 0;
        int counter = 0;
        n_index = 0;
        while (k == 0 && counter < 4) { 
            n_index = rand() % (6343 - rand() % 2243);// (row * 29 + 1));
            n_index %= 4;
           // n_index = rand() % 4;
            row = start_row+neighbors_matrix[n_index][0];
            col = start_col+neighbors_matrix[n_index][1];
            neighbors_matrix[n_index][2] = 1; 

            if (!(row < 0 || col < 0 || (row == 0 || col == 0 || row == matrix_dim-1 || col == matrix_dim-1))) { 

                if (visited[row][col] == 0) { 
                    counter0 = 0;
                    maze[start_row][start_col] = 10*(row) + col;


                    if (start_row == row && start_col != col) {
                        if (start_col > col) {
                            temp_col1 = temp_col;
                            temp_row1 = temp_row;

                            maze_plan[temp_row][temp_col-1] = ' ';
                            maze_plan[temp_row][temp_col-2] = ' ';
                            temp_col -= 2;

                        } else {
                            temp_col1 = temp_col;
                            temp_row1 = temp_row;

                            maze_plan[temp_row][temp_col+1] = ' ';
                            maze_plan[temp_row][temp_col+2] = ' ';
                            temp_col += 2;
                        }
                    } else if (start_col == col && start_row != row) {
                        if (start_row > row) {
                            temp_col1 = temp_col;
                            temp_row1 = temp_row;

                            maze_plan[temp_row-1][temp_col] = ' ';
                            maze_plan[temp_row-2][temp_col] = ' ';
                            temp_row -= 2;
                        } else {
                            temp_col1 = temp_col;
                            temp_row1 = temp_row;

                            maze_plan[temp_row+1][temp_col] = ' ';
                            maze_plan[temp_row+2][temp_col] = ' ';
                            temp_row += 2;
                        }
                    }

                    push(start_row, start_col); 
                    push(row, col); 
                    visited[row][col] = 1;
                    k = 1;


                } else {
                    counter = neighbors_matrix[0][2] + neighbors_matrix[1][2] + neighbors_matrix[2][2] + neighbors_matrix[3][2]; 
                }
            }
        }

        if (counter == 4) {
            pop(&row, &col);

            if (start_row == row && start_col != col) {
                if (start_col > col) {
                    temp_col -= 2;

                } else {
                    temp_col += 2;
                }
            } else if (start_col == col && start_row != row) {
                if (start_row > row) {
                    temp_row -= 2;
                } else {
                    temp_row += 2;
                }
            }
            push(row, col);
        }

        neighbors_matrix[0][2] = 0; 
        neighbors_matrix[1][2] = 0;
        neighbors_matrix[2][2] = 0;
        neighbors_matrix[3][2] = 0;

       

    }

    maze_plan[maze_dim-2][maze_dim-2] = 'X';

    return maze_plan;
}



#endif // MAIN_H

