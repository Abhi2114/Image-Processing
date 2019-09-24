/*
  main driver program for the project
  contains all the user interaction stuff using GLUT and
  procedures for reading and writing an image to and from the display
  to disk and vice versa
*/

#include <OpenImageIO/imageio.h>
#include <iostream>
#include "Image.h"
#include <vector>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING

// window dimensions
#define WIDTH 300
#define HEIGHT 200

static int ipicture = -1;

// keep track of the current window size at all times
int windowWidth = WIDTH;
int windowHeight = HEIGHT;

int num;  // number of command line args specified
char** imagenames;  // list of image names supplied in the command line args

string currentImageName = "";  // the name of the image on file currently being displayed

Image *picture = NULL;  // a reference stored to the Image object

/*
  read an image from the file whose name is specified in the argument.
  if no name is provided, ask the user for a file name.
  store all the read image information into the image object 'picture'
*/
void readimage(string name="") {

    string inputfilename;

    if (name.empty()) {
        // ask the user the name of file(current working director default) to be read
        cout << "enter input image filename: ";
        cin >> inputfilename;
    }
    else
        inputfilename = name;

    // string filepath = "/home/abhinit/Documents/codeblocks/test/images/" + inputfilename;
    currentImageName = inputfilename;  // set the current image name

    // read the image
    ImageInput* input = ImageInput::open(inputfilename);
    if (! input) {
        cerr << "Could not read image " << inputfilename << ", error = " << geterror() << endl;
        return;
    }

    const ImageSpec &spec = input->spec();
    // get the metadata for the image(dimensions and number of channels)
    int width = spec.width;
    int height = spec.height;
    int channels = spec.nchannels;

    // allocate space in memory to store the image data
    unsigned char pixmap[channels * width * height];

    if (!input->read_image(TypeDesc::UINT8, pixmap)) {
        cerr << "Could not read image " << inputfilename << ", error = " << geterror() << endl;
        ImageInput::destroy (input);
        return;
    }
    // close the file handle
    if (!input->close()) {
      cerr << "Could not close " << inputfilename << ", error = " << geterror() << endl;
      ImageInput::destroy (input);
      return;
    }

    ImageInput::destroy(input);

    // before reading a new image, destroy the old one if it exists
    if (picture) {
        picture->destroy();
        delete picture;
    }

    // copy the pixmap into the image
    picture = new Image(width, height, channels);
    picture->copyImage(pixmap);   // make a deep copy of the pixmap
}

/*
    Routine to write the current framebuffer to an image file
*/
void writeimage(){

    if (!picture)
        return;

    int w = picture->getWidth();
    int h = picture->getHeight();

    string outfilename;

    // get a filename for the image. The file suffix should indicate the image file
    // type. For example: output.png to create a png image file named output
    cout << "enter output image filename: ";
    cin >> outfilename;

    // create the oiio file handler for the image
    ImageOutput *outfile = ImageOutput::create(outfilename);
    if(!outfile){
        cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
        return;
    }

    // open a file for writing the image. The file header will indicate an image of
    // width w, height h, and 4 channels per pixel (RGBA). All channels will be of
    // type unsigned char
    ImageSpec spec(w, h, 4, TypeDesc::UINT8);
    if(!outfile->open(outfilename, spec)){
        cerr << "Could not open " << outfilename << ", error = " << geterror() << endl;
        ImageOutput::destroy (outfile);
        return;
    }

    // write the image to the file. All channel values in the pixmap are taken to be
    // unsigned chars
    if(!outfile->write_image(TypeDesc::UINT8, picture->getPixmap())){
        cerr << "Could not write image to " << outfilename << ", error = " << geterror() << endl;
        ImageOutput::destroy (outfile);
        return;
    }
    else
        cout << "Saved successfully." << endl;

    // free up space associated with the oiio file handler
    if (!outfile->close()) {
      cerr << "Could not close " << outfilename << ", error = " << geterror() << endl;
      ImageOutput::destroy (outfile);
      return;
    }

    ImageOutput::destroy (outfile);
}
/*
   Reshape Callback Routine: sets up the viewport and drawing coordinates
   This routine is called when the window is created and every time the window
   is resized, by the program or by the user
*/
void handleReshape(int w, int h) {

    // update the width and height info for the display window
    windowWidth = w;
    windowHeight = h;

    // set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // define the drawing coordinate system on the viewport
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
}

/*
  this is the main display routine
  using pixelZoom to always fit the image into the display window
  in the end, we flip the image so that it doesn't display upside down
*/
void drawImage() {

    if (picture) {

        glClear(GL_COLOR_BUFFER_BIT);  // clear window to background color

        int width = picture->getWidth();
        int height = picture->getHeight();

        // ensure that the image is centered
        glRasterPos2i(0, 0);

        // zoom the image according to the window size
        double xr = windowWidth / (double)width;
        double yr = windowHeight / (double)height;
        glPixelZoom(xr, yr);

        // flip the image so that we can see it straight
        Image* flipped = picture->flip();
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, flipped->getPixmap());
        flipped->destroy();
        delete flipped;

        glFlush();
    }
}

/*
   This routine is called every time a key is pressed on the keyboard
*/
void handleKey(unsigned char key, int x, int y) {

    switch(key) {
        case 'r':
        case 'R':
            readimage();
            if (picture)
                glutPostRedisplay();
            break;
        case 'w':
        case 'W':
            writeimage();
            break;
        case 'p':
        case 'P':
            // convert to bitmap
            if (picture) {
                picture->toBitmap();
                glutPostRedisplay();
            }
            break;
        case 'd':
        case 'D':
            // reduce the palette
            if (picture) {
              // std::vector<pixel> palette(2, pixel());
              /*
              palette.push_back(pixel(64, 49, 174, 255));
              palette.push_back(pixel(225, 13, 193, 255));
              palette.push_back(pixel(187, 168, 255, 255));
              palette.push_back(pixel(255, 255, 255, 255));
              palette.push_back(pixel(254, 147, 155, 255));
              palette.push_back(pixel(232, 2, 2, 255));
              palette.push_back(pixel(123, 35, 60, 255));
              palette.push_back(pixel(0, 0, 0, 255));
              palette.push_back(pixel(25, 86, 71, 255));
              palette.push_back(pixel(21, 236, 115, 255));
              palette.push_back(pixel(49, 193, 194, 255));
              palette.push_back(pixel(4, 126, 193, 255));
              palette.push_back(pixel(109, 78, 34, 255));
              palette.push_back(pixel(200, 143, 76, 255));
              palette.push_back(pixel(237, 225, 3, 255));
              */
              // picture->getReducedPalette(palette);

              std::vector<pixel> palette;
              palette.push_back(pixel(255, 255, 255, 255));
              palette.push_back(pixel(0, 0, 0, 255));

              picture->reducePalette(palette);
              glutPostRedisplay();
            }
            break;

        case 'c':
        case 'C':
            if (picture) {
              std::vector<pixel> colors(16, pixel());  // 16 colors
              picture->getReducedPalette(colors);

              for (int i = 0; i < 16; ++i)
                std::cout << "(" << (int)colors[i].r << ", " << (int)colors[i].g << ", " << (int)colors[i].b << ")\n";
            }
            break;

        case 'f':
        case 'F':
            if (picture) {
              std::vector<pixel> palette(16, pixel());

              picture->getReducedPalette(palette);
              // std::vector<pixel> palette;
              // palette.push_back(pixel(255, 255, 255, 255));
              // palette.push_back(pixel(0, 0, 0, 255));
              picture->floydSteinberg(palette);
              glutPostRedisplay();
            }
            break;

        case '1':
            // red
            if (picture) {
                picture->greyscaleRed();
                cout << "Hit 'o' to get the original image back before doing any other operation\n";
                glutPostRedisplay();
            }
            break;
        case '2':
            // green
            if (picture) {
                picture->greyscaleGreen();
                cout << "Hit 'o' to get the original image back before doing any other operation\n";
                glutPostRedisplay();
            }
            break;
        case '3':
            // blue
            if (picture) {
                picture->greyscaleBlue();
                cout << "Hit 'o' to get the original image back before doing any other operation\n";
                glutPostRedisplay();
            }
            break;
        case 'o':
            // restore the image again
            if (ipicture >= 0)
                currentImageName = imagenames[ipicture];

            //  read the current image
            readimage(currentImageName);

            if (picture)
                glutPostRedisplay();
            break;
        case 'i':
            if (picture) {
                picture->inverse();
                glutPostRedisplay();
            }
            break;
        case 'q':		// q - quit
        case 'Q':
        case 27:		// esc - quit
            exit(0);
        default:		// not a valid key -- just ignore it
            return;
    }
}

void handleArrowKeys(int key, int x, int y) {

    // if no command line args are specified, dont do anything
    if (num == 0)
        return;

    switch (key) {
        case 100:
            // left key pressed
            // if on the first picture, go back to the last one
            if (ipicture - 1 < 0)
                ipicture = num - 1;
            else
                ipicture -= 1;
            break;
        case 102:
            // right key pressed
            ipicture = (ipicture + 1) % num;  // cycle through all the images
            break;
    }

    // read the image at the given index
    readimage(imagenames[ipicture]);

    if (picture)
        glutPostRedisplay();
}

/*
   Main program to draw the square, change colors, and wait for quit
*/
int main(int argc, char* argv[]) {

    imagenames = argv + 1;
    num = argc - 1;

    cout << "Args: " << num << "\n";

    // start up the glut utilities
    glutInit(&argc, argv);

    // create the graphics window, giving width, height, and title text
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Assignment 1");

    // set up the callback routines to be called when glutMainLoop() detects
    // an event
    glutDisplayFunc(drawImage);	  // display callback
    glutKeyboardFunc(handleKey);	  // keyboard callback
    glutReshapeFunc(handleReshape); // window resize callback
    glutSpecialFunc(handleArrowKeys);

    // Routine that loops forever looking for events. It calls the registered
    // callback routine to handle each event that is detected
    glutMainLoop();
    return 0;
}
