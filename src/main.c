#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 8
#define QUIT -1

#define EMPTY 0
#define WHITE 'B'
#define BLACK 'N'

#define PAWN 'P'
#define ROOK 'T'
#define KNIGHT 'C'
#define BISHOP 'F'
#define QUEEN 'D'
#define KING 'R'

typedef struct {
    char type;
    char color;
} Piece;

typedef struct {
    int row;
    int column;
} Position;

typedef struct {
    char text[10];
} Move;

typedef struct {
    Move *moves;
    int count;
    int capacity;
} History;

typedef struct {
    Piece **board;
    History history;
    char player;
} Game;


/* creation du plateau */
Piece **createBoard(void)
{
    Piece **board = malloc(BOARD_SIZE * sizeof(Piece *));
    if (board == NULL)
        return NULL;

    for (int row = 0; row < BOARD_SIZE; row++) {
        board[row] = calloc(BOARD_SIZE, sizeof(Piece));
        if (board[row] == NULL) {
            for (int created = 0; created < row; created++)
                free(board[created]);
            free(board);
            return NULL;
        }
    }
    return board;
}

/* liberation du plateau */
void freeBoard(Piece **board)
{
    for (int row = 0; row < BOARD_SIZE; row++)
        free(board[row]);
    free(board);
}

/* pose une piece sur une case */
void placePiece(Piece **board, int row, int column, char type, char color)
{
    board[row][column].type = type;
    board[row][column].color = color;
}

/* position de depart */
void initBoard(Piece **board)
{
    char order[BOARD_SIZE] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};

    for (int column = 0; column < BOARD_SIZE; column++) {
        placePiece(board, 0, column, order[column], WHITE);
        placePiece(board, 1, column, PAWN, WHITE);
        placePiece(board, 6, column, PAWN, BLACK);
        placePiece(board, 7, column, order[column], BLACK);
    }
}


/* code d'une piece : PB, CN, FBB... */
void pieceCode(Piece piece, Position position, char code[4])
{
    code[0] = '\0';
    if (piece.type == EMPTY)
        return;

    code[0] = piece.type;
    code[1] = piece.color;
    code[2] = '\0';

    if (piece.type == BISHOP) {
        code[2] = ((position.row + position.column) % 2 == 0) ? BLACK : WHITE;
        code[3] = '\0';
    }
}

/* affichage du plateau */
void printBoard(Piece **board)
{
    char code[4];

    printf("\n    A   B   C   D   E   F   G   H\n");
    printf("  +---+---+---+---+---+---+---+---+\n");

    for (int row = BOARD_SIZE - 1; row >= 0; row--) {
        printf("%d ", row + 1);
        for (int column = 0; column < BOARD_SIZE; column++) {
            Position position = {row, column};
            pieceCode(board[row][column], position, code);
            printf("|%-3s", code);
        }
        printf("| %d\n", row + 1);
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
}


/* creation de l'historique */
int createHistory(History *history)
{
    history->count = 0;
    history->capacity = 4;
    history->moves = malloc(history->capacity * sizeof(Move));
    return history->moves != NULL;
}

/* liberation de l'historique */
void freeHistory(History *history)
{
    free(history->moves);
    history->moves = NULL;
}

/* agrandit l'historique quand il est plein */
int growHistory(History *history)
{
    int newCapacity = history->capacity * 2;
    Move *newMoves = realloc(history->moves, newCapacity * sizeof(Move));
    if (newMoves == NULL)
        return 0;

    history->moves = newMoves;
    history->capacity = newCapacity;
    return 1;
}

/* ajoute un coup, ex : PB E4 ou PB E8=D */
int addMove(History *history, Piece piece, Position from, Position to, char promotion)
{
    if (history->count == history->capacity && !growHistory(history))
        return 0;

    char code[4];
    pieceCode(piece, from, code);

    Move *move = &history->moves[history->count];
    if (promotion == EMPTY)
        sprintf(move->text, "%s %c%c", code, 'A' + to.column, '1' + to.row);
    else
        sprintf(move->text, "%s %c%c=%c", code, 'A' + to.column, '1' + to.row, promotion);

    history->count++;
    return 1;
}

/* affiche les coups joues */
void printHistory(History *history)
{
    printf("Coups joues : %d\n", history->count);
    for (int index = 0; index < history->count; index++) {
        if (index % 2 == 0)
            printf("%d. ", index / 2 + 1);
        printf("%s  ", history->moves[index].text);
        if (index % 2 == 1)
            printf("\n");
    }
    printf("\n");
}


/* deplacement du pion */
int pawnMove(Piece **board, Position from, Position to)
{
    Piece pawn = board[from.row][from.column];
    Piece target = board[to.row][to.column];
    int direction = (pawn.color == WHITE) ? 1 : -1;
    int startRow = (pawn.color == WHITE) ? 1 : 6;
    int rowsMoved = to.row - from.row;
    int columnGap = abs(to.column - from.column);

    if (columnGap == 0 && target.type == EMPTY) {
        if (rowsMoved == direction)
            return 1;
        if (from.row == startRow && rowsMoved == 2 * direction
            && board[from.row + direction][from.column].type == EMPTY)
            return 1;
    }

    if (columnGap == 1 && rowsMoved == direction && target.type != EMPTY)
        return 1;

    return 0;
}

/* deplacement du cavalier */
int knightMove(Position from, Position to)
{
    int rowGap = abs(to.row - from.row);
    int columnGap = abs(to.column - from.column);
    return (rowGap == 2 && columnGap == 1) || (rowGap == 1 && columnGap == 2);
}

/* deplacement du fou */
int bishopMove(Piece **board, Position from, Position to)
{
    if (abs(to.row - from.row) != abs(to.column - from.column))
        return 0;

    int rowStep = (to.row > from.row) ? 1 : -1;
    int columnStep = (to.column > from.column) ? 1 : -1;
    Position current = {from.row + rowStep, from.column + columnStep};

    while (current.row != to.row) {
        if (board[current.row][current.column].type != EMPTY)
            return 0;
        current.row += rowStep;
        current.column += columnStep;
    }
    return 1;
}

/* verification d'un coup */
int isValidMove(Piece **board, Position from, Position to)
{
    Piece piece = board[from.row][from.column];
    Piece target = board[to.row][to.column];

    if (from.row == to.row && from.column == to.column)
        return 0;
    if (target.type != EMPTY && target.color == piece.color)
        return 0;

    switch (piece.type) {
        case PAWN:   return pawnMove(board, from, to);
        case KNIGHT: return knightMove(from, to);
        case BISHOP: return bishopMove(board, from, to);
        default:     return 1; /* tour, dame, roi : a coder */
    }
}

/* deplace la piece sur le plateau */
void movePiece(Piece **board, Position from, Position to)
{
    board[to.row][to.column] = board[from.row][from.column];
    placePiece(board, from.row, from.column, EMPTY, EMPTY);
}


/* passe une lettre en majuscule */
char toUpper(char letter)
{
    if (letter >= 'a' && letter <= 'z')
        return letter - 'a' + 'A';
    return letter;
}

/* verifie qu'une case est sur le plateau */
int isOnBoard(Position position)
{
    return position.row >= 0 && position.row < BOARD_SIZE
        && position.column >= 0 && position.column < BOARD_SIZE;
}

/* lit une case, ex : E2 */
int readSquare(Position *position)
{
    char input[32] = {0};

    if (fgets(input, sizeof(input), stdin) == NULL)
        return QUIT;
    if (toUpper(input[0]) == 'Q')
        return QUIT;

    position->column = toUpper(input[0]) - 'A';
    position->row = input[1] - '1';
    return isOnBoard(*position);
}

/* promotion du pion */
char promotePawn(Piece **board, Position position)
{
    Piece *piece = &board[position.row][position.column];
    int lastRow = (piece->color == WHITE) ? BOARD_SIZE - 1 : 0;

    if (piece->type != PAWN || position.row != lastRow)
        return EMPTY;

    char input[32] = {0};
    char choice = EMPTY;

    while (choice != QUEEN && choice != ROOK && choice != BISHOP && choice != KNIGHT) {
        printf("Promotion ! Choisir D, T, F ou C : ");
        if (fgets(input, sizeof(input), stdin) == NULL)
            choice = QUEEN;
        else
            choice = toUpper(input[0]);
    }
    piece->type = choice;
    return choice;
}


/* creation de la partie */
Game *createGame(void)
{
    Game *game = malloc(sizeof(Game));
    if (game == NULL)
        return NULL;

    game->board = createBoard();
    if (game->board == NULL) {
        free(game);
        return NULL;
    }

    if (!createHistory(&game->history)) {
        freeBoard(game->board);
        free(game);
        return NULL;
    }

    initBoard(game->board);
    game->player = WHITE;
    return game;
}

/* liberation de la partie */
void freeGame(Game *game)
{
    freeHistory(&game->history);
    freeBoard(game->board);
    free(game);
}

/* verifie que la case contient une piece du joueur */
int isPlayerPiece(Game *game, Position position)
{
    Piece piece = game->board[position.row][position.column];
    return piece.type != EMPTY && piece.color == game->player;
}

/* choix de la piece a deplacer */
int choosePiece(Game *game, Position *from)
{
    printf("Au tour des %s\n", (game->player == WHITE) ? "Blancs" : "Noirs");
    printf("Entrez votre position (ex : E2, Q pour quitter) : ");

    int result = readSquare(from);
    if (result == QUIT)
        return QUIT;

    if (result == 0 || !isPlayerPiece(game, *from)) {
        printf("Position invalide, pas de piece a vous ici\n\n");
        return 0;
    }

    char code[4];
    pieceCode(game->board[from->row][from->column], *from, code);
    printf("Piece choisie : %s\n", code);
    return 1;
}

/* choix de la case d'arrivee */
int chooseDestination(Game *game, Position from, Position *to)
{
    printf("Entrez la destination : ");

    int result = readSquare(to);
    if (result == QUIT)
        return QUIT;

    if (result == 0 || !isValidMove(game->board, from, *to)) {
        printf("Cette piece ne peut pas se deplacer ici\n\n");
        return 0;
    }
    return 1;
}

/* joue le coup et passe au joueur suivant */
int playMove(Game *game, Position from, Position to)
{
    Piece piece = game->board[from.row][from.column];

    movePiece(game->board, from, to);
    char promotion = promotePawn(game->board, to);

    if (!addMove(&game->history, piece, from, to, promotion)) {
        printf("Erreur memoire\n");
        return 0;
    }

    game->player = (game->player == WHITE) ? BLACK : WHITE;
    printBoard(game->board);
    printHistory(&game->history);
    return 1;
}

/* un tour de jeu */
int playTurn(Game *game)
{
    Position from;
    Position to;

    int result = choosePiece(game, &from);
    if (result == QUIT)
        return 0;
    if (result == 0)
        return 1;

    result = chooseDestination(game, from, &to);
    if (result == QUIT)
        return 0;
    if (result == 0)
        return 1;

    return playMove(game, from, to);
}

/* boucle de jeu */
void playGame(Game *game)
{
    int running = 1;

    printBoard(game->board);
    while (running)
        running = playTurn(game);
}


int main(void)
{
    Game *game = createGame();
    if (game == NULL) {
        printf("Erreur memoire\n");
        return 1;
    }

    playGame(game);
    freeGame(game);
    return 0;
}
