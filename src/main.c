#include <stdio.h>
#include <stdlib.h>

#define TAILLE 8

typedef struct {
    char type;
    char couleur;
} Piece;

typedef struct {
    char texte[8];
} Coup;

typedef struct {
    Coup *coups;
    int nb;
    int capacite;
} Historique;


/* creation du plateau */
Piece **creerPlateau(void)
{
    Piece **plateau = malloc(TAILLE * sizeof(Piece *));
    if (plateau == NULL)
        return NULL;

    for (int i = 0; i < TAILLE; i++) {
        plateau[i] = calloc(TAILLE, sizeof(Piece));
        if (plateau[i] == NULL)
            return NULL;
    }
    return plateau;
}

/* position de depart */
void initialiserPlateau(Piece **plateau)
{
    char ordre[] = "TCFDRFCT";

    for (int c = 0; c < TAILLE; c++) {
        plateau[0][c].type = ordre[c];
        plateau[0][c].couleur = 'B';
        plateau[1][c].type = 'P';
        plateau[1][c].couleur = 'B';

        plateau[6][c].type = 'P';
        plateau[6][c].couleur = 'N';
        plateau[7][c].type = ordre[c];
        plateau[7][c].couleur = 'N';
    }
}

/* code d'une piece : PB, CN, FBB... */
void codePiece(Piece p, int lig, int col, char code[4])
{
    code[0] = ' ';
    code[1] = ' ';
    code[2] = ' ';
    code[3] = '\0';

    if (p.type == 0)
        return;

    code[0] = p.type;
    code[1] = p.couleur;
    if (p.type == 'F')
        code[2] = ((lig + col) % 2 == 0) ? 'N' : 'B';
}

/* affichage du plateau */
void afficherPlateau(Piece **plateau)
{
    char code[4];

    printf("\n    A   B   C   D   E   F   G   H\n");
    printf("  +---+---+---+---+---+---+---+---+\n");

    for (int lig = TAILLE - 1; lig >= 0; lig--) {
        printf("%d ", lig + 1);
        for (int col = 0; col < TAILLE; col++) {
            codePiece(plateau[lig][col], lig, col, code);
            printf("|%s", code);
        }
        printf("| %d\n", lig + 1);
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    A   B   C   D   E   F   G   H\n\n");
}


/* historique des coups */
void initHistorique(Historique *h)
{
    h->nb = 0;
    h->capacite = 4;
    h->coups = malloc(h->capacite * sizeof(Coup));
}

/* ajoute un coup, ex : PB E4 */
void ajouterCoup(Historique *h, Piece p, int lig, int col)
{
    if (h->nb == h->capacite) {
        int nouvelleCapacite = h->capacite * 2;
        Coup *tmp = realloc(h->coups, nouvelleCapacite * sizeof(Coup));
        if (tmp == NULL)
            return;
        h->coups = tmp;
        h->capacite = nouvelleCapacite;
    }

    Coup *c = &h->coups[h->nb];
    c->texte[0] = p.type;
    c->texte[1] = p.couleur;
    c->texte[2] = ' ';
    c->texte[3] = 'A' + col;
    c->texte[4] = '1' + lig;
    c->texte[5] = '\0';
    h->nb++;
}

/* affiche les coups joues */
void afficherHistorique(Historique *h)
{
    printf("Coups joues : %d\n", h->nb);
    for (int i = 0; i < h->nb; i++) {
        if (i % 2 == 0)
            printf("%d. ", i / 2 + 1);
        printf("%s  ", h->coups[i].texte);
        if (i % 2 == 1)
            printf("\n");
    }
    printf("\n");
}


/* deplacement du pion */
int deplacementPion(Piece **plateau, int l1, int c1, int l2, int c2)
{
    int sens = (plateau[l1][c1].couleur == 'B') ? 1 : -1;
    int depart = (plateau[l1][c1].couleur == 'B') ? 1 : 6;

    if (c1 == c2 && plateau[l2][c2].type == 0) {
        if (l2 - l1 == sens)
            return 1;
        if (l1 == depart && l2 - l1 == 2 * sens && plateau[l1 + sens][c1].type == 0)
            return 1;
    }

    if (abs(c2 - c1) == 1 && l2 - l1 == sens && plateau[l2][c2].type != 0)
        return 1;

    return 0;
}

/* deplacement du cavalier */
int deplacementCavalier(int l1, int c1, int l2, int c2)
{
    int dl = abs(l2 - l1);
    int dc = abs(c2 - c1);
    return (dl == 2 && dc == 1) || (dl == 1 && dc == 2);
}

/* deplacement du fou */
int deplacementFou(Piece **plateau, int l1, int c1, int l2, int c2)
{
    if (abs(l2 - l1) != abs(c2 - c1))
        return 0;

    int pasL = (l2 > l1) ? 1 : -1;
    int pasC = (c2 > c1) ? 1 : -1;
    int l = l1 + pasL;
    int c = c1 + pasC;

    while (l != l2) {
        if (plateau[l][c].type != 0)
            return 0;
        l += pasL;
        c += pasC;
    }
    return 1;
}

/* verifie que la case contient une piece du joueur */
int pieceDuJoueur(Piece **plateau, int lig, int col, char joueur)
{
    Piece *p = &plateau[lig][col];
    return p->type != 0 && p->couleur == joueur;
}

/* verification d'un coup */
int coupValide(Piece **plateau, int l1, int c1, int l2, int c2, char joueur)
{
    Piece p = plateau[l1][c1];

    if (l1 == l2 && c1 == c2)
        return 0;
    if (plateau[l2][c2].type != 0 && plateau[l2][c2].couleur == joueur)
        return 0;

    switch (p.type) {
        case 'P': return deplacementPion(plateau, l1, c1, l2, c2);
        case 'C': return deplacementCavalier(l1, c1, l2, c2);
        case 'F': return deplacementFou(plateau, l1, c1, l2, c2);
        default:  return 1; /* tour, dame, roi : a coder */
    }
}

/* deplace la piece sur le plateau */
void jouerCoup(Piece **plateau, int l1, int c1, int l2, int c2)
{
    plateau[l2][c2] = plateau[l1][c1];
    plateau[l1][c1].type = 0;
    plateau[l1][c1].couleur = 0;
}


/* saisie d'un coup */
char majuscule(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    return c;
}

int dansPlateau(int lig, int col)
{
    return lig >= 0 && lig < TAILLE && col >= 0 && col < TAILLE;
}

/* lit une case, ex : E2 (retourne -1 pour quitter) */
int lireCase(int *lig, int *col)
{
    char buf[32] = {0};

    if (fgets(buf, sizeof(buf), stdin) == NULL)
        return -1;
    if (buf[0] == 'q' || buf[0] == 'Q')
        return -1;

    *col = majuscule(buf[0]) - 'A';
    *lig = buf[1] - '1';

    return dansPlateau(*lig, *col);
}

/* promotion du pion */
void promotion(Piece **plateau, Historique *h, int lig, int col)
{
    Piece *p = &plateau[lig][col];

    if (p->type != 'P')
        return;
    if (!(p->couleur == 'B' && lig == 7) && !(p->couleur == 'N' && lig == 0))
        return;

    char buf[32] = {0};
    char choix = 0;

    while (choix != 'D' && choix != 'T' && choix != 'F' && choix != 'C') {
        printf("Promotion ! Choisir D, T, F ou C : ");
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            choix = 'D';
        else
            choix = majuscule(buf[0]);
    }
    p->type = choix;

    Coup *dernier = &h->coups[h->nb - 1];
    dernier->texte[5] = '=';
    dernier->texte[6] = choix;
    dernier->texte[7] = '\0';
}


/* liberation de la memoire */
void libererPlateau(Piece **plateau)
{
    for (int i = 0; i < TAILLE; i++)
        free(plateau[i]);
    free(plateau);
}


int main(void)
{
    Piece **plateau = creerPlateau();
    if (plateau == NULL)
        return 1;
    initialiserPlateau(plateau);

    Historique h;
    initHistorique(&h);
    if (h.coups == NULL)
        return 1;

    char joueur = 'B';
    int l1, c1, l2, c2;
    char code[4];

    afficherPlateau(plateau);

    while (1) {
        printf("Au tour des %s\n", joueur == 'B' ? "Blancs" : "Noirs");
        printf("Entrez votre position (ex : E2, Q pour quitter) : ");

        int ok = lireCase(&l1, &c1);
        if (ok == -1)
            break;
        if (ok == 0 || !pieceDuJoueur(plateau, l1, c1, joueur)) {
            printf("Position invalide, pas de piece a vous ici\n\n");
            continue;
        }

        codePiece(plateau[l1][c1], l1, c1, code);
        printf("Piece choisie : %s\n", code);
        printf("Entrez la destination : ");

        ok = lireCase(&l2, &c2);
        if (ok == -1)
            break;
        if (ok == 0 || !coupValide(plateau, l1, c1, l2, c2, joueur)) {
            printf("Cette piece ne peut pas se deplacer ici\n\n");
            continue;
        }

        ajouterCoup(&h, plateau[l1][c1], l2, c2);
        jouerCoup(plateau, l1, c1, l2, c2);
        promotion(plateau, &h, l2, c2);
        joueur = (joueur == 'B') ? 'N' : 'B';

        afficherPlateau(plateau);
        afficherHistorique(&h);
    }

    free(h.coups);
    libererPlateau(plateau);
    return 0;
}
