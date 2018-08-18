#ifndef _LINE_MAP_H_
#define _LINE_MAP_H_

#define MAX_LINES_IN_LINE_MAP 200

typedef struct {
    double x;
    double y;
} point;

typedef struct {
   double x1;
   double y1;
   double x2;
   double y2;
   int id;
} line;


/* loads the specified map from file. It should contain only <line> tags, each must have lineId attribute,
 * otherwise it will be ignored. Dimensions in x1,y1, x2,y2 attributes are expected to be centimeters,
 * they are converted to millimeters during loading. All internal measurements here are then in millimeters. */
void init_line_map(char *filename);

/* determines if two line segments intersect, and if yes, where
 * returns negative number, when no intersection, otherwise
 * the intersection point in X,Y */
double line_intersection(double x1, double y1, double x2, double y2,
                         double x3, double y3, double x4, double y4,
                         double *X, double *Y);

/* finds a point where the given line segment specified by its starting point x1,y1, heading alpha, and length
 * intersects with some line in the map (intersection point is returned in nx,ny)
 * if the line does not intersect with map, it returns -1, otherwise the distance of the intersection point is returned. */
double find_intersection_of_line_with_line_map(double x1, double y1, double alpha, double length, double *nx, double *ny);

#endif
