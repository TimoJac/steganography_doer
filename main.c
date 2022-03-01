/*!
 *
 * \file main.c
 * \brief Programme créé dans le cadre de l'UV NF05 enseignée à l'UTT dont le but est la Stéganographie: Cacher des messages dans des images
 *
 * Ce programme permet de cacher un message (sous forme de texte ou un fichier) dans un fichier portable pixmap codé en binaire.\n
 * Il utilise plusieurs algorithme pour cacher un message: \n
 * -# Insertion classique du message pixel par pixel.
 * -# Insertion crypté du message en changeant l'ordre de parcour des pixels en fonction d'une clé secrete fournit par l'utilisateur.
 * -# Insertion en utilisant la méthode de Hamming et la matrice de vérification pour ne modifier qu'une portion de bits et augmenter l'imperceptibilité
 * -# Possibilité également de mélanger son message secret en utilisant une clé secrete fournit par l'utilisateur.
 *
 * \n Une fois ces opérations effectuées sur le message, celui-ci est caché dans une copie de l'image d'origine, avec un nom spécifié par l'utilisateur.
 *
 * \version 1.0
 * \date 24/12/2020 11:12
 * \authors Melvin AUVRAY
 * \authors Timothée JACOB
 * \see https://moodle.utt.fr/pluginfile.php/144979/mod_resource/content/1/Projet_NF05_A20_stegano.pdf
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "header.h"

int main() {

    /* --------- DEFINITION DES VARIABLES --------- */
    error_t error;
    char typeFile[50];
    long int imageWidth, imageHeight, pixelIntensity, beginningImage, beginningNewImage, dimension, i, tailleMsgDecrypt;
    int *matriceImage = NULL, userMenu, longueurExtensionPixelMap, lengthDimensionPrefix, prefixInt, lengthmsgSecret;
    unsigned char* messageSecretBit = NULL;
    unsigned char *messageSecretBitOutput = NULL;
    size_t tailleMsgBit, longueurExtensionFileToCryptBinary;

    unsigned char* msgSecret = NULL;

    // Partie cryptage
    char* messageSecret = NULL, *pathToFile = NULL, *fileOutput, *fileToCrypt, *cryptKey, *extensionPixelMap = NULL, *extensionFileToCrypt = NULL;


    // Hamming
    unsigned int **matriceHamming,columns, rows;

    unsigned int compteurNbBitsModif;

    /* ------- FIN DEFINITION DES VARIABLES ------- */

    h1("Menu");

    p("Que souhaitez-vous faire ?");

    li(1, "Crypter un message dans une image");
    li(2, "Décrypter un message depuis une image");

    switch(reponseMenu(2)) {
        case 1:

            h1("Cryptage");

            p("Que souhaitez-vous faire ?");

            li(1, "Crypter un texte dans un ppm/pgm");
            li(2, "Crypter un fichier dans un ppm/pgm");

            switch(reponseMenu(2)) {
                case 1:

                    /* -----------------------------------------------------------
                    * ---------------- PARTIE RECUPERATION DU MSG ---------------
                    * -----------------------------------------------------------
                    */
                    p("Entrez le texte à crypter.");
                    printf("> ");
                    messageSecret = inputString(stdin, 5);

                    messageSecretBit = stringToBinary(messageSecret, &tailleMsgBit);

                    break;
                case 2:

                    /* -----------------------------------------------------------
                    * ----------- PARTIE RECUPERATION DU FICHIER DE SORTIE -------
                    * -----------------------------------------------------------
                    */
                    p("Entrez le chemin vers le fichier que vous voulez cacher (exemple: pdf, docx, mp3, jpg) (avec l'extension).");
                    printf("> ");
                    fileToCrypt = inputString(stdin, 5);

                    error = fileToBinary(fileToCrypt, &messageSecretBit, &tailleMsgBit);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    error = getExtension(fileToCrypt, &extensionFileToCrypt);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    error = addExtensionSuffix(extensionFileToCrypt, &messageSecretBit, &tailleMsgBit);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    break;
                default:
                    printf("Erreur: %s", error_str(ERROR_INVARG));
                    return 0;
            }


            /* -----------------------------------------------------------
            * ---------------- PARTIE RECUPERATION DU FICHIER ------------
            * -----------------------------------------------------------
            */
            p("Message secret enregistré. Entrez maintenant le chemin vers le fichier image qui accueillera votre fichier (Un fichier .PPM ou .PGM) (avec l'extension). ");
            printf("> ");
            pathToFile = inputString(stdin, 5);

            /* On récupère l'extension : */



            error = getExtension(pathToFile, &extensionPixelMap);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }


            /* -----------------------------------------------------------
            * ----------- PARTIE RECUPERATION DU FICHIER DE SORTIE -------
            * -----------------------------------------------------------
            */
            p("Chemin vers le fichier image enregistré. Entrez maintenant le nom de l'image que vous voulez créer, qui contiendra votre message secret (SANS l'extension, juste le nom). Vous pouvez spécifier un chemin afin de placer votre nouveau fichier où vous voulez (toujours SANS l'extension)");
            printf("> ");
            fileOutput = inputString(stdin, 1);

            // On ajouté la même extension que le fichier source
            error = addExtension(&fileOutput, extensionPixelMap);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }

            p("Chemin vers le fichier image cible enregistré.");


            /* ===========================================================
             * ------------------ PARTIE CRYPTAGE ----------------------
             * ===========================================================
             */


            /* -----------------------------------------------------------
             * ---------------- PARTIE LECTURE DU FICHIER ----------------
             * -----------------------------------------------------------
             */

            // On appelle la fonction pour lire les headers du fichier et on stock l'erreur de retour dans la variable error
            error = readHeader(pathToFile, typeFile, &imageWidth, &imageHeight, &pixelIntensity, &beginningImage);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }

            if(strcmp(typeFile, "P6") == 0) {
                dimension = imageWidth*imageHeight*3;
            } else if(strcmp(typeFile, "P5") == 0) {
                dimension = imageWidth*imageHeight;
            } else {
                error = ERROR_INVARG;
                error_str(error);
                return 0;
            }

            //On affiche les différents paramètres.
            //printf("Type du fichier: %s\nLargeur de l'image: %ld\nHauteur de l'image: %ld\nIntensité des pixels: %ld", typeFile, imageWidth, imageHeight, pixelIntensity);


            /* Maintenant on va récuperer la valeur de la matrice de l'image dans un tableau
             * Pour ça, on va crée un tableau dynamique de taille imageWidth*imageHeight
             */


            matriceImage = (int*) malloc(dimension * sizeof(int*));
            if(matriceImage == NULL) { // On test s'il y a une erreur
                error = ERROR_NOMEM;
                error_str(error);
                return 0;
            }

            error = readImage(pathToFile, matriceImage, beginningImage, dimension);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }





            /* -----------------------------------------------------------
             * ---------------- PARTIE CRYPTAGE DU MSG ---------------
             * -----------------------------------------------------------
             */
            p("Souhaitez vous crypter votre message en utilisant une table de permutation liée à une clé ?");

            li(1, "Oui.");
            li(2, "Non.");

            switch(reponseMenu(2)) {
                case 1:
                    p("Entrez la clé de chiffrement.");
                    printf("> ");
                    cryptKey = inputString(stdin, 5);


                    error = permuterTableau(cryptKey,tailleMsgBit, &messageSecretBit);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    break;
                case 2:
                    break;
                default:
                    printf("Erreur: %s", error_str(ERROR_INVARG));
                    return 0;
            }


            int* dimMaxBinary = num_to_bit(dimension, &lengthDimensionPrefix);

            error = determineBestHammingSize(dimension-lengthDimensionPrefix, tailleMsgBit, &rows, &columns);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }


            p("Quel type de cryptage souhaitez vous utiliser ?");

            li(1, "Classique: insertion un par un.");
            li(2, "Chiffré: modifier le sens de parcours grâce à une clé de chiffrement.");
            if(columns > 1)
                li(3, "Hamming: insertion par syndrome.");
            else
                li(3, "Votre message est trop grand pour utiliser l'insertion de Hamming.");

            error = hideDimMsg(tailleMsgBit, matriceImage, dimension, pixelIntensity, &lengthDimensionPrefix);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }



            switch(reponseMenu(3)) {
                case 1:

                    //printf("\nNombre de bits: %zu\n", tailleMsgBit);
                    error = hideMessage(messageSecretBit, tailleMsgBit, matriceImage, dimension, pixelIntensity, lengthDimensionPrefix,0,NULL);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }


                    break;
                case 2:

                    p("Entrez la clé de chiffrement.");
                    printf("> ");
                    cryptKey = inputString(stdin, 5);
                    //printf("\ntailleMsgBit: %zu dans dimension: %zu", tailleMsgBit, dimension);
                    error = hideMessage(messageSecretBit, tailleMsgBit, matriceImage, dimension, pixelIntensity, lengthDimensionPrefix,1,cryptKey);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    break;
                case 3:

                    if(columns > 2) {

                        error = hideMessageHamming(messageSecretBit, tailleMsgBit, matriceImage, dimension, pixelIntensity, lengthDimensionPrefix, rows, columns, &compteurNbBitsModif);
                        if(error != ERROR_OK) {
                            error_str(error);
                            return 0;
                        }

                        printf("Taille de la matrice de vérification optimale: (%d, %d). Nombre de bits modifiés: %d sur %zu (%lu%%)\n", columns, rows, compteurNbBitsModif, tailleMsgBit, (compteurNbBitsModif*100)/tailleMsgBit);


                    } else {
                        p("Votre message est trop grand pour utiliser l'insertion de Hamming, nous procedons à l'insertion classique.");

                        error = hideMessage(messageSecretBit, tailleMsgBit, matriceImage, dimension, pixelIntensity, lengthDimensionPrefix, 0, NULL);
                        if(error != ERROR_OK) {
                            error_str(error);
                            return 0;
                        }
                    }


                    break;
                default:
                    printf("Erreur: %s", error_str(ERROR_INVARG));
                    return 0;
            }







            /* -----------------------------------------------------------
             * ------------ PARTIE ECRITURE DE LA NVL IMG ----------------
             * -----------------------------------------------------------
             */

            error = writeHeader(fileOutput, typeFile, imageWidth, imageHeight, pixelIntensity, &beginningNewImage);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }


            error = writeImage(fileOutput, matriceImage, beginningNewImage, dimension);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }


            p("Votre message a bien été ajouté à votre image !");
            p("Appuyez sur <Entrée> pour quitter le programme");
            getchar();

            break;

        case 2:
            h1("Décryptage");

            /* -----------------------------------------------------------
            * ---------------- PARTIE RECUPERATION DU FICHIER ------------
            * -----------------------------------------------------------
            */
            p("Entrez le chemin vers le fichier image (avec l'extension).");
            printf("> ");
            pathToFile = inputString(stdin, 5);

            /* ===========================================================
             * ------------------ PARTIE DECRYPTAGE ----------------------
             * ===========================================================
             */

            /* -----------------------------------------------------------
             * ---------------- PARTIE LECTURE DU FICHIER ----------------
             * -----------------------------------------------------------
             */

            // On appelle la fonction pour lire les headers du fichier et on stock l'erreur de retour dans la variable error
            error = readHeader(pathToFile, typeFile, &imageWidth, &imageHeight, &pixelIntensity, &beginningImage);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }


            // Calcul de la dimension de l'image
            if(strcmp(typeFile, "P6") == 0) {
                dimension = imageWidth*imageHeight*3;
            } else if(strcmp(typeFile, "P5") == 0) {
                dimension = imageWidth*imageHeight;
            } else {
                error = ERROR_INVARG;
                error_str(error);
                return 0;
            }

            //On affiche les différents paramètres.
            // printf("Type du fichier: %s\nLargeur de l'image: %ld\nHauteur de l'image: %ld\nIntensité des pixels: %ld", typeFile, imageWidth, imageHeight, pixelIntensity);


            /* Maintenant on va récuperer la valeur de la matrice de l'image dans un tableau
             * Pour ça, on va crée un tableau dynamique de taille imageWidth*imageHeight
             */


            matriceImage = (int*)malloc(dimension * sizeof(int*));
            if(matriceImage == NULL) { // On test s'il y a une erreur
                error = ERROR_NOMEM;
                error_str(error);
                return 0;
            }

            error = readImage(pathToFile, matriceImage, beginningImage, dimension);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }



            /* -----------------------------------------------------------
             * ------------ PARTIE RECUPERATION DU MSG CRYPTE ------------
             * -----------------------------------------------------------
             */

            error = decryptPrefix(matriceImage, dimension, &prefixInt, &lengthDimensionPrefix);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }

            p("Quel type de decryptage souhaitez vous utiliser ?");

            li(1, "Classique: récupération un par un.");
            li(2, "Chiffré: modifier le sens de parcour grâce à une clé de chiffrement.");
            li(3, "Hamming: Vecteur de syndrome.");

            // A modifier
            switch(reponseMenu(3)) {
                case 1:
                    error = decryptMessage(matriceImage, dimension, prefixInt, lengthDimensionPrefix,0,NULL, &tailleMsgDecrypt, &messageSecretBitOutput);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    break;
                case 2:

                    p("Entrez la clé de chiffrement.");
                    printf("> ");
                    cryptKey = inputString(stdin, 5);

                    error = decryptMessage(matriceImage, dimension, prefixInt, lengthDimensionPrefix,1,cryptKey, &tailleMsgDecrypt, &messageSecretBitOutput);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }
                    break;
                case 3:

                    determineBestHammingSize(dimension, prefixInt-lengthDimensionPrefix, &rows, &columns);

                    if(columns>2 && rows > 1) {

                        error = decryptMessageHamming(matriceImage, dimension, prefixInt, lengthDimensionPrefix, rows, columns, &tailleMsgDecrypt, &messageSecretBitOutput);
                        if(error != ERROR_OK) {
                            error_str(error);
                            return 0;
                        }

                    } else {

                        error = decryptMessage(matriceImage, dimension, prefixInt, lengthDimensionPrefix,0,NULL, &tailleMsgDecrypt, &messageSecretBitOutput);
                        if(error != ERROR_OK) {
                            error_str(error);
                            return 0;
                        }

                        p("Votre message secret était trop volumineux pour avoir été inséré avec la méthode de Hamming, nous l'avons donc décrypté classiquement.");
                    }

                    break;
                default:
                    printf("Erreur: %s", error_str(ERROR_INVARG));
                    return 0;
            }




            // On vérifie si le message a été permuté avec une clé

            p("Votre message a-t-il été crypté en utilisant une table de permutation ?");

            li(1, "Oui.");
            li(2, "Non.");

            switch(reponseMenu(2)) {
                case 1:
                    p("Entrez la clé de chiffrement.");
                    printf("> ");
                    cryptKey = inputString(stdin, 5);

                    error = depermuterTableau(cryptKey, tailleMsgDecrypt, &messageSecretBitOutput);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    break;
                case 2:
                    break;
                default:
                    printf("Erreur: %s", error_str(ERROR_INVARG));
                    return 0;
            }

            // On converti le message de bits en unsigned char (octet)
            error = binaryToUChar(tailleMsgDecrypt, messageSecretBitOutput, &msgSecret, &lengthmsgSecret);
            if(error != ERROR_OK) {
                error_str(error);
                return 0;
            }


            p("Que souhaitez-vous faire ?");

            li(1, "Décrypter un texte.");
            li(2, "Décrypter un fichier.");

            switch(reponseMenu(2)) {
                case 1:

                    /* -----------------------------------------------------------
                     * -------------- PARTIE TRADUCTION DU MSG CRYPTE ------------
                     * -----------------------------------------------------------
                     */
                    //printf("\nBits de votre message secret décrypté: ");

                    msgSecret[lengthmsgSecret] = '\0';

                    p("Votre message secret est:");
                    p((const char*)msgSecret);

                    p("Appuyez sur <Entrée> pour quitter le programme");
                    getchar();

                    break;
                case 2:

                    /* -----------------------------------------------------------
                    * ---------------- PARTIE CREATION DU FICHIER DEPUIS MSG -----
                    * -----------------------------------------------------------
                    */
                    p("Entrez le nom du fichier à créer (SANS l'extension).");
                    printf("> ");
                    fileToCrypt = inputString(stdin, 5);

                    error = readExtensionSuffix(msgSecret, lengthmsgSecret, &extensionPixelMap);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }


                    error = addExtension(&fileToCrypt, extensionPixelMap);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    error = createFileFromByte(fileToCrypt, msgSecret, tailleMsgDecrypt);
                    if(error != ERROR_OK) {
                        error_str(error);
                        return 0;
                    }

                    printf("\nExtension de votre fichier: %s", extensionPixelMap);
                    p("Votre fichier a été créé avec succès !");

                    p("Appuyez sur <Entrée> pour quitter le programme");
                    getchar();

                    break;
                default:
                    printf("Erreur: %s", error_str(ERROR_INVARG));
                    return 0;
            }


            break;

        default:
            printf("Erreur: %s", error_str(ERROR_INVARG));
            return 0;


    }


    freeAllVar(msgSecret, messageSecretBitOutput, pathToFile, matriceImage, messageSecretBit, messageSecret, extensionPixelMap);

    return 0;
}


int addExtensionSuffix(char* extensionFileToCrypt, unsigned char** messageSecretBit, size_t *tailleMsgBit) {

    unsigned char* extensionBinary = NULL;
    size_t longueurExtensionFileToCryptBinary;

    extensionBinary = stringToBinary(extensionFileToCrypt, &longueurExtensionFileToCryptBinary);
    if(extensionBinary == NULL)
        return ERROR_NOMEM;

    (*messageSecretBit) = realloc((*messageSecretBit), (*tailleMsgBit)+40);
    if((*messageSecretBit) == NULL)
        return ERROR_NOMEM;

    int q = 8; // On initialise q à 8 car on ne veut pas le point initial de l'extension
    for(int s = (*tailleMsgBit); s<(*tailleMsgBit)+longueurExtensionFileToCryptBinary-8; s++) {
        (*messageSecretBit)[s] = extensionBinary[q]-'0';

        q++;
    }

    if(longueurExtensionFileToCryptBinary<40) {
        for(int s = (int)((*tailleMsgBit)+longueurExtensionFileToCryptBinary); s<(*tailleMsgBit)+40; s++) {
            (*messageSecretBit)[s] = 0;
        }
    }

    (*tailleMsgBit) = (*tailleMsgBit)+40;

    if(extensionBinary != NULL)
        free(extensionBinary);

    if(extensionFileToCrypt != NULL)
        free(extensionFileToCrypt);

    return ERROR_OK;
}

int createFileFromByte(const char* fileToCrypt, const unsigned char* msgSecret, long int tailleMsgDecrypt) {

    FILE *newFile = fopen(fileToCrypt, "w");
    if(newFile == NULL)
        return ERROR_OPEN;

    fwrite(msgSecret, 1, (tailleMsgDecrypt/8)-6, newFile);
    fclose(newFile);

    return ERROR_OK;
}

int readExtensionSuffix(const unsigned char* msgSecret, int lengthmsgSecret, char** extensionPixelMap) {

    int y;

    (*extensionPixelMap) = malloc(sizeof(char)*5);
    if((*extensionPixelMap) == NULL)
        return ERROR_NOMEM;

    (*extensionPixelMap)[0] = '.';
    for(y = 1;y<6;y++) {
        (*extensionPixelMap)[y] = (char)msgSecret[lengthmsgSecret-6+y];
    }

    return ERROR_OK;
}






int binaryToUChar(long int tailleMsgDecrypt, const unsigned char *messageSecretBitOutput, unsigned char **msgSecret, int* lengthmsgSecret) {

    int i,j;
    unsigned char* data;
    data = (unsigned char*) malloc(sizeof(unsigned char)*9);
    if(data== NULL)
        return ERROR_NOMEM;

    (*msgSecret) = (unsigned char*) malloc(sizeof(unsigned char) * (tailleMsgDecrypt / 8) + 1);
    if((*msgSecret)== NULL)
        return ERROR_NOMEM;

    j = 0;
    for(i = 0; i<tailleMsgDecrypt; i++) {

        data[i%8] = messageSecretBitOutput[i] + '0';
        if(i%8 == 7) {
            (*msgSecret)[j] = strtol((const char*)data, 0, 2);
            j = j+1;
        }
    }

    (*lengthmsgSecret) = j;

    free(data);

    return ERROR_OK;
}


int depermuterTableau(char* key,int tailleTable, unsigned char** table) {

    unsigned long keyHash;
    int *tablepermutation,i;
    int temp;

    keyHash = hash((unsigned char *) key);
    srand(keyHash);

    tablepermutation = (int *) malloc(sizeof(int) * tailleTable);
    if(tablepermutation == NULL)
        return ERROR_NOMEM;

    for (i = tailleTable - 1; i >= 0; --i) {
        tablepermutation[i] = rand() % (i + 1);// NOLINT(cert-msc30-c, cert-msc50-cpp)
    }

    for (i = 0; i < tailleTable; i++) {
        temp = (*table)[i];
        (*table)[i] = (*table)[tablepermutation[i]];
        (*table)[tablepermutation[i]] = temp;
    }

    free(tablepermutation);


    return ERROR_OK;
}



int permuterTableau(char* key,int tailleTable, unsigned char** table) {

    unsigned long keyHash;
    int i,j;
    int temp;

    keyHash = hash((unsigned char *) key);
    srand(keyHash);

    for (i = tailleTable - 1; i >= 0; --i) {
        //generate a random number [0, n-1]
        j = rand() % (i + 1);// NOLINT(cert-msc30-c, cert-msc50-cpp)

        //swap the last element with element at random index
        temp = (*table)[i];
        (*table)[i] = (*table)[j];
        (*table)[j] = temp;
    }


    return ERROR_OK;
}


// Cette fonction ajoute une extension à la fin d'un string, les deux se terminant par '\0'
int addExtension(char** fileOutput, const char *extensionPixelMap) {

    int longueurfileOutput = 0, longueurExtensionPixelMap = 0;

    while((*fileOutput)[longueurfileOutput] != '\0') {
        longueurfileOutput++;
    }

    while(extensionPixelMap[longueurExtensionPixelMap] != '\0') {
        longueurExtensionPixelMap++;
    }


    (*fileOutput) = realloc((*fileOutput), longueurfileOutput+longueurExtensionPixelMap+2);
    if((*fileOutput) == NULL)
        return ERROR_NOMEM;

    strcat((*fileOutput), extensionPixelMap);

    return ERROR_OK;
}


// Fonction qui renvoit la taille (en place mémoire, donc en incluant le . et le \0) et un string correspond à l'extension d'un fichier.
int getExtension(const char* pathToFile, char** extensionPixelMap) {

    int longueurPathToFile = 0;
    int i, positionExt;

    while(pathToFile[longueurPathToFile] != '\0') {
        if(pathToFile[longueurPathToFile] == '.') {
            positionExt = longueurPathToFile;
        }
        longueurPathToFile++;
    }


    *extensionPixelMap = (char*) malloc(sizeof(char)*(longueurPathToFile-positionExt)+1);
    if(*extensionPixelMap == NULL)
        return ERROR_NOMEM;

    (*extensionPixelMap)[0] = '.';

    for(i = 1; i<(longueurPathToFile-positionExt); i++) {
        (*extensionPixelMap)[i] = pathToFile[longueurPathToFile-((longueurPathToFile-positionExt)-i)];
    }

    (*extensionPixelMap)[(longueurPathToFile-positionExt)] = '\0';

    return ERROR_OK;
}



void freeAllVar(void* pointer1, void* pointer2, void* pointer3, void* pointer4, void* pointer5, void* pointer6, void* pointer7) {

    if(pointer1 != NULL)
        free(pointer1);

    if(pointer2 != NULL)
        free(pointer2);

    if(pointer3 != NULL)
        free(pointer3);

    if(pointer4 != NULL)
        free(pointer4);

    if(pointer5 != NULL)
        free(pointer5);

    if(pointer6 != NULL)
        free(pointer6);

    if(pointer7 != NULL)
        free(pointer7);


}






// http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


// Convert char to binary table
void printbincharpad(char c, char* tabBit)
{

    for(int n = 0; n < 8; n++)
        tabBit[7-n] = (char) ((c >> n) & 1);

}

/* Copié collé depuis https://stackoverflow.com/questions/16870485/how-can-i-read-an-input-string-of-unknown-length */
char *inputString(FILE* fp, size_t size){
//The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=(char)ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}










unsigned char* stringToBinary(char* s, size_t *length) {
    if(s == NULL) return 0; /* no input string */
    size_t len = strlen(s);
    unsigned char *binary = malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    for(size_t i = 0; i < len; ++i) {
        char ch = s[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat((char*)binary,"1");
            } else {
                strcat((char*)binary,"0");
            }
        }
    }
    *length = len*8;
    return binary;
}

int* num_to_bit(int a, int *len){
    int arrayLen=0,i=1;
    while (i<a){
        arrayLen++;
        i*=2;
    }
    *len=arrayLen;
    int *bits;
    bits=(int*)malloc(arrayLen*sizeof(int));
    arrayLen--;
    while(a>0){
        bits[arrayLen--]=a&1;
        a>>=1;
    }
    return bits;
}

int decryptPrefix(const int *matriceImage, long int dimension, int* prefixInt, int* lengthDimensionPrefix) {
    int i,j;
    int *prefixBinary, *dimMaxBinary;
    //printf("Dimension: %zd", dimension);

    dimMaxBinary = num_to_bit(dimension, lengthDimensionPrefix);


    //printf("\nTaille en bits de la dimension max: %d", lengthDimensionPrefix);
    /*printf("\nTaille en bits de la dimension max en binaire: ");
    for(i = 0; i<lengthDimensionPrefix; i++) {
        printf("%d", dimMaxBinary[i]);
    }*/

    prefixBinary = (int*) malloc((*lengthDimensionPrefix) * sizeof(int));
    if(prefixBinary == NULL)
        return ERROR_NOMEM;
    for(i=0; i<(*lengthDimensionPrefix); i++) {
        prefixBinary[i] = (matriceImage[i] & ( 1 << 0)) >> 0;
    }

    /*printf("\nBits du prefixe (%d premiers bits): ", lengthDimensionPrefix);
    for(i=0; i<lengthDimensionPrefix; i++) {
        printf("%d", prefixBinary[i]);
    }*/
    (*prefixInt) = 0;
    j=0;
    for(i=(*lengthDimensionPrefix)-1; i>=0; i--) {
        (*prefixInt) += prefixBinary[j] * ( (int) pow(2, i));
        j +=1;
    }
    return ERROR_OK;
}

int decryptMessageHamming(const int *matriceImage, long int dimension, int prefixInt, int lengthDimensionPrefix, unsigned int rows, unsigned int columns, long int *tailleMsgDecrypt, unsigned char **messageSecretBitOutput) {



    if(columns>2 && rows>1) {

        unsigned int **matriceHamming, j, k, o, i, **matriceResMul = NULL, **matriceResAdd = NULL, position, randomNumber;
        unsigned int **bufferImage, **bufferMsg;


        genererHamming(&matriceHamming, rows, columns);



        bufferImage = (unsigned int **) malloc(sizeof(unsigned int) * 1);
        if (bufferImage == NULL)
            return ERROR_NOMEM;

        for (i = 0; i < 1; i++) {
            bufferImage[i] = (unsigned int *) malloc(sizeof(unsigned int) * columns);
            if (bufferImage[i] == NULL)
                return ERROR_NOMEM;
        }

        matriceResMul = NULL;

        (*messageSecretBitOutput) = malloc(sizeof(unsigned int) * (prefixInt - lengthDimensionPrefix));
        if ((*messageSecretBitOutput) == NULL)
            return ERROR_NOMEM;

        for (i = 0; i < dimension / columns; i++) {
            for (j = 0; j < columns; j++) {

                // On crée une matrice bufferImage qui va contenir columns fois le nombre de bits de la matrice image (pour la séparé en séquence)
                if (j + (i + (i * (columns - 1))) < dimension)
                    (*bufferImage)[j] = matriceImage[lengthDimensionPrefix + j + (i + (i * (columns - 1)))];
                else
                    (*bufferImage)[j] = 1;

            }

            // On crée la matrice en mémoire qui va accueillir le résultat de la multiplication
            if (matriceResMul != NULL)
                free(matriceResMul);

            matriceResMul = (unsigned int **) malloc(sizeof(unsigned int) * 1);
            if (matriceResMul == NULL)
                return ERROR_NOMEM;

            for (k = 0; k < 1; k++) {
                matriceResMul[k] = (unsigned int *) malloc(sizeof(unsigned int) * rows);
                if (matriceResMul[k] == NULL)
                    return ERROR_NOMEM;
            }

            // On effectue la multiplication (Pas besoin de passer toutes les dimensions de la matrice 1 car n1=m1
            multiplyMatrice(rows, matriceHamming, 1, columns, bufferImage, &matriceResMul);

            for (o = 0; o < rows; o++) {

                if (o + (i + (i * (rows - 1))) < (prefixInt - lengthDimensionPrefix)) {
                    (*messageSecretBitOutput)[o + (i + (i * (rows - 1)))] = (*matriceResMul)[o];

                }

            }


        }

        (*tailleMsgDecrypt) = prefixInt - lengthDimensionPrefix;

        for(i = 0; i < 1; i++) {
            free(bufferImage[i]);
        }
        free(bufferImage);


        for(k = 0; k < 1; k++) {
            free(matriceResMul[k]);
        }
        free(matriceResMul);


        for (i = 0; i < columns; i++) {
            free(matriceHamming[i]);
        }

        free(matriceHamming);


        return ERROR_OK;
    } else {
        return ERROR_INVARG;
    }
}

int decryptMessage(const int *matriceImage, long int dimension, int prefixInt, int lengthDimensionPrefix,int crypt, char* keyCript, long int *tailleMsgDecrypt, unsigned char **messageSecretBitOutput) {

    int i,j;
    int *dimMaxBinary;
    unsigned long keyHash;
    int *tablepermutation,*tablepermuteIndex;
    int temp;
    //printf("\nPosition du message: %d", prefixInt);

    //printf("\nBits du message: ");
    *messageSecretBitOutput = (unsigned char *) malloc(prefixInt * sizeof(unsigned char));
    if(messageSecretBitOutput == NULL)
        return ERROR_NOMEM;

    if(crypt == 1) {


        keyHash = hash((unsigned char *) keyCript);


        tablepermuteIndex = (int *) malloc(sizeof(int) * (dimension-lengthDimensionPrefix));
        if(tablepermuteIndex == NULL)
            return ERROR_NOMEM;

        for(i = lengthDimensionPrefix; i<dimension; i++) {
            tablepermuteIndex[i] = i;
        }

        srand(keyHash);
        for (i = dimension - 1; i >= lengthDimensionPrefix; --i) {
            //generate a random number [0, n-1]
            j = (lengthDimensionPrefix) + ( rand() % (i + 1));// NOLINT(cert-msc30-c, cert-msc50-cpp)



            //swap the last element with element at random index
            temp = tablepermuteIndex[i];
            tablepermuteIndex[i] = tablepermuteIndex[j];
            tablepermuteIndex[j] = temp;
        }


    }

j=0;
    for(i=lengthDimensionPrefix; i<prefixInt; i++) {
        if(crypt == 0)
            (*messageSecretBitOutput)[j] = (matriceImage[i] & ( 1 << 0)) >> 0;
        else
            (*messageSecretBitOutput)[j] = (matriceImage[tablepermuteIndex[i]] & (1 << 0)) >> 0;

        j += 1;
    }

    *tailleMsgDecrypt = prefixInt - lengthDimensionPrefix;

    if(crypt == 1)
        free(tablepermuteIndex);

    return ERROR_OK;
}


int hideDimMsg(size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int* lengthDimensionPrefix) {
    int i, randomNumber,j, lengthEndOfMsgBinary;
    unsigned int endOfMsg;
    int *endOfMsgBinary, *dimMaxBinary;

    //printf("tailleMsgBit: %ld", tailleMsgBit);

    //printf("Dimension: %zd", dimension);
    dimMaxBinary = num_to_bit(dimension, lengthDimensionPrefix);
    //printf("\nTaille en bits de la dimension max: %d", lengthDimensionPrefix);
    //printf("\nTaille en bits de la dimension max en binaire: ");
    /*for(i = 0; i<lengthDimensionPrefix; i++) {
        printf("%d", dimMaxBinary[i]);
    }*/

    if(tailleMsgBit <= (dimension - (*lengthDimensionPrefix))) {

        //printf("\nTest de mémoire passé");
        endOfMsg = tailleMsgBit + (*lengthDimensionPrefix);
        //printf("\nPosition de la fin du message: %d", endOfMsg);

        endOfMsgBinary = num_to_bit((int) endOfMsg, &lengthEndOfMsgBinary);
        //printf("\nTaille en bits de la fin du message: %d", lengthEndOfMsgBinary);
        //printf("\nPosition de la fin du message en binaire: ");
        /*for(i = 0; i<lengthEndOfMsgBinary; i++) {
            printf("%d", endOfMsgBinary[i]);
        }*/
        //printf("\n");

        if (((*lengthDimensionPrefix) - lengthEndOfMsgBinary) > 0) {

            for (i = 0; i < ((*lengthDimensionPrefix) - lengthEndOfMsgBinary); i++) {
                randomNumber = rand() % 2; // On génère un nombre aléatoire entre 0 et 1;// NOLINT(cert-msc30-c, cert-msc50-cpp)

                if (0 != ((matriceImage[i] & (1 << 0)) >> 0)) {

                    if (randomNumber == 1) {
                        if (matriceImage[i] != pixelIntensity) {
                            matriceImage[i] = matriceImage[i] + 1;
                        } else {
                            matriceImage[i] = matriceImage[i] - 1;
                        }
                    } else {
                        if (matriceImage[i] != 0) {
                            matriceImage[i] = matriceImage[i] - 1;
                        } else {
                            matriceImage[i] = matriceImage[i] + 1;
                        }
                    }

                }
            }

        }

        //printf("\n%d zéros ont été ajouté !", (lengthDimensionPrefix - lengthEndOfMsgBinary));
        //printf("\nOn ajoute la fin du message en binaire: ");
        j = 0;
        for (i = ((*lengthDimensionPrefix) - lengthEndOfMsgBinary); i < (*lengthDimensionPrefix); i++) {
            randomNumber = rand() % 2; // On génère un nombre aléatoire entre 0 et 1;// NOLINT(cert-msc30-c, cert-msc50-cpp)
            //printf("\n%d avec un i=%d", endOfMsgBinary[j], i);
            if (endOfMsgBinary[j] != ((matriceImage[i] & (1 << 0)) >> 0)) {

                if (randomNumber == 1) {
                    if (matriceImage[i] != pixelIntensity) {
                        matriceImage[i] = matriceImage[i] + 1;
                    } else {
                        matriceImage[i] = matriceImage[i] - 1;
                    }
                } else {
                    if (matriceImage[i] != 0) {
                        matriceImage[i] = matriceImage[i] - 1;
                    } else {
                        matriceImage[i] = matriceImage[i] + 1;
                    }
                }

            }

            j += 1;
        }

    }

    return ERROR_OK;
}

int hideMessageHamming(const unsigned char* messageBinary, size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int lengthDimensionPrefix, unsigned int rows, unsigned int columns, unsigned int* compteurNbBitsModif) {

    unsigned int **matriceHamming, j,k,o,i, **matriceResMul=NULL, **matriceResAdd = NULL, position, randomNumber;
    unsigned int** bufferImage, **bufferMsg;
    int error;

    *compteurNbBitsModif = 0;

    // On génère la matrice de Hamming
    error = genererHamming(&matriceHamming,rows, columns);
    if(error != ERROR_OK)
        return error;



    // On crée les tableaux bufferImage et bufferMsg et on les initialises. Ces tableaux contiendront les segments de l'image et du message utilisés pour faire les opérations de matrice.
    bufferImage = (unsigned int**) malloc(sizeof(unsigned int) * 1);
    if(bufferImage == NULL)
        return ERROR_NOMEM;
    for(i = 0; i < 1; i++) {
        bufferImage[i] = (unsigned int*) malloc(sizeof(unsigned int) * columns);
        if(bufferImage[i] == NULL)
            return ERROR_NOMEM;
    }

    bufferMsg = (unsigned int**) malloc(sizeof(unsigned int) * 1);
    if(bufferMsg == NULL)
        return ERROR_NOMEM;
    for(i = 0; i < 1; i++) {
        bufferMsg[i] = (unsigned int*) malloc(sizeof(unsigned int) * rows);
        if(bufferMsg[i] == NULL)
            return ERROR_NOMEM;
    }


    srand(time(NULL)); // On rend le rank aléatoire pour la modification des bits

    for(i = 0; i<ceil((double)tailleMsgBit/(double)rows); i++) {

        // On Met à jour les segments de l'image et du message afin d'effectuer les calculs de matrice.
        for(j = 0; j<columns; j++) {

            // On crée une matrice bufferImage qui va contenir columns fois le nombre de bits de la matrice image (pour la séparé en séquence)
            if(j+(i+(i*(columns-1))) < dimension)
                (*bufferImage)[j] = matriceImage[lengthDimensionPrefix + j+(i+(i*(columns-1)))];
            else
                (*bufferImage)[j] = 1;

        }

        for(o=0; o<rows; o++) {
            // On crée une matrice bufferMsg qui va contenir rows fois le nombre de bits de la matrice message (pour la séparé en séquence)
            if(o+(i+(i*(rows-1))) < tailleMsgBit)
                (*bufferMsg)[o] = messageBinary[o+(i+(i*(rows-1)))];
            else
                (*bufferMsg)[o] = 1;


        }

        // On crée la matrice en mémoire qui va accueillir le résultat de la multiplication
        if(matriceResMul != NULL)
            free(matriceResMul);

        matriceResMul = (unsigned int**) malloc(sizeof(unsigned int) * 1);
        if(matriceResMul == NULL)
            return ERROR_NOMEM;

        for(k = 0; k < 1; k++) {
            matriceResMul[k] = (unsigned int*) malloc(sizeof(unsigned int) * rows);
            if(matriceResMul[k] == NULL)
                return ERROR_NOMEM;
        }

        // On effectue la multiplication (Pas besoin de passer toutes les dimensions de la matrice 1 car n1=m1
        multiplyMatrice(rows, matriceHamming, 1, columns, bufferImage, &matriceResMul);


        // On crée la matrice en mémoire qui va accueillir le résultat de l'addition
        if(matriceResAdd != NULL)
            free(matriceResAdd);

        matriceResAdd = (unsigned int**) malloc(sizeof(unsigned int) * 1);
        if(matriceResAdd == NULL)
            return ERROR_NOMEM;

        for(k = 0; k < 1; k++) {
            matriceResAdd[k] = (unsigned int*) malloc(sizeof(unsigned int) * rows);
            if(matriceResAdd[k] == NULL)
                return ERROR_NOMEM;
        }

        // On aditionne le résultat précédent avec la matrice message
        additionMatrice(1,rows, matriceResMul, bufferMsg, &matriceResAdd);

        // Si le vecteur n'est pas nul, c'est qu'il faut effectuer une modification
        if(!isVectorNull(matriceResAdd, rows)) {

            // On recherche à quelle ligne on doit effectuer la modification
            error = findInHamming(matriceHamming, columns, rows, matriceResAdd, &position);
            if(error != ERROR_OK)
                return error;

            // On modifie le tableau en utilisant la même méthode du nombre aléatoire que l'insertion classique
            randomNumber = rand() % 2; // On génère un nombre aléatoire entre 0 et 1;// NOLINT(cert-msc30-c, cert-msc50-cpp)
            if(randomNumber == 1) {
                if(matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] != pixelIntensity) {
                    matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] = matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] + 1;
                } else {
                    matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] = matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] - 1;
                }
            } else {
                if(matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] != 0) {
                    matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] = matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] - 1;
                } else {
                    matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] = matriceImage[lengthDimensionPrefix+position+(i+(i*(columns-1)))] + 1;
                }
            }

            (*compteurNbBitsModif)++;


        }


    }

    // On free toutes les variables que l'on a utilisées
    for(i = 0; i < 1; i++) {
        free(bufferImage[i]);
    }
    free(bufferImage);

    for(i = 0; i < 1; i++) {
        free(bufferMsg[i]);
    }
    free(bufferMsg);


    for(k = 0; k < 1; k++) {
        free(matriceResMul[k]);
    }
    free(matriceResMul);


    for(k = 0; k < 1; k++) {
        free(matriceResAdd[k]);
    }

    free(matriceResAdd);

    for (i = 0; i < columns; i++) {
        free(matriceHamming[i]);
    }

    free(matriceHamming);

    return ERROR_OK;

}



int hideMessage(const unsigned char* messageBinary, size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int lengthDimensionPrefix, int crypt, char* keyCrypt) {

    int i, randomNumber,j, lengthEndOfMsgBinary;
    int *endOfMsgBinary, *dimMaxBinary, *tablePermuteIndex;

    // On setup le random
    if(tailleMsgBit <= (dimension - lengthDimensionPrefix)) {


        j = 0;
        int p = 0;
        int bufferBitBinary;



        if(crypt == 1) {
            tablePermuteIndex = (int*) malloc(sizeof(int)*(dimension));
            if(tablePermuteIndex == NULL)
                return ERROR_NOMEM;
            for(i = lengthDimensionPrefix; i<dimension; i++) {
                tablePermuteIndex[i] = i;
            }


            unsigned long keyHash = hash((unsigned char *) keyCrypt);
            srand(keyHash);
            int f = 0;
            for (i = dimension - 1; i >= lengthDimensionPrefix; --i) {
                //generate a random number [0, n-1]
                j = (lengthDimensionPrefix) + ( rand() % (i + 1));// NOLINT(cert-msc30-c, cert-msc50-cpp)

                //swap the last element with element at random index
                int temp = tablePermuteIndex[i];
                tablePermuteIndex[i] = tablePermuteIndex[j];
                tablePermuteIndex[j] = temp;
                f++;
            }

        }


        srand(time(NULL)); // NOLINT(cert-msc30-c, cert-msc50-cpp)

        j=0;
        for(i = lengthDimensionPrefix; i<(tailleMsgBit + lengthDimensionPrefix); i++) {

            randomNumber = rand() % 2; // On génère un nombre aléatoire entre 0 et 1; // NOLINT(cert-msc30-c, cert-msc50-cpp)


            /* Petit fix pour choisir si on met une img ou un texte */

            if(((messageBinary[j] - '0') == 0 ) || ((messageBinary[j] - '0') == 1 )) {
                bufferBitBinary = (messageBinary[j] - '0');
            } else {
                bufferBitBinary = messageBinary[j];
            }

            /* Fin du petit fix */
            if(crypt == 0) {
                if (bufferBitBinary != ((matriceImage[i] & (1 << 0)) >> 0)) {

                    if (randomNumber == 1) {
                        if (matriceImage[i] != pixelIntensity) {
                            matriceImage[i] = matriceImage[i] + 1;
                        } else {
                            matriceImage[i] = matriceImage[i] - 1;
                        }
                    } else {
                        if (matriceImage[i] != 0) {
                            matriceImage[i] = matriceImage[i] - 1;
                        } else {
                            matriceImage[i] = matriceImage[i] + 1;
                        }
                    }


                }
            } else if(crypt == 1) {
                if (bufferBitBinary != ((matriceImage[tablePermuteIndex[i]] & (1 << 0)) >> 0)) {

                    if (randomNumber == 1) {
                        if (matriceImage[tablePermuteIndex[i]] != pixelIntensity) {
                            matriceImage[tablePermuteIndex[i]] = matriceImage[tablePermuteIndex[i]] + 1;
                        } else {
                            matriceImage[tablePermuteIndex[i]] = matriceImage[tablePermuteIndex[i]] - 1;
                        }
                    } else {
                        if (matriceImage[tablePermuteIndex[i]] != 0) {
                            matriceImage[tablePermuteIndex[i]] = matriceImage[tablePermuteIndex[i]] - 1;
                        } else {
                            matriceImage[tablePermuteIndex[i]] = matriceImage[tablePermuteIndex[i]] + 1;
                        }
                    }


                }
            } else {
                return ERROR_INVARG;
            }


            j += 1;
            p += 1;

        }

        if(crypt == 1)
            free(tablePermuteIndex);

        return ERROR_OK;
    } else {
        return ERROR_NOMEM;
    }

}





/*
 *  Nom: error_str
 *  But: Retourner une chaîne de caractères correspondante à l'erreur fournit en paramètre
 *
 *  Paramètres d'entrées:
 *      err: Le code d'erreur (error_t ou int)
 *
 *  Paramètres de sorties (sous forme de pointer):
 *
 *  Valeur de retour:
 *      (const char*) La chaine de caractère puisée dans le tableau ERROR_STRS du fichier header.h correspondant à l'erreur fournit en paramètre.
 *
 */
const char* error_str(error_t err)
{
    const char* err_str = NULL;

    // On vérifie que l'erreur fournit en paramètre fait bien parti de la liste des erreurs du type error_t, sinon on rejoint "done" pour retourner NULL;
    if (err >= ERROR_COUNT) {
        goto done;
    }

    err_str = ERROR_STRS[err]; // On va chercher la bonne chaine de caractère correspondant à l'erreur dans le tableau ERROR_STRS initialisé dans header.h. Puisque error_t est un enum alors on peut le passé en indice du tableau car il est considéré comme un int.

    done:
    return err_str;
}



/*
 *  Nom: writeImage
 *  But: Ecrire les valeurs de la matrice de la photo dans un nouveau fichier
 *
 *  Paramètres d'entrées:
 *      pathFile:       Le chemin vers le NOUVEAU fichier Portable pixmap (char*)
 *      matrice:        La matrice (1d) où les données vont être récuperées (int*)
 *      beginningImage: La position du curseur où commence les données de l'image (long int)
 *      dimension:      Les dimensions de l'image (longueur*largeur et * 3 si couleur) (long int)
 *
 *  Paramètres de sorties (sous forme de pointer):
 *
 *  Valeur de retour:
 *      (int) Le code erreur de type error_t à traduire grâce à la fonction error_str. Si la valeur de retour est égal à 0 (ERROR_OK) alors tout c'est déroulé normalement.
 *
 */
int writeImage(char* pathFile, int* matrice, long int beginningImage, long int dimension) {
    /* --------- DEFINITION DES VARIABLES --------- */
    FILE* image = NULL;
    int i = 0, caractereActuel, error;
    /* ------- FIN DEFINITION DES VARIABLES ------- */

    // On ouvre l'image
    image = fopen(pathFile, "r+");

    if(image != NULL) {

        error = fseek(image, beginningImage, SEEK_SET);

        if(error == 0) {
            for(i=0; i<dimension; i++) {
                fwrite(&matrice[i], 1, 1, image);
            }

        } else {
            return ERROR_INVARG;
        }

    } else {
        return ERROR_OPEN;
    }

    return 0;

}


/*
 *  Nom: writeHeader
 *  But: Ecrire les valeurs des paramètres du fichier dans un nouveau fichier
 *
 *  Paramètres d'entrées:
 *      pathFile:       Le chemin vers le NOUVEAU fichier Portable pixmap (char*)
 *      typeFile:       Le type du fichier (exemple "P6") (char*)
 *      imageWidth:     la largeur de l'image (long int)
 *      imageHeight:    La hauteur de l'image (long int)
 *      pixelIntensity: L'intensité des pixels (exemple "255") (long int)
 *
 *  Paramètres de sorties (sous forme de pointer):
 *      positionCursor: La position du curseur à la fin du header (*long int)
 *
 *  Valeur de retour:
 *      (int) Le code erreur de type error_t à traduire grâce à la fonction error_str. Si la valeur de retour est égal à 0 (ERROR_OK) alors tout c'est déroulé normalement.
 *
 */
int writeHeader(char* pathFile, char* typeFile, long int imageWidth, long int imageHeight, long int pixelIntensity, long int *positionCursor) {
    /* --------- DEFINITION DES VARIABLES --------- */
    FILE* image = NULL;
    /* ------- FIN DEFINITION DES VARIABLES ------- */

    // On ouvre l'image
    image = fopen(pathFile, "w");

    // On vérifie si l'ouverture de l'image s'est passée correctement
    if (image != NULL) {

        fputs(typeFile, image);
        fprintf(image, "\n%ld %ld", imageWidth, imageHeight);
        fprintf(image, "\n%ld\n", pixelIntensity);

        *positionCursor = ftell(image);
        fclose(image);   // On ferme le fichier

        return ERROR_OK;

    } else {
        // Le fichier n'a pas pu s'ouvrir donc on retourne une erreur
        return ERROR_OPEN;
    }
}


/*
 *  Nom: readImage
 *  But: Retourner les valeurs de la matrice de la photo
 *
 *  Paramètres d'entrées:
 *      pathFile:       Le chemin vers le fichier Portable pixmap (char*)
 *
 *  Paramètres de sorties (sous forme de pointer):
 *      matrice:        La matrice (1d) où les données vont être écritent (int*)
 *      beginningImage: La position du curseur où commence les données de l'image (long int)
 *      dimension:      Les dimensions de l'image (longueur*largeur et * 3 si couleur) (long int)

 *
 *  Valeur de retour:
 *      (int) Le code erreur de type error_t à traduire grâce à la fonction error_str. Si la valeur de retour est égal à 0 (ERROR_OK) alors tout c'est déroulé normalement.
 *
 */
int readImage(char* pathFile, int* matrice, long int beginningImage, long int dimension) {
    /* --------- DEFINITION DES VARIABLES --------- */
    FILE* image = NULL;
    int i = 0, error;
    /* ------- FIN DEFINITION DES VARIABLES ------- */

    // On ouvre l'image
    image = fopen(pathFile, "r");

    if(image != NULL) {

        error = fseek(image, beginningImage, SEEK_SET);

        if(error == 0) {
            for(i=0; i<dimension; i++) {
                matrice[i] = fgetc(image);
            }

            fclose(image);

            return ERROR_OK;

        } else {
            return ERROR_INVARG;
        }

    } else {
        return ERROR_OPEN;
    }

}


/*
 *  Nom: readHeader
 *  But: Retourner les valeurs des paramètres du header des fichiers Portable pixmap
 *
 *  Paramètres d'entrées:
 *      pathFile:       Le chemin vers le fichier Portable pixmap (char*)
 *
 *  Paramètres de sorties (sous forme de pointer):
 *      typeFile:       Le type du fichier (exemple "P6") (char*)
 *      imageWidth:     la largeur de l'image (pointer to long int)
 *      imageHeight:    La hauteur de l'image (pointer to long int)
 *      pixelIntensity: L'intensité des pixels (exemple "255") (pointer to long int)
 *      positionCursor: La position du curseur à la fin du header (*long int)
 *
 *  Valeur de retour:
 *      (int) Le code erreur de type error_t à traduire grâce à la fonction error_str. Si la valeur de retour est égal à 0 (ERROR_OK) alors tout c'est déroulé normalement.
 *
 */
int readHeader(char* pathFile, char* typeFile, long int *imageWidth, long int *imageHeight, long int *pixelIntensity, long int *positionCursor) {

    /* --------- DEFINITION DES VARIABLES --------- */
    FILE* image = NULL;
    char *ptr;
    int compteurLigne = 0;
    char buffer[50];

    /* ------- FIN DEFINITION DES VARIABLES ------- */

    // On ouvre l'image
    image = fopen(pathFile, "r");

    // On vérifie si l'ouverture de l'image s'est passée correctement
    if (image != NULL) {

        // Cette boucle va tourner dans le header du fixer Portable pixelmap pour récuperer toutes les informations.
        while(compteurLigne < 3) {

            // On récupère la neme ligne du header que l'on stock dans buffer
            fgets(buffer, 50, image);
            // On supprime le '\n' qui se rajoute automatiquement à cause de la fonction fgets
            strtok(buffer, "\n");

            // Si la ligne est un commentaire (i.e commençant par '#' alors on saute la boucle)
            if(buffer[0] == '#')
                continue;

            /*  Ce switch en fonction de compteurLigne nous aide à determiner si la ligne actuelle correspond au type de fichier, à sa largeur etc.....
             *  On a:
             *      case 0: La ligne correspond au type du fichier
             *      case 1: La ligne correspond aux dimensions de l'image (longueur*hauteur)
             *      case 2: La ligne correspond à l'intensité des pixels
             *
             */
            switch(compteurLigne) {
                case 0:
                    strncpy(typeFile, buffer, 50); // On retourne le type de l'image en copiant le buffer dans typeFile
                    break;
                case 1:

                    /* Ici on rencontre un problème: les dimensions de l'image (longueur*hauteur) sont sur la même ligne.
                     * On doit donc les séparer en deux en utilisant comme séparateur l'espace vide " ".
                     * La fonction strtok va nous aider à faire cela.
                     */


                    ptr = strtok(buffer, " ");

                    for(int j=0; j<2; j++) {
                        if(j == 0) {                  // Si on est à la première partie de la ligne alors c'est la longueur donc on retourne la valeur dans imageWidth en convertissant buffer (chaine de caractère) en long int grâce à la fonction strtol
                            *imageWidth = strtol(ptr, NULL, 10);
                        } else if(j == 1) {           // Si on est à la deuxème partie de la ligne alors c'est la hauteur donc on retourne la valeur dans imageHeight en convertissant buffer (chaine de caractère) en long int grâce à la fonction strtol
                            *imageHeight = strtol(ptr, NULL, 10);
                        }
                        ptr = strtok(NULL, " ");
                    }
                    break;
                case 2:
                    *pixelIntensity = strtol(buffer, NULL, 10); // On retourne la valeur de la ligne dans pixelIntensity en convertissant buffer (chaine de caractère) en long int grâce à la fonction strtol
                    break;

                default: // S'il y a plus de trois paramètres alors il y a une erreur.
                    return ERROR_HANDLE;
            }

            compteurLigne++;
        }

        *positionCursor = ftell(image); // Position du curseur
        fclose(image);   // On ferme le fichier
        return ERROR_OK; // Pas d'erreurs on retourne la valeur normal

    } else {
        // Le fichier n'a pas pu s'ouvrir donc on retourne une erreur
        return ERROR_OPEN;
    }

}



int fileToBinary(char* fileToCrypt,unsigned char** msgSecretBit, size_t *length) {

    int i;
    long fsize;
    unsigned char *string;

    FILE *f = fopen(fileToCrypt, "rb");
    if(f == NULL)
        return ERROR_OPEN;

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    string = malloc(fsize + 1);
    if(string == NULL)
        return ERROR_NOMEM;
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;

    *msgSecretBit = malloc(fsize*8+1);
    if(msgSecretBit == NULL)
        return ERROR_NOMEM;
    char bits[8];
    int k = 0, j=0;
    for(i=0; i<fsize; i++) {
        printbincharpad((char)string[i], bits);


        for (j = 0; j < 8; ++j) {
            (*msgSecretBit)[k] = bits[j];
            k++;

        }
    }
    *length = k;

    free(string);
    return ERROR_OK;
}




/* Hamming */
int determineBestHammingSize(unsigned int tailleImg, unsigned int tailleMsg, unsigned int* rows, unsigned int* columns) {


    unsigned int capacity;
    // On initialise la matrice de hamming la plus petite: rows=2 et columns=3
    *rows = 2;
    *columns =  (unsigned int)pow(2,(*rows)) - 1;

    // On calcul la capacité
    capacity = ( tailleImg / (*columns) ) * (*rows);

    // Tant que la capacité est supérieur à la taille du message, alors on augmente de 1 le nombre de lignes et on recalcul le nombre de colonnes, tout en recalculant la capacité.
    while(capacity > tailleMsg) {
        (*rows)++;
        *columns =  (unsigned int)pow(2,(*rows)) - 1;
        capacity = ( tailleImg / (*columns) ) * (*rows);
    }

    // Si la capacité est inférieur à la taille du message, alors on prend la matrice de hamming précédente, c'est à dire celle avec un nombre de lignes moins 1.
    if(capacity < tailleMsg) {
        (*rows)--;
        *columns =  (unsigned int)pow(2,(*rows)) - 1;
    }

    return ERROR_OK;
}

int isVectorNull(unsigned int **vector, unsigned int columns) {


    int x,null=1;
    for(x = 0; x < columns; x++) {
        if(vector[0][x] != 0)
            null = 0;
    }

    return null;
}

int findInHamming(unsigned int **matriceHamming, unsigned int columns, unsigned int rows, unsigned int **matriceRes, unsigned int *position) {
    int i,j;
    unsigned int same;

    if((columns>2)&&(rows>1)) {
        for (i = 0; i < columns; i++) {
            same = 1;
            for (j = 0; j < rows; j++) {
                if (matriceHamming[i][j] != matriceRes[0][j]) {
                    same = 0;
                }
            }
            if (same)
                *position = i;
        }
        return ERROR_OK;
    } else {
        return ERROR_INVARG;
    }
}



void additionMatrice(unsigned int m, unsigned int n, unsigned int** mat1, unsigned int** mat2, unsigned int*** res)
{
    unsigned int i, j;
    for(i=0;i<m;i++)
        for(j=0;j<n;j++)
            (*res)[i][j] = (mat1[i][j] + mat2[i][j]) % 2 ;

}


void multiplyMatrice(unsigned int m2, unsigned int** mat1, unsigned int n1, unsigned int n2, unsigned int** mat2, unsigned int*** res)
{
    int k, i, j;

    for(i = 0; i<n1; i++) {
        for(j = 0; j < m2; j++) {
            (*res)[i][j] = 0;
            for(k=0; k<n2; k++) {
                (*res)[i][j] +=  ((mat1[k][j]*((mat2[i][k] & ( 1u << 0u)) >> 0u) ) );

            }
            (*res)[i][j] %= 2;
        }
    }

}


int genererHamming(unsigned int*** hamming, unsigned int r, unsigned int c) {

    (*hamming) = (unsigned int **)calloc(c , sizeof(int *));         //Création d'un pointeur => tableau 1 colonne
    if((*hamming) == NULL)
        return ERROR_NOMEM;

    for (int i = 0; i < c; i++) {                           //Pour chacune des lignes de ce tableau
        (*hamming)[i] = (unsigned int *) calloc(r, sizeof(int));     //On malloc le nombre de colonnes que l'on veut
        if((*hamming)[i] == NULL)
            return ERROR_NOMEM;
    }

    int count;                                              //Variable qui permet de calculer le binaire
    for (int j = 1; j <= c; j++) {                          //On commence à 1 pour pouvoir convertir chaque numéro de colonne en binaire (sauf la colonne 0 => 0 en binaire)
        count = j;                                          //Pour ne pas modifier le comptage de boucle, on l'attribue à count
        for (int i = 0; i < r; i++) {                       //Pour chaque ligne
            if (count > 0) {                                //Tant que le chiffre convertit en binaire est > 0
                (*hamming)[j-1][i] = count % 2;             //On donne son binaire correspondant à au bit de la ligne
                count /= 2;
            }
        }
    }

    return ERROR_OK;
}


/* UI */


long reponseMenu(int choixMax) {

    long resultUser = 0;
    printf("> ");

    char *ptr;
    char str[10];

    // Ce code permet de récuperer un entier saisit par l'utilisateur et en verifiant le type et la taille. (Contrairement à scanf)
    size_t len;
    do {
        fgets(str, 2, stdin);
        len = strlen(str) ;
    } while ( len > 1 && str[len-1] != '\n');
    resultUser = strtol(str, &ptr, 10);

    // On vérifie si l'utilisateur à bien rentré un resultat qui rentre bien dans le type long et qui n'est pas null.
    if( resultUser == LONG_MAX || resultUser == LONG_MIN || !resultUser) {
        return -1;
    }


    // On vide le buffer du \n final
    viderBuffer();

    // Si ce qu'a rentré l'utilisateur ne correspond pas au choix max alors on retourne -1
    if((resultUser > choixMax) || (resultUser < 1))
        return -1;

    return resultUser;

}

void li(int position, char* text) {
    printf("[%d] %s\n", position, text);
}

void p(const char* text) {

    printf("\n");
    printf("    %s", text);
    printf("\n\n");

}

void h1(char* text) {
    printf("\n==================================================\n");

    unsigned long sizeTextRemaning, middleOfText, i;
    sizeTextRemaning = 50 - (strlen(text)+2);

    middleOfText = sizeTextRemaning / 2;

    for(i=0; i<middleOfText; i++) {
        printf("-");
    }
    printf(" %s ", text);

    if (sizeTextRemaning % 2)
        middleOfText += 1;

    for(i=0; i<middleOfText; i++) {
        printf("-");
    }

    printf("\n");

    printf("==================================================\n");
}

void viderBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
    {
        c = getchar();
    }
}