
typedef void* luatype;

// i dont want to cast to void* each time ....
// a drawback is thats these are now normal static variables wich are initialised at initialisation time (you shoudnt use these at/before initialisation time).



extern luatype const dlgclbkKey;
extern luatype const executeKey;
extern luatype const getsideKey;
extern luatype const gettextKey;
extern luatype const gettypeKey;
extern luatype const getraceKey;
extern luatype const getunitKey;
extern luatype const tstringKey;
extern luatype const unitvarKey;
extern luatype const ustatusKey;
extern luatype const vconfigKey;