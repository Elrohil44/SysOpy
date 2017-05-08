#ifndef BARBER_H
#define BARBER_H

#define COMMON_S 255
#define COMMON_S2 254
#define SEM_PRIMARY 1
#define SEMS 2
#define SEM_ROOM 3

struct waiting_room
{
  int sleeping;
  int seat;
  int count;
  int taken;
  int first;
};


#endif
