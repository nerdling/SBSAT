/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark,  service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#ifdef HAVE_IMAGE_JPEG

#include "ite.h"

/* Bring in gd library functions */
#include "gd.h"

typedef struct _t_graph_pt {
  double progress;
  long   cp;
  _t_graph_pt *next;
} t_graph_pt;

t_graph_pt *graph_pt_head=NULL;
t_graph_pt *graph_pt=NULL;

ITE_INLINE void
graph_record(double progress)
{
   t_graph_pt *tmp_graph_pt;
   tmp_graph_pt = (t_graph_pt*)ite_calloc(1, sizeof(t_graph_pt),
         9, "tmp_graph_pt");
   tmp_graph_pt->progress = progress;
   tmp_graph_pt->cp = ite_counters[NO_ERROR];
   tmp_graph_pt->next = NULL;
   if (graph_pt) {
	graph_pt->next = tmp_graph_pt;
	graph_pt = tmp_graph_pt;
   } else {
	graph_pt_head = tmp_graph_pt;
	graph_pt = tmp_graph_pt;
   }
}

ITE_INLINE void
graph_free()
{
  while (graph_pt_head) {
    t_graph_pt *tmp_graph_pt = graph_pt_head->next;
    free(graph_pt_head);
    graph_pt_head = tmp_graph_pt;
  }
  graph_pt = NULL;
}

ITE_INLINE void
graph_generate(char *filename)
{
	/* Declare the image */
	gdImagePtr im;

	/* Declare output files */
	FILE *jpgout;

	/* Declare color indexes */
	int black;
	int white;
        double size_x = 320;
        double size_y = 200;

	/* Allocate the image: 320 pixels across by 200 pixels tall */
	im = gdImageCreate((int)size_x, (int)size_y);

	/* Allocate the color black (red, green and blue all minimum).
	 * 		Since this is the first color in a new image, it will
	 * 				be the background color. */
	black = gdImageColorAllocate(im, 0, 0, 0);	

	/* Allocate the color white (red, green and blue all maximum). */
	white = gdImageColorAllocate(im, 255, 255, 255);	
							
	/* Draw a line from the upper left to the lower right,
	 * 		using white color index. */
        double ppx = 100/(size_x-5);
        double ppy = ite_counters[NO_ERROR]/(size_y-5);
	int x=2;
	int y=2;
        t_graph_pt *tmp_graph_pt = graph_pt_head;
        while (tmp_graph_pt) {
	   int next_x = (int)(tmp_graph_pt->progress/ppx);
	   int next_y = (int)(tmp_graph_pt->cp/ppy);
	   gdImageLine(im, x, (int)(size_y-y), next_x, (int)(size_y-next_y), white);	
	   x = next_x;
	   y = next_y;

           tmp_graph_pt = tmp_graph_pt->next;
        }

	/* Open a file for writing. "wb" means "write binary", important
	 * 		under MSDOS, harmless under Unix. */
	/*
	pngout = fopen(filename, "wb");
        if (pngout) {

	   // Output the image to the disk file in PNG format. 
	   gdImagePng(im, pngout);

	   // Close the files. 
	   fclose(pngout);
	}
	*/
	/* Do the same for a JPEG-format file. */
	jpgout = fopen(filename, "wb");
	if (jpgout) {

	   /* Output the same image in JPEG format, using the default
	    * 		JPEG quality setting. */
	   gdImageJpeg(im, jpgout, -1);

	   // Close the files. 
	   fclose(jpgout);
	}

	/* Destroy the image in memory. */
	gdImageDestroy(im);
}

#endif
