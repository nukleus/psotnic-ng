#ifndef PSOTNIC_CLASSES_H
#define PSOTNIC_CLASSES_H 1

class chan;
class fifo;
class inetconn;
class inet;
class adns;
class chanuser;
class ent;
class CONFIG;
class client;
class CBlowFish;

struct HANDLE;

#include <map>
#include <netinet/in.h> // AF_INET
#include <string>

using std::map;
using std::string;

#include "class-ent.h"
#include "config.h"
#include "CustomDataStorage.hpp"
#include "fastptrlist.h"
#include "grass.h"
#include "hashlist.h"
#include "Inetconn.hpp"
#include "Modeq.hpp"
#include "Protmodelist.hpp"
#include "pstring.h"
#include "structs.h"

#endif
