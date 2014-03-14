#ifndef MAINMENU_H
#define MAINMENU_H

extern MenuLayer *mainMenu_layer;
extern MenuLayerCallbacks mainMenu_callbacks;

extern char stops[];
extern char distances[];

extern char numBuses[];
extern char menuIdx;

void mainMenu_load();

#endif /* MAINMENU_H */
