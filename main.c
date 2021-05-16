#include "image.h"
#include <stdio.h>

int main()
{
    image *test = load_image_from_file("./images/a.bmp");
    print_image(test);
    destroy_image(test);
    return 0;
}