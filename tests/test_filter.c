#include <stdlib.h>

#include "../bites/filter.h"=

void create_test_lines(lines_data *lines)
{
  lines->count = 50;

  lines->lines[0].line.distance = 1200.0000;
  lines->lines[0].line.angle = 190.0000;
  lines->lines[0].votes = 388;

  lines->lines[1].line.distance = 1185.0000;
  lines->lines[1].line.angle = 190.0000;
  lines->lines[1].votes = 383;

  lines->lines[2].line.distance = 2655.0000;
  lines->lines[2].line.angle = 100.0000;
  lines->lines[2].votes = 324;

  lines->lines[3].line.distance = 1200.0000;
  lines->lines[3].line.angle = 195.0000;
  lines->lines[3].votes = 309;

  lines->lines[4].line.distance = 2145.0000;
  lines->lines[4].line.angle = 10.0000;
  lines->lines[4].votes = 298;

  lines->lines[5].line.distance = 2160.0000;
  lines->lines[5].line.angle = 10.0000;
  lines->lines[5].votes = 297;

  lines->lines[6].line.distance = 1185.0000;
  lines->lines[6].line.angle = 195.0000;
  lines->lines[6].votes = 214;

  lines->lines[7].line.distance = 2175.0000;
  lines->lines[7].line.angle = 10.0000;
  lines->lines[7].votes = 203;

  lines->lines[8].line.distance = 2145.0000;
  lines->lines[8].line.angle = 5.0000;
  lines->lines[8].votes = 195;

  lines->lines[9].line.distance = 1215.0000;
  lines->lines[9].line.angle = 190.0000;
  lines->lines[9].votes = 189;

  lines->lines[10].line.distance = 2670.0000;
  lines->lines[10].line.angle = 100.0000;
  lines->lines[10].votes = 178;

  lines->lines[11].line.distance = 1170.0000;
  lines->lines[11].line.angle = 190.0000;
  lines->lines[11].votes = 178;

  lines->lines[12].line.distance = 2640.0000;
  lines->lines[12].line.angle = 100.0000;
  lines->lines[12].votes = 163;

  lines->lines[13].line.distance = 1170.0000;
  lines->lines[13].line.angle = 195.0000;
  lines->lines[13].votes = 161;

  lines->lines[14].line.distance = 2130.0000;
  lines->lines[14].line.angle = 10.0000;
  lines->lines[14].votes = 154;

  lines->lines[15].line.distance = 2190.0000;
  lines->lines[15].line.angle = 10.0000;
  lines->lines[15].votes = 144;

  lines->lines[16].line.distance = 1155.0000;
  lines->lines[16].line.angle = 195.0000;
  lines->lines[16].votes = 119;

  lines->lines[17].line.distance = 2130.0000;
  lines->lines[17].line.angle = 5.0000;
  lines->lines[17].votes = 119;

  lines->lines[18].line.distance = 1230.0000;
  lines->lines[18].line.angle = 190.0000;
  lines->lines[18].votes = 117;

  lines->lines[19].line.distance = 1140.0000;
  lines->lines[19].line.angle = 195.0000;
  lines->lines[19].votes = 110;

  lines->lines[20].line.distance = 2205.0000;
  lines->lines[20].line.angle = 10.0000;
  lines->lines[20].votes = 105;

  lines->lines[21].line.distance = 1230.0000;
  lines->lines[21].line.angle = 185.0000;
  lines->lines[21].votes = 105;

  lines->lines[22].line.distance = 2115.0000;
  lines->lines[22].line.angle = 5.0000;
  lines->lines[22].votes = 98;

  lines->lines[23].line.distance = 2625.0000;
  lines->lines[23].line.angle = 100.0000;
  lines->lines[23].votes = 96;

  lines->lines[24].line.distance = 1200.0000;
  lines->lines[24].line.angle = 185.0000;
  lines->lines[24].votes = 96;

  lines->lines[25].line.distance = 1185.0000;
  lines->lines[25].line.angle = 200.0000;
  lines->lines[25].votes = 95;

  lines->lines[26].line.distance = 2160.0000;
  lines->lines[26].line.angle = 5.0000;
  lines->lines[26].votes = 95;

  lines->lines[27].line.distance = 1215.0000;
  lines->lines[27].line.angle = 195.0000;
  lines->lines[27].votes = 93;

  lines->lines[28].line.distance = 1245.0000;
  lines->lines[28].line.angle = 185.0000;
  lines->lines[28].votes = 91;

  lines->lines[29].line.distance = 2100.0000;
  lines->lines[29].line.angle = 15.0000;
  lines->lines[29].votes = 91;

  lines->lines[30].line.distance = 1215.0000;
  lines->lines[30].line.angle = 200.0000;
  lines->lines[30].votes = 91;

  lines->lines[31].line.distance = 1155.0000;
  lines->lines[31].line.angle = 190.0000;
  lines->lines[31].votes = 89;

  lines->lines[32].line.distance = 1125.0000;
  lines->lines[32].line.angle = 195.0000;
  lines->lines[32].votes = 88;

  lines->lines[33].line.distance = 1185.0000;
  lines->lines[33].line.angle = 185.0000;
  lines->lines[33].votes = 87;

  lines->lines[34].line.distance = 2085.0000;
  lines->lines[34].line.angle = 15.0000;
  lines->lines[34].votes = 84;

  lines->lines[35].line.distance = 2070.0000;
  lines->lines[35].line.angle = 15.0000;
  lines->lines[35].votes = 82;

  lines->lines[36].line.distance = 1200.0000;
  lines->lines[36].line.angle = 200.0000;
  lines->lines[36].votes = 81;

  lines->lines[37].line.distance = 1230.0000;
  lines->lines[37].line.angle = 200.0000;
  lines->lines[37].votes = 81;

  lines->lines[38].line.distance = 2100.0000;
  lines->lines[38].line.angle = 5.0000;
  lines->lines[38].votes = 81;

  lines->lines[39].line.distance = 2700.0000;
  lines->lines[39].line.angle = 95.0000;
  lines->lines[39].votes = 79;

  lines->lines[40].line.distance = 2115.0000;
  lines->lines[40].line.angle = 15.0000;
  lines->lines[40].votes = 78;

  lines->lines[41].line.distance = 1965.0000;
  lines->lines[41].line.angle = 55.0000;
  lines->lines[41].votes = 78;

  lines->lines[42].line.distance = 1215.0000;
  lines->lines[42].line.angle = 185.0000;
  lines->lines[42].votes = 78;

  lines->lines[43].line.distance = 1140.0000;
  lines->lines[43].line.angle = 185.0000;
  lines->lines[43].votes = 78;

  lines->lines[44].line.distance = 1260.0000;
  lines->lines[44].line.angle = 185.0000;
  lines->lines[44].votes = 77;

  lines->lines[45].line.distance = 1980.0000;
  lines->lines[45].line.angle = 60.0000;
  lines->lines[45].votes = 75;

  lines->lines[46].line.distance = 2685.0000;
  lines->lines[46].line.angle = 105.0000;
  lines->lines[46].votes = 75;

  lines->lines[47].line.distance = 1155.0000;
  lines->lines[47].line.angle = 185.0000;
  lines->lines[47].votes = 74;

  lines->lines[48].line.distance = 2055.0000;
  lines->lines[48].line.angle = 15.0000;
  lines->lines[48].votes = 72;

  lines->lines[49].line.distance = 2685.0000;
  lines->lines[49].line.angle = 95.0000;
  lines->lines[49].votes = 72;
}

int main(int argc,char **argv)
{
  lines_data lines, filtered_lines;
  create_test_lines(&lines);

  filter_lines(&lines, &filtered_lines);
  hough_print_lines_data(&filtered_lines);

  exit(0);
}
