#ifndef _TEST_MENU_H_
#define _TEST_MENU_H_


typedef void (*InitCmdProc)(void);
typedef void (*MainLoopCallback)(void);

extern MainLoopCallback g_mainloop_callback;

extern void initMenu(void);
extern void returnMenu(void);


#endif
