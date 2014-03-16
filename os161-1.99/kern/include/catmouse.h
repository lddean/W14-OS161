
/* TBB: Created because newer compilers want prototypes for these functions */

/* For the catmouse problem */
/* this must be called before any calls to cat_eat or mouse_eat */

//struct lock* initialBowel(int x);
struct lock* returnLock(int x);

void cat_simulation(void * unusedpointer,
               unsigned long catnumber);
void
mouse_simulation(void * unusedpointer,
          unsigned long mousenumber);

int
catmouse(int nargs,
         char ** args);



