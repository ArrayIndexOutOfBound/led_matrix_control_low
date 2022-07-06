// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how write text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h> // Pour cut des strings ?
#include <time.h>

using namespace rgb_matrix;

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Reads text from stdin and displays it. "
          "Empty string: clear screen\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "\t-f <font-file>    : Use given font.\n"
          "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
          "\t-S <spacing>      : Spacing pixels between letters (Default: 0)\n"
          "\t-C <r,g,b>        : Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Font Background-Color. Default 0,0,0\n"
          "\t-O <r,g,b>        : Outline-Color, e.g. to increase contrast.\n"
          "\t-F <r,g,b>        : Panel flooding-background color. Default 0,0,0\n"
          "\n"
          );
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

static bool FullSaturation(const Color &c) {
  return (c.r == 0 || c.r == 255)
    && (c.g == 0 || c.g == 255)
    && (c.b == 0 || c.b == 255);
}

static bool consumeStdin(char texte){
  return 0;
}

// MON BORDEL INTERPRETEUR
char** str_split(char* texte, const char delim);
int configPosX(char* texte);
int configPosY(char* texte);
Color configCouleur(char* texte);
char* configTexte(char* texte);
char* str_slice(char* who,int from,int to);
// MON BORDEL INTERPRETEUR

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  Color color(255, 255, 0);
  Color bg_color(0, 0, 0);
  Color flood_color(0, 0, 0);
  Color outline_color(0,0,0);
  bool with_outline = false;

  const char *bdf_font_file = NULL;
  int x_orig = 0;
  int y_orig = 0;
  int letter_spacing = 0;

  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:C:B:O:S:F:")) != -1) {
    switch (opt) {
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 'f': bdf_font_file = strdup(optarg); break;
    case 'S': letter_spacing = atoi(optarg); break;
    case 'C':
      if (!parseColor(&color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      else {
        fprintf(stderr, "Color is set \n"); // VALIDER LA COULEUR
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
    case 'F':
      if (!parseColor(&flood_color, optarg)) {
        fprintf(stderr, "Invalid background color spec: %s\n", optarg);
        return usage(argv[0]);
      }
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

  /*
   * If we want an outline around the font, we create a new font with
   * the original font as a template that is just an outline font.
   */
  rgb_matrix::Font *outline_font = NULL;
  if (with_outline) {
    outline_font = font.CreateOutlineFont();
  }

  RGBMatrix *canvas = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (canvas == NULL)
    return 1;
  // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = canvas->CreateFrameCanvas();

  const bool all_extreme_colors = (matrix_options.brightness == 100)
    && FullSaturation(color)
    && FullSaturation(bg_color)
    && FullSaturation(outline_color);
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

  int x = x_orig;
  int y = y_orig;

  if (isatty(STDIN_FILENO)) {
    // Only give a message if we are interactive. If connected via pipe, be quiet
    fprintf(stderr, "Enter lines. Full screen or empty line clears screen.\n"
           "Supports UTF-8. CTRL-D for exit.\n");
  }

  canvas->Fill(flood_color.r, flood_color.g, flood_color.b);
  char line[1024];
  char* texte=" ";
  time_t temps;
  char * temps_string = ctime(&temps);
  while (fgets(line, sizeof(line), stdin)) {
    const size_t last = strlen(line);
    if (last > 0) line[last - 1] = '\0';  // remove newline.
    temps_string = ctime (&temps);
    if (line!=NULL && line[0]!='\0' && strlen(line)>=2) fprintf(stderr, "Ligne recu dans le .cc : %s\n", line); // PRINT CA QUE LE PROG A RECU
    // fprintf(stderr, "Temps actuel : %s\n", temps_string);
    bool line_empty = strlen(line) == 0;
    if ((y + font.height() > canvas->height()) || line_empty) { // Clean le board ?
      offscreen_canvas->Fill(flood_color.r, flood_color.g, flood_color.b);
      // printf("Je vais print des trucs \n"); // LE DEBUG
      // printf("y: %d fh: %d ch: %d\n", y, font.height(), canvas->height());
      y = y_orig;
    }
    if (line_empty)
    {
      // fprintf(stderr, "La ligne recu est vide, ne rien faire (devrait supp) \n"); // LE DEBUG
      continue;
    }
    if (outline_font) {
      // The outline font, we need to write with a negative (-2) text-spacing,
      // as we want to have the same letter pitch as the regular text that
      // we then write on top.
      // fprintf(stderr, "if (outline_font) --> clean");
      rgb_matrix::DrawText(offscreen_canvas, *outline_font,
                           x - 1, y + font.baseline(),
                           outline_color, &bg_color, line, letter_spacing - 2);
    }
    // The regular text. Unless we already have filled the background with
    // the outline font, we also fill the background here.

    // EN GROS, CA SERAIT ICI QUE JE FAIS MON MINI ALGO POUR GERER LE CHANGEMENT DE COULEUR, LES ESPACEMENT, ETC...
    // fprintf(stderr, "Je vais commencer l'interpreteur ici\n");
    char** listeArgument;
    listeArgument = str_split(line, ';');
    if (listeArgument)
    {
      int i;
      for (i=0; *(listeArgument+i); i++)
      {
          // printf("Argument=%s\n", *(listeArgument+i));
          char lettre = (*(listeArgument+i)[0]);
          if (lettre=='C') color = configCouleur(*(listeArgument+i));
          if (lettre=='X') x = configPosX(*(listeArgument+i));
          if (lettre=='Y') y = configPosY(*(listeArgument+i));
          if (lettre=='T') texte = configTexte(*(listeArgument+i));
          if (lettre=='B') bg_color = configCouleur(*(listeArgument+i)); // Pas utilisé
          if (lettre=='O') outline_color = configCouleur(*(listeArgument+i)); // Pas utilisé
          if (lettre=='F') flood_color = configCouleur(*(listeArgument+i)); // Pas utilisé
      }
    }

    // fprintf(stderr, "Je vais écrire sur l'afficheur maintenant \n");
    // ON PEUT ENFIN PRINT LE TEXTE AVEC LES ARGUMENT TRAITES PLUS HAUT
    /* rgb_matrix::DrawText(offscreen_canvas, font, x, y + font.baseline(),
                         color, outline_font ? NULL : &bg_color, line,
                         letter_spacing); */

    rgb_matrix::DrawText(offscreen_canvas, font, x, y + font.baseline(),
                         color, outline_font ? NULL : &bg_color, texte,
                         letter_spacing);


   // Atomic swap with double buffer
    offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);

    y += font.height();
    memset(line, 0, 1024); // Clear la ligne
    fprintf(stderr,"Fini, en attente\n");
  }

  // Finished. Shut down the RGB matrix.
  delete canvas;
  fprintf(stderr,"Fin du programme, return 0;");
  return 0;
}

/////////////////////////////////////// MON BORDEL INTERPRETEUR
char** str_split(char* texte, const char delim)
{
  char** result=0;
  size_t count=0;
  char* temp = texte;
  char* last_delim =0;
  char delimiteur[2];
  delimiteur[0]=delim;
  delimiteur[1]=0;
  while (*temp) // Combien d'élement
  {
    if(delim == *temp)
    {
      count++;
      last_delim=temp;
    }
    temp++;
  }
  // Ajouter un espace "for trailing token"
  count += last_delim < (texte + strlen(texte)-1);

  // Add space for terminating null string so caller knows where the list of returned strings ends.
  count++;

  result = (char**)malloc(sizeof(char*) * count); // A FAIRE EXTREMEMENT ATTENTION ///////////////////////////////

  // Aucune idée de ce que fait cette formule
  if (result)
  {
    size_t idx = 0;
    char* token = strtok(texte, delimiteur);
    while (token)
    {
      assert(idx < count);
      *(result + idx++) = strdup(token); // A FAIRE EXTREMEMENT ATTENTION //////////////////////
      token = strtok (0,delimiteur);
    }
    assert( idx == count - 1 );
    *(result + idx) = 0;
  }
  return result;
}

int configPosX(char* texte)
{
  int resultat = atoi(str_slice(texte, 2, strlen(texte)));
  return resultat;
}

int configPosY(char* texte)
{
  int resultat = atoi(str_slice(texte, 2, strlen(texte)));
  return resultat;
}

Color configCouleur(char* texte)
{
  char* resultat = str_slice(texte, 2, strlen(texte));
  char** listeCouleur = str_split(resultat,',');

  int r = 255;
  int g = 255;
  int b = 255;

  if (listeCouleur)
  {
    int i;
    for (i=0; *(listeCouleur+i); i++)
    {
        if (i==0) r = atoi(*(listeCouleur+i));
        if (i==1) g = atoi(*(listeCouleur+i));
        if (i==2) b = atoi(*(listeCouleur+i));
    }
  }

  Color couleur(r,g,b);
  return couleur;
}

char* configTexte(char* texte)
{
  char* resultat = str_slice(texte, 2, strlen(texte));
  return resultat;
}

char* str_slice(char str[], int slice_from, int slice_to)
{
    // if a string is empty, returns nothing
    if (str[0] == '\0')
        return NULL;

    char *buffer;
    size_t str_len, buffer_len;

    // for negative indexes "slice_from" must be less "slice_to"
    if (slice_to < 0 && slice_from < slice_to) {
        str_len = strlen(str);

        // if "slice_to" goes beyond permissible limits
        if (abs(slice_to) > str_len - 1)
            return NULL;

        // if "slice_from" goes beyond permissible limits
        if (abs(slice_from) > str_len)
            slice_from = (-1) * str_len;

        buffer_len = slice_to - slice_from;
        str += (str_len + slice_from);

    // for positive indexes "slice_from" must be more "slice_to"
    } else if (slice_from >= 0 && slice_to > slice_from) {
        str_len = strlen(str);

        // if "slice_from" goes beyond permissible limits
        if (slice_from > str_len - 1)
            return NULL;

        buffer_len = slice_to - slice_from;
        str += slice_from;

    // otherwise, returns NULL
    } else
        return NULL;

    buffer = (char*)calloc(buffer_len, sizeof(char)); // ATTENTION ////////////////
    strncpy(buffer, str, buffer_len);
    return buffer;
}
