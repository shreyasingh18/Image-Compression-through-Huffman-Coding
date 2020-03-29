// C Code for
// Image Compression
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// function to calculate word length
int codelen(char* code)
{
	int l = 0;
	while (*(code + l) != '\0')
		l++;
	return l;
}

// function to concatenate the words
void strconcat(char* str, char* parentcode, char add)
{
	int i = 0;
	while (*(parentcode + i) != '\0')
	{
		*(str + i) = *(parentcode + i);
		i++;
	}
	if (add != '2')
	{
		str[i] = add;
		str[i + 1] = '\0';
	}
	else
		str[i] = '\0';
}

// function to find fibonacci number
int fib(int n)
{
	if (n <= 1)
		return n;
	return fib(n - 1) + fib(n - 2);
}

// Driver code
int main()
{
	int i, j;
	char filename[] = "Input.bmp";
	int data = 0, offset, bpp = 0, width, height;
	long bmpsize = 0, bmpdataoff = 0;
	int** image;
	int temp = 0;

	// Reading the BMP File
	FILE* image_file;

	image_file = fopen(filename, "rb");
	if (image_file == NULL)
	{
		printf("Error Opening File!!");
		exit(1);
	}
	else
	{
        printf("Image is processing\n");
		// Set file position of the
		// stream to the beginning
		// Contains file signature
		// or ID "BM"
		offset = 0;

		// Set offset to 2, which
		// contains size of BMP File
		offset = 2;

		fseek(image_file, offset, SEEK_SET);

		// Getting size of BMP File
		fread(&bmpsize, 4, 1, image_file);

		// Getting offset where the
		// pixel array starts
		// Since the information is
		// at offset 10 from the start,
		// as given in BMP Header
		offset = 10;

		fseek(image_file, offset, SEEK_SET);

		// Bitmap data offset
		fread(&bmpdataoff, 4, 1, image_file);

		// Getting height and width of the image
		// Width is stored at offset 18 and
		// height at offset 22, each of 4 bytes
		fseek(image_file, 18, SEEK_SET);

		fread(&width, 4, 1, image_file);

		fread(&height, 4, 1, image_file);

		// Number of bits per pixel
		fseek(image_file, 2, SEEK_CUR);

		fread(&bpp, 2, 1, image_file);

		// Setting offset to start of pixel data
		fseek(image_file, bmpdataoff, SEEK_SET);

		// Creating Image array
		image = (int**)malloc(height * sizeof(int*));

		for (i = 0; i < height; i++)
		{
			image[i] = (int*)malloc(width * sizeof(int));
		}

		// int image[height][width]
		// can also be done
		// Number of bytes in
		// the Image pixel array
		int numbytes = (bmpsize - bmpdataoff) / 3;

		// Reading the BMP File
		// into Image Array
		for (i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++)
			{
				fread(&temp, 3, 1, image_file);

				// the Image is a
				// 24-bit BMP Image
				temp = temp & 0x0000FF;
				image[i][j] = temp;
			}
		}
	}

	// Finding the probability
	// of occurence
	int hist[256];
	for (i = 0; i < 256; i++)
		hist[i] = 0;
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
			hist[image[i][j]] += 1;

	// Finding number of
	// non-zero occurences
	int nodes = 0;
	for (i = 0; i < 256; i++)
		if (hist[i] != 0)
			nodes += 1;

	// Calculating minimum probability
	float p = 1.0, ptemp;
	for (i = 0; i < 256; i++)
	{
		ptemp = (hist[i] / (float)(height * width));
		if (ptemp > 0 && ptemp <= p)
			p = ptemp;
	}

	// Calculating max length
	// of code word
	i = 0;
	while ((1 / p) > fib(i))
		i++;
	int maxcodelen = i - 3;

	// Defining Structures pixfreq
	struct pixfreq
	{
		int pix, larrloc, rarrloc;
		float freq;
		struct pixfreq *left, *right;
		char code[maxcodelen];
	};

	// Defining Structures
	// huffcode
	struct huffcode
	{
		int pix, arrloc;
		float freq;
	};

	// Declaring structs
	struct pixfreq* pix_freq;
	struct huffcode* huffcodes;
	int totalnodes = 2 * nodes - 1;
	pix_freq = (struct pixfreq*)malloc(sizeof(struct pixfreq) * totalnodes);
	huffcodes = (struct huffcode*)malloc(sizeof(struct huffcode) * nodes);

	// Initializing
	j = 0;
	int totpix = height * width;
	float tempprob;
	for (i = 0; i < 256; i++)
	{
		if (hist[i] != 0)
		{

			// pixel intensity value
			huffcodes[j].pix = i;
			pix_freq[j].pix = i;

			// location of the node
			// in the pix_freq array
			huffcodes[j].arrloc = j;

			// probability of occurrence
			tempprob = (float)hist[i] / (float)totpix;
			pix_freq[j].freq = tempprob;
			huffcodes[j].freq = tempprob;

			// Declaring the child of leaf
			// node as NULL pointer
			pix_freq[j].left = NULL;
			pix_freq[j].right = NULL;

			// initializing the code
			// word as end of line
			pix_freq[j].code[0] = '\0';
			j++;
		}
	}

	// Sorting the histogram
	struct huffcode temphuff;

	// Sorting w.r.t probability
	// of occurence
	for (i = 0; i < nodes; i++)
	{
		for (j = i + 1; j < nodes; j++)
		{
			if (huffcodes[i].freq < huffcodes[j].freq)
			{
				temphuff = huffcodes[i];
				huffcodes[i] = huffcodes[j];
				huffcodes[j] = temphuff;
			}
		}
	}

	// Building Huffman Tree
	float sumprob;
	int sumpix;
	int n = 0, k = 0;
	int nextnode = nodes;

	// Since total number of
	// nodes in Huffman Tree
	// is 2*nodes-1
	while (n < nodes - 1)
	{

		// Adding the lowest two probabilities
		sumprob = huffcodes[nodes - n - 1].freq + huffcodes[nodes - n - 2].freq;
		sumpix = huffcodes[nodes - n - 1].pix + huffcodes[nodes - n - 2].pix;

		// Appending to the pix_freq Array
		pix_freq[nextnode].pix = sumpix;
		pix_freq[nextnode].freq = sumprob;
		pix_freq[nextnode].left = &pix_freq[huffcodes[nodes - n - 2].arrloc];
		pix_freq[nextnode].right = &pix_freq[huffcodes[nodes - n - 1].arrloc];
		pix_freq[nextnode].code[0] = '\0';
		i = 0;

		// Sorting and Updating the
		// huffcodes array simultaneously
		// New position of the combined node
		while (sumprob <= huffcodes[i].freq)
			i++;

		// Inserting the new node
		// in the huffcodes array
		for (k = nodes; k >= 0; k--)
		{
			if (k == i)
			{
				huffcodes[k].pix = sumpix;
				huffcodes[k].freq = sumprob;
				huffcodes[k].arrloc = nextnode;
			}
			else if (k > i)

				// Shifting the nodes below
				// the new node by 1
				// For inserting the new node
				// at the updated position k
				huffcodes[k] = huffcodes[k - 1];

		}
		n += 1;
		nextnode += 1;
	}

	// Assigning Code through
	// backtracking
	char left = '0';
	char right = '1';
	int index;
	for (i = totalnodes - 1; i >= nodes; i--)
	{
		if (pix_freq[i].left != NULL)
			strconcat(pix_freq[i].left->code, pix_freq[i].code, left);
		if (pix_freq[i].right != NULL)
			strconcat(pix_freq[i].right->code, pix_freq[i].code, right);
	}

	// Printing Huffman Codes
	printf("Huffmann Codes::\n\n");
	printf("pixel values -> Code\n\n");
	for (i = 0; i < nodes; i++) {
		if (snprintf(NULL, 0, "%d", pix_freq[i].pix) == 2)
			printf("	 %d	 -> %s\n", pix_freq[i].pix, pix_freq[i].code);
		else
			printf(" %d	 -> %s\n", pix_freq[i].pix, pix_freq[i].code);
	}

	// Encode the Image
	printf("\n\n The Encoded 8*8 image with pixel intensity is :\n\n");
	FILE* textfile;
	textfile=fopen("encoded_image.txt","wb");
	int pix_val;
	int l;
	//char encode[]="";
	for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
		{
			pix_val = image[i][j];
			for (l = 0; l < nodes; l++)
				if (pix_val == pix_freq[l].pix)
                {
                    printf("%s ", pix_freq[l].code);
					fprintf(textfile,"%s ", pix_freq[l].code);
					//strcat(encode,pix_freq[l].code);
                }

		}
		printf("\n");
		fprintf(textfile,"\n");
    }
    fclose(textfile);
    //printf("\n\n%s",encode);



}
