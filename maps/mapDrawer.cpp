#include <stdio.h>
#include <png++/png.hpp>

using namespace png;

int main(int argc, char* argv[]){

	FILE* file1, *file2, *file3;
	int height, width, i, j, ents = 0, chests = 0;

	image<rgb_pixel> img(argv[1]);
	file1 = fopen(argv[2], "w");
	file2 = fopen(argv[3], "w");
	file3 = fopen(argv[4], "w");

	height = img.get_height();
	width = img.get_width();

	fprintf(file1, "%d %d\n", height, width);
	fprintf(file2, "      \n");
	fprintf(file3, "      \n");

	for(i = 0; i < height; i++){
		for(j=0; j < width; j++){
			int color = img.get_pixel(j, i).blue + img.get_pixel(j, i).green*256 + img.get_pixel(j, i).red*256*256;
			switch(color){
				case 0xFFFF00: fprintf(file1, "-1 "); break;
				case 0x000000: fprintf(file1, "1 "); break;
				case 0xFF0000: fprintf(file1, "0 "); fprintf(file2, "%d %d 3\n", j, i); ents++; break;
				case 0x00FF00: fprintf(file1, "0 "); fprintf(file2, "%d %d 2\n", j, i); ents++; break;
				case 0x0000FF: fprintf(file1, "0 "); fprintf(file3, "%d %d\n", j, i); chests++; break;
				case 0x00FFFF: fprintf(file1, "0 "); fprintf(file2, "%d %d 4\n", j, i); ents++; break;
				case 0xFF00FF: fprintf(file1, "0 "); fprintf(file2, "%d %d 7\n", j, i); ents++; break;
				case 0xFFFFFF: fprintf(file1, "0 "); break;
			}
		}
		fprintf(file1, "\n");
	}

	fseek(file2, 0, SEEK_SET);
	fprintf(file2, "%d", ents);
	fseek(file3, 0, SEEK_SET);
	fprintf(file3, "%d", chests);

	fclose(file1);
	fclose(file2);
	fclose(file3);

	return 0;
}