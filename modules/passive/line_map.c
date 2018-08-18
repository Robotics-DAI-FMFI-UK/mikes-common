#include <libxml/xmlreader.h>
#include <math.h>

#include "line_map.h"
#include "mikes_logs.h"

static int lines_count;
static line lines[MAX_LINES_IN_LINE_MAP];
static const double epsilon = 0.000001;

int parse_element(xmlNode* a_node, line *lines)
{
   xmlNode *cur_node = NULL;

   int i = 0;

   for (cur_node = a_node; cur_node; cur_node = cur_node->next)
   {
      if (cur_node->type == XML_ELEMENT_NODE)
      {
         if (xmlStrEqual(cur_node->name, (const xmlChar *) "line")
             && xmlGetProp(cur_node, (const xmlChar *) "lineId") != NULL)
         {
            xmlChar *x1 = xmlGetProp(cur_node, (const xmlChar *) "x1");
            xmlChar *y1 = xmlGetProp(cur_node, (const xmlChar *) "y1");
            xmlChar *x2 = xmlGetProp(cur_node, (const xmlChar *) "x2");
            xmlChar *y2 = xmlGetProp(cur_node, (const xmlChar *) "y2");
            xmlChar *lineId = xmlGetProp(cur_node, (const xmlChar *) "lineId");
            lines[i].x1 = 10.0 * (double) atoi((const char *) x1);
            lines[i].y1 = 10.0 * (double) atoi((const char *) y1);
            lines[i].x2 = 10.0 * (double) atoi((const char *) x2);
            lines[i].y2 = 10.0 * (double) atoi((const char *) y2);
            lines[i].x1 = 10.0 * (double) atoi((const char *) x1);
            lines[i].id = atoi((const char *) lineId);
            i++;
            if (i == MAX_LINES_IN_LINE_MAP)
            {
               mikes_log(ML_ERR, "too many lines in line map");
               break;
            }
         }
      }
   }
   return i;
}

int get_lines_from_file(const char *filename, line *lines)
{

   xmlDoc *doc = NULL;
   xmlNode *root_element = NULL;

   /*
    * this initialize the library and check potential ABI mismatches
    * between the version it was compiled for and the actual shared
    * library used.
    */
   LIBXML_TEST_VERSION

   /*parse the file and get the DOM */
   doc = xmlReadFile(filename, NULL, 0);

   if (doc == NULL) {
      mikes_log(ML_ERR, "SVG parse error: could not parse svg file");
   }

   /*Get the root element node */
   root_element = xmlDocGetRootElement(doc);

   int line_n = parse_element(root_element->children, lines);

   /* free the document */
   xmlFreeDoc(doc);
   /* Free the global variables that may have been allocated by the parser. */
   xmlCleanupParser();

   return line_n;
}

void swap(double *a, double *b)
{
        double p = *a;
        *a = *b;
        *b = p;
}

void collapse_interval(double left, double right, double *pt)
{
        if (*pt > left)
        {
                if (*pt < right) *pt = left;
                else *pt -= (right - left);
        }
}

/* box1: x1,y1 - x2,y2, box2: x3,y3 - x4,y4 */
int bouding_boxes_intersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
        if (x2 < x1) swap(&x1, &x2);
        if (y2 < y1) swap(&y1, &y2);
        if (x4 < x3) swap(&x3, &x4);
        if (y4 < y3) swap(&y3, &y4);

        collapse_interval(x1, x2, &x3);
        collapse_interval(x1, x2, &x4);
        collapse_interval(y1, y2, &y3);
        collapse_interval(y1, y2, &y4);

        if((x3 <= x1) && (x4 >= x1) &&
           (y3 <= y1) && (y4 >= y1))
                return 1;
        else
                return 0;
}

/* line1: x1,y1 - x2,y2, line2: x3,y3 - x4,y4, intersection: X,Y */
double line_intersection(
                         double x1, double y1,
                         double x2, double y2,
                         double x3, double y3,
                         double x4, double y4,
                         double *X, double *Y)
{
   if (!bouding_boxes_intersect(x1,y1,x2,y2,x3,y3,x4,y4))
      return -3;

   double A1 = y2 - y1;
   double B1 = x1 - x2;
   double C1 = A1 * x1 + B1 * y1;

   double A2 = y4 - y3;
   double B2 = x3 - x4;
   double C2 = A2 * x3 + B2 * y3;

   double det = A1 * B2 - A2 * B1;
   if (det == 0) {
      //Lines are parallel
      return -1;
   } else {
      double x = (B2*C1 - B1*C2)/det;
      double y = (A1*C2 - A2*C1)/det;
      if ((fmin(x1,x2) < (x + epsilon))
          && (x < (fmax(x1,x2) + epsilon))
          && (fmin(y1,y2) < (y + epsilon))
          && (y < (fmax(y1,y2) + epsilon))
          && (fmin(x3,x4) < (x + epsilon))
          && (x < (fmax(x3,x4) + epsilon))
          && (fmin(y3,y4) < (y + epsilon))
          && (y < (fmax(y3,y4) + epsilon)))
      {
         *X = x;
         *Y = y;
         return 1;
      } else
         return -2;
   }
}

/* finds a point where the given line segment specified by its starting point x1,y1, heading alpha, and length
 * intersects with some line in the map (intersection point is returned in nx,ny)
 * if the line does not intersect with map, it returns -1, otherwise the distance of the intersection point is returned. */
double find_intersection_of_line_with_line_map(double x1, double y1, double alpha, double length, double *nx, double *ny) 
{
   double min_sq_dist = length * length;
   double sq_dist;

   double x2 = x1 + length * sin(alpha);
   double y2 = y1 - length * cos(alpha);

   double intx, inty;
   int found = 0;

   for (int i = 0; i < lines_count; i++) 
   {
      if (line_intersection(x1, y1, x2, y2, 
                            lines[i].x1, lines[i].y1, lines[i].x2, lines[i].y2, 
                            &intx, &inty) > 0)
      {
        sq_dist = (intx - x1) * (intx - x1) + (inty - y1) * (inty - y1);
        if (sq_dist < min_sq_dist)
        {
          min_sq_dist = sq_dist;
          *nx = intx; *ny = inty;
          found = 1;
        }
      }
   }

   if (!found) return -1;

   return sqrt(min_sq_dist);
}

void init_line_map(char *filename)
{
   lines_count = get_lines_from_file(filename, lines);
}

