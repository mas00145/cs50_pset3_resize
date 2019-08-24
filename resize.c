// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "bmp.h"

bool isInteger(char *a);

const int MIN_ALLOWED = 1;
const int MAX_ALLOWED = 100;


int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];
    int n = 0;

    // check the first argument, and make sure it is a valid integer
    if (isInteger(argv[1]))
    {
        n = (int)strtol(argv[1], NULL, 10);

        // now make sure n is between MIN_ALLOWED and MAX_ALLOWED values inclusive

        if (!(n >= MIN_ALLOWED && n <= MAX_ALLOWED))
        {
            fprintf(stderr, "'n' must be between %i and %i inclusive\n", MIN_ALLOWED, MAX_ALLOWED);
            fprintf(stderr, "Usage: resize n infile outfile\n");
            return 1;
        }

    }
    else
    {
        fprintf(stderr, "Usage: resize n infile outfile\n");
        return 1;
    }


    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // get padding for input file - before we change the values below
    int paddingIn = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int widthInput = bi.biWidth;
    int heightInput = abs(bi.biHeight);

    // ##########  make changes to height, width and new file size info ########### //
    bi.biWidth = bi.biWidth *= n;
    bi.biHeight = bi.biHeight *= n;
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4; // determine total padding required per scanline
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * bi.biWidth) + padding) * abs(bi.biHeight);
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    // ############################################################################ //


    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);


    // iterate over infile's scanlines
    for (int i = 0; i < heightInput; i++)
    {
        // vertical resizing
        for (int a = 0; a < n; a++)
        {
            // iterate over pixels in scanline
            for (int j = 0; j < widthInput; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);


                // horizontal resizing
                for (int h = 0; h < n; h++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }

            }

            // skip over padding, if any in the input file
            fseek(inptr, paddingIn, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int k = 0; k < padding; k++)
            {
                fputc(0x00, outptr);
            }

            // move back to start of current row
            fseek(inptr, -((widthInput * sizeof(RGBTRIPLE)) + paddingIn), SEEK_CUR);
        }

        // move pointer to end of scanline - so we can get the next row
        fseek(inptr, (widthInput * sizeof(RGBTRIPLE)) + paddingIn, SEEK_CUR);

    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}




// check if the argument passed is an valid integer
bool isInteger(char *a)
{
    char *endptr;
    errno = 0;

    // if any valies not an int then return false - also removes any fload values with decimal point
    int i = 0;
    while (a[i] != '\0')
    {
        if (!isdigit(a[i]))
        {
            return false;
        }
        i++;
    }

    long result; // strtol returns a long int, store in this variable

    // convert to decimal value
    result = strtol(a, &endptr, 10);

    if (endptr == a)
    {
        // nothing parsed from the string error
        return false;
    }

    if ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE)
    {
        // out of range error
        return false;
    }

    return true;
}
