#include <stdio.h>
#include <stdlib.h>

#define TAILLE 8
#define QUITTER -1

#define VIDE 0
#define BLANC 'B'
#define NOIR 'N'

#define PION 'P'
#define TOUR 'T'
#define CAVALIER 'C'
#define FOU 'F'
#define DAME 'D'
#define ROI 'R'

typedef struct {
    char type;
    char couleur;
} Piece;

typedef struct {
    int ligne;
    int colonne;
} Position;

typedef struct {
    char texte[10];
} Coup;

typedef struct {
    Coup *coups;
    int nombre;
    int capacite;
} Historique;

typedef struct {
    Piece **plateau;
    Historique historique;
    char joueur;
} Partie;


/* creation du plateau */
Piece **creerPlateau(void)
{
    Piece **plateau = malloc(TAILLE * sizeof(Piece *));
    if (plateau == NULL)
        return NULL;

    for (int ligne = 0; ligne < TAILLE; ligne++) {
        plateau[ligne] = calloc(TAILLE, sizeof(Piece));
        if (plateau[ligne] == NULL) {
            for (int dejaCreee = 0; dejaCreee < ligne; dejaCreee++)
                free(plateau[dejaCreee]);
            free(plateau);
            return NULL;
        }
    }
    return plateau;
}

/* liberation du plateau */
void libererPlateau(Piece **plateau)
{
    for (int ligne = 0; ligne < TAILLE; ligne++)
        free(plateau[ligne]);
    free(plateau);
}

/* pose une piece sur une case */
void placerPiece(Piece **plateau, int ligne, int colonne, char type, char couleur)
{
    plateau[ligne][colonne].type = type;
    plateau[ligne][colonne].couleur = couleur;
}

/* position de depart */
void initialiserPlateau(Piece **plateau)
{
    char ordre[TAILLE] = {TOUR, CAVALIER, FOU, DAME, ROI, FOU, CAVALIER, TOUR};

    for (int colonne = 0; colonne < TAILLE; colonne++) {
        placerPiece(plateau, 0, colonne, ordre[colonne], BLANC);
        placerPiece(plateau, 1, colonne, PION, BLANC);
        placerPiece(plateau, 6, colonne, PION, NOIR);
        placerPiece(plateau, 7, colonne, ordre[colonne], NOIR);
    }
}


/* code d'une piece : PB, CN, FBB... */
void codePiece(Piece piece, Position position, char code[4])
{
    code[0] = '\0';
    if (piece.type == VIDE)
        return;

    code[0] = piece.type;
    code[1] = piece.couleur;
    code[2] = '\0';

    if (piece.type == FOU) {
        code[2] = ((position.ligne + position.colonne) % 2 == 0) ? NOIR : BLANC;
        code[3] = '\0';
    }
}

/* affichage du plateau */
void afficherPlateau(Piece **plateau)
{
    char code[4];

    printf("\n    A   B   C   D   E   F   G   H\n");
    printf("  +---+---+---+---+---+---+---+---+\n");

    for (int ligne = TAILLE - 1; ligne >= 0; ligne--) {
        printf("%d ", ligne + 1);
        for (int colonne = 0; colonne < TAILLE; colonne++) {
            Position position = {ligne, colonne};
            codePiece(plateau[ligne][colonne], position, code);
            printf("|%-3s", code);
        }
        printf("| %d\n", ligne + 1);
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
}


/* creation de l'historique */
int creerHistorique(Historique *historique)
{
    historique->nombre = 0;
    historique->capacite = 4;
    historique->coups = malloc(historique->capacite * sizeof(Coup));
    return historique->coups != NULL;
}

/* liberation de l'historique */
void libererHistorique(Historique *historique)
{
    free(historique->coups);
    historique->coups = NULL;
}

/* agrandit l'historique quand il est plein */
int agrandirHistorique(Historique *historique)
{
    int nouvelleCapacite = historique->capacite * 2;
    Coup *nouveauxCoups = realloc(historique->coups, nouvelleCapacite * sizeof(Coup));
    if (nouveauxCoups == NULL)
        return 0;

    historique->coups = nouveauxCoups;
    historique->capacite = nouvelleCapacite;
    return 1;
}

/* ajoute un coup, ex : PB E4 ou PB E8=D */
int ajouterCoup(Historique *historique, Piece piece, Position depart, Position arrivee, char promotion)
{
    if (historique->nombre == historique->capacite && !agrandirHistorique(historique))
        return 0;

    char code[4];
    codePiece(piece, depart, code);

    Coup *coup = &historique->coups[historique->nombre];
    if (promotion == VIDE)
        sprintf(coup->texte, "%s %c%c", code, 'A' + arrivee.colonne, '1' + arrivee.ligne);
    else
        sprintf(coup->texte, "%s %c%c=%c", code, 'A' + arrivee.colonne, '1' + arrivee.ligne, promotion);

    historique->nombre++;
    return 1;
}

/* affiche les coups joues */
void afficherHistorique(Historique *historique)
{
    printf("Coups joues : %d\n", historique->nombre);
    for (int numero = 0; numero < historique->nombre; numero++) {
        if (numero % 2 == 0)
            printf("%d. ", numero / 2 + 1);
        printf("%s  ", historique->coups[numero].texte);
        if (numero % 2 == 1)
            printf("\n");
    }
    printf("\n");
}


/* deplacement du pion */
int deplacementPion(Piece **plateau, Position depart, Position arrivee)
{
    Piece pion = plateau[depart.ligne][depart.colonne];
    Piece cible = plateau[arrivee.ligne][arrivee.colonne];
    int sens = (pion.couleur == BLANC) ? 1 : -1;
    int ligneInitiale = (pion.couleur == BLANC) ? 1 : 6;
    int avancee = arrivee.ligne - depart.ligne;
    int ecartColonnes = abs(arrivee.colonne - depart.colonne);

    if (ecartColonnes == 0 && cible.type == VIDE) {
        if (avancee == sens)
            return 1;
        if (depart.ligne == ligneInitiale && avancee == 2 * sens
            && plateau[depart.ligne + sens][depart.colonne].type == VIDE)
            return 1;
    }

    if (ecartColonnes == 1 && avancee == sens && cible.type != VIDE)
        return 1;

    return 0;
}

/* deplacement du cavalier */
int deplacementCavalier(Position depart, Position arrivee)
{
    int ecartLignes = abs(arrivee.ligne - depart.ligne);
    int ecartColonnes = abs(arrivee.colonne - depart.colonne);
    return (ecartLignes == 2 && ecartColonnes == 1) || (ecartLignes == 1 && ecartColonnes == 2);
}

/* deplacement du fou */
int deplacementFou(Piece **plateau, Position depart, Position arrivee)
{
    if (abs(arrivee.ligne - depart.ligne) != abs(arrivee.colonne - depart.colonne))
        return 0;

    int sensLigne = (arrivee.ligne > depart.ligne) ? 1 : -1;
    int sensColonne = (arrivee.colonne > depart.colonne) ? 1 : -1;
    Position courante = {depart.ligne + sensLigne, depart.colonne + sensColonne};

    while (courante.ligne != arrivee.ligne) {
        if (plateau[courante.ligne][courante.colonne].type != VIDE)
            return 0;
        courante.ligne += sensLigne;
        courante.colonne += sensColonne;
    }
    return 1;
}

/* verification d'un coup */
int coupValide(Piece **plateau, Position depart, Position arrivee)
{
    Piece piece = plateau[depart.ligne][depart.colonne];
    Piece cible = plateau[arrivee.ligne][arrivee.colonne];

    if (depart.ligne == arrivee.ligne && depart.colonne == arrivee.colonne)
        return 0;
    if (cible.type != VIDE && cible.couleur == piece.couleur)
        return 0;

    switch (piece.type) {
        case PION:     return deplacementPion(plateau, depart, arrivee);
        case CAVALIER: return deplacementCavalier(depart, arrivee);
        case FOU:      return deplacementFou(plateau, depart, arrivee);
        default:       return 1; /* tour, dame, roi : a coder */
    }
}

/* deplace la piece sur le plateau */
void deplacerPiece(Piece **plateau, Position depart, Position arrivee)
{
    plateau[arrivee.ligne][arrivee.colonne] = plateau[depart.ligne][depart.colonne];
    placerPiece(plateau, depart.ligne, depart.colonne, VIDE, VIDE);
}


/* passe une lettre en majuscule */
char majuscule(char lettre)
{
    if (lettre >= 'a' && lettre <= 'z')
        return lettre - 'a' + 'A';
    return lettre;
}

/* verifie qu'une case est sur le plateau */
int dansPlateau(Position position)
{
    return position.ligne >= 0 && position.ligne < TAILLE
        && position.colonne >= 0 && position.colonne < TAILLE;
}

/* lit une case, ex : E2 */
int lireCase(Position *position)
{
    char saisie[32] = {0};

    if (fgets(saisie, sizeof(saisie), stdin) == NULL)
        return QUITTER;
    if (majuscule(saisie[0]) == 'Q')
        return QUITTER;

    position->colonne = majuscule(saisie[0]) - 'A';
    position->ligne = saisie[1] - '1';
    return dansPlateau(*position);
}

/* promotion du pion */
char promotion(Piece **plateau, Position position)
{
    Piece *piece = &plateau[position.ligne][position.colonne];
    int derniereLigne = (piece->couleur == BLANC) ? TAILLE - 1 : 0;

    if (piece->type != PION || position.ligne != derniereLigne)
        return VIDE;

    char saisie[32] = {0};
    char choix = VIDE;

    while (choix != DAME && choix != TOUR && choix != FOU && choix != CAVALIER) {
        printf("Promotion ! Choisir D, T, F ou C : ");
        if (fgets(saisie, sizeof(saisie), stdin) == NULL)
            choix = DAME;
        else
            choix = majuscule(saisie[0]);
    }
    piece->type = choix;
    return choix;
}


/* creation de la partie */
Partie *creerPartie(void)
{
    Partie *partie = malloc(sizeof(Partie));
    if (partie == NULL)
        return NULL;

    partie->plateau = creerPlateau();
    if (partie->plateau == NULL) {
        free(partie);
        return NULL;
    }

    if (!creerHistorique(&partie->historique)) {
        libererPlateau(partie->plateau);
        free(partie);
        return NULL;
    }

    initialiserPlateau(partie->plateau);
    partie->joueur = BLANC;
    return partie;
}

/* liberation de la partie */
void libererPartie(Partie *partie)
{
    libererHistorique(&partie->historique);
    libererPlateau(partie->plateau);
    free(partie);
}

/* verifie que la case contient une piece du joueur */
int pieceDuJoueur(Partie *partie, Position position)
{
    Piece piece = partie->plateau[position.ligne][position.colonne];
    return piece.type != VIDE && piece.couleur == partie->joueur;
}

/* choix de la piece a deplacer */
int choisirPiece(Partie *partie, Position *depart)
{
    printf("Au tour des %s\n", (partie->joueur == BLANC) ? "Blancs" : "Noirs");
    printf("Entrez votre position (ex : E2, Q pour quitter) : ");

    int resultat = lireCase(depart);
    if (resultat == QUITTER)
        return QUITTER;

    if (resultat == 0 || !pieceDuJoueur(partie, *depart)) {
        printf("Position invalide, pas de piece a vous ici\n\n");
        return 0;
    }

    char code[4];
    codePiece(partie->plateau[depart->ligne][depart->colonne], *depart, code);
    printf("Piece choisie : %s\n", code);
    return 1;
}

/* choix de la case d'arrivee */
int choisirDestination(Partie *partie, Position depart, Position *arrivee)
{
    printf("Entrez la destination : ");

    int resultat = lireCase(arrivee);
    if (resultat == QUITTER)
        return QUITTER;

    if (resultat == 0 || !coupValide(partie->plateau, depart, *arrivee)) {
        printf("Cette piece ne peut pas se deplacer ici\n\n");
        return 0;
    }
    return 1;
}

/* joue le coup et passe au joueur suivant */
int jouerCoup(Partie *partie, Position depart, Position arrivee)
{
    Piece piece = partie->plateau[depart.ligne][depart.colonne];

    deplacerPiece(partie->plateau, depart, arrivee);
    char choix = promotion(partie->plateau, arrivee);

    if (!ajouterCoup(&partie->historique, piece, depart, arrivee, choix)) {
        printf("Erreur memoire\n");
        return 0;
    }

    partie->joueur = (partie->joueur == BLANC) ? NOIR : BLANC;
    afficherPlateau(partie->plateau);
    afficherHistorique(&partie->historique);
    return 1;
}

/* un tour de jeu */
int jouerTour(Partie *partie)
{
    Position depart;
    Position arrivee;

    int resultat = choisirPiece(partie, &depart);
    if (resultat == QUITTER)
        return 0;
    if (resultat == 0)
        return 1;

    resultat = choisirDestination(partie, depart, &arrivee);
    if (resultat == QUITTER)
        return 0;
    if (resultat == 0)
        return 1;

    return jouerCoup(partie, depart, arrivee);
}

/* boucle de jeu */
void jouerPartie(Partie *partie)
{
    int continuer = 1;

    afficherPlateau(partie->plateau);
    while (continuer)
        continuer = jouerTour(partie);
}


int main(void)
{
    Partie *partie = creerPartie();
    if (partie == NULL) {
        printf("Erreur memoire\n");
        return 1;
    }

    jouerPartie(partie);
    libererPartie(partie);
    return 0;
}
