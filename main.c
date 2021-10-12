/*
Written by Pepsalt#1124

Red is equivalent to Black, Blue to White
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void clearscrn()
{
  HANDLE hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD count, cellCount;
  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return;
  if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
  cellCount = csbi.dwSize.X *csbi.dwSize.Y;
  if (!FillConsoleOutputCharacter(hStdOut,(TCHAR) ' ',cellCount,(COORD){0,0},&count)) return;
  if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount,(COORD){0,0}, &count)) return;
  SetConsoleCursorPosition(hStdOut, (COORD){0,0});
}
#else // !_WIN32
#include <unistd.h>
void clearscrn()
{
    printf("\033[H\033[J");
}
#endif
// ANSI colour
#define COLOUR_RED     "\x1b[31m"
#define COLOUR_BLUE    "\x1b[34m"
#define COLOUR_RESET   "\x1b[0m"
typedef struct piece_data{
    char character;
    int data;
} piece;
void output_board(piece * board_pieces){
    char board_under[8*10] = "1|# # # # 2| # # # #3|# # # # 4| # # # #5|# # # # 6| # # # #7|# # # # 8| # # # #";
    puts("0|12345678\n-+--------");
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 10; x++){
            if(board_pieces[y*8+x-2].character==' ' || x<2){
                putchar((int)board_under[y*10+x]);
            }else if(x>0){
                printf("%s%c%s",(board_pieces[y*8+x-2].data%2 ? COLOUR_RED : COLOUR_BLUE), board_pieces[y*8+x-2].character, COLOUR_RESET);
            }
        }
        putchar('\n');
    }
}
    
// 1 true, 0 false
int validate_row_movement(int x1, int y1, int x2, int y2, piece* pieces){
    if(y1!=y2) return 0;
    int rbegin = x1<x2?x1:x2, rend = x1<x2?x2:x1; // dynamic row start/end to account for backwards movement
    for(int xindex = rbegin+1; xindex< rend; xindex++)
        if(pieces[y1*8+xindex].character != ' '){
            return 0; // piece blocks movement
        }
    return 1;
}
int validate_column_movement(int x1, int y1, int x2, int y2, piece* pieces){
    if(x1!=x2) return 0;
    int cbegin = y1<y2?y1:y2, cend = y1<y2?y2:y1; // dynamic column start/end to account for backwards movement
    for(int yindex = cbegin+1; yindex < cend; yindex++){
        if(pieces[yindex*8+x1].character != ' ') {
            return 0; // piece blocks movement
        }
    }
    return 1;
}
int validate_diagonal_movement(int x1, int y1, int x2, int y2, piece* pieces){
    if(abs(x2-x1)-abs(y2-y1)!=0) return 0; // non diagonal movement 
    int xbegin = x1<x2?x1:x2, ybegin = y1<y2?y1:y2, yend = y1<y2?y2:y1;
    if (ybegin==y1&&xbegin==x1)
        for(int i = 0; i < abs(x1-x2); i++)
            if(pieces[(ybegin+i)*8+(xbegin+i)].character != ' ' && ((ybegin+i!=y1) && (xbegin+i!=x1))){
                return 0;
            }
    else 
        for(int i = 0; i < abs(x1-x2); i++)
            if(pieces[(yend-i)*8+(xbegin+i)].character != ' ' && ((yend-i!=y1) && (xbegin+i!=x1))){
                return 0;
            }
    return 1;
}
int in_bounds(int xord, int yord){
    return (int)!(xord < 0 || yord < 0 || xord > 7 || yord > 7);
}

int get_king(int colour, piece* pieces){
    for(int i = 0; i < 8*8; i++)
        if(pieces[i].data%2 == colour && pieces[i].character == 'K')
            return i;
}
// 1 for not in check, 0 for in check
int place_in_check(int xord, int yord, piece* pieces, piece king){
    piece* copy_board = (piece*)malloc(sizeof(piece)*8*8);
    memcpy(copy_board, pieces, sizeof(piece)*8*8);
    if(pieces[yord*8+xord].data == king.data && pieces[yord*8+xord].character == 'K')
        copy_board[yord*8+xord].character = ' ', copy_board[yord*8+xord].data = 0;
    for(int y = 0; y<8; y++)
        for(int x = 0; x<8; x++) 
            if(pieces[y*8+x].data%2 != king.data % 2)
                if(validate_movement_rules(x,y,xord,yord,copy_board)){
                    return y*8+x;
                }
    return 0;
}

void move(int x1, int y1, int x2, int y2, piece* pieces){
    piece current = pieces[y1*8+x1];
    pieces[y1*8+x1].character = ' ', pieces[y1*8+x1].data = 0;
    if(current.character == 'P' && current.data <5) current.data += 2; // change pawn data to show it has moved 1+ times
    pieces[y2*8+x2] = current;
}

// ensures move lies within rules of the piece being moved
int validate_movement_rules(int x1, int y1, int x2, int y2, piece* pieces){
    piece current = pieces[y1*8+x1];
    piece destination = pieces[y2*8+x2];
    // 1 for black, 0 for white
    int curcolour = current.data%2; 
    int destcolour = destination.data%2;
    //printf("Piece: %c FROM (%d, %d) to (%d, %d).\n", pieces[y1*8+x1], x1, y1, x2, y2);
    if(current.character == ' ') return 0; // can't move a none piece
    if(destination.data%2 == current.data%2 && destination.character!=' ') return 0; //can't take same colour piece
    if(destination.character == 'K') return 0; // can't take king

    if(x1==x2 && y1==y2) return 0; // no movement made
    // linear based movement validations
    // rook
    switch(current.character){
    case'R':;
        return validate_row_movement(x1, y1, x2, y2, pieces) || validate_column_movement(x1, y1, x2, y2, pieces);
    // bishop
    case 'B':;
        return validate_diagonal_movement(x1, y1, x2, y2, pieces);
    // queen
    case 'Q':;
        return validate_diagonal_movement(x1, y1, x2, y2, pieces) || validate_column_movement(x1, y1, x2, y2, pieces) || validate_row_movement(x1, y1, x2, y2, pieces);
    // pawn
    case 'P':;
        if(x1 != x2){ // pawn take
            if(destcolour == curcolour || destination.character==' ') return 0; // can't take own piece or take empty space
            if(abs(y2-y1)>1 || abs(x2-x1)>1) return 0; // check it is a 1 diagonal move (un percent added in future)
						return 1;
        }
        if(abs(y2-y1)>1){ // moves more than 1 forward
            if(current.data<5 && abs(y2-y1)==2 && pieces[((y1<y2?y1:y2)+1)*8+x1].character==' ') return 1; // 1st move allow it
            return 0; // invalid move!
        }
        if(x2-x1<0 && curcolour) return 0; //black backwards check
        if(x2-x1>0 && !curcolour) return 0; //white backwards check
        break;
    // knight
    case 'N':;
        if(x1==x2 || y1==y2) return 0; // cannot move sideways or upwards only
        if(!(abs(x2-x1)==1&&abs(y2-y1)==2 || abs(x2-x1)==2&&abs(y2-y1)==1)) return 0; // not L shaped movement
        break;
    // king
    case 'K':;
        if(abs(x1-x2)>1 || abs(y1-y2)>1) return 0; // cant move more than 1 distance away
        return place_in_check(x2,y2, pieces, pieces[y1*8+x1]); // pretend the king to be in location, check if location in check
    }
    return 1;
}

// validates the logic behind a move and considers whole board
int validate_movement(int x1, int y1, int x2, int y2, piece* pieces){
    if(!in_bounds(x1, y1) || !in_bounds(x2, y2)) return 0; // bounds check
    piece current = pieces[y1*8+x1];
    piece destination = pieces[y2*8+x2];
    // 1 for black, 0 for white
    int curcolour = current.data%2; 
    int destcolour = destination.data%2;
    // create board that would hold the potentially invalid movement
    piece* supposed_movement = (piece*)malloc(sizeof(piece)*8*8);
    memcpy(supposed_movement, pieces, sizeof(piece)*8*8);

    move(x1, y1, x2, y2, supposed_movement); // force movement 
    int king = get_king(curcolour, supposed_movement); // get index of king
    if(place_in_check(king%8, (king-king%8)/8, supposed_movement, supposed_movement[king])) return 0; // check if this move will put same colour's king in check
    // memory is freed after fn, no need for free()
    return validate_movement_rules(x1, y1, x2, y2, pieces); // move is valid
}

int pinning_piece(int xord, int yord, piece* pieces){
    int colour = pieces[yord*8+xord].data%2;
    piece* supposed_movement = (piece*)malloc(sizeof(piece)*8*8);
    memcpy(supposed_movement, pieces, sizeof(piece)*8*8);
    supposed_movement[yord*8+xord].data = 0;
    supposed_movement[yord*8+xord].character = ' ';

    int king = get_king(colour, supposed_movement); // get index of king
    return place_in_check(king%8, (king-king%8)/8, supposed_movement, supposed_movement[king]); //check if in check after piece removed;
}
// excludes king from calculation
int legal_move(int xord, int yord, piece* pieces){
    piece current = pieces[yord*8+xord];
    int pinnedBy = pinning_piece(xord, yord, pieces), cur_x, cur_y;

    int possible_movements[16];
    switch(current.character){
    case 'P':;
        int forwardY = current.data%2 ? yord+1 : yord-1; 
        if(in_bounds(xord, forwardY)&&pieces[forwardY*8+xord].character!=' '){
            if(in_bounds(xord-1, forwardY)&&pieces[forwardY*8+xord-1].data%2 != current.data%2&&pieces[forwardY*8+xord-1].character!=' '&&pieces[forwardY*8+xord-1].character!='K') return 1;
            if(in_bounds(xord+1, forwardY)&&pieces[forwardY*8+xord+1].data%2 != current.data%2&&pieces[forwardY*8+xord+1].character!=' '&&pieces[forwardY*8+xord+1].character!='K') return 1;
            return 0;
        }
        if(pinnedBy)
            return (in_bounds(xord-1, forwardY)&&(pinnedBy==yord*8+xord-1)) || (in_bounds(xord+1, forwardY)&&(pinnedBy==yord*8+xord-1));
        return 1;
    case 'N':;
        possible_movements[0] = xord+2, possible_movements[1] = yord+1;
        possible_movements[2] = xord+2, possible_movements[3] = yord-1;
        possible_movements[4] = xord-2, possible_movements[5] = yord+1;
        possible_movements[6] = xord-2, possible_movements[7] = yord-1;
        possible_movements[8] = xord+1, possible_movements[8] = yord-2;
        possible_movements[10]= xord+1, possible_movements[11]= yord+2;
        possible_movements[12]= xord-1, possible_movements[13]= yord-2;
        possible_movements[14]= xord+1, possible_movements[15]= yord-1;
        if(pinnedBy)
            return 0;
        for(int i = 0; i < 16; i+=2){
            cur_x = possible_movements[i], cur_y = possible_movements[i+1];
            if(in_bounds(cur_x, cur_y))
                if(!pieces[cur_y*8+cur_x].data%2==current.data%2 || pieces[cur_y*8+cur_x].character == ' ' && pieces[cur_y*8+cur_x].character != 'K')
                    return 1;
        }
        return 0;
    case 'B':;
        possible_movements[0] = xord-1, possible_movements[1] = yord-1;
        possible_movements[2] = xord+1, possible_movements[3] = yord-1;
        possible_movements[4] = xord-1, possible_movements[5] = yord+1;
        possible_movements[6] = xord+1, possible_movements[7] = yord+1;
        if(pinnedBy){
            if(pieces[pinnedBy].character == 'B') return 1;
            if(pieces[pinnedBy].character == 'Q') if(validate_movement(xord, yord, (pinnedBy-(pinnedBy%8))/8, pinnedBy%8, pieces)) return 1;
            return 0;
        }
        for(int i = 0; i < 8; i+=2){
            cur_x = possible_movements[i], cur_y = possible_movements[i+1];
            if(in_bounds(cur_x, cur_y))
                if(!pieces[cur_y*8+cur_x].data%2==current.data%2 || pieces[cur_y*8+cur_x].character == ' ' && pieces[cur_y*8+cur_x].character != 'K')
                    return 1;
        }
        return 0;
    case 'Q':;
        
        possible_movements[0] = xord-1, possible_movements[1] = yord-1;
        possible_movements[2] = xord, possible_movements[3] = yord-1;
        possible_movements[4] = xord+1, possible_movements[5] = yord-1;
        possible_movements[6] = xord-1, possible_movements[7] = yord;
        possible_movements[8] = xord+1, possible_movements[9] = yord;
        possible_movements[10] = xord-1, possible_movements[11] = yord+1;
        possible_movements[12] = xord, possible_movements[13] = yord+1;
        possible_movements[14] = xord+1, possible_movements[15] = yord+1;

        for(int i = 0; i < 16; i+=2){
            cur_x = possible_movements[i], cur_y = possible_movements[i+1];
            if(in_bounds(cur_x, cur_y))
                if(!pieces[cur_y*8+cur_x].data%2==current.data%2 || pieces[cur_y*8+cur_x].character == ' ' && pieces[cur_y*8+cur_x].character != 'K')
                    return 1;
        }
        return 0;
    case 'R':;
            possible_movements[0] = xord, possible_movements[1] = yord+1;
            possible_movements[2] = xord, possible_movements[3] = yord-1;
            possible_movements[4] = xord+1, possible_movements[5] = yord;
            possible_movements[6] = xord-1, possible_movements[7] = yord;
        if(pinnedBy){
            if(pieces[pinnedBy].character == 'R') return 1;
            if(pieces[pinnedBy].character == 'Q') if(validate_movement(xord, yord, (pinnedBy-(pinnedBy%8))/8, pinnedBy%8, pieces)) return 1;
            return 0;
        }
        for(int i = 0; i < 8; i+=2){
            cur_x = possible_movements[i], cur_y = possible_movements[i+1];
            if(in_bounds(cur_x, cur_y))
                if(!pieces[cur_y*8+cur_x].data%2==current.data%2 || pieces[cur_y*8+cur_x].character == ' ' && pieces[cur_y*8+cur_x].character != 'K')
                    return 1;
        }
        return 0;
    }
    return 0;
}


int stalemate(int xord, int yord, piece* pieces){
    const int possible_movements[] = {xord-1, yord-1, xord, yord-1, xord+1, yord-1, xord-1, yord, xord+1, yord, xord-1, yord+1, xord, yord+1, xord+1, yord+1};
    for(int y = 0; y < 8; y++)
        for(int x = 0; x < 8; x++)
            if(pieces[y*8+x].data%2==pieces[yord*8+xord].data%2)
                if(legal_move(x, y, pieces)){
                    return 0;
                }
    int cur_x,cur_y;
    for(int i = 0; i < 16; i+=2){
        cur_x = possible_movements[i], cur_y = possible_movements[i+1];
        if(in_bounds(cur_x, cur_y))
            if(!place_in_check(cur_x, cur_y, pieces, pieces[yord*8+xord]))
                if(!(pieces[cur_y*8+cur_x].data%2==pieces[yord*8+xord].data%2&&pieces[cur_y*8+cur_x].character!=' '))
                    return 0;
    }
    return 1;
}

int checkmate(int xord, int yord, piece* pieces){
    return place_in_check(xord, yord, pieces, pieces[yord*8+xord]) && stalemate(xord, yord, pieces);
}

int turn(int colour, piece* pieces){
    char text_colour[11];
    if(colour) strcpy(text_colour, COLOUR_RED "RED");
    else strcpy(text_colour, COLOUR_BLUE "BLUE");
    clearscrn();
    int king, x1, y1, x2, y2;
    begin:
        clearscrn();
        king = get_king(!colour, pieces);
        output_board(pieces);
        printf("%s%s PICK A PIECE\nX>", text_colour, COLOUR_RESET);
        scanf("%d", &x1);
        printf("Y>");
        scanf("%d", &y1);
        x1 -= 1, y1 -= 1;

        if(pieces[y1*8+x1].data%2&&!colour) goto begin;

        clearscrn();
        output_board(pieces);
        printf("%s%s WITH (%d, %d) MOVE\nX>",text_colour, COLOUR_RESET, x1+1, y1+1);
        scanf("%d", &x2);
        printf("Y>");
        scanf("%d", &y2);
        x2 -= 1, y2 -= 1;

        if(validate_movement(x1, y1, x2, y2, pieces)) move(x1, y1, x2, y2, pieces);
        else goto begin;

        clearscrn();
        output_board(pieces);

        if(checkmate(king%8,(king-king%8)/8, pieces)){
            printf("\nCHECKMATE %s%s WINS!", text_colour,  COLOUR_RESET);
            return 1;
        }else if(stalemate(king%8,(king-king%8)/8, pieces)){
            printf("\nSTALEMATE DRAW!");
            return 1;
        };
    return 0;
}
int main(){
    clearscrn();
    piece pieces[8*8] = {
         {'R', 1}, {'N', 1}, {'B', 1}, {'K', 1}, {'Q', 1}, {'B', 1}, {'N', 1}, {'R', 1},
         {'P', 3}, {'P', 3}, {'P', 3}, {'P', 3}, {'P', 3}, {'P', 3}, {'P', 3}, {'P', 3},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {'P', 4}, {'P', 4}, {'P', 4}, {'P', 4}, {'P', 4}, {'P', 4}, {'P', 4}, {'P', 4},
         {'R', 2}, {'N', 2}, {'B', 2}, {'Q', 2}, {'K', 2}, {'B', 2}, {'N', 2}, {'R', 2}
    };
		/*
		stalemate/checkmate with pawn-forward test
		piece pieces[8*8] = {
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {'R', 2}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {'B', 2}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {'P', 5},
         {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {' ', 0}, {'K', 1},
    };
		7,5 to 6,6 causes BLUE checkmate
		7,5 to any other available square causes stalemate
		*/
    while(1){ 
        if(turn(0, pieces)) return 0;
        if(turn(1, pieces)) return 0;
    }
    return 0;
}