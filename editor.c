// Copyright Octavian Armasu 315CAa 2022 - 2023
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NMAX 300
typedef struct {
  // structura in care se vor retine valorile respectivei imagini
  int valmax;
  char magic_number[3];
  int m, n;
  int **matrix;
  int **red, **green, **blue;
} image;
int **alloc(int **matrix, int n, int m) {
  int i;
  // aloca dinamic o matrice cu n linii si m coloane
  matrix = (int **)malloc(n * sizeof(int *));
  if (!matrix) {
    fprintf(stderr, "malloc() failed\n");
    return NULL;
  }
  for (i = 0; i < n; i++) {
    matrix[i] = (int *)malloc(m * sizeof(int));
    if (!matrix[i]) {
      fprintf(stderr, "malloc() failed\n");
      while (--i >= 0) {
        free(matrix[i]);
      }
      free(matrix);
      return NULL;
    }
  }
  return matrix;
}
void free_matrix(int n, int **matrix) {
  // elibereaza memoria alocate unei matrice
  for (int i = 0; i < n; i++) {
    free(matrix[i]);
  }
  free(matrix);
}
int suma(int vfrec[], int a) {
  // calculeaza suma valorilor dintr-un vector de frecventa
  int i;
  int s = 0;
  for (i = 0; i <= a; i++) {
    s += vfrec[i];
  }
  return s;
}
int suma_rgb(int parametru[][3], int i, int j, int **matrix) {
  // calculeaza noul pixel rezultat in urma operatiei APPLY
  int s = 0;
  int k, z, i_aux = 0, j_aux;
  for (k = i - 1; k <= i + 1; k++) {
    j_aux = 0;
    for (z = j - 1; z <= j + 1; z++) {
      s = s + matrix[k][z] * parametru[i_aux][j_aux];
      j_aux++;
    }
    i_aux++;
  }
  return s;
}
int clamp(int a) {
  if (a > 255) return 255;
  if (a < 0) return 0;
  return a;
}
void ignore(FILE *in) {
  // functia ignora comentariile din fisiere
  char aux[NMAX];
  int pos = ftell(in);
  fgets(aux, NMAX, in);
  fgets(aux, NMAX, in);
  if (aux[0] != '#') {
    fseek(in, pos, SEEK_SET);
  }
}
int verificare(char read[NMAX]) {
  // verifica daca este o comanda valida cea introdusa
  if (strcmp(read, "LOAD") == 0) return 1;
  if (strcmp(read, "APPLY") == 0) return 1;
  if (strcmp(read, "CROP") == 0) return 1;
  if (strcmp(read, "SAVE") == 0) return 1;
  if (strcmp(read, "HISTOGRAM") == 0) return 1;
  if (strcmp(read, "EQUALIZE") == 0) return 1;
  if (strcmp(read, "SELECT") == 0) return 1;
  if (strcmp(read, "EXIT") == 0) return 1;
  if (strcmp(read, "ROTATE") == 0) return 1;
  return 0;
}
void citire_ascii(int **matrix, int n, int m, FILE *in) {
  // citirea unei matrice ascii din fisier
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      fscanf(in, "%d", &matrix[i][j]);
    }
  }
}
void init_rgb(int **matrix, int **red, int **green, int **blue, int n, int m) {
  int k, i, j;
  // formarea celor 3 matrice de culori (red, green, blue)
  for (i = 0; i < n; i++) {
    k = 0;
    for (j = 0; j < m; j = j + 3) {
      red[i][k++] = matrix[i][j];
    }
  }
  for (i = 0; i < n; i++) {
    k = 0;
    for (j = 1; j < m; j = j + 3) {
      green[i][k++] = matrix[i][j];
    }
  }
  for (i = 0; i < n; i++) {
    k = 0;
    for (j = 2; j < m; j = j + 3) {
      blue[i][k++] = matrix[i][j];
    }
  }
}
void citire_binar(int **matrix, int n, int m, FILE *in) {
  // citirea unei matrice in binar din fisier
  unsigned char auxiliar;
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      fread(&auxiliar, sizeof(unsigned char), 1, in);
      matrix[i][j] = auxiliar;
    }
  }
}
void selected(int **matrix, int **matrix_aux, int x1, int x2, int y1, int y2) {
  // functie folosita la crop pentru a decupa matricea dupa selectie
  int k = 0, z, i, j;
  for (i = y1; i < y2; i++) {
    z = 0;
    for (j = x1; j < x2; j++) {
      matrix_aux[k][z++] = matrix[i][j];
    }
    k++;
  }
}
void matrix_rgb(int **matrix, int n, int m, int **r, int **g, int **b) {
  // formeaza matricea mare din valorile celor 3 matrice color
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < 3 * m; j++) {
      if (j % 3 == 0) {
        matrix[i][j] = r[i][j / 3];
      }
      if (j % 3 == 1) {
        matrix[i][j] = g[i][j / 3];
      }
      if (j % 3 == 2) {
        matrix[i][j] = b[i][j / 3];
      }
    }
  }
}
void scriere_ascii(int **matrix, int n, int m, FILE *out) {
  // scrie in fisier matricea in format ascii
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      fprintf(out, "%d ", matrix[i][j]);
    }
    fprintf(out, "\n");
  }
}
void scriere_binar(int **matrix, int n, int m, FILE *out) {
  // scrie in fisier matricea in format binar
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      fwrite(&matrix[i][j], sizeof(unsigned char), 1, out);
    }
  }
}
void edge(int parametru[3][3]) {
  // initializeaza matricea parametru cu valorile aflate in matricea nucleu la
  // operatie APPLY EDGE
  int i, j;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      if (i == 1 && j == 1)
        parametru[i][j] = 8;
      else {
        parametru[i][j] = -1;
      }
    }
  }
}
void sharpen(int parametru[][3]) {
  // initializeaza matricea parametru cu valorile aflate in matricea nucleu la
  // operatie APPLY SHARPEN
  parametru[0][0] = parametru[0][2] = 0;
  parametru[2][0] = parametru[2][2] = 0;
  parametru[0][1] = parametru[2][1] = -1;
  parametru[1][0] = parametru[1][2] = -1;
  parametru[1][1] = 5;
}
void blur(int parametru[][3]) {
  // initializeaza matricea parametru cu valorile aflate in matricea nucleu la
  // operatie APPLY BLUR
  int i, j;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      parametru[i][j] = 1;
    }
  }
}
void gaussian(int parametru[][3]) {
  // initializeaza matricea parametru cu valorile aflate in matricea nucleu la
  // operatie APPLY GAUSSIAN_BLUR
  parametru[0][0] = parametru[0][2] = 1;
  parametru[2][0] = parametru[2][2] = 1;
  parametru[0][1] = parametru[2][1] = 2;
  parametru[1][0] = parametru[1][2] = 2;
  parametru[1][1] = 4;
}
void matrice_aux(int **a, int **b, int n, int m) {
  // initializeaza o matrice auxiliara cu toate valorile din prima matrice
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      b[i][j] = a[i][j];
    }
  }
}
void swap(int *a, int *b) {
  // schimba valorile a doi parametri
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
int **rotate_90(int **a, int **b, int x1, int x2, int y1, int y2) {
  // roteste selectia la 90 de grade si -270 garde
  int i, j;
  int k = 0;
  for (j = x1; j < x2; j++) {
    k = 0;
    for (i = y2 - 1; i >= y1; i--) {
      a[j][k++] = b[i][j];
    }
  }
  return a;
}
int **rotate_180(int **a, int **b, int x1, int x2, int y1, int y2) {
  // roteste selectia la 180 de grade si -180 garde
  int i, j;
  int k = 0;
  for (j = x1; j < x2; j++) {
    k = 0;
    for (i = y2 - 1; i >= y1; i--) {
      a[j][k++] = b[i][j];
    }
  }
  for (j = x1; j < x2; j++) {
    k = 0;
    for (i = y2 - 1; i >= y1; i--) {
      a[j][k++] = b[i][j];
    }
  }
  return a;
}
int **rotate_270(int **a, int **b, int x1, int x2, int y1, int y2) {
  // roteste selectia la 270 de grade si -90 garde
  int i, j;
  int k = 0;
  for (j = x2 - 1; j >= x1; j--) {
    for (i = y1; i < y2; i++) {
      a[k][i] = b[i][j];
    }
    k++;
  }
  return a;
}
int **rotate_90_all(int **a, int **b, int n, int m) {
  // roteste toata matricea la 90 de grade si -270 garde
  int i, j;
  int k = 0;
  for (j = 0; j < m; j++) {
    k = 0;
    for (i = n - 1; i >= 0; i--) {
      a[j][k++] = b[i][j];
    }
  }
  return a;
}
void adaugare(int **a, int **b, int x1, int x2, int y1, int y2) {
  int i, j;
  for (i = y1; i < y2; i++) {
    for (j = x1; j < x2; j++) {
      a[i][j] = b[i][j];
    }
  }
}
int **rotate_180_all(int **a, int **b, int n, int m) {
  // roteste toata matricea la 180 de grade si -180 garde
  int i, j, k = 0, z = 0;
  for (i = n - 1; i >= 0; i--) {
    z = 0;
    for (j = m - 1; j >= 0; j--) {
      a[k][z++] = b[i][j];
    }
    k++;
  }
  return a;
}
int **rotate_270_all(int **a, int **b, int n, int m) {
  // roteste toata matricea la 270 de grade si -90 garde
  int i, j;
  int k = 0;
  for (j = m - 1; j >= 0; j--) {
    for (i = 0; i < n; i++) {
      a[k][i] = b[i][j];
    }
    k++;
  }
  return a;
}
int main() {
  char comanda[NMAX], filename[NMAX];
  char read[NMAX];
  image a;
  int binar, ascii, load = 0, rgb, i, j;
  char aux[NMAX];
  int x1, x2, y1, y2, slct = 0;
  int cx1, cx2, cy1, cy2;
  char *token;
  while (1) {
    binar = 0;
    ascii = 0;
    // citim tot randul pe care se afla comanda
    fgets(read, NMAX, stdin);
    int len = strlen(read);
    for (i = 0; i < len; i++) {
      if (read[i] == '\n') read[i] = '\0';
    }
    // separam in cuvinte penttru a vedea ce comanda a fost introdusa
    token = strtok(read, " \n");
    strcpy(comanda, token);
    if (verificare(comanda) == 0) {
      printf("Invalid command\n");
    }
    if (strcmp(comanda, "EXIT") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
      } else {
        if (rgb == 1) {
          free_matrix(a.n, a.red);
          free_matrix(a.n, a.green);
          free_matrix(a.n, a.blue);
        }
        free_matrix(a.n, a.matrix);
      }
      break;
    }
    if (strcmp(comanda, "LOAD") == 0) {
      if (load == 1) {
        // daca au mai fost introduse pzoe, eliberam memoria vechilor matrice
        free_matrix(a.n, a.matrix);
        if (rgb == 1) {
          free_matrix(a.n, a.red);
          free_matrix(a.n, a.green);
          free_matrix(a.n, a.blue);
        }
      }
      token = strtok(NULL, " \n");
      strcpy(filename, token);
      FILE *in = fopen(filename, "r");
      if (!in) {
        printf("Failed to load %s\n", filename);
        load = 0;
        slct = 0;
        continue;
      }
      printf("Loaded %s\n", filename);
      load = 1;
      fscanf(in, "%s", a.magic_number);
      if (a.magic_number[1] == '2' || a.magic_number[1] == '3') {
        ascii = 1;
      } else {
        binar = 1;
      }
      ignore(in);
      fscanf(in, "%d %d", &a.m, &a.n);
      // la LOAD, poza trebuie sa fie selectata toata
      slct = 1;
      x1 = 0;
      y1 = 0;
      x2 = a.m;
      y2 = a.n;
      // cx1,cx2,cy1,cy2 sunt copii in care sunt retinute ultimele valori
      // corecte pentru selectie
      cx1 = x1;
      cx2 = x2;
      cy1 = y1;
      cy2 = y2;
      ignore(in);
      fscanf(in, "%d ", &a.valmax);
      ignore(in);
      if (ascii == 1) {
        if (a.magic_number[1] == '3') {
          rgb = 1;
          a.red = alloc(a.red, a.n, a.m);
          a.green = alloc(a.green, a.n, a.m);
          a.blue = alloc(a.blue, a.n, a.m);
          int m = 3 * a.m;
          a.matrix = alloc(a.matrix, a.n, m);
          citire_ascii(a.matrix, a.n, m, in);
          init_rgb(a.matrix, a.red, a.green, a.blue, a.n, m);
        } else {
          rgb = 0;
          a.matrix = alloc(a.matrix, a.n, a.m);
          citire_ascii(a.matrix, a.n, a.m, in);
        }
      }
      if (binar == 1) {
        if (a.magic_number[1] == '6') {
          rgb = 1;
          a.red = alloc(a.red, a.n, a.m);
          a.green = alloc(a.green, a.n, a.m);
          a.blue = alloc(a.blue, a.n, a.m);
          int m = 3 * a.m;
          a.matrix = alloc(a.matrix, a.n, m);
          citire_binar(a.matrix, a.n, m, in);
          init_rgb(a.matrix, a.red, a.green, a.blue, a.n, m);
        } else {
          rgb = 0;
          a.matrix = alloc(a.matrix, a.n, a.m);
          citire_binar(a.matrix, a.n, a.m, in);
        }
      }
      fclose(in);
    }
    if (strcmp(comanda, "SELECT") == 0) {
      token = strtok(NULL, " \n");
      strcpy(aux, token);
      if (strcmp(aux, "ALL") == 0) {
        if (load == 0) {
          printf("No image loaded\n");
        } else {
          printf("Selected ALL\n");
          slct = 1;
          x1 = 0;
          y1 = 0;
          x2 = a.m;
          y2 = a.n;
          cx1 = x1;
          cx2 = x2;
          cy1 = y1;
          cy2 = y2;
        }
      } else {
        if (load == 0) {
          printf("No image loaded\n");
          continue;
        }
        int contor = 0;
        char aux1[NMAX], aux2[NMAX];
        char aux3[NMAX], aux4[NMAX];
        while (token) {
          // ne asiguram ca au fost introdusi 4 parametrii si toti sunt numere
          // intregi
          if ((token[0] >= '0' && token[0] <= '9') || token[0] == '-') contor++;
          if (contor == 1) strcpy(aux1, token);
          if (contor == 2) strcpy(aux2, token);
          if (contor == 3) strcpy(aux3, token);
          if (contor == 4) strcpy(aux4, token);
          token = strtok(NULL, " \n");
        }
        if (contor != 4) {
          printf("Invalid command\n");
          x1 = cx1;
          x2 = cx2;
          y1 = cy1;
          y2 = cy2;
          continue;
        }
        x1 = atoi(aux1);
        x2 = atoi(aux3);
        y1 = atoi(aux2);
        y2 = atoi(aux4);
        int ok = 1;
        if (x1 == x2 || y1 == y2) ok = 0;
        if (x1 < x2) {
          if (x2 > a.m || x1 < 0 || x2 < 0) {
            ok = 0;
          }
        }
        if (x1 > x2) {
          swap(&x1, &x2);
          if (x2 > a.m || x1 < 0 || x2 < 0) {
            ok = 0;
          }
        }
        if (y1 < y2) {
          if (y2 > a.n || x1 < 0 || x2 < 0) {
            ok = 0;
          }
        }
        if (y1 > y2) {
          swap(&y1, &y2);
          if (y2 > a.n || x1 < 0 || x2 < 0) {
            ok = 0;
          }
        }
        if (ok == 1) {
          printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
          cx1 = x1;
          cx2 = x2;
          cy1 = y1;
          cy2 = y2;
        }
        if (ok == 0) {
          printf("Invalid set of coordinates\n");
          if (slct == 1) {
            x1 = cx1;
            x2 = cx2;
            y1 = cy1;
            y2 = cy2;
          }
        }
      }
    }
    if (strcmp(comanda, "CROP") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
      } else {
        if (rgb == 0) {
          int n = a.n;
          a.m = x2 - x1;
          a.n = y2 - y1;
          int **matrix_aux = alloc(matrix_aux, a.n, a.m);
          selected(a.matrix, matrix_aux, x1, x2, y1, y2);
          free_matrix(n, a.matrix);
          a.matrix = matrix_aux;
        }
        if (rgb == 1) {
          // atunci cand poza este color, facem CROP pe cele 3 matrice de
          // culoare(red,green,blue) pe care dupa le punem inapoi in matricea
          // mare
          int n = a.n;
          a.n = y2 - y1;
          a.m = x2 - x1;
          int **red_aux = alloc(red_aux, a.n, a.m);
          int **green_aux = alloc(green_aux, a.n, a.m);
          int **blue_aux = alloc(blue_aux, a.n, a.m);
          selected(a.red, red_aux, x1, x2, y1, y2);
          selected(a.green, green_aux, x1, x2, y1, y2);
          selected(a.blue, blue_aux, x1, x2, y1, y2);
          free_matrix(n, a.red);
          free_matrix(n, a.green);
          free_matrix(n, a.blue);
          a.red = red_aux;
          a.green = green_aux;
          a.blue = blue_aux;
          int **matrix = alloc(matrix, a.n, 3 * a.m);
          matrix_rgb(matrix, a.n, a.m, a.red, a.green, a.blue);
          free_matrix(n, a.matrix);
          a.matrix = matrix;
        }
        x2 = x2 - x1;
        y2 = y2 - y1;
        x1 = y1 = 0;
        cx1 = x1;
        cx2 = x2;
        cy1 = y1;
        cy2 = y2;
        printf("Image cropped\n");
      }
    }
    if (strcmp(comanda, "SAVE") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
      } else {
        int contor = 0;
        ascii = 0;
        binar = 0;
        token = strtok(NULL, " \n");
        strcpy(filename, token);
        FILE *out = fopen(filename, "w");
        while (token) {
          // cu ajutorul variabilei contor verificam cate cuvinte sunt dupa SAVE
          contor++;
          token = strtok(NULL, " \n");
        }
        if (contor == 2) {
          // daca sunt 2 cuvinte, inseamna ca pe langa filename exista si
          // parametrul ascii, care ne indica in ce format sa introducem
          // matricea
          ascii = 1;
        } else {
          binar = 1;
        }
        if (ascii == 1) {
          // magic number trebuie schimbat pentru a corespunde noului format
          if (a.magic_number[1] == '5') {
            a.magic_number[1] = '2';
          }
          if (a.magic_number[1] == '6') {
            a.magic_number[1] = '3';
          }
          fprintf(out, "%s\n", a.magic_number);
          fprintf(out, "%d %d\n", a.m, a.n);
          fprintf(out, "%d\n", a.valmax);
          if (rgb == 0) {
            scriere_ascii(a.matrix, a.n, a.m, out);
          } else {
            scriere_ascii(a.matrix, a.n, 3 * a.m, out);
          }
        }
        if (binar == 1) {
          if (a.magic_number[1] == '2') {
            a.magic_number[1] = '5';
          }
          if (a.magic_number[1] == '3') {
            a.magic_number[1] = '6';
          }
          fprintf(out, "%s\n", a.magic_number);
          fprintf(out, "%d %d\n", a.m, a.n);
          fprintf(out, "%d\n", a.valmax);
          if (rgb == 0) {
            scriere_binar(a.matrix, a.n, a.m, out);
          } else {
            scriere_binar(a.matrix, a.n, 3 * a.m, out);
          }
        }
        printf("Saved %s\n", filename);
        fclose(out);
      }
    }
    if (strcmp(comanda, "HISTOGRAM") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
        continue;
      }
      int x, y;
      int contor = 0;
      token = strtok(NULL, " \n");
      while (token) {
        // verificam ca au fost introdusi 2 parametri
        contor++;
        if (contor == 1) x = atoi(token);
        if (contor == 2) y = atoi(token);
        token = strtok(NULL, " \n");
      }
      if (contor != 2) {
        printf("Invalid command\n");
        continue;
      }
      if (rgb == 1) {
        printf("Black and white image needed\n");
        continue;
      }
      int vfrec[257] = {0};
      int frec[256] = {0}, maxfrec = 0;
      for (i = 0; i < a.n; i++) {
        for (j = 0; j < a.m; j++) {
          vfrec[a.matrix[i][j]]++;
        }
      }
      int interval = 256 / y;
      int k = 0;
      for (i = 1; i <= y; i++) {
        for (j = interval * i - interval; j < interval * i; j++) {
          frec[k] += vfrec[j];
        }
        if (maxfrec < frec[k]) {
          maxfrec = frec[k];
        }
        k++;
      }
      k--;
      double calcul;
      // variabila folosita la efectuarea calculelor
      for (i = 0; i <= k; i++) {
        calcul = frec[i] * x / maxfrec;
        int aprox = (int)calcul;
        printf("%d\t|\t", aprox);
        for (j = 0; j < aprox; j++) {
          printf("*");
        }
        printf("\n");
      }
    }
    if (strcmp(comanda, "EQUALIZE") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
        continue;
      }
      if (rgb == 1) {
        printf("Black and white image needed\n");
        continue;
      }
      printf("Equalize done\n");
      int vfrec[257] = {0};
      for (i = 0; i < a.n; i++) {
        for (j = 0; j < a.m; j++) {
          vfrec[a.matrix[i][j]]++;
        }
      }
      int area = a.m * a.n;
      int **matrix_aux = alloc(matrix_aux, a.n, a.m);
      for (i = 0; i < a.n; i++) {
        for (j = 0; j < a.m; j++) {
          int s = suma(vfrec, a.matrix[i][j]);
          double calcul = 255 * s / area;
          // variabila folosita la efectuarea calculelor
          matrix_aux[i][j] = round(calcul);
          matrix_aux[i][j] = clamp(matrix_aux[i][j]);
        }
      }
      free_matrix(a.n, a.matrix);
      a.matrix = matrix_aux;
    }
    if (strcmp(comanda, "APPLY") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
        continue;
      }
      int contor = 1;
      token = strtok(NULL, " \n");
      if (token) {
        contor++;
        strcpy(aux, token);
      }
      if (contor == 1) {
        printf("Invalid command\n");
        continue;
      }
      if (rgb == 0) {
        printf("Easy, Charlie Chaplin\n");
        continue;
      }
      int ok = 0, b = 0, gb = 0;
      int parametru[3][3];
      if (strcmp(aux, "EDGE") == 0) {
        ok = 1;
        edge(parametru);
      }
      if (strcmp(aux, "SHARPEN") == 0) {
        ok = 1;
        sharpen(parametru);
      }
      if (strcmp(aux, "BLUR") == 0) {
        ok = 1;
        b = 1;
        blur(parametru);
      }
      if (strcmp(aux, "GAUSSIAN_BLUR") == 0) {
        ok = 1;
        gb = 1;
        gaussian(parametru);
      }
      if (ok == 1 && rgb == 1) {
        int **red_aux = alloc(red_aux, a.n, a.m);
        int **green_aux = alloc(red_aux, a.n, a.m);
        int **blue_aux = alloc(red_aux, a.n, a.m);
        // 3 matrice auxiliare in care vor fi introduse noile valori ale
        // pixelilor dupa calcule
        matrice_aux(a.red, red_aux, a.n, a.m);
        matrice_aux(a.green, green_aux, a.n, a.m);
        matrice_aux(a.blue, blue_aux, a.n, a.m);
        for (i = y1; i < y2; i++) {
          for (j = x1; j < x2; j++) {
            if (!(i == 0 || i == a.n - 1 || j == 0 || j == a.m - 1)) {
              int s1 = suma_rgb(parametru, i, j, a.red);
              int s2 = suma_rgb(parametru, i, j, a.green);
              int s3 = suma_rgb(parametru, i, j, a.blue);
              if (b == 1) {
                double calcul1 = s1 * (1.0) / 9;
                double calcul2 = s2 * (1.0) / 9;
                double calcul3 = s3 * (1.0) / 9;
                // variabile folosite la efectuarea calculelor
                s1 = round(calcul1);
                s2 = round(calcul2);
                s3 = round(calcul3);
              }
              if (gb == 1) {
                double calcul1 = s1 * (1.0) / 16;
                double calcul2 = s2 * (1.0) / 16;
                double calcul3 = s3 * (1.0) / 16;
                // variabile folosite la efectuarea calculelor
                s1 = round(calcul1);
                s2 = round(calcul2);
                s3 = round(calcul3);
              }
              s1 = clamp(s1);
              s2 = clamp(s2);
              s3 = clamp(s3);
              red_aux[i][j] = s1;
              green_aux[i][j] = s2;
              blue_aux[i][j] = s3;
            }
          }
        }
        free_matrix(a.n, a.red);
        free_matrix(a.n, a.green);
        free_matrix(a.n, a.blue);
        a.red = red_aux;
        a.green = green_aux;
        a.blue = blue_aux;
        matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
        printf("APPLY %s done\n", aux);
      } else {
        printf("APPLY parameter invalid\n");
      }
    }
    if (strcmp(comanda, "ROTATE") == 0) {
      if (load == 0) {
        printf("No image loaded\n");
        continue;
      }
      token = strtok(NULL, " \n");
      int unghi = atoi(token);
      if (unghi % 90 != 0) {
        printf("Unsupported rotation angle\n");
        continue;
      }
      int all = 0;
      // verificam daca este selectata toata matricea
      if (x1 == 0 && y1 == 0 && x2 == a.m && y2 == a.n) all = 1;
      if (!all) {
        int n = y2 - y1;
        int m = x2 - x1;
        if (n != m) {
          printf("The selection must be square\n");
          continue;
        }
        if (unghi == 0 || unghi == 360 || unghi == -360) {
          printf("Rotated %d\n", unghi);
        }
        if (unghi == 90 || unghi == -270) {
          printf("Rotated %d\n", unghi);
          if (rgb == 0) {
            int **matrix = alloc(matrix, n, m);
            matrix = rotate_90(matrix, a.matrix, x1, x2, y1, y2);
            adaugare(a.matrix, matrix, x1, x2, y1, y2);
            free_matrix(n, matrix);
          }
          if (rgb == 1) {
            int **red = alloc(a.red, n, m);
            int **green = alloc(a.green, n, m);
            int **blue = alloc(a.blue, n, m);
            red = rotate_90(red, a.red, x1, x2, y1, y2);
            green = rotate_90(green, a.green, x1, x2, y1, y2);
            blue = rotate_90(blue, a.blue, x1, x2, y1, y2);
            adaugare(a.red, red, x1, x2, y1, y2);
            adaugare(a.green, blue, x1, x2, y1, y2);
            adaugare(a.blue, green, x1, x2, y1, y2);
            matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
            free_matrix(n, red);
            free_matrix(n, green);
            free_matrix(n, blue);
          }
        }
        if (unghi == 180 || unghi == -180) {
          printf("Rotated %d\n", unghi);
          if (rgb == 0) {
            int **matrix = alloc(matrix, n, m);
            matrix = rotate_180(matrix, a.matrix, x1, x2, y1, y2);
            adaugare(a.matrix, matrix, x1, x2, y1, y2);
            free_matrix(n, matrix);
          }
          if (rgb == 1) {
            int **red = alloc(a.red, n, m);
            int **green = alloc(a.green, n, m);
            int **blue = alloc(a.blue, n, m);
            red = rotate_180(red, a.red, x1, x2, y1, y2);
            green = rotate_180(green, a.green, x1, x2, y1, y2);
            blue = rotate_180(blue, a.blue, x1, x2, y1, y2);
            adaugare(a.red, red, x1, x2, y1, y2);
            adaugare(a.green, blue, x1, x2, y1, y2);
            adaugare(a.blue, green, x1, x2, y1, y2);
            matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
            free_matrix(n, red);
            free_matrix(n, green);
            free_matrix(n, blue);
          }
        }
        if (unghi == 270 || unghi == -90) {
          printf("Rotated %d\n", unghi);
          if (rgb == 0) {
            int **matrix = alloc(matrix, n, m);
            matrix = rotate_270(matrix, a.matrix, x1, x2, y1, y2);
            adaugare(a.matrix, matrix, x1, x2, y1, y2);
            free_matrix(n, matrix);
          }
          if (rgb == 1) {
            int **red = alloc(a.red, n, m);
            int **green = alloc(a.green, n, m);
            int **blue = alloc(a.blue, n, m);
            red = rotate_270(red, a.red, x1, x2, y1, y2);
            green = rotate_270(green, a.green, x1, x2, y1, y2);
            blue = rotate_270(blue, a.blue, x1, x2, y1, y2);
            adaugare(a.red, red, x1, x2, y1, y2);
            adaugare(a.green, blue, x1, x2, y1, y2);
            adaugare(a.blue, green, x1, x2, y1, y2);
            matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
            free_matrix(n, red);
            free_matrix(n, green);
            free_matrix(n, blue);
          }
        }
      }
      if (all == 1) {
        int n = y2 - y1;
        int m = x2 - x1;
        if (unghi == 0 || unghi == 360 || unghi == -360) {
          printf("Rotated %d\n", unghi);
        }
        if (unghi == 90 || unghi == -270) {
          printf("Rotated %d\n", unghi);
          if (rgb == 0) {
            int **matrix = alloc(matrix, m, n);
            matrix = rotate_90_all(matrix, a.matrix, a.n, a.m);
            free_matrix(a.n, a.matrix);
            swap(&a.n, &a.m);
            swap(&x1, &y1);
            swap(&x2, &y2);
            a.matrix = matrix;
          }
          if (rgb == 1) {
            int **red = alloc(red, m, n);
            int **green = alloc(green, m, n);
            int **blue = alloc(blue, m, n);
            red = rotate_90_all(red, a.red, a.n, a.m);
            green = rotate_90_all(green, a.green, a.n, a.m);
            blue = rotate_90_all(blue, a.blue, a.n, a.m);
            free_matrix(a.n, a.red);
            free_matrix(a.n, a.green);
            free_matrix(a.n, a.blue);
            free_matrix(a.n, a.matrix);
            swap(&a.n, &a.m);
            swap(&x1, &y1);
            swap(&x2, &y2);
            a.red = red;
            a.green = green;
            a.blue = blue;
            a.matrix = alloc(a.matrix, a.n, 3 * a.m);
            matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
          }
        }
        if (unghi == 180 || unghi == -180) {
          printf("Rotated %d\n", unghi);
          if (rgb == 0) {
            int **matrix = alloc(matrix, n, m);
            matrix = rotate_180_all(matrix, a.matrix, a.n, a.m);
            free_matrix(a.n, a.matrix);
            a.matrix = matrix;
          }
          if (rgb == 1) {
            int **red = alloc(red, n, m);
            int **green = alloc(green, n, m);
            int **blue = alloc(blue, n, m);
            red = rotate_180_all(red, a.red, a.n, a.m);
            green = rotate_180_all(green, a.green, a.n, a.m);
            blue = rotate_180_all(blue, a.blue, a.n, a.m);
            free_matrix(a.n, a.red);
            free_matrix(a.n, a.green);
            free_matrix(a.n, a.blue);
            free_matrix(a.n, a.matrix);
            a.red = red;
            a.green = green;
            a.blue = blue;
            a.matrix = alloc(a.matrix, a.n, 3 * a.m);
            matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
          }
        }
        if (unghi == 270 || unghi == -90) {
          printf("Rotated %d\n", unghi);
          if (rgb == 0) {
            int **matrix = alloc(matrix, m, n);
            matrix = rotate_270_all(matrix, a.matrix, a.n, a.m);
            free_matrix(a.n, a.matrix);
            swap(&a.n, &a.m);
            swap(&x1, &y1);
            swap(&x2, &y2);
            a.matrix = matrix;
          }
          if (rgb == 1) {
            int **red = alloc(red, m, n);
            int **green = alloc(green, m, n);
            int **blue = alloc(blue, m, n);
            red = rotate_270_all(red, a.red, a.n, a.m);
            green = rotate_270_all(green, a.green, a.n, a.m);
            blue = rotate_270_all(blue, a.blue, a.n, a.m);
            free_matrix(a.n, a.red);
            free_matrix(a.n, a.green);
            free_matrix(a.n, a.blue);
            free_matrix(a.n, a.matrix);
            swap(&a.n, &a.m);
            swap(&x1, &y1);
            swap(&x2, &y2);
            a.red = red;
            a.green = green;
            a.blue = blue;
            a.matrix = alloc(a.matrix, a.n, 3 * a.m);
            matrix_rgb(a.matrix, a.n, a.m, a.red, a.green, a.blue);
          }
        }
      }
    }
  }
  return 0;
}