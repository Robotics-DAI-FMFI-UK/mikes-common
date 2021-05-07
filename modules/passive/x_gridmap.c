#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../live/gui.h"
#include "../live/base_module.h"    /* WHEELS_DISTANCE */
#include "../live/t265.h"
#include "gridmap.h"
#include "mikes_logs.h"
#include "core/config_mikes.h"
#include "../live/mapping_navig.h"

#define BORDER_LEFT 20
#define BORDER_RIGHT 20
#define BORDER_TOP 20
#define BORDER_BOTTOM 20

#define POSE_ARROW_LENGTH (WHEELS_DISTANCE / 1.0)    /* in cm */

static int win;
int counter;
int pose_changed;

static int width;
static int height;
static int pose_visible;
static t265_pose_type 	pose_local_copy;
static int initial_pose_x;
static int initial_pose_y;
static double map_scale;
static double cell_width;
static double cell_height;


// project coordinates in mm to map window pixel coordinates
#define XMM2WIN(x) ((int)(BORDER_LEFT             + (x) * map_scale))
#define YMM2WIN(y) ((int)(height - 1 - BORDER_TOP - (y) * map_scale))


void x_gridmap_draw_pose(cairo_t *w){
    get_t265_pose(&pose_local_copy);
    double heading;
    get_t265_heading(&pose_local_copy, &heading);
    heading = -heading;
    double x = initial_pose_x+pose_local_copy.translation.x*100;
    double y = initial_pose_y-pose_local_copy.translation.z*100;
    
     cairo_set_source_rgb(w, 0.1, 0.1, 0.5);
     cairo_set_line_width(w, 1);
     int px1_win = XMM2WIN(x - POSE_ARROW_LENGTH / 2.0 * sin(heading));
     int py1_win = YMM2WIN(y - POSE_ARROW_LENGTH / 2.0 * cos(heading));
     int px2_win = XMM2WIN(x + POSE_ARROW_LENGTH / 2.0 * sin(heading));
     int py2_win = YMM2WIN(y + POSE_ARROW_LENGTH / 2.0 * cos(heading));
     int px3_win = XMM2WIN(x + POSE_ARROW_LENGTH / 2.0 * sin(heading + M_PI / 2.0));
     int py3_win = YMM2WIN(y + POSE_ARROW_LENGTH / 2.0 * cos(heading + M_PI / 2.0));
     int px4_win = XMM2WIN(x + POSE_ARROW_LENGTH / 2.0 * sin(heading - M_PI / 2.0));
     int py4_win = YMM2WIN(y + POSE_ARROW_LENGTH / 2.0 * cos(heading - M_PI / 2.0));

     cairo_move_to(w, px2_win, py2_win);
     cairo_line_to(w, px3_win, py3_win);
     cairo_line_to(w, px4_win, py4_win);
     cairo_line_to(w, px2_win, py2_win);     
     cairo_fill(w);
     cairo_move_to(w, px1_win, py1_win);
     cairo_line_to(w, px2_win, py2_win);
     cairo_stroke(w);
}

void draw_map(cairo_t *w){
	for (int i = 0; i< mikes_config.gridmap_height; i++){
		for (int j= 0; j< mikes_config.gridmap_width; j++){
			uint32_t total = grid_empty[i][j]+grid_occupied[i][j];
			if (total>0){
				double density = grid_occupied[i][j]/(double) total;
				cairo_set_source_rgb(w, 1-density, 1-density, 1-density);
				cairo_rectangle(w, BORDER_LEFT + cell_width*j, height - 1 - BORDER_TOP - cell_height * i,cell_width, cell_height );
				cairo_fill(w);
			}
		}
	}
	
}

void draw_trajectory(cairo_t *w){
   cairo_set_source_rgb(w, 1.0, 0.1, 0.1);
   int px1_win = XMM2WIN(robot_trajectory[0][1]); //*10 ???
   int py1_win = YMM2WIN(robot_trajectory[0][0]);
   cairo_move_to(w, px1_win, py1_win);
   for (int i = 1; i < trajectory_size; i++){
      px1_win = XMM2WIN(robot_trajectory[i][1]); //*10 ???
      py1_win = YMM2WIN(robot_trajectory[i][0]);
      cairo_line_to(w, px1_win, py1_win);
   }
   cairo_stroke(w);
}

void save_to_file(){
   char name[48];
   sprintf(name, "mapping/pictures/%dmap.png",counter);
   write_to_png(win, name);
   counter++;
   pose_changed = 0;
}

void x_gridmap_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 0.5);
   cairo_paint(w);
 
   draw_map(w);
   
   draw_trajectory(w);
      
   if (pose_visible) x_gridmap_draw_pose(w);
   
   cairo_pop_group_to_source(w);
   cairo_paint(w);
   if (pose_changed) save_to_file();
}

void x_gridmap_toggle_pose_visible(int visible)
{
   if (pose_visible != visible)
   {
       pose_visible = visible;
       request_async_repaint(win);
   }
}

void gridmap_mouse_listener(int x, int y, int button)
{
   double map_x = (x - BORDER_LEFT) / map_scale;  // * 10.0  // assuming dimensions in svg are centimeters
   double map_y = (y - BORDER_TOP) / map_scale;   // * 10.0  // calculated coordinates are in centimeters
   printf("line map click: win_x=%d, win_y=%d, map_x=%.3lf, map_y=%.3lf\n", x, y, map_x, map_y);
}


void x_gridmap_pose_changed(){
   pose_changed = 1;
}


void init_x_gridmap(int win_width, int win_height, int window_update_period_in_ms)
{
   if (!mikes_config.with_gui) return;
   pose_visible = 0;
   width = win_width;
   height = win_height;
   counter = 1000;
   pose_changed = 0;
   initial_pose_x = mikes_config.gridmap_width/2*10;
   initial_pose_y = mikes_config.gridmap_height/2*10;
   map_scale = (width- BORDER_LEFT - BORDER_RIGHT) / (double)(mikes_config.gridmap_width * 10);
   win = gui_open_window(x_gridmap_paint, win_width, win_height, window_update_period_in_ms);
   gui_set_window_title(win, "MAP");
   gui_add_mouse_listener(win, gridmap_mouse_listener);
   cell_width = (width- BORDER_LEFT - BORDER_RIGHT) / (double)mikes_config.gridmap_width;
   cell_height = (height- BORDER_TOP - BORDER_BOTTOM) / (double)mikes_config.gridmap_height;
}

void shutdown_x_gridmap()
{
   if (!mikes_config.with_gui) return;
   gui_close_window(win);
}

