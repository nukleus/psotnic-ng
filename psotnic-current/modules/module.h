// descr: this function registers custom constructor and destructor for
//        given `Class', function returns 1 if `Class' supports custom
//        constructors and destructors, otherwise 0 is returned, please
//        check framework.h for classes supporting custom constructors
//        destructors
//
// example: see repeat.cpp

int initCustomData (const char *Class, FUNCTION constructor, FUNCTION destructor);


// desc: tells the bot that it should not do any further parsing
void stop ();
