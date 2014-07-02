#ifndef ROUTEBUSMENU_H
#define ROUTEBUSMENU_H

extern MenuLayer *routeBusMenu_layer;

#define LINENAME_LEN 16
extern char lineName[];

extern char stops[];

extern char distances[];

extern char numBuses[];

extern char routes[];
extern int numRoutes;

void routeBusMenu_load();

#endif /* ROUTEBUSMENU_H */
