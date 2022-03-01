/**
 * @file header.h
 * @brief Définit les prototypes des fonctions du fichier main.c
 *
 * \version 1.0
 * \date 24/12/2020 11:12
 * \authors Melvin AUVRAY
 * \authors Timothée JACOB
 * \see main.c
 *
 */

#ifndef PROJETSTEGANO_HEADER_H
#define PROJETSTEGANO_HEADER_H

#endif //PROJETSTEGANO_HEADER_H

/** \enum error_t header.h
 *  \brief Liste les types d'erreurs que toutes les fonctions du programme peuvent renvoyer afin de permettre un retour utilisateur plus clair.
 *
 *  Ce enum permet de donner des noms à des entiers pour faciliter le codage des retours erreur des fonctions.\n
 *  En effet, au lieu de retourner 1,2,3, une fonction quelconque du programme retournera ERROR_OK, ERROR_INVARG, etc afin d'identifier le type d'erreur (Erreur d'allocation de mémoire, erreur d'ouverture de fichier, etc...)
 *
 *  \warning Le dernier type "ERROR_COUNT" n'est pas un véritable code d'erreur et ne doit être utilisé. Il est utile dans la fonction error_str et permet de savoir si le code d'erreur passé en paramètre est valide ou non.
 *  \see error_t
 *  \see error_str
 *
 */
typedef enum error_t {
    /// ERROR_OK = Aucune erreur
    ERROR_OK = 0,

    /// Invalid arguments (ex: problème d'argument(s) passé(s) en paramètres d'une fonction)
    ERROR_INVARG,

    /// Out of memory (Problème d'allocation dynamique)
    ERROR_NOMEM,

    /// Erreur de problème d'ouverture des fichiers
    ERROR_OPEN,

    /// Erreur de cas dans un if ou un switch (default alors que ça ne devrait pas)
    ERROR_HANDLE,


    /// Nombre total d'erreur de la liste. Pas un véritable code d'erreur
    ERROR_COUNT,
} error_t;



/**
 * \brief Tableau de char* ERROR_STRS permettant de faire un lien entre l'enum error_t et ses codes d'erreurs à une description plus détaillée de ces erreurs afin de les afficher à l'utilisateur.
 *
 */
const char* const ERROR_STRS[] =
        {
                "ERROR_OK: Aucune erreur detectées.",
                "ERROR_INVARG: Problème d'argument passés.",
                "ERROR_NOMEM: Impossible d'assigner la mémoire.",
                "ERROR_OPEN: Impossible d'ouvrir le fichier.",
                "ERROR_HANDLE: Erreur de cas dans un if ou un switch."
        };
























/************************************************
 *  Fonctions manipulation de tableau binaire
 ***********************************************/

/**
 * @fn int addExtensionSuffix(char* extensionFileToCrypt, unsigned char** msgSecret, size_t *tailleMsgBit)
 * @brief Cette fonction ajoute dans un tableau binaire un suffixe de 40 bits representant du texte (dans notre cas une extension) de 5 caractères maximum passé en entrée.
 *
 * @param extensionFileToCrypt L'extension que l'on souhaite ajouter en tant que suffixe dans le tableau binaire. Elle doit être de 5 char max.
 * @param msgSecret Le tableau de bits dans lequel le suffixe sera créé.
 * @param tailleMsgBit La taille du tableau de bits.
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @warning La variable extensionFileToCrypt doit être de 5 caractères maximum.
 */
int addExtensionSuffix(char* extensionFileToCrypt, unsigned char** msgSecret, size_t *tailleMsgBit);



/**
 * @fn int permuterTableau(char* key,int tailleTable, unsigned char** table)
 * @brief Cette fonction permet de permuter les éléments d'un tableau en fonction d'un mot de passe donné en entrée.
 *
 * Cette fonction utilise la fonction srand(seed) avec comme seed le hash du mot de passe donné en entrée. \n
 * Le hash est généré par la fonction "hash" utilisant l'algorithme "djb2".
 * La permutation est produite en utilisant le mélange de Fisher-Yates.
 * L'exact inverse est proposé par la fonction depermuterTableau.
 *
 * @param key Le mot de passe utilisé pour initialiser la fonction rand(), utilisée dans la permutation.
 * @param tailleTable La taille du tableau à mélanger.
 * @param table Le tableau que l'on souhaite mélanger.
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @see http://www.cse.yorku.ca/~oz/hash.html
 * @see https://stackoverflow.com/a/15961211/7817207
 * @see https://fr.wikipedia.org/wiki/M%C3%A9lange_de_Fisher-Yates
 * @see depermuterTableau
 */
int permuterTableau(char* key,int tailleTable, unsigned char** table);



/**
 * @fn int depermuterTableau(char* key,int tailleTable, unsigned char** table)
 * @brief Cette fonction est l'exact opposé de la fonction permuterTableau
 *
 * @param key Le mot de passe utilisé pour initialiser la fonction rand(), utilisée dans la permutation.
 * @param tailleTable La taille du tableau à mélanger.
 * @param table Le tableau que l'on souhaite mélanger.
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @see permuterTableau
 */
int depermuterTableau(char* key,int tailleTable, unsigned char** table);


/**
 * @fn int hideDimMsg(size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int* lengthDimensionPrefix)
 * @brief Cette fonction modifie les LSB d'un tableau de pixel pour y ajouter le prefixe correspondant à la taille du message secret.
 *
 * La taille du prefixe est determinée en convertissant la taille du tableau de pixel en binaire, puis en comptant le nombre de bits inclus dans le nombre binaire. \n
 * Ce nombre de bits correspond donc à la taille reservée du prefix. La valeur du prefixe est la taille du message secret plus la taille du prefixe. \n
 * Si la taille de la valeur du prefixe est inférieure à la taille totale du prefixe, des zeros pour remplir la taille sont ajoutés avant d'insérer la valeur de la taille du message secret + la taille du prefixe en binaire. \n\n
 *
 * # Exemple: #\n
 * Taille de l'image: 5*5=25 et taille du message à cacher: 8.\n
 * On convertit 25 en binaire: 25(10)=11001(2).\n
 * 25 en binaire possède 5 bits, on aura donc un prefix de taille 5 bits.\n
 * La fin de notre message secret aura comme position finale sa taille + la taille du prefix donc 8+5=13.\n
 * 13(10)=1101(2). Notre prefix aura donc comme valeur 1101. Comme cette valeur est inferieur à la taille total du prefix, on rajoute des zeros avant.\n
 * On aura donc un prefix total de 5 bits dont la valeur est: 01101.
 *
 *
 * @param tailleMsgBit Taille du message que l'on va cacher ultérieurement.
 * @param matriceImage Tableau de pixels dans lequel le prefixe va être ajouté en modifiant les LSBs.
 * @param dimension Taille du tableau de pixels.
 * @param pixelIntensity Intensité maximale des pixels du tableau. Cette variable est utile lors de la modification des LSBs
 * @param lengthDimensionPrefix Taille du prefixe, cette variable est un passage par adresse.
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int hideDimMsg(size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int* lengthDimensionPrefix);




/**
 * @fn int hideMessage(const unsigned char* messageBinary, size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int lengthDimensionPrefix, int crypt, char* keyCrypt)
 * @brief Cette fonction permet de cacher un message secret (sous forme de tableau de bits) dans un tableau de pixels.
 *
 * Cette fonction possède deux mode:
 * -# Un où l'insertion se fait pixel par pixel (i.e insertion classique)
 * -# Un autre mode où les pixels sont parcourus par un chemin pseudo aléatoire determiné par une clé secrete passée en paramètre.
 *
 * @param messageBinary Le message secret que l'on veut cacher, sous forme de tableau de bits.
 * @param tailleMsgBit La taille du tableau précédent. De type size_t.
 * @param matriceImage Le tableau de pixel (i.e l'image) dans lequel on va cacher notre message.
 * @param dimension La taille du tableau précédent (i.e taille de l'image). De type long int
 * @param pixelIntensity L'intensité maximale des pixels de l'image.
 * @param lengthDimensionPrefix Taille du prefixe. Le prefixe correspond aux pixels dont les LSBs forment la taille du message secret. Il est inséré par la fonction hideDimMsg.
 * @param crypt Cette variable de type int permet de spécifier si on insère les bits un par un (=0) où si on les insère dans un chemin pseudo aléatoire (=1)
 * @param keyCrypt Cette variable correspond à la clé secrète utile pour générer le chemin pseudo aléatoire si la variable crypt est égal à 1.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @warning La variable crypt doit valoir uniquement 0 ou 1. \n La variable keyCrypt doit valoir NULL si crypt vaut 0, sinon elle doit étre égale à un mot de passe secret.
 */
int hideMessage(const unsigned char* messageBinary, size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int lengthDimensionPrefix, int crypt, char* keyCrypt);







/************************************************
 *  Fonctions lecture de tableau binaire
 ***********************************************/

/**
 * @fn int readExtensionSuffix(const unsigned char* msgSecret, int lengthmsgSecret, char** extensionPixelMap)
 * @brief Fonction qui lit les 40 derniers bits d'un tableau de bits et les convertit en tableau de caractères, correspondant dans notre cas à l'extension du fichier.
 *
 * Cette fonction est appelé que si l'utilisateur souhaite decrypter un fichier, dans l'autre cas les 40 derniers LSBs ne correspondront à rien.
 *
 * @param msgSecret Le tableau de bits dans lequel le suffixe est présent.
 * @param lengthmsgSecret La taille du tableau de bit.
 * @param extensionPixelMap Passage par adresse d'un tableau de caractères de la valeur de l'extension (correspondant aux 40 derniers bits)
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int readExtensionSuffix(const unsigned char* msgSecret, int lengthmsgSecret, char** extensionPixelMap);


/**
 * @fn int decryptPrefix(const int *matriceImage, long int dimension, int* prefixInt, int* lengthDimensionPrefix)
 * @brief Cette fonction lit les n premiers LSBs d'un tableau de pixel et les convertit en entier. n étant passé en paramètre.
 *
 * @param matriceImage Tableau de pixels dans lequel on souhaite lire le prefixe.
 * @param dimension Dimension (Taille) du tableau de pixels.
 * @param prefixInt Passage par adresse de la valeur entière du prefixe.
 * @param lengthDimensionPrefix Passage par adresse de la taille du prefixe. (Utile ensuite pour savoir où commencer le décodage)
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @see hideDimMsg
 */
int decryptPrefix(const int *matriceImage, long int dimension, int* prefixInt, int* lengthDimensionPrefix);


/**
 * @fn int decryptMessage(const int *matriceImage, long int dimension, int prefixInt, int lengthDimensionPrefix,int crypt, char* keyCript, long int *tailleMsgDecrypt, unsigned char **messageSecretBitOutput)
 * @brief Cette fonction lit les LSBs d'un tableau de bits et les place dans un tableau de bits.
 *
 * Tout comme la fonction hideMessage, cette fonction possède deux mode: \n
 * -# Un mode (crypt = 0) où la lecture de l'image se fait pixel par pixel.
 * -# Un autre mode (crypt = 1) où la lecture de l'image se fait suivant un chemin pseudo aléatoire généré par un message secret passé en paramètre.
 *
 * @param matriceImage Tableau de pixels dans lequel on va lire les LSBs.
 * @param dimension Taille du tableau de pixels.
 * @param prefixInt Valeur du prefixe, ce qui correspond au nombre de LSBs que l'on va lire (i.e la taille du message secret)
 * @param lengthDimensionPrefix Taille du prefixe, ce qui correspond à la position à partir de laquelle nous allons commencer à lire les LSBs
 * @param crypt Cette variable de type int permet de spécifier si on lit les bits un par un (=0) où si on les lit dans un chemin pseudo aléatoire (=1)
 * @param keyCrypt Cette variable correspond à la clé secrète utile pour générer le chemin pseudo aléatoire si la variable crypt est égale à 1.
 * @param tailleMsgDecrypt Passage par adresse de la taille du message que l'on vient de décrypter
 * @param messageSecretBitOutput Passage par adresse
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int decryptMessage(const int *matriceImage, long int dimension, int prefixInt, int lengthDimensionPrefix,int crypt, char* keyCript, long int *tailleMsgDecrypt, unsigned char **messageSecretBitOutput);







/************************************************
 *  Fonctions Conversion
 ***********************************************/

/**
 * @fn int* num_to_bit(int a, int *len)
 * @brief Convertit un entier en un tableau binaire.
 *
 * La fonction retourne un tableau binaire en fonction d'un entier passé en paramètre. La fonction retourne aussi en passage par adresse la taille du tableau binaire.
 *
 * @param a L'entier que l'on veut convertir
 * @param len Passage par adresse de la valeur de la taille du tableau binaire
 * @return Tableau binaire
 *
 * @warning Le tableau binaire ne doit pas être initialisé avant d'utiliser la fonction. L'allocation dynamique se fait dans la fonction.
 */
int* num_to_bit(int a, int *len);

/**
 * @fn unsigned char* stringToBinary(char* s, size_t *length)
 * @brief Convertit une chaine de caractère en un tableau binaire de type unsigned char.
 *
 * La fonction retourne un pointer vers un unsigned char comportant la chaine de caractère passée en paramètre sous format binaire. La taille de ce tableau est également renvoyé en passage par adresse.
 * \n L'allocation dynamique du tableau se fait dans la fonction.
 *
 * @param s La chaine de caractère (Pointer vers char)
 * @param length Passage par adresse de la taille du tableau binaire (type size_t)
 * @return Tableau binaire de type pointeur vers unsigned char
 *
 * @warning Le tableau binaire ne doit pas être initialisé avant d'utiliser la fonction. L'allocation dynamique se fait dans la fonction.
 */
unsigned char* stringToBinary(char* s, size_t *length);


/**
 * @fn unsigned long hash(unsigned char *str)
 * @brief Renvoit le hash de type unsigned long d'une chaine de caractère (unsigned également). Le hash est calculé suivant l'algorithme djb2 créé par Dan Bernstein.
 *
 * @param str La chaine de caractère que l'on souhaite hasher, de type unsigned char.
 * @return Renvoit le hash correspondant à la chaine de caractère, de type unsigned long.
 *
 * @see http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long hash(unsigned char *str);

/**
 * @fn int binaryToUChar(long int tailleMsgDecrypt, const unsigned char *messageSecretBitOutput, unsigned char **msgSecret, int* lengthmsgSecret)
 * @brief Cette fonction prend en entrée un tableau binaire et renvoit un tableau de unsigned char, convertit par groupe d'octets.
 *
 * Le tableau de sortie est alloué dans la fonction et sa taille est renvoyé en passage par adresse.
 *
 * @param tailleMsgDecrypt La taille du tableau binaire que l'on souhaite convertir, de type long int.
 * @param messageSecretBitOutput Le tableau binaire que l'on souhaite convertir de type unsigned char. (Mais stocké avec du binaire).
 * @param msgSecret Passage par adresse du tableau des unsigned char.
 * @param lengthmsgSecret Passage par adresse de la taille du tableau converti.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @warning Le tableau de sortie msgSecret ne doit pas être alloué avant la fonction, l'allocation dynamique de mémoire ce fait dans la fonction. L'utilisateur doit free ce tableau après utilisation.
 */
int binaryToUChar(long int tailleMsgDecrypt, const unsigned char *messageSecretBitOutput, unsigned char **msgSecret, int* lengthmsgSecret);

/**
 * @fn void printbincharpad(char c, char* tabBit)
 * @brief Cette fonction convertit un caractère en un tableau de binaire.
 *
 * Le tableau passé en passage par adresse contenant le tableau binaire du caractère doit être alloué avant la fonction.
 *
 * @param c Le caractère que l'on souhaite convertir en binaire.
 * @param tabBit Passage par adresse du tableau binaire de taille 8 et de type char.
 */
void printbincharpad(char c, char* tabBit);


/**
 * @fn int addExtension(char** fileOutput, const char *extensionPixelMap)
 * @brief Cette fonction ajoute une chaine de caractère (Une extension dans notre cas) à une autre chaine de caractère (un nom de fichier dans notre cas)
 *
 * La réallocation dynamique se fait à l'interieur de la fonction.
 * \n Nul besoin de passer la taille des deux chaines de caractères.
 * \n L'extension se rajoute à la toute fin de la chaine de caractère.
 * \n L'extension doit contenir un '.' en début de chaine.
 *
 * @param fileOutput Passage par adresse de la chaine de caractère à laquelle on souhaite ajouter l'extension.
 * @param extensionPixelMap Chaine de caractère (i.e l'extension) que l'on souhaite ajouter à la fin.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int addExtension(char** fileOutput, const char *extensionPixelMap);







/************************************************
 *  Fonctions manipulation de fichiers
 ***********************************************/

/**
 * @fn int readHeader(char* pathFile, char* typeFile, long int *imageWidth, long int *imageHeight, long int *pixelIntensity, long int *positionCursor)
 * @brief Cette fonction récupère les informations du header d'un fichier portable pixel map.
 *
 * La fonction ouvre un fichier portable pixmap et lit le header qui la compose. Elle récupère les dimensions de l'image, la profondeur des pixels ainsi que le type de fichier. Elle retourne également la position du curseur à laquelle se trouve la fin du header.
 * \n La fonction fait abstraction des commentaires présents dans le header (lignes commençant par '#')
 *
 * @param pathFile Chaine de caractères representant le chemin vers le fichier portable pixmap que l'on souhaite analyser.
 * @param typeFile Passage par adresse d'une chaine de caractère representant le type de fichier. (Exemple: P5 ou P6)
 * @param imageWidth Passage par adresse de la largeur de l'image en pixel. Type long int.
 * @param imageHeight Passage par adresse de la hauteur de l'image en pixel. Type long int.
 * @param pixelIntensity Passage par adresse de l'intensité des pixels (par exemple 255). Type long int.
 * @param positionCursor Passage par adresse de la position du curseur de la fin du header, utile afin de savoir à partir de quelle ligne commence la matrice image. Type long int.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int readHeader(char* pathFile, char* typeFile, long int *imageWidth, long int *imageHeight, long int *pixelIntensity, long int *positionCursor);

/**
 * @fn int readImage(char* pathFile, int* matrice, long int beginningImage, long int dimension)
 * @brief Cette fonction lit une image portable pixmap et retourne un tableau de 1 dimension contenant la matrice des pixels de l'image.
 *
 * La dimension du tableau doit être calculé au préalable (i.e longueur*hauteur ou longueur*hauteur*3 dépendant de si l'image est en noir et blanc ou non).
 *
 * @param pathFile Chaine de caractères représentant le chemin vers le fichier portable pixmap que l'on souhaite analyser.
 * @param matrice Tableau d'entiers qui contiendra la matrice des pixels de l'image.
 * @param beginningImage Position du curseur à partir de laquelle la matrice de l'image commence. Cette position peut être determinée grâce à la fonction readHeader.
 * @param dimension Dimension du tableau de la matrice image, à determiner avant d'utiliser la fonction.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @see readHeader
 * @warning Le tableau qui contiendra la matrice des pixels de l'image doit être alloué avant d'appeller la fonction. Et l'utilisateur est libre de le free après.
 */
int readImage(char* pathFile, int* matrice, long int beginningImage, long int dimension);

/**
 * @fn int writeHeader(char* pathFile, char* typeFile, long int imageWidth, long int imageHeight, long int pixelIntensity, long int *positionCursor)
 * @brief Cette fonction recrée un header d'un fichier portable pixmap en fonction des données passées en paramètre.
 *
 * Cette fonction est la complémentaire de la fonction readHeader.
 *
 * @param pathFile Chaine de caractère representant le chemin vers le fichier portable pixmap où l'on souhaite ajouter le header. Si le fichier n'existe pas, il sera créé.
 * @param typeFile Chaine de caractère representant le type de fichier. (Exemple: P5 ou P6)
 * @param imageWidth Largeur de l'image en pixel. Type long int.
 * @param imageHeight Hauteur de l'image en pixel. Type long int.
 * @param pixelIntensity Intensité des pixels (par exemple 255). Type long int.
 * @param positionCursor Passage par adresse de la position du curseur de la fin du header, utile afin de savoir à partir de quelle ligne commence la matrice image. Type long int.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @warning Le fichier peut ou ne peut pas exister. Le cas échéant, il sera crée.
 * @warning Les commentaires ne peuvent pas être ajouté dans ce nouveau header.
 */
int writeHeader(char* pathFile, char* typeFile, long int imageWidth, long int imageHeight, long int pixelIntensity, long int *positionCursor);

/**
 * @fn int writeImage(char* pathFile, int* matrice, long int beginningImage, long int dimension)
 * @brief Cette fonction ajoute une matrice de pixel passée en paramètre à un fichier portable pixmap.
 *
 * Cette fonction est la complémentaire de la fonction readImage.
 *
 * @param pathFile Chaine de caractère representant le chemin vers le fichier portable pixmap où l'on souhaite ajouter le header. Le fichier doit être créé au préalable.
 * @param matrice Matrice de pixel que l'on souhaite ajouter à l'image.
 * @param beginningImage Position du curseur representant la fin du header de l'image. Peut être determiné grâce à la fonction writeHeader
 * @param dimension Dimension du tableau de pixel.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @warning Le fichier doit être créé avant d'appeller cette fonction.
 * @see writeHeader
 */
int writeImage(char* pathFile, int* matrice, long int beginningImage, long int dimension);

/**
 * @fn int fileToBinary(char* fileToCrypt,unsigned char** msgSecretBit, size_t *length)
 * @brief Cette fonction lit un fichier et le convertit en un tableau binaire.
 *
 * Cette fonction prend un chemin vers un fichier en paramètre d'entrée et convertit le contenu de ce fichier en tableau binaire.
 * \n Une étape intermediaire est utilisée pour ce faire: d'abord la fonction lit le fichier char par char, puis grâce à la fonction printbincharpad, la fonction convertit les char en tableau binaire.
 *
 * @param fileToCrypt Chaine de caractères correspondant au chemin vers le fichier que l'on veut convertir.
 * @param msgSecretBit Passage par adresse d'un tableau 1D contenant le tableau binaire correspondant au fichier.
 * @param length Passage par adresse de la taille du tableau binaire généré. De type size_t.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @warning Le tableau binaire est alloué dans la fonction. L'user doit le free après utilisation.
 * @see printbincharpad
 */
int fileToBinary(char* fileToCrypt,unsigned char** msgSecretBit, size_t *length);

/**
 * @fn int createFileFromByte(const char* fileToCrypt, const unsigned char* msgSecret, long int tailleMsgDecrypt)
 * @brief Crée un fichier à partir d'un tableau d'octet (unsigned char)
 *
 * Le fichier est créé par la fonction.
 *
 * @param fileToCrypt Chemin vers le fichier à créer.
 * @param msgSecret Tableau d'octet correspondant au fichier (type unsigned char)
 * @param tailleMsgDecrypt Taille du tableau BINAIRE correspondant au tableau d'octet.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 * @warning La taille passée doit être la taille du tableau binaire et non la taille du tableau d'octet, puisque la fonction calcul automatiquement la division par 8.
 */
int createFileFromByte(const char* fileToCrypt, const unsigned char* msgSecret, long int tailleMsgDecrypt);

/**
 * @fn int getExtension(const char* pathToFile, char** extensionPixelMap)
 * @brief Cette fonction renvoit l'extension d'un fichier ('.' inclus)
 *
 * L'allocation de la mémoire de la chaine de caractère correspondant à l'extension se fait dans la fonction. C'est à l'utilisateur de la libérer après utilisation.
 *
 * @param pathToFile Chemin vers le fichier que l'on souhaite récuperer l'extension.
 * @param extensionPixelMap Passage par adresse de la chaine de caractères qui contient l'extension du fichier.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @note Cette fonction correspond à une fonction "split"
 */
int getExtension(const char* pathToFile, char** extensionPixelMap);





/************************************************
 *  Fonctions Hamming
 ***********************************************/

/**
 * @fn int determineBestHammingSize(unsigned int tailleImg, unsigned int tailleMsg, unsigned int* rows, unsigned int* columns)
 * @brief Cette fonction determine la taille la plus adaptée pour une matrice de hamming en fonction de la taille d'une image et d'un message.
 *
 * Cette fonction calcule la capacité de chaque matrice de hamming jusqu'à trouver celle qui correspond le mieux à la taille du message.
 *
 * @param tailleImg Taille de l'image.
 * @param tailleMsg Taille du message.
 * @param rows Passage par adresse du nombre de lignes de la matrice de hamming.
 * @param columns Passage par adresse du nombre de colonnes de la matrice de hamming.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int determineBestHammingSize(unsigned int tailleImg, unsigned int tailleMsg, unsigned int* rows, unsigned int* columns);

/**
 * @fn int isVectorNull(unsigned int **vector, unsigned int columns)
 * @brief Cette fonction determine si un vecteur possède des composantes toutes égales à 0 ou non.
 *
 * Un vecteur est codé de cette manière: tableau 2D dont la première case est égale à 0.
 * \n Pour un vecteur de 5 composantes on a donc: vect[0][5];
 *
 * @param vector Tableau 2D vers le vecteur que l'on souhaite tester.
 * @param columns Nombre de composantes du vecteur.
 *
 * @return 1 si le vecteur est le vecteur NULL, 0 sinon.
 *
 * @warning Le vecteur doit être bien formaté selon ce format: tableau 2D dont la première case est égale à 0. Pour un vecteur de 5 composantes on a donc: vect[0][5];
 */
int isVectorNull(unsigned int **vector, unsigned int columns);

/**
 * @fn int findInHamming(unsigned int **matriceHamming, unsigned int columns, unsigned int rows, unsigned int **matriceRes, unsigned int *position)
 * @brief Cette fonction permet de faire une correspondance dans une matrice de Hamming.
 *
 * Un vecteur est donné en paramètre et la fonction permet de dire si ce vecteur correspond à l'une des colonnes de la matrice de Hamming.
 *
 * @param matriceHamming Tableau 2D correspondant à la matrice de Hamming.
 * @param columns Nombre de colonnes de la matrice de Hamming
 * @param rows Nombre de lignes de la matrice de Hamming
 * @param matriceRes Vecteur que l'on souhaite faire correspondre. Ce vecteur ne doit pas être nul. Tableau 2D codé suivant la manière vu dans le warning.
 * @param position Passage par adresse de la position dans la matrice de hamming dont la colonne correspond au vecteur passé en paramètre.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 */
int findInHamming(unsigned int **matriceHamming, unsigned int columns, unsigned int rows, unsigned int **matriceRes, unsigned int *position);

/**
 * @fn void additionMatrice(unsigned int m, unsigned int n, unsigned int** mat1, unsigned int** mat2, unsigned int*** res)
 * @brief Additionne deux matrices en modulo 2.
 *
 * Cette fonction prend deux tableau 2D en paramètre (de même dimension) et renvoit le résultat de l'addition en passage par adresse.
 *
 * @param m Nombre de colonnes des matrices.
 * @param n Nombre de ligne des matrices.
 * @param mat1 Tableau 2D correspondant à la matrice 1.
 * @param mat2 Tableau 2D correspondnat à la matrice 2.
 * @param res Passage par adresse de la matrice contenant le résultat de l'addition.
 *
 * @warning Les deux matrices doivent avoir les mêmes dimensions.
 * @warning Les matrices doivent être codées de la sorte que les colonnes soient le premier indice.
 * @warning Les calculs sont effectué en base 2 (modulo 2)
 */
void additionMatrice(unsigned int m, unsigned int n, unsigned int** mat1, unsigned int** mat2, unsigned int*** res);

/**
 * @fn void multiplyMatrice(unsigned int m2, unsigned int** mat1, unsigned int n1, unsigned int n2, unsigned int** mat2, unsigned int*** res)
 * @brief Multiplie deux matrices en modulo 2.
 *
 * Cette fonction prend deux tableau 2D en paramètre (avec leurs dimensions respectives) et renvoit le résultat de la multiplication en passage par adresse.
 *
 * @param m2 Nombre de ligne de la matrice 1
 * @param mat1 Tableau 2D correspondant à la matrice 1
 * @param n1 Nombre de colonnes des deux matrices.
 * @param n2 Nombre de ligne de la matrice 2.
 * @param mat2 Tableau 2D correspondant à la matrice 2
 * @param res Passage par adresse d'un tableau 2D correspondnat à la matrice contenant le résultat de la multiplication.
 *
 * @warning Les matrices doivent être codées de la sorte que les colonnes soient le premier indice.
 * @warning Les matrices doivent avoir le même nombre de colonnes.
 * @warning Les calculs sont effectué en base 2 (modulo 2)
 */
void multiplyMatrice(unsigned int m2, unsigned int** mat1, unsigned int n1, unsigned int n2, unsigned int** mat2, unsigned int*** res);


/**
 * @fn int genererHamming(unsigned int*** hamming, unsigned int r, unsigned int c)
 * @brief Génère une matrice de hamming en fonction des dimensions passées en paramètre.
 *
 * @param hamming Passage par adresse d'un tableau 2D correspondant à la matrice de hamming.
 * @param r Nombre de lignes de la matrice de Hamming
 * @param c Nombre de colonnes de la matrice de Hamming
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @note La matrice de hamming est un tableau 2D codé de manière à ce que les colonnes sont les premiers indices du tableau.
 */
int genererHamming(unsigned int*** hamming, unsigned int r, unsigned int c);



/**
 * @fn int hideMessageHamming(const unsigned char* messageBinary, size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int lengthDimensionPrefix, unsigned int rows, unsigned int columns, unsigned int* compteurNbBitsModif)
 * @brief Cette fonction cache un message (tableau binaire) dans un tableau de pixel en utilisant la méthode de Hamming.
 *
 * On utilise une matrice de hamming de taille donnée en paramètre. La matrice sera de taille par exemple (N,M).
 * Pour crypter, on récupère une séquence de N LSB de notre image que l'on multiplie à la matrice de hamming. On additionne le résultat (XOR) avec une séquence de M bits du message à cacher. On aura un output de taille (1, M) qui correspondra à l'unes des N colonnes de la matrice de hamming. On a plus qu'à reporter sur la matrice de hamming pour savoir à quelle numéro de colonne cela correspond. Si notre résultat est égal à la 2eme colonne de la matrice de hamming on va modifier notre 2eme/M bits LSB de notre image de départ.
 *
 * @param messageBinary Le tableau binaire du message que l'on souhaite cacher.
 * @param tailleMsgBit La taille du tableau binaire du message.
 * @param matriceImage Le tableau 1D de pixel de l'image dans laquelle on va cacher le message.
 * @param dimension La taille de la matrice de pixel de l'image.
 * @param pixelIntensity L'intensité maximale des pixels de l'image.
 * @param lengthDimensionPrefix La taille du prefixe de l'image.
 * @param rows Le nombre de lignes de la matrice de Hamming.
 * @param columns Le nombre de colonnes de la matrice de Hamming
 * @param compteurNbBitsModif Passage par adresse du nombre de bits modifié par cette méthode.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @note Les dimensions de la matrice de Hamming doivent être determinées avant d'appeller cette fonction.
 *
 * @see hideDimMsg
 * @see decryptMessageHamming
 */
int hideMessageHamming(const unsigned char* messageBinary, size_t tailleMsgBit, int* matriceImage, long int dimension, long int pixelIntensity, int lengthDimensionPrefix, unsigned int rows, unsigned int columns, unsigned int* compteurNbBitsModif);


/**
 * @fn int decryptMessageHamming(const int *matriceImage, long int dimension, int prefixInt, int lengthDimensionPrefix, unsigned int rows, unsigned int columns, long int *tailleMsgDecrypt, unsigned char **messageSecretBitOutput)
 * @brief Cette fonction décode un message caché dans une image en utilisant la méthode de Hamming.
 *
 * Méthode pour décrypter: On reprend notre matrice de hamming de (N,M) taille que l'on multiplie par des séquences de N LSB de l'image. On a à chaque fois une matrice output de taille M qui est notre message décodé si on les met toutes côte à côte.
 *
 * @param matriceImage Tableau 1D de pixels de l'image dans laquelle le message est caché.
 * @param dimension Dimension du tableau de pixels.
 * @param prefixInt Valeur du prefixe de l'image, ce qui correspond à la taille de notre message secret.
 * @param lengthDimensionPrefix Longueur du prefixe, ce qui correspond à l'endroit où commence notre message secret.
 * @param rows Nombre de lignes de la matrice de Hamming
 * @param columns Nombre de colonnes de la matrice de Hamming.
 * @param tailleMsgDecrypt Passage par adresse de la taille du message secret.
 * @param messageSecretBitOutput Passage par adresse du tableau 1D contenant le message secret.
 *
 * @return Retourne un int correspondant au code d'erreur. Celui-ci vaut ERROR_OK si tout s'est bien passé.
 *
 * @note Les dimensions de la matrice de Hamming doivent être determinées avant d'appeller cette fonction.
 *
 * @warning Le tableau de sortie contenant le message secret ne doit pas être alloué avant la fonction. L'utilisateur est libre de le libérer après utilisation.
 *
 * @see hideMessageHamming
 *
 */
int decryptMessageHamming(const int *matriceImage, long int dimension, int prefixInt, int lengthDimensionPrefix, unsigned int rows, unsigned int columns, long int *tailleMsgDecrypt, unsigned char **messageSecretBitOutput);


/************************************************
 *  Fonctions UI
 ***********************************************/

/**
 * @fn void h1(char* text)
 * @brief Affiche du texte en le stylisant comme un titre de 1ere importance.
 *
 * Cette fonction peut être assimilée comme une balise <h1> du langage HTML.
 *
 * @param text Le texte que l'on souhaite afficher à l'écran.
 */
void h1(char* text);

/**
 * @fn void p(const char* text)
 * @brief Affiche du texte en le stylisant comme un paragraphe.
 *
 * Cette fonction peut être assimilée comme une balise <p> du langage HTML.
 *
 * @param text Le texte que l'on souhaite afficher à l'écran.
 */
void p(const char* text);

/**
 * @fn void li(int position, char* text)
 * @brief Affiche du texte en le stylisant comme une liste avec comme index un chiffre passé en paramètre.
 *
 * Cette fonction peut être assimilée comme une balise <li> du langage HTML.
 *
 * @param position L'index de l'element de la liste
 * @param text Le texte que l'on souhaite afficher à l'écran.
 *
 * @see reponseMenu
 */
void li(int position, char* text);

/**
 * @fn long reponseMenu(int choixMax)
 * @brief Cette fonction renvoit un entier donné par l'utilisateur compris entre 1 (inclus) et choixMax (inclus)
 *
 * Cette fonction peut être assimilée comme la complémentaire de la fonction li qui permet de créer une liste. Puis cette fonction récupère le choix de l'utilisateur.
 *
 * @param choixMax L'entier maximal que l'utilisateur pourra rentrer.
 * @return Le choix de l'utilisateur. -1 si le choix n'est pas comprit entre 1 et choixMax (inclus)
 *
 * @see li
 */
long reponseMenu(int choixMax);

/**
 * @fn char *inputString(FILE* fp, size_t size)
 * @brief Cette fonction récupère une chaine de caractères rentrée par l'utilisateur de taille quelconque.
 *
 * L'allocation dynamique se fait au fur et à mesure que l'utilisateur rentre la chaine de caractère.
 *
 * @param fp Le buffer d'entrée de la chaine. Dans notre cas on le met à stdin pour récuperer les saisies de l'utilisateur.
 * @param size La taille minimale (et point de départ de l'allocation dynamique) de la chaine de caractères.
 * @return Tableau de caractères correspondant à la chaine de caractères, finissant par '\0'
 *
 * @warning La chaine de caractère doit être libérée par l'utilisateur après utilisation.
 */
char *inputString(FILE* fp, size_t size);

/**
 * @fn void viderBuffer()
 * @brief Vide le buffer du clavier en absorbant tous les caractères jusqu'a trouver '\0' ou EOF
 *
 */
void viderBuffer();

/**
 * @fn const char* error_str(error_t err)
 * @brief Retourner une chaîne de caractères correspondant à l'erreur fournie en paramètre
 * @param err Le code d'erreur (error_t ou int)
 *
 * @return La chaine de caractère puisée dans le tableau ERROR_STRS du fichier header.h correspondant à l'erreur fournit en paramètre.
 */
const char* error_str(error_t err);


/************************************************
 *  Autre
 ***********************************************/

/**
 * @fn void freeAllVar(void* pointer1, void* pointer2, void* pointer3, void* pointer4, void* pointer5, void* pointer6, void* pointer7)
 * @brief Libère toutes les variables passées en paramètre si elles ne sont pas égales à NULL
 *
 * @param pointer1 Pointeur à libérer.
 * @param pointer2 Pointeur à libérer.
 * @param pointer3 Pointeur à libérer.
 * @param pointer4 Pointeur à libérer.
 * @param pointer5 Pointeur à libérer.
 * @param pointer6 Pointeur à libérer.
 * @param pointer7 Pointeur à libérer.
 *
 * @warning Les pointeurs doivent avoir été initialisés à NULL lors de la création de la variable pour ne pas libérer de la mémoire qui n'a pas été allouée.
 */
void freeAllVar(void* pointer1, void* pointer2, void* pointer3, void* pointer4, void* pointer5, void* pointer6, void* pointer7);