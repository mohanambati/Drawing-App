
/***********************************************************************************************************************************************/
/*  Title      : CSCI 8626 Computer Graphics  Assignment - 2                                                                                   */     
/*  Program    : draw.c                                                                                                                        */
/*  Written by : Mohan Sai Ambati                                                                                                              */
/*  NUID       : 10072130                                                                                                                      */   
/*  Descrption :  implementing a small drawing program that allows the drawing and editing of                                                  */
/*                line segments.                                                                                                               */
/*********************************************************************************************/
//Header Section.                                                                            //
/*********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glut.h"
#include <math.h>
/*********************************************************************************************/
//Global Variables.                                                                          //
/*********************************************************************************************/
#define MIN(a,b) ( (a < b) ? a : b )
#define MAX(a,b) ( (a > b) ? a : b )

/*-----------------------------------------------*/
/* These are somewhat arbitrary constant values. */
/*-----------------------------------------------*/
#define MAXSEGS 100		/* max # of line segments */
#define IWINWIDTH 700		/* initial window width */
#define IWINHEIGHT 700		/* initial window height */
#define SELDIST 10		/* max distance of mouse from selected line */

struct point {
    GLfloat x, y;
} line_start, line_end;

struct segment {
    struct point p[2];			/* line endpoints */
    char selected;			/* non-zero if line is selected */
} seg[MAXSEGS];

int nsegs = 100;				/* number of line segments */
int i = 0;
int sel_i,count;

GLfloat minx = 0, maxx = 1, miny = 0, maxy = 1;		/* x and y data extents */
float mvx_start,mvy_start, mvx_end,mvy_end;



int verbose = 0;			/* non-zero if verbose output desired */

int mousex, mousey;			/* mouse position, window relative */
struct point pmouse;			/* mouse position in projection coord */

int winWidth = IWINWIDTH;		/* initial window size */
int winHeight = IWINHEIGHT;


/*-------------------------------------------------------------*/
/* This callback function is invoked to draw/redraw the lines. */
/*-------------------------------------------------------------*/
void draw (void)
{
    int i;

    glClear (GL_COLOR_BUFFER_BIT);	/* clear display window */

    /*-----------------*/
    /* Draw the lines. */
    /*-----------------*/
    for(i=0;i<nsegs;i++) {

	if (seg[i].selected)
		
		{glColor3f (1.0, 0.0, 0.0);		/* red if selected */
		count = count + 1;
		}
	else
	    glColor3f (0.0, 0.0, 0.0);		/* black if not selected */

	glBegin (GL_LINES);			/* draw the line */
	glVertex2f(seg[i].p[0].x,seg[i].p[0].y);
	glVertex2f(seg[i].p[1].x,seg[i].p[1].y);
	glEnd();
    }

    glFlush();
}

/*------------------------------*/
/* Handle reshaping the window. */
/*------------------------------*/
void winReshapeFcn (GLint newWidth, GLint newHeight)
{
    glViewport(0,0,newWidth,newHeight);
    winWidth = newWidth;
    winHeight = newHeight;
}


/*********************************************************************************************/
//This function deletes the line segment.                                                    //
/*********************************************************************************************/

void delete_line()
{
	
    seg[sel_i].p[0].x = 0;
	seg[sel_i].p[0].y = 0;
	seg[sel_i].p[1].x = 0;
	seg[sel_i].p[1].y = 0;
	
	
}

/*---------------------------------*/
/* Handle "normal" keyboard input. */
/* We only deal with q/Q here.     */
/*---------------------------------*/
void kf(unsigned char key, int x, int y)
{
    int i;

    switch(key) {
	case 'q':			// terminate the program
	case 'Q':
	    exit(0);
	case 'd':
	case 'D':
	  delete_line();
	  glutPostRedisplay();
    }
}


/*-------------------------------------------------------------------*/
/* Set or clear each line segment's 'selected' flag, as appropriate. */
/* Return zero if no flags were changed. Otherwise return 1.         */
/*-------------------------------------------------------------------*/
int set_select(void)
{
    int i, j;
    double num, den;	/* u's numerator, denominator */
    double u;		/* line parameter */
    double x, y;	/* intersection point, projection coordinates */
    double dx, dy;	/* delta x, y for mouse to line in pixels */
    double dsq;		/* squared dist, mouse to line, in pixels */
    int select_change;	/* did any segment's selection flag change? */
    int done;
    
	
    select_change = 0;			/* assume nothing was changed */
	for(i=0;i<nsegs;i++)
		seg[i].selected = 0;

    
    for(i=0;i<nsegs;i++) {
		done = 0;
			
		
	/*--------------------------------------------------------*/
	/* Compute and check distance to segment endpoints first. */
	/* These might have 'u' values outside the 0-1 range, but */
	/* might still be within the gravity field.               */
	/* Note: nearness check is done using window-relative     */
	/* units -- that is, pixels.                              */
	/*--------------------------------------------------------*/
	
	for(j=0;j<2;j++) {
	    dx = (double)(pmouse.x - seg[i].p[j].x) * winWidth / (maxx - minx);
	    dy = (double)(pmouse.y - seg[i].p[j].y) * winHeight / (maxy - miny);
	    dsq = dx * dx + dy * dy;
		
	    if (dsq <= SELDIST * SELDIST) {
			
		if (!seg[i].selected)	/* was not selected, now it is. */
		    select_change = 1;
		seg[i].selected = 1;	/* selected the line. */
		sel_i = i;
		done = 1;		/* seg needs no further testing */
		for(int j =0; j < i; j++)
			seg[j].selected = 0;
		
		break;
	    }
		
	}
	if (done)
	    continue;

	/*---------------------------------------------------------*/
	/* Now find the 'u' parameter for the closest intersection */
	/* of the line through the mouse position and the segment. */
	/*---------------------------------------------------------*/
	
	num = (pmouse.x - seg[i].p[0].x) * (seg[i].p[1].x - seg[i].p[0].x)
	    + (pmouse.y - seg[i].p[0].y) * (seg[i].p[1].y - seg[i].p[0].y);
	den = pow(seg[i].p[1].x - seg[i].p[0].x, 2.0)
            + pow(seg[i].p[1].y - seg[i].p[0].y, 2.0);

	if (den <= 1.0e-10) {		/* check value of den is large enough */
	    if (seg[i].selected)	/* was selected, now it's not */
		select_change = 1;
	    seg[i].selected = 0;
	    continue;
	}

	u = num / den;

	/*------------------------------------------------------------*/
	/* If intersection is not within the line segment, ignore it. */
	/* Recall "end of segment" closeness was tested first.        */
	/*------------------------------------------------------------*/
	if (u < 0 || u > 1) {		/* no intersection with line segment */
	    if (seg[i].selected)	/* was selected, now it's not */
		select_change = 1;
	    seg[i].selected = 0;
	    continue;
	}

	/*---------------------------------------------*/
	/* Determine point of intersection of the line */
	/* from the mouse pointer to the line segment. */
	/*---------------------------------------------*/
	x = seg[i].p[0].x + u * (seg[i].p[1].x - seg[i].p[0].x);
	y = seg[i].p[0].y + u * (seg[i].p[1].y - seg[i].p[0].y);

	/*---------------------------------------------------------*/
	/* Determine distance from mouse to point of intersection. */
	/* Note this distance is in pixels, NOT projection units.  */
	/*---------------------------------------------------------*/
	dx = (double)(pmouse.x - x) * winWidth / (maxx - minx);
	dy = (double)(pmouse.y - y) * winHeight / (maxy - miny);
	dsq = dx * dx + dy * dy;

	/*----------------------*/
	/* Are we close enough? */
	/*----------------------*/
	if (dsq > SELDIST * SELDIST) {	/* no */
	    if (seg[i].selected)	/* was selected, now it's not */
		select_change = 1;
	    seg[i].selected = 0;
	    continue;
	}

	if (!seg[i].selected)		/* was not selected, now it is. */
	{select_change = 1;

	}
	seg[i].selected = 1;	/* line selected */
		sel_i = i;
		for(int j =0; j < i; j++)
			seg[j].selected = 0;
		
		return select_change;
		break;
    }
	

   

  
}

/*-----------------------------------------------------*/
/* GLUT Passive Mouse Motion Function                  */
/* This function is called when the mouse moves within */
/* the window while NO mouse buttons are pressed.      */
/*-----------------------------------------------------*/
void gpmf(int x, int y)
{
    mousex = x;			/* save position, for possible later use */
    mousey = y;

    /*------------------------------------------------------------------*/
    /* Convert position from window relative to projection coordinates. */
    /*------------------------------------------------------------------*/
    pmouse.x = minx + x * (double)(maxx - minx) / winWidth;
    pmouse.y = miny + (winHeight - y) * (double)(maxy - miny) / winHeight;

    if (verbose)
	printf("mouse @ (%d,%d) win-rel, (%g,%g) projection\n",
	    x, y, pmouse.x, pmouse.y);

    /*--------------------------------------------------------------*/
    /* Set the selection flags for each of the line segments.       */
    /* If any flag changed, request redisplay of all line segments. */
    /*--------------------------------------------------------------*/
     if (set_select())
	glutPostRedisplay();
}


/*********************************************************************************************/
//This function handels the code when mouse is pressed and it is moving.                     //
/*********************************************************************************************/

void mousemotion(int x, int y)
{
	if(seg[sel_i].selected)
	{
		mvx_end = ((float)x)/winWidth;
		mvy_end = ((float)(winHeight - y))/winHeight;
		float dmvx = mvx_end - mvx_start;
		float dmvy = mvy_end - mvy_start;
		seg[sel_i].p[0].x = seg[sel_i].p[0].x + dmvx; 
		seg[sel_i].p[0].y = seg[sel_i].p[0].y + dmvy;
		seg[sel_i].p[1].x = seg[sel_i].p[1].x + dmvx; 
		seg[sel_i].p[1].y = seg[sel_i].p[1].y + dmvy;
		mvx_start = mvx_end;
		mvy_start = mvy_end; 
		glutPostRedisplay();
		
	}
	else{
   seg[i].p[1].x = ((float)x)/winWidth;
   seg[i].p[1].y = ((float)(winHeight - y))/winHeight;
    glutPostRedisplay();}
	
}

/*********************************************************************************************/
//This function handels the code when mouse is pressed.                                      //
/*********************************************************************************************/

void mousefunc(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON)	// ignore all but left button
	return;		
	float mfx = ((float)x)/winWidth;
	float mfy = ((float)(winHeight - y))/winHeight;
	
	
	if (state == GLUT_DOWN){
		
			if (seg[sel_i].selected)
			{	
			 mvx_start =  mvx_end  = ((float)x)/winWidth;
			 mvy_start = mvy_end =  ((float)(winHeight - y))/winHeight;
			glutMotionFunc(mousemotion);
			
			}
			else
			{
			glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			i = i + 1;
			seg[i].p[0].x = seg[i].p[1].x = mfx;
			seg[i].p[0].y = seg[i].p[1].y = mfy;
			glutMotionFunc(mousemotion);		// install mouse motion callback
			}
		
	}
    // else ...

    if (state == GLUT_UP) {
	glutMotionFunc(NULL);		// remove mouse motion callback
	glutSetCursor(GLUT_CURSOR_INHERIT);	// restore cursor
	glutPostRedisplay();		// redisplay window
	printf("\n line from (%f, %f) to (%f, %f)", seg[i].p[0].x,seg[i].p[0].y,  seg[i].p[1].x,seg[i].p[1].y);
	return;
    }

    // we should never reach this point.
}



int main(int argc, char **argv)
{
    glutInit(&argc,argv);

    if (verbose) {
	printf("Window size: %d by %d pixels\n", winWidth, winHeight);
        printf("Data extents: %f <= x <= %f, %f <= y <= %f\n",
	    minx, maxx, miny, maxy);
    }

    /*-----------------------------------*/
    /* Do typical OpenGL initialization. */
    /*-----------------------------------*/
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow ("CSCI 8626 Computer Graphics  Assignment - 2");

    glClearColor (1.0, 1.0, 1.0, 1.0);	/* white display window */

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D (minx, maxx, miny, maxy);

    glutDisplayFunc(draw);		/* draw the lines */
    
	 glutMouseFunc(mousefunc); 	// glutMotionFunc installed only when left mouse button is down.
     glutPassiveMotionFunc(gpmf);	/* passive mouse motion */
	glutReshapeFunc(winReshapeFcn);	/* window reshape */
    glutKeyboardFunc(kf);		/* "normal" keyboard entry */

    glutMainLoop();
}
