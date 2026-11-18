#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
    char column;
    int row;
} Position;

typedef struct {
    Piece **board;
    char player;
} Game;


/* creer plateau */
Piece **createBoard(void)
{
    Piece **board = NULL;
    assert((board = (Piece **)malloc(BOARD_SIZE * sizeof(Piece *))) != NULL);

    for (int row = 0; row < BOARD_SIZE; row++)
        assert((board[row] = (Piece *)calloc(BOARD_SIZE, sizeof(Piece))) != NULL);

    return board;
}

/* placer piece */
void placePiece(Piece **board, int row, int column, char type, char color)
{
    board[row][column].type = type;
    board[row][column].color = color;
}

/* depart */
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


/* pieces: type, couleur et si fou couleur des cases */
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

/* nom des piece */
const char *pieceName(char type)
{
    switch (type) {
        case PAWN:   return "pion";
        case ROOK:   return "tour";
        case KNIGHT: return "cavalier";
        case BISHOP: return "fou";
        case QUEEN:  return "dame";
        case KING:   return "roi";
        default:     return "";
    }
}

/* affichage plateau */
void printBoard(Piece **board)
{
    char code[4];

    printf("\n    A   B   C   D   E   F   G   H\n");
    printf("  +---+---+---+---+---+---+---+---+\n");

    for (int row = BOARD_SIZE; row >= 1; row--) {
        printf("%d ", row);
        for (char column = 'A'; column <= 'H'; column++) {
            Position position = {column, row};
            pieceCode(board[row - 1][column - 'A'], position, code);
            printf("|%-3s", code);
        }
        printf("| %d\n", row);
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
}


/* deplacement  pion */
int pawnMove(Piece **board, Position from, Position to)
{
    Piece pawn = board[from.row - 1][from.column - 'A'];
    Piece target = board[to.row - 1][to.column - 'A'];
    int direction = (pawn.color == WHITE) ? 1 : -1;
    int startRow = (pawn.color == WHITE) ? 2 : 7;
    int rowsMoved = to.row - from.row;
    int columnGap = abs(to.column - from.column);

    if (columnGap == 0 && target.type == EMPTY) {
        if (rowsMoved == direction)
            return 1;
        if (from.row == startRow && rowsMoved == 2 * direction
            && board[from.row + direction - 1][from.column - 'A'].type == EMPTY)
            return 1;
    }

    if (columnGap == 1 && rowsMoved == direction && target.type != EMPTY)
        return 1;

    return 0;
}

/* deplacement cavalier */
int knightMove(Position from, Position to)
{
    int rowGap = abs(to.row - from.row);
    int columnGap = abs(to.column - from.column);
    return (rowGap == 2 && columnGap == 1) || (rowGap == 1 && columnGap == 2);
}

/* deplacement  fou */
int bishopMove(Piece **board, Position from, Position to)
{
    if (abs(to.row - from.row) != abs(to.column - from.column))
        return 0;

    int rowStep = (to.row > from.row) ? 1 : -1;
    int columnStep = (to.column > from.column) ? 1 : -1;
    Position current = {from.column + columnStep, from.row + rowStep};

    while (current.row != to.row) {
        if (board[current.row - 1][current.column - 'A'].type != EMPTY)
            return 0;
        current.row += rowStep;
        current.column += columnStep;
    }
    return 1;
}

/* verif deplacement selon la piece */
int isValidMove(Piece **board, Position from, Position to)
{
    switch (board[from.row - 1][from.column - 'A'].type) {
        case PAWN:   return pawnMove(board, from, to);
        case KNIGHT: return knightMove(from, to);
        case BISHOP: return bishopMove(board, from, to);
        default:     return 1; /* tour, dame, roi : a coder */
    }
}

/* deplace la piece  */
void movePiece(Piece **board, Position from, Position to)
{
    board[to.row - 1][to.column - 'A'] = board[from.row - 1][from.column - 'A'];
    board[from.row - 1][from.column - 'A'].type = EMPTY;
    board[from.row - 1][from.column - 'A'].color = EMPTY;
}


/*  lettre en majuscule */
char toUpper(char letter)
{
    if (letter >= 'a' && letter <= 'z')
        return letter - 'a' + 'A';
    return letter;
}

/* verifier case plateau */
int isOnBoard(Position position)
{
    return position.column >= 'A' && position.column <= 'H'
        && position.row >= 1 && position.row <= BOARD_SIZE;
}

/* promotion */
void promotePawn(Piece **board, Position position)
{
    Piece *piece = &board[position.row - 1][position.column - 'A'];
    int lastRow = (piece->color == WHITE) ? BOARD_SIZE : 1;

    if (piece->type != PAWN || position.row != lastRow)
        return;

    char choice = EMPTY;

    while (choice != QUEEN && choice != ROOK && choice != BISHOP && choice != KNIGHT) {
        printf("Promotion ! Choisir D, T, F ou C : ");
        if (scanf(" %c", &choice) != 1)
            choice = QUEEN;
        choice = toUpper(choice);
    }
    piece->type = choice;
}


/* creation */
Game *createGame(void)
{
    Game *game = NULL;
    assert((game = (Game *)malloc(sizeof(Game))) != NULL);

    game->board = createBoard();
    initBoard(game->board);
    game->player = WHITE;
    return game;
}

/* verifier la case avec une piece du joueur */
int isPlayerPiece(Game *game, Position position)
{
    Piece piece = game->board[position.row - 1][position.column - 'A'];
    return piece.type != EMPTY && piece.color == game->player;
}

/* choix de la piece a deplacer */
int choosePiece(Game *game, Position *from)
{
    printf("Au tour des %s\n", (game->player == WHITE) ? "Blancs" : "Noirs");
    printf("Position initiale (ex : E2, Q pour quitter) : ");

    if (scanf(" %c", &from->column) != 1)
        return QUIT;
    from->column = toUpper(from->column);
    if (from->column == 'Q')
        return QUIT;

    if (scanf("%d", &from->row) != 1 || !isOnBoard(*from) || !isPlayerPiece(game, *from)) {
        printf("Position invalide, pas de piece a vous ici\n\n");
        return 0;
    }

    char code[4];
    pieceCode(game->board[from->row - 1][from->column - 'A'], *from, code);
    printf("Piece selectionnee : %s\n", code);
    return 1;
}

/* choix de la case d'arrivee */
int chooseDestination(Game *game, Position from, Position *to)
{
    printf("Position finale : ");

    if (scanf(" %c", &to->column) != 1)
        return QUIT;
    to->column = toUpper(to->column);
    if (to->column == 'Q')
        return QUIT;

    if (scanf("%d", &to->row) != 1 || !isOnBoard(*to) || isPlayerPiece(game, *to)) {
        printf("Position impossible\n\n");
        return 0;
    }

    if (!isValidMove(game->board, from, *to)) {
        printf("Cette piece ne peut pas se deplacer ici\n\n");
        return 0;
    }
    return 1;
}

/*  passe au joueur suivant */
void playMove(Game *game, Position from, Position to)
{
    Piece piece = game->board[from.row - 1][from.column - 'A'];
    Piece target = game->board[to.row - 1][to.column - 'A'];

    if (target.type != EMPTY)
        printf("%s %c%d mange %s %c%d\n",
               pieceName(piece.type), from.column, from.row,
               pieceName(target.type), to.column, to.row);

    movePiece(game->board, from, to);
    promotePawn(game->board, to);

    game->player = (game->player == WHITE) ? BLACK : WHITE;
    printBoard(game->board);
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

    playMove(game, from, to);
    return 1;
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

    playGame(game);

    for (int row = 0; row < BOARD_SIZE; row++) {
        free(game->board[row]);
        game->board[row] = NULL;
    }
    free(game->board);
    game->board = NULL;
    free(game);
    game = NULL;

    return 0;
}
