#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define BMP_HEADER_SIZE 54

typedef struct {
	unsigned char b, g, r;
} pixel;

typedef struct {
	int width, height;
	pixel** pixels;
} image;


//initialize black image
static image *init_image(int w, int h) {
	image* new_image = (image*)malloc(sizeof(image));
	if (!new_image) {
		printf("Error allocating memory for image\n");
		return 0;
	}
	new_image->pixels = (pixel**)malloc(h * sizeof(pixel *));
	if (!new_image->pixels) {
		printf("Error allocating memory for image\n");
		return 0;
	}
	new_image->height = h;
	new_image->width = w;

	for (int i = 0; i < h; i++) {
		new_image->pixels[i] = (pixel*)malloc(w * sizeof(pixel));
		if (!new_image->pixels[i]) {
			printf("Error allocating memory for image\n");
			return 0;
		}
	}
	return new_image;
}

image* load_image_from_file(char* filename) {
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Error reading file %s, be sure you have read permissions and that the filename was typed correctly\n", filename);
		return 0;
	}

	unsigned char info[BMP_HEADER_SIZE];
	fread(info, sizeof(unsigned char), BMP_HEADER_SIZE, file);
	int width = *(int*)&info[18], height = *(int*)&info[22];
	printf("%d %d\n", width, height);
	image* new_image = init_image(width, height);
	printf("Loaded garbage image with address %p\n", new_image);

	int padding = (width%4 == 0) ? width%4 : 4 - width%4;
	char temp[4];

	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			fread(&(new_image->pixels[i][j]), 3, 1, file);
		}
		if(padding) {
			fread(&temp, padding, 1, file);
		}
	}

	fclose(file);
	return new_image;
}

static char get_pixel_char(pixel p) {
	char* chars = "$@B%%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`\'. ";
	int length = strlen(chars);

	double gray_scale = 0.21* p.r + 0.72 * p.g + 0.07 * p.b;

	return chars[(int)ceil((length - 1) * gray_scale/255.0f)];
}

static double absolute(double value) {
	if (value < 0)
		return -value;


	return value;
}

static int get_pixel_color(pixel p) {
	double luminosity = 0.5 * (fmax(fmax(p.r / 255.0f, p.g / 255.0f), p.b / 255.0f) + fmin(fmin(p.r / 255.0f, p.g / 255.0f), p.b / 255.0f));
	double saturation = (luminosity == 1) ? 0 : (fmax(fmax(p.r / 255.0f, p.g / 255.0f), p.b / 255.0f) - fmin(fmin(p.r / 255.0f, p.g / 255.0f), p.b / 255.0f)) / (1 - absolute(2 * luminosity - 1));
	if (saturation < 0.2 || luminosity > 0.9 || luminosity < 0.1) {
		return 0;//white
	}

	double hue;

	if (p.r >= p.g && p.g >= p.b) {
		hue = 60 * (p.g / 255.0f - p.b / 255.0f) / ((p.r / 255.0f - p.b / 255.0f));
	}
	else if (p.g > p.r && p.r >= p.b) {
		hue = 60 * (2 - (p.r / 255.0f - p.b / 255.0f) / (p.g / 255.0f - p.b / 255.0f));
	}
	else if (p.g >= p.b && p.b > p.r) {
		hue = 60 * (2 + (p.b / 255.0f - p.r / 255.0f) / (p.g / 255.0f - p.r / 255.0f));
	}
	else if(p.b > p.g && p.g > p.r) {
		hue = 60 * (4 - (p.g / 255.0f - p.r / 255.0f)/(p.b / 255.0f - p.r / 255.0f));
	}
	else if(p.b > p.r && p.r >= p.g) {
		hue = 60 * (4 + (p.r / 255.0f - p.g / 255.0f)/(p.b / 255.0f - p.g / 255.0f));
	}
	else {
		hue = 60 * (6 * (p.b / 255.0f - p.g / 255.0f)/(p.r - p.g / 255.0f));
	}
	//hue = floor(hue);
	if(hue >= 330 || hue < 16) {
		return 1;//red
	}
	else if(hue >=16 && hue < 90) {
		return 2;//yellow
	}
	else if(hue >=90 && hue < 160) {
		return 3;//green
	}
	else if(hue >=160 && hue < 210) {
		return 4;// cyan
	}
	else if(hue >=210 && hue < 270) {
		return 5;//blue
	} 
	else {
		return 6; //magenta
	}
}

void print_image(image *img) {
	printf("Printing image with width %d and height %d\n", img->width, img->height);
	for(int y = img->height-1; y >=0 ; y--) {
		for(int x = 0; x < img->width ; x++) {
			char c = get_pixel_char(img->pixels[y][x]);
			int col = get_pixel_color(img->pixels[y][x]);
			switch(col) {
				case 0:
					printf("\033[97m%c\033[0m", c);
					break;
				case 1:
					printf("\033[31m%c\033[0m", c);
					break;
				case 2:
					printf("\033[33m%c\033[0m", c);
					break;
				case 3:
					printf("\033[32m%c\033[0m", c);
					break;
				case 4:
					printf("\033[36m%c\033[0m", c);
					break;
				case 5:
					printf("\033[34m%c\033[0m", c);
					break;
				case 6:
					printf("\033[95m%c\033[0m", c);
					break;
			}
			
		}
		printf("\n");
	}

}

void destroy_image(image* img) {
	for (int i = 0; i < img->height; i++) {
		free(img->pixels[i]);
	}
	free(img->pixels);
	free(img);
}
