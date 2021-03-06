#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <GLUT/GLUT.h>

#define H 500
#define W 600
#define mH 15
#define mW 20
#define MINE  1
#define FLAG  2
#define RESET 3
#define AR 0.017453
#define DENTH 4
#define DOUBLECLICK 1
#define LEFTCLICK   2
#define DOUBLECLICKTIME 0.06

const int D = W / mW;
const int mTop = H - D * mH;
const int rLeft   = W * 1 / 5;
const int rRight  = W * 3 / 10;
const int rTop    = 10;
const int rBottom = 40;
int mCount = 0;
int mineN = 0;
int openCount = 0;
int initFlag = 0;
int gameoverFlag = 0;
int gameclearFlag = 0;
time_t start;
const double cColor[3][3] =
  {{0.8, 0.8, 0.8},  //CLOSE
   {0.8, 0.8, 0.8},  //FRAG
   {0.6, 0.6, 0.6}}; //OPEN
const double nColor[9][3] =
  {{1    , 1    , 1    }, //0
   {0    , 0    , 1    }, //1
   {0    , 1    , 0    }, //2
   {1    , 0    , 0    }, //3
   {0.000, 0.031, 0.188}, //4
   {0.439, 0.318, 0.125}, //5
   {0.125, 0.008, 0.502}, //6
   {0    , 0    , 0    }, //7
   {0.506, 0.161, 0.502}};//8

enum Status {CLOSE, CHECK, OPEN};

double getRnd(int min, int max);
void strout(int x, int y, char *string);
void surroundOpen(int xIndex, int yIndex);
void init();
void gameclear();
void gameover();

class Cell {
public:
  int x, y;
  int num;
  int mine;
  int mouse;

  Status status;

  Cell() {}

  Cell(int i, int j) {
    x = i * D + D / 2;
    y = mTop + D / 2 + j * D;
    num = 0;
    mine = (getRnd(0, DENTH) < 1);
    mouse = 0;
    status = CLOSE;
  }

  void open() {
    if (status == CLOSE) {
      status = OPEN;
      if (mine == 1) {
	gameover();
      } else {
	openCount++;
      }
    }
    if (openCount == mH * mW - mineN) {
      gameclear();
    }
  }

  void check() {
    status = CHECK;
    mCount--;
  }

  void close() {
    status = CLOSE;
    mCount++;
  }

  void draw() {
    if((status == CLOSE || status == CHECK) &&
       mouse && gameoverFlag == 0 && gameclearFlag == 0) {
      glColor3f(0.945, 0.537, 0.000);
    } else {
      glColor3f(cColor[status][0],
		cColor[status][1],
		cColor[status][2]);
    }
    glPointSize(D-1);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
    if (status == OPEN) {
      if (mine > 0) {
	glPushMatrix();
	  glTranslatef((double)x, (double)y, 0);
	  glCallList(MINE);
	glPopMatrix();
      } else if (num > 0) {
	glColor3f(nColor[num][0],
		  nColor[num][1],
		  nColor[num][2]);
	char s[2];
	sprintf(s, "%d", num);
	strout(x - D / 4, y + D / 4, s);
      }
    } else if (status == CHECK) {
      glPushMatrix();
        glTranslatef((double)x, (double)y, 0);
	glCallList(FLAG);
      glPopMatrix();
    }
  }
};

Cell cell[mW][mH];

double getRnd(int min, int max) {
  return min + (double)(max - min) * rand() / RAND_MAX;
}

void strout(int x, int y, char *string)
{
  void *font = GLUT_BITMAP_TIMES_ROMAN_24;
  glRasterPos2i(x, y);
  int len = (int) strlen(string);
  for (int i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}

void reset() {
  init();
  mCount = 0;
  mineN = 0;
  openCount = 0;
  initFlag = 0;
  gameoverFlag = 0;
  gameclearFlag = 0;
  glClearColor(0.7, 0.7, 0.7, 0.);
}

void figureInit() {
  glNewList( MINE, GL_COMPILE );
    glColor3f(0, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
      glVertex2i(0,0);
      for(int i=0; i<=360; i+=10 )
	if (i % 30 == 0) {
	  glVertex2f(11*cos(AR*(double)i), 11*sin(AR*(double)i));
	} else {
	  glVertex2f(7*cos(AR*(double)i), 7*sin(AR*(double)i));
	}
    glEnd();
  glEndList();

  glNewList( FLAG, GL_COMPILE );
    glColor3f(1, 0, 0);
    glBegin(GL_POLYGON);
      glVertex2i(-2 ,10 );
      glVertex2i(2  ,10 );
      glVertex2i(2  ,-2 );
      glVertex2i(14 ,-6 );
      glVertex2i(2  ,-10);
      glVertex2i(-4 ,-10);
      glVertex2i(-4 ,10 );
    glEnd();
  glEndList();

  glNewList( RESET, GL_COMPILE );
    glColor3f(0.945, 0.816, 0.251);
    glBegin(GL_TRIANGLE_FAN);
      glVertex2i(rLeft , rTop);
      glVertex2i(rLeft , rBottom);
      glVertex2i(rRight, rBottom);
      glVertex2i(rRight, rTop);
    glEnd();
  glEndList();
}

void edge(int xIndex, int yIndex,
	  int *left, int *right, int *top, int *bottom) {
  *left   =  (xIndex == 0)      ? 0 : -1;
  *right  =  (xIndex == mW - 1) ? 0 :  1;
  *top    =  (yIndex == 0)      ? 0 : -1;
  *bottom =  (yIndex == mH - 1) ? 0 :  1;
}

void init() {
  figureInit();
  for (int i = 0; i < mW; i++) {
    for (int j = 0; j < mH; j++) {
      cell[i][j] = Cell(i, j);
    }
  }
  glutPostRedisplay();
}

void init(int xIndex, int yIndex) {
  int left, right, top, bottom;
  edge(xIndex, yIndex, &left, &right, &top, &bottom);
  for (int k = left; k <= right; k++) {
    for (int n = top; n <= bottom; n++) {
      cell[xIndex + k][yIndex + n].mine = 0;
    }
  }
  left = -1; right = 1; top = -1; bottom = 1;
  for (int i = 0; i < mW; i++) {
    for (int j = 0; j < mH; j++) {
      if (cell[i][j].mine > 0) {
	mCount++;
	edge(i, j, &left, &right, &top, &bottom);
	for (int k = left; k <= right; k++) {
	  for (int n = top; n <= bottom; n++) {
	    cell[i+k][j+n].num++;
	  }
	}
	cell[i][j].num--;
      }
    }
  }
  mineN = mCount;
  initFlag = 1;
  start = time(NULL);
}

void surroundOpen(int xIndex, int yIndex, int mode) {
  if (gameoverFlag == 0) {
    int left, right, top, bottom;
    edge(xIndex, yIndex, &left, &right, &top, &bottom);
    for (int k = left; k <= right; k++) {
      for (int n = top; n <= bottom; n++) {
	int cx = xIndex + k, cy = yIndex + n;
	switch (mode) {
	case LEFTCLICK: {
	  if (cell[cx][cy].status == CLOSE) {
	    if (!(k == 0 && n == 0) && cell[cx][cy].num == 0) {
	      surroundOpen(cx, cy, LEFTCLICK);
	    }
	    cell[cx][cy].open();
	  }
	}
	case DOUBLECLICK: {
	  if (!(k == 0 && n == 0) && cell[cx][cy].num == 0) {
	    //surroundOpen(cx, cy, LEFTCLICK);
	  }
	  cell[cx][cy].open();
	}
	}
      }
    }
  }
}

void score() {
  long cleartime = time(NULL) - start;
  long hiscore;
  FILE *fp;
  char fname[10] = "score.txt";
  char s[10];

  if((fp=fopen(fname,"r"))==NULL) {
    printf("%s�t�@�C�����J���܂����B\n", fname);
    return;
  }
  if(fgets(s, 100, fp)!=NULL) {
    char **gomi;
    hiscore = strtol(s, gomi, 10);
  }
  fclose(fp);

  if (cleartime < hiscore) {
    fp=fopen(fname, "w");
    fprintf(fp, "%d\n", cleartime);
  }
  fclose(fp);
}

void gameover() {
  glClearColor(0, 0, 0, 0);
  gameoverFlag = 1;
  for (int i = 0; i < mW; i++) {
    for (int j = 0; j < mH; j++) {
      if (cell[i][j].mine == 1) {
	cell[i][j].open();
      }
    }
  }
}

void gameclear() {
  glClearColor(0.961, 0.847, 0.000, 0);
  gameclearFlag = 1;
  score();
}

int checkMouseOnReset(int x, int y) {
  return x >= rLeft && x <= rRight && y >= rTop && y <= rBottom;
}

int checkMouseOnCell(int x, int y, int *xIndex, int *yIndex) {
  if (x >= 0 && x <= W && y >= mTop && y <= H) {
    *xIndex = x / D;
    *yIndex = (y - mTop) / D;
    return 1;
  }
  return 0;
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  for (int i = 0; i < mW; i++) {
    for (int j = 0; j < mH; j++) {
      cell[i][j].draw();
    }
  }
  if (gameoverFlag == 1) {
    glColor3f(1, 0, 0);
    char s[11] = "Game Over!";
    strout(W / 2, 30, s);
  } else if (gameclearFlag == 1) {
    glColor3f(1, 0, 0);
    char s[12] = "Game Clear!";
    strout(W / 2, 30, s);
  }

  glColor3f(0, 0, 1);
  char s[10];
  sprintf(s, "rest:%d", mCount);
  strout(10, 30, s);

  glCallList(RESET);
  glColor3f(0, 0, 0);
  sprintf(s, "reset");
  strout(W * 1 / 5 + 5, 35, s);
  glutSwapBuffers();
}

clock_t mouseclickclock = 0;
int clickCount = 0;
void mouseclick(int button, int state, int x, int y) {
  if(state == GLUT_DOWN) {
    if ((clock() - mouseclickclock) <= DOUBLECLICKTIME * CLOCKS_PER_SEC) {
      clickCount = (clickCount + 1) % 2;
    } else {
      clickCount = 0;
    }
    if (gameoverFlag == 0 && gameclearFlag == 0) {
      int xIndex, yIndex;
      switch (button) {
      case GLUT_LEFT_BUTTON:
	{
	  if(checkMouseOnCell(x, y, &xIndex, &yIndex)) {
	      if (cell[xIndex][yIndex].status == CLOSE) {
		if (initFlag == 0) {
		  init(xIndex, yIndex);
		}
		cell[xIndex][yIndex].open();
		if (cell[xIndex][yIndex].num == 0 && gameoverFlag == 0) {
		  surroundOpen(xIndex, yIndex, LEFTCLICK);
		}
		//	      cell[xIndex][yIndex].open();
		glutPostRedisplay();
	      }
	      //}
	  }
	  mouseclickclock = clock();
	  break;
	}
      case GLUT_RIGHT_BUTTON:
	{
	  if(checkMouseOnCell(x, y, &xIndex, &yIndex)) {
	    if (cell[xIndex][yIndex].status == CHECK) {
	      cell[xIndex][yIndex].close();
	    } else if (cell[xIndex][yIndex].status == CLOSE) {
	      cell[xIndex][yIndex].check();
	    }
	    glutPostRedisplay();
	  }
	  break;
	}
      default: break;
      }
    }
    if (checkMouseOnReset(x, y)) {
      reset();
    }
  }
}

int mx, my;
void passivemousemotion(int x, int y) {
  cell[mx][my].mouse = 0;
  if (checkMouseOnCell(x, y, &mx, &my)) {
    cell[mx][my].mouse = 1;
    glutPostRedisplay();
  }
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case '\033': case 13: exit(0);
  }
}

//**** MAIN FUNCTION ****
int main(int argc, char *argv[]) {
  glutInitWindowSize(W, H);
  glutInitWindowPosition(100, 10);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutCreateWindow("MineSweeper");
  glClearColor(0.7, 0.7, 0.7, 0.);
  glViewport(0, 0, W, H);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, W, H, 0, -10., 10.);
  glMatrixMode(GL_MODELVIEW);
  glutDisplayFunc(display);
  glutMouseFunc(mouseclick);
  glutPassiveMotionFunc(passivemousemotion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  return 0;
}
