#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXSIZE 255
#define NUMROADS 5
#define MAXSTREAMS 10  /* Max. number of stream crossings per road id. */
#define MAXLINES 200 /* Maximum number of road segments. */

// Program to fix the roads in DHSVM input files

int main(int argc, char *argv[]) {
    FILE *fi, *fo;
    int col, row, ncol, nrow, ntot, id, maxid;
    float xll, yll, nodata, cellsize;
    int gotroad[MAXLINES], sscol[MAXLINES], ssrow[MAXLINES], icountsinks[MAXLINES]; 
    int gotstream;
    int mecol, merow;
    float minelev;
    int stcol[MAXSTREAMS], strow[MAXSTREAMS], stelev[MAXSTREAMS];
    int minstid, minstelev;
    float **elev;
    int **stream;
    int ***road;
    int **nroad;
    int **sink;
    int **output;
    int igotsink;
    int scts, sctrf;
    char cjunk[MAXSIZE];
    char inroad[50], instream[50], outroad[50], indem[50];
    float length, height, width, aspect;
    int nskips, nskipr, nlsf, nlrf;
    int i, j, ik;
    int totalsinks;

    if (argc != 7) {
        fprintf(stderr, "USAGE: fixroads <dem> <stream map> <road map in> <road map out> <stream header> <road header>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(indem, argv[1]);
    strcpy(instream, argv[2]);
    strcpy(inroad, argv[3]);
    strcpy(outroad, argv[4]);
    nskips = atoi(argv[5]);
    nskipr = atoi(argv[6]);

    scts = 0;
    sctrf = 0;
    igotsink = 0;

    fprintf(stderr, "Reading in elevation data and inverting nodata value\n");

    // Open input file
    if ((fi = fopen(indem, "r")) == NULL) {
        printf("Error Opening Input File, %s.\n", indem);
        exit(EXIT_FAILURE);
    }

    fscanf(fi, "%*s %d\n", &ncol);
    fscanf(fi, "%*s %d\n", &nrow);
    fscanf(fi, "%*s %f\n", &xll);
    fscanf(fi, "%*s %f\n", &yll);
    fscanf(fi, "%*s %f\n", &cellsize);
    fscanf(fi, "%*s %f\n", &nodata);

    // Allocate memory
    if (!(elev = (float **)calloc(nrow, sizeof(float *)))) {
        printf("Cannot allocate memory for elevation.\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nrow; i++) {
        if (!(elev[i] = (float *)calloc(ncol, sizeof(float)))) {
            printf("Cannot allocate memory for elevation.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!(stream = (int **)calloc(nrow, sizeof(int *)))) {
        printf("Cannot allocate memory for stream.\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nrow; i++) {
        if (!(stream[i] = (int *)calloc(ncol, sizeof(int)))) {
            printf("Cannot allocate memory for stream.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!(road = (int ***)calloc(nrow, sizeof(int **)))) {
        printf("Cannot allocate memory for road.\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nrow; i++) {
        if (!(road[i] = (int **)calloc(ncol, sizeof(int *)))) {
            printf("Cannot allocate memory for road.\n");
            exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < nrow; i++) {
        for (j = 0; j < ncol; j++) {
            if (!(road[i][j] = (int *)calloc(NUMROADS, sizeof(int)))) {
                printf("Cannot allocate memory for road.\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (!(nroad = (int **)calloc(nrow, sizeof(int *)))) {
        printf("Cannot allocate memory for nroad.\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nrow; i++) {
        if (!(nroad[i] = (int *)calloc(ncol, sizeof(int)))) {
            printf("Cannot allocate memory for nroad.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!(sink = (int **)calloc(nrow, sizeof(int *)))) {
        printf("Cannot allocate memory for sink.\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nrow; i++) {
        if (!(sink[i] = (int *)calloc(ncol, sizeof(int)))) {
            printf("Cannot allocate memory for sink.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!(output = (int **)calloc(nrow, sizeof(int *)))) {
        printf("Cannot allocate memory for output.\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nrow; i++) {
        if (!(output[i] = (int *)calloc(ncol, sizeof(int)))) {
            printf("Cannot allocate memory for output.\n");
            exit(EXIT_FAILURE);
        }
    }

    ntot = ncol * nrow;

    for (row = 0; row < nrow; row++) {
        for (col = 0; col < ncol; col++) {
            fscanf(fi, "%f", &elev[row][col]);

            if (elev[row][col] == -9999) 
                elev[row][col] = 9999;
        }
    }
    fclose(fi);

    scts = 0;
    fprintf(stderr, "Initializing stream and road locations and sinks to blank\n");

    for (row = 0; row < nrow; row++) {
        for (col = 0; col < ncol; col++) {
            stream[row][col] = 0;
            sink[row][col] = 0;
            for (ik = 0; ik < NUMROADS; ik++) 
                road[row][col][ik] = 0;
        }
    }

    fprintf(stderr, "Assuming that the road and stream files have 0,0 as their top left corner \n");
    fprintf(stderr, "\nReading in stream locations...\n");

    if ((fi = fopen(instream, "r")) == NULL) {
        printf("Error Opening Input File, %s.\n", instream);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < nskips; i++) 
        fgets(cjunk, MAXSIZE, fi);

    nlsf = 0;
    while (fscanf(fi, "%d %d", &col, &row) != EOF) {
        nlsf += 1;
        stream[row][col] = 1;
        fgets(cjunk, MAXSIZE, fi);
    }

    fclose(fi);
    fprintf(stderr, "Number of lines in the input streams files = %d\n", nlsf + nskips);
    fprintf(stderr, "Reached end of stream locations file\n");
    fprintf(stderr, "Final location was col: %d, row: %d\n", col, row);

    fprintf(stderr, "Reading in road locations\n");

    if ((fi = fopen(inroad, "r")) == NULL) {
        printf("Error Opening Input File, %s.\n", inroad);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < nskipr; i++) 
        fgets(cjunk, MAXSIZE, fi);

    maxid = 0;

    nlrf = 0;
    while (fscanf(fi, "%d %d %d", &col, &row, &id) != EOF) {
        nlrf += 1;
        fgets(cjunk, MAXSIZE, fi);

        if (nroad[row][col] >= NUMROADS) {
            fprintf(stderr, "The number of roads per grid cell exceeds the defined maximum %d\n", nroad[row][col]);
            fprintf(stderr, "Change #define NUMROADS and recompile.\n");
            exit(EXIT_FAILURE);
        }

        road[row][col][nroad[row][col]] = id;
        nroad[row][col] += 1;

        gotroad[id - 1] = 1;

        if (id > maxid) {
            maxid = id;
            if (maxid >= MAXLINES) {
                fprintf(stderr, "Number of road segments exceeds defined maximum.\n");
                fprintf(stderr, "Edit #define MAXLINES and recompile.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(fi);
    fprintf(stderr, "Number of lines in the input roads files = %d\n", nlrf + nskipr);
    fprintf(stderr, "Reached end of road locations file\n");
    fprintf(stderr, "Final location was col %d row %d id %d\n", col, row, id);
    fprintf(stderr, "Maximum road id number is %d\n", maxid);

    for (i = 0; i < maxid; i++) {
        if (gotroad[i] == 0) 
            fprintf(stderr, "Missing segment id #%d\n", i + 1);
    }

    fprintf(stderr, "Defining sinks over entire basin\n");
    fprintf(stderr, "If road segment intersects stream channel then sink is at stream channel;\n");
    fprintf(stderr, "otherwise sink is at minimum elevation\n");

    for (i = 0; i < maxid; i++) {
        // Must find sink for each road id.
        gotstream = 0;
        minelev = 200000.;

        for (row = 0; row < nrow; row++) {
            for (col = 0; col < ncol; col++) {
                if (nroad[row][col] > 0) {
                    for (ik = 0; ik < nroad[row][col]; ik++) {
                        if (road[row][col][ik] == i + 1) {
                            if (elev[row][col] < minelev) {
                                minelev = elev[row][col];
                                mecol = col;
                                merow = row;
                            }
                        }
                    }
                }
            }
        }

        if (gotstream > 0) {
            if (gotstream == 1) {
                sscol[i] = stcol[0];
                ssrow[i] = strow[0];
                igotsink += 1;
            } else {
                minstelev = 999999;
                for (j = 0; j < gotstream; j++) {
                    if (stelev[j] < minstelev) {
                        minstelev = stelev[j];
                        minstid = gotstream - 1;
                    }
                }

                sscol[i] = stcol[minstid];
                ssrow[i] = strow[minstid];
                igotsink += 1;
            }
            scts += 1;
        }

        if (gotstream == 0) {
            sscol[i] = mecol;
            ssrow[i] = merow;
            igotsink += 1;
            sctrf += 1;
        }

        sink[ssrow[i]][sscol[i]] = 1;
    }

    fprintf(stderr, "Got the sinks for %d out of %d road segments.\n", igotsink, maxid);
    fprintf(stderr, "Total number of sinks = %d\n", scts + sctrf);

    fprintf(stderr, "Writing new output file...\n ");

    if ((fi = fopen(inroad, "r")) == NULL) {
        printf("Error Opening Input File, %s.\n", inroad);
        exit(EXIT_FAILURE);
    }

    if ((fo = fopen(outroad, "w")) == NULL) {
        printf("Error Opening Output File, %s.\n", outroad);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < maxid; i++) 
        icountsinks[i] = 0;

    for (i = 0; i < nskipr; i++) {
        fgets(cjunk, MAXSIZE, fi);
        fprintf(fo, "%s", cjunk);
    }

    for (i = 0; i < nlrf; i++) {
        fscanf(fi, "%d %d %d %f %f %f %f", &col, &row, &id, &length, &height, &width, &aspect);

        if (col == sscol[id - 1] && row == ssrow[id - 1]) {
            icountsinks[id - 1] += 1;
            fprintf(fo, "%4d%4d%4d%12.4f%10.4f%10.4f%10.4f\tSINK\n", col, row, id, length, height, width, aspect);
        } else {
            fprintf(fo, "%4d%4d%4d%12.4f%10.4f%10.4f%10.4f\n", col, row, id, length, height, width, aspect);
        }
    }

    totalsinks = 0;
    for (i = 0; i < maxid; i++) {
        if (icountsinks[i] == 1) 
            totalsinks += 1;
        if (icountsinks[i] == 0) 
            fprintf(stderr, "No sink located for segment %d\n", i);
        if (icountsinks[i] > 1) 
            fprintf(stderr, "More than one sink for segment %d\n", i);
    }

    fclose(fi);
    fclose(fo);

    if ((fo = fopen("raster.dat", "w")) == NULL) {
        printf("Error Opening Output File, raster.dat.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(fo, "ncols %d\n", ncol);
    fprintf(fo, "nrows %d\n", nrow);
    fprintf(fo, "xllcorner %f\n", xll);
    fprintf(fo, "yllcorner %f\n", yll);
    fprintf(fo, "cellsize %f\n", cellsize);
    fprintf(fo, "NODATA_value 0\n");

    for (row = 0; row < nrow; row++) {
        for (col = 0; col < ncol; col++) {
            output[row][col] = 0;
            if (stream[row][col] == 1) 
                output[row][col] = 2;
            if (nroad[row][col] > 0) 
                output[row][col] = 3;
            if (sink[row][col] == 1 && stream[row][col] != 1)
                output[row][col] = 4;
            if (sink[row][col] == 1 && stream[row][col] == 1)
                output[row][col] = 5;

            fprintf(fo, "%d ", output[row][col]);
        }
        fprintf(fo, "\n");
    }

    fclose(fo);

    // Free allocated memory
    for (i = 0; i < nrow; i++) {
        free(elev[i]);
        free(stream[i]);
        free(nroad[i]);
        free(sink[i]);
        free(output[i]);
        for (j = 0; j < ncol; j++) {
            free(road[i][j]);
        }
        free(road[i]);
    }
    free(elev);
    free(stream);
    free(nroad);
    free(sink);
    free(output);
    free(road);

    return 0;
}

