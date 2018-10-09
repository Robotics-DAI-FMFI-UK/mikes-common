module mikes_basic()
{
    frame();
    bottom_base();
    wheels();
    top();
}

module frame()
{
    bottom_frame();
    vertical_frame();
    top_frame();
}

module bottom_frame()
{
    //left
    translate([-136,-217,23])
      cube([15,322,15]);
    //right
    translate([121,-217,23])
      cube([15,322,15]);
    //rear
    translate([-136,-217,44])
      cube([272,15,15]);
    //front
    translate([-136,90,8])
      cube([272,15,15]);
}

module vertical_frame()
{
    //rear right
    translate([37.5, -232, 44])
      cube([15,15,234]);
    //rear left
    translate([-52.5, -232, 44])
      cube([15,15,234]);
    //right
    translate([136, -217+115,23])
      cube([15,15,270]);
    //left
    translate([-151, -217+115,23])
      cube([15,15,270]);    
}

module top_frame()
{
    //front
    translate([-67.5,-27,44+234])
      cube([135,15,15]);
    //middle
    translate([-151,-217+100, 23+255])
      cube([302,15,15]);
    //back
    translate([-67.5,-232,44+234])
      cube([135,15,15]);
    //left
    translate([-67.5,-232,44+219])
      cube([15,220,15]);
    //right
    translate([52.5,-232,44+219])
      cube([15,220,15]);    
}

module bottom_base()
{
    translate([-136,-217,38])
      cube([272, 322, 6]);
}

module wheels()
{
    translate([148, 0, 0])
    {
      wheel(-20);
      wheel_holder(-32);
    }
    translate([-148-29, 0, 0])
    {
      wheel(20);
      wheel_holder(40);
    }
    rear_wheel();
}

module wheel(dx)
{
    rotate([0,90,00])
    {
      cylinder(h=29, d=158, $fn=100);
      translate([0,0,dx])
        cylinder(h=29, d=10, $fn=20);
    }
}

module wheel_holder(dx)
{
    translate([dx,-42.5,-43])
      cube([20, 95, 66]);
}

module rear_wheel()
{
    translate([-12.5,-152, -39])
    {
       rear_wheel_holder();
        translate([0,-45,0])
          rotate([0,90,00])
          {
            cylinder(h=25, d=80, $fn=100);
            translate([0,0,-5])
              cylinder(h=35, d=10, $fn=100);
          }
    }
}

module rear_wheel_holder()
{
    translate([-12.5 ,-5,-5])
      cube([10,10,82]);
    translate([15+12.5,-5,-5])
      cube([10,10,82]);
    translate([-12.5 ,-50,-5])
      cube([10,45,10]);
    translate([15 + 12.5 ,-50,-5])
      cube([10,45,10]);
}

module top()
{
    translate([0,0,15])
    polyhedron(
     points=[ 
        //left front  0
        [-151,-217+100+75, 6 + 23+255],
        //left back  1
        [-151,-217+100-40, 6 + 23+255],
        //rear left  2
        [-67.5,-232,6 + 23+255],
        //rear right  3
        [67.5,-232,6 + 23+255],
        //right back  4
        [151,-217+100-40, 6 + 23+255],
        //right front  5
        [151,-217+100+75, 6 + 23+255],
        //front right  6
        [67.5,43, 6 + 23+255],
        //front left  7
        [-67.5,43, 6 + 23+255],
    
        //left front  8
        [-151,-217+100+75, 23+255],
        //left back  9
        [-151,-217+100-40, 23+255],
        //rear left  10
        [-67.5,-232,23+255],
        //rear right  11
        [67.5,-232,23+255],
        //right back  12
        [151,-217+100-40, 23+255],
        //right front  13
        [151,-217+100+75, 23+255],
        //front right  14
        [67.5,43, 23+255],
        //front left   15
        [-67.5,43, 23+255] ],
     faces=[ [7, 6, 5, 4, 3, 2, 1, 0],
             [8, 9, 10, 11, 12, 13, 14, 15],
             [0, 1, 9, 8],
             [1, 2, 10, 9],
             [2, 3, 11, 10],
             [3, 4, 12, 11],
             [4, 5, 13, 12],
             [5, 6, 14, 13],
             [6, 7, 15, 14],
             [7, 0, 8, 15]],
     convexity=2);
}

module tray()
{  
    difference()
    {
        polyhedron(
         points=[ 
            //left back  0
            [-151,-217, 4 + 167 - 25],
            //rear right  1
            [151,-217,4 + 117],
            //front right  2
            [151,105, 4 + 306-79-50],
            //front left  3
            [-151,105, 4 + 306-79 - 25],

            //left back  4
            [-151,-217, 167 - 25],
            //rear right  5
            [151,-217, 117],
            //front right  6
            [151,105, 306-79-50],
            //front left  7
            [-151,105, 306-79 - 25] ],
         faces=[ [3, 2, 1, 0],
                 [4, 5, 6, 7],
                 [0, 1, 5, 4],
                 [1, 2, 6, 5],
                 [2, 3, 7, 6],
                 [3, 0, 4, 7] ],
         convexity=2); 
      polyhedron(
         points=[
            //front right 0 
            [-136,-217+115, 165 - 55],
            //left front 1
            [-152,-217+115, 165 - 55],
            //left back 2
            [-152,-234, 165 - 55],
            //rear right 3
            [-51.5,-234, 165 - 55],            
            //right front 4
            [-51.5,-219, 165 - 55],            

            //front right 5 
            [-136,-217+115, 165 + 25],
            //left front 6
            [-152,-217+115, 165 + 25],
            //left back 7
            [-152,-234, 165 + 25],
            //rear right 8
            [-51.5,-234, 165 + 25],           
            //right front 9
            [-51.5,-219, 165 + 25]],
         faces=[
            [0, 1, 2, 3, 4],
            [9, 8, 7, 6, 5],
            [5, 6, 1, 0],
            [6, 7, 2, 1],
            [7, 8, 3, 2],
            [8, 9, 4, 3],
            [9, 5, 0, 4] ],
         convexity = 2);
      translate([136, -217+115,23])
        cube([15,15,270]);
    }
}

module tray_in_2D()
{
    projection(cut=false)
      rotate([-10.555181433,-4.732238142,0])
        tray();
}

mikes_basic();
tray();
//tray_in_2D();
