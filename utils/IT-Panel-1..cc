// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Example of a clock. This is very similar to the text-example,
// except that it shows the time :)
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <Magick++.h>

#include <fstream>

using namespace Magick;
using namespace rgb_matrix;
using namespace std;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Reads text from stdin and displays it. "
          "Empty string: clear screen\n");
  fprintf(stderr, "Options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  fprintf(stderr,
          "\t-d <time-format>  : Default '%%H:%%M:%%S'. See strftime()\n"
          "\t-f <font-file>    : Use given font.\n"
          "\t-b <brightness>   : Sets brightness percent. Default: 100.\n"
          "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
          "\t-S <spacing>      : Spacing pixels between letters (Default: 0)\n"
          "\t-C <r,g,b>        : Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Background-Color. Default 0,0,0\n"
          "\t-O <r,g,b>        : Outline-Color, e.g. to increase contrast.\n"
          );

  return 1;
}

static bool parseColor(rgb_matrix::Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  rgb_matrix::Color color(255, 255, 0);
  rgb_matrix::Color bg_color(0, 0, 0);
  rgb_matrix::Color outline_color(0,0,0);
  bool with_outline = false;

  const char *bdf_font_file = NULL;
  int brightness = 100;
  int letter_spacing = 0;

  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:C:B:O:b:S:d:")) != -1) {
    switch (opt) {
    case 'd': /*time_format = strdup(optarg)*/; break;
    case 'b': brightness = atoi(optarg); break;
    case 'f': bdf_font_file = strdup(optarg); break;
    case 'S': letter_spacing = atoi(optarg); break;
    case 'C':
      if (!parseColor(&color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'B':
      if (!parseColor(&bg_color, optarg)) {
        fprintf(stderr, "Invalid background color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'O':
      if (!parseColor(&outline_color, optarg)) {
        fprintf(stderr, "Invalid outline color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      with_outline = true;
      break;
    default:
      return usage(argv[0]);
    }
  }

  if (bdf_font_file == NULL) {
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return usage(argv[0]);
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }
  rgb_matrix::Font *outline_font = NULL;
  if (with_outline) {
      outline_font = font.CreateOutlineFont();
  }

  if (brightness < 1 || brightness > 100) {
    fprintf(stderr, "Brightness is outside usable range.\n");
    return 1;
  }

  RGBMatrix *matrix = rgb_matrix::CreateMatrixFromOptions(matrix_options,
                                                          runtime_opt);
  if (matrix == NULL)
    return 1;

  matrix->SetBrightness(brightness);

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();

  char text_buffer[256];
  
  int x_orig = 48;
  int y_orig = 0;  

    InitializeMagick(*argv);

    //Fleche_Gauche
//    Image img_FG("./images/Fleche_Gauche_32x32.png");
	Image img_FG("./images/Lidl-32x32.png");
    int nx_FG = img_FG.columns();
    int ny_FG = img_FG.rows();
	fprintf(stdout,"nx_FG= %d ny_FG= %d\n", nx_FG,ny_FG);
	//Croix
    Image img_CR("./images/Croix_32x32.png");
    int nx_CR = img_CR.columns();
    int ny_CR = img_CR.rows();
	fprintf(stdout,"nx_CR= %d ny_CR= %d\n", nx_CR,ny_CR);
	//PMR
    Image img_PMR("./images/PMR_32x32.png");
    int nx_PMR = img_PMR.columns();
    int ny_PMR = img_PMR.rows();
	fprintf(stdout,"nx_PMR= %d ny_PMR= %d\n", nx_PMR,ny_PMR);
  
  if (isatty(STDIN_FILENO)) {
    // Only give a message if we are interactive. If connected via pipe, be quiet
    printf("Enter lines. Full screen or empty line clears screen.\n"
           "Supports UTF-8. CTRL-D for exit.\n");
  }
 char line[1024];
      rgb_matrix::DrawText(offscreen, font, 0, y_orig + font.baseline(),
                           color, NULL, " Pret !",
                           letter_spacing);

      // Atomic swap with double buffer
    offscreen = matrix->SwapOnVSync(offscreen);

 
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  while (fgets(line, sizeof(line), stdin) && !interrupt_received) {

      const size_t last = strlen(line);
      if (last > 0) line[last - 1] = '\0';  // remove newline.
      bool line_empty = strlen(line) == 0;
	  strcpy(text_buffer,line);
	  
      offscreen->Fill(bg_color.r, bg_color.g, bg_color.b);

    // logos
 
 		for (int x = 0; x < nx_PMR; x++) {
		  for (int y = 0; y < ny_PMR; y++) {
			const Magick::Color &c = img_PMR.pixelColor(x, y);  
			if (c.alphaQuantum() < 256) {
				offscreen->SetPixel(x, y, ScaleQuantumToChar(c.redQuantum()),
								   ScaleQuantumToChar(c.greenQuantum()),
								   ScaleQuantumToChar(c.blueQuantum()));
			}
		  }
		}
 
    if (strcmp(text_buffer,"0") && strcmp(text_buffer,"-1")) {
	//Flèche	
		for (int x = 0; x < nx_FG; x++) {
		  for (int y = 0; y < ny_FG; y++) {
			const Magick::Color &c = img_FG.pixelColor(x, y);  
			if (c.alphaQuantum() < 256) {
				offscreen->SetPixel(x+32, y, ScaleQuantumToChar(c.redQuantum()),
								   ScaleQuantumToChar(c.greenQuantum()),
								   ScaleQuantumToChar(c.blueQuantum()));
			}
		  }
		}
		x_orig = 96 - 16*strlen(text_buffer)/2;
	} else if (!strcmp(text_buffer,"-1")) {
    //Rien
		offscreen->Fill(bg_color.r, bg_color.g, bg_color.b);
		sprintf(text_buffer, "Erreur!"); 
		x_orig = 128 - 16*strlen(text_buffer);		
		
	} else {
    //Croix
		offscreen->Fill(bg_color.r, bg_color.g, bg_color.b);
		sprintf(text_buffer, "Complet"); 
		for (int x = 0; x < nx_CR; x++) {
		  for (int y = 0; y < ny_CR; y++) {
			const Magick::Color &c = img_CR.pixelColor(x, y);  
			if (c.alphaQuantum() < 256) {
				offscreen->SetPixel(x, y, ScaleQuantumToChar(c.redQuantum()),
								   ScaleQuantumToChar(c.greenQuantum()),
								   ScaleQuantumToChar(c.blueQuantum()));
			}
		  }
		}
		x_orig = 128 - 16*strlen(text_buffer);;
	}
      if (outline_font) {
          rgb_matrix::DrawText(offscreen, *outline_font, x_orig - 1, y_orig + font.baseline(),
                               outline_color, NULL, text_buffer,
                               letter_spacing - 2);
      }

      //Compteur
      rgb_matrix::DrawText(offscreen, font, x_orig, y_orig + font.baseline(),
                           color, NULL, text_buffer,
                           letter_spacing);

      // Atomic swap with double buffer
    offscreen = matrix->SwapOnVSync(offscreen);

  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  write(STDOUT_FILENO, "\n", 1);  // Create a fresh new line after ^C on screen
  return 0;
}
