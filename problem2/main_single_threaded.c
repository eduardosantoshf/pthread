#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>

static void printUsage (char *cmdName);

int main(int argc, char * argv[]) {
    FILE *fp;
    double determinant;
    int i, j, k;

    double t0, t1, t2; /* time limits */
    t2 = 0.0;

    int opt;                  /* selected option */
    char *fName = "no name";  /* file name (initialized to "no name" by default) */
    int val = -1;             /* numeric value (initialized to -1 by default) */
    opterr = 0;
    do
    { switch ((opt = getopt (argc, argv, "f:n:h"))) { 
        case 'f': /* file name */
                if (optarg[0] == '-')
                    { fprintf (stderr, "%s: file name is missing\n", basename (argv[0]));
                    printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
                    }
                    fName = optarg;
                    break;
        case 'n': /* numeric argument */
                    if (atoi (optarg) <= 0)
                    { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
                        printUsage (basename (argv[0]));
                        return EXIT_FAILURE;
                    }
                    val = (int) atoi (optarg);
                    break;
        case 'h': /* help mode */
                    printUsage (basename (argv[0]));
                    return EXIT_SUCCESS;
        case '?': /* invalid option */
                    fprintf (stderr, "%s: invalid option\n", basename (argv[0]));
                printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
        case -1:  break;
        }
    } while (opt != -1);

    if (argc == 1){ 
        fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
        printUsage (basename (argv[0]));
        return EXIT_FAILURE;
    }

    int o; /* counting variable */

    printf ("File name = %s\n", fName);
    printf ("Numeric value = %d\n", val);

    for (o = 0; o < argc; o++)
        printf ("Word %d = %s\n", o, argv[o]);

    printf("\n");

    fp = fopen(fName,"rb");  // r for read, b for binary

    int amount = 0;
    fread(&amount, sizeof(int), 1, fp);

    int order = 0;
    fread(&order, sizeof(int), 1, fp);

    double matrix[order][order];
    int l = 0;

    while (fread(&matrix, sizeof(matrix), 1, fp)) {
        double ratio;
        double det = 1;

        t0 = ((double) clock ()) / CLOCKS_PER_SEC;

        /* Here we are using Gauss Elimination
        Technique for transforming matrix to
        upper triangular matrix */

        /* Applying Gauss Elimination */        
        for (i = 0; i < order; i++)
        {
            if (matrix[i][i] == 0.0)
            {
                printf("Mathematical Error!");
                exit(0);
            }
            for (j = i + 1; j < order; j++)
            {
                ratio = matrix[j][i] / matrix[i][i];

                for (k = 0; k < order; k++)
                {
                        matrix[j][k] = matrix[j][k] - ratio * matrix[i][k];
                }
            }
        }
        
        /* Displaying upper triangular matrix */
        /*
        printf("\nUpper Triangular Matrix: \n");

        for(i = 0; i < order; i++)
        {
            for(j = 0; j < order; j++)
            {
                printf("%7.2f\t",matrix[i][j]);
            }
            printf("\n");
        }
        */

        /* Finding determinant by multiplying
        elements in principal diagonal elements */
        for (i = 0; i < order; i++)
        {
            det = det * matrix[i][i];
        }

        printf("Processing matrix %d\n", l + 1);
        printf("The determinant is %.3e \n", det);

        t1 = ((double) clock ()) / CLOCKS_PER_SEC;
        t2 += t1 - t0;
        
        l++;
    }

    printf ("\nElapsed time = %.6f s\n", t2);

    return 0;

}

static void printUsage (char *cmdName)
{
  fprintf (stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
           "  OPTIONS:\n"
           "  -h      --- print this help\n"
           "  -f      --- filename\n"
           "  -n      --- positive number\n", cmdName);
}