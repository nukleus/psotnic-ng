#ifndef PSOTNIC_DEFINES_H
#define PSOTNIC_DEFINES_H 1

//channel flags
#define FLAG_I					0x00000001
#define FLAG_N					0x00000002
#define FLAG_S					0x00000004
#define FLAG_M					0x00000008
#define FLAG_T					0x00000010
#define FLAG_R					0x00000020
#define FLAG_P					0x00000040
#define FLAG_Q					0x00000080

#define SENT_I					0x00000100
#define SENT_N					0x00000200
#define SENT_S					0x00000400
#define SENT_M					0x00000800
#define SENT_T					0x00001000
#define SENT_R					0x00002000
#define SENT_P					0x00004000
#define SENT_Q					0x00008000

#define SENT_MINUS_I			0x00010000
#define SENT_MINUS_N			0x00020000
#define SENT_MINUS_S			0x00040000
#define SENT_MINUS_M			0x00080000
#define SENT_MINUS_T			0x00100000
#define SENT_MINUS_R			0x00200000
#define SENT_MINUS_P			0x00400000
#define SENT_MINUS_Q			0x00800000
#define SENT_MINUS_L			0x01000000

#define CRITICAL_LOCK			0x02000000

#define CHFLAGS_SENT_ALL		0x01FFFF00

//user flags
#define HAS_A					0x00000001
#define HAS_D					0x00000002
#define HAS_O					0x00000004
#define HAS_F					0x00000008
#define HAS_M					0x00000010
#define HAS_N					0x00000020
#define HAS_S					0x00000040
#define HAS_X					0x00000080
#define HAS_V					0x00000100
#define HAS_Q					0x00000200
#define HAS_R					0x00000400
#define HAS_K					0x00000800
#define HAS_I					0x00001000
#define HAS_E					0x00002000
#define HAS_C					0x00004000
#define HAS_ALL					0x00008000 - 1

//                              0x00008000
//          					0x00010000
#define HAS_B                 	0x00020000
#define HAS_L					0x00040000
#define HAS_H					0x00080000
//                              0x00100000
//                              0x00200000
//                              0x00800000
//                              0x01000000
//                              0x02000000
//                              0x04000000
#define HAS_P					0x08000000
#define HAS_Z					0x10000000
#define CHECK_SHIT				0x20000000
//								0x40000000
//								0x80000000

#define B_FLAGS					HAS_B | HAS_O | HAS_A | HAS_F | HAS_I | HAS_R | HAS_C | HAS_E
#define F_FLAGS					HAS_O | HAS_F
#define M_FLAGS					F_FLAGS | HAS_M | HAS_I
#define N_FLAGS					M_FLAGS | HAS_N | HAS_C | HAS_R
#define S_FLAGS					N_FLAGS | HAS_S | HAS_P | HAS_E
#define X_FLAGS					S_FLAGS | HAS_X

#define IS_OP                 	0x00100000
#define KICK_SENT				0x00200000
#define OP_SENT					0x00400000
#define IS_VOICE              	0x00800000
#define VOICE_SENT				0x01000000
//#define DEOP_SENT				0x02000000
#define NET_JOINED				0x04000000
#define IS_LUSER		0x08000000


//dns info
#define	HOST_IPV4				0x00000001
#define HOST_DOMAIN				0x00000002
#define HOST_IPV6				0x00000004

//clone check types
#define CLONE_IPV4				0x00000001
#define CLONE_HOST				0x00000002
#define CLONE_IPV6				0x00000004	
#define CLONE_PROXY				0x00000008
#define CLONE_IDENT				0x00000010

//channel stuff
#define ARG_MODES				"oblkeIvR"
#define NONARG_MODES			"imntspr"
#define CHATTR_MODES                    "imntsprlk"

//inetconn flags
#define STATUS_CONNECTED		0x00000001
#define STATUS_REGISTERED		0x00000002
#define STATUS_SYNSENT			0x00000004
#define STATUS_PARTY			0x00000008
#define STATUS_BOT				0x00000010
#define STATUS_SILENT			0x00000020
#define STATUS_KLINED			0x00000040
#define STATUS_TELNET			0x00000080
#define STATUS_REDIR			0x00000100
#define STATUS_FILE				0x00000200
#define STATUS_SOCKS5			0x00000400
#define STATUS_SOCKS5_CONNECTED	0x00000800
#define STATUS_NOECHO			0x00001000
#define STATUS_BNC				0x00002000
#define STATUS_211				0x00004000
#define STATUS_ROUTER			0x00008000
#define STATUS_SSL				0x00010000
#define STATUS_SSL_HANDSHAKING	0x00020000
#define STATUS_SSL_WANT_CONNECT 0x00040000
#define STATUS_SSL_WANT_ACCEPT  0x00080000
#define	STATUS_ULCRYPT			0x80000000
#define STATUS_NEED_WHOIS                  0x00100000


//chanlist flags
#define SET_TOPIC				0x00000001
#define JOIN_SENT				0x00000002
#define PRIVATE_CHAN			0x00000004
#define WHO_SENT				0x00000008

//mode queue
#define PRIO_LOW				0
#define PRIO_HIGH				1

//other shit
#define MAX_LEN					4096
#define MAX_SERVERS				64
#define MAX_CHANNELS			32
#define GLOBAL					MAX_CHANNELS
#define MAX_HOSTS				64
#define MD5_HEXDIGEST_LEN		32
#define AUTHSTR_LEN				32
#define MAX_INT					0x7fffffff
#define MODES_PER_LINE			10
#define MAX_HANDLE_LEN			15
#define SAVEDELAY				10
#define MAX_ALTS				16
#define MAX_MODULES				16
#define MAX_ALIASES				32

//#define RESOLV_TH				4
#define MAX_OFFENCES				16
#define OFFENCE_DUPLICATE_TIME			10 /* secs */
#define MAX_WHOIS_OFFENCES			3 /* number of offences showed in 'whois' */
//#define RERSTART_AFTER_UPDATE			1 /* if this is defined bot restart after succesufull update */

//used in str2args function for detecting BEGIN and END of argument
#define BEGIN_ARG_CHAR				'"'
#define END_ARG_CHAR				'"' 

#define CHAN_LEN				50

#define BOT_LEAF				1
#define BOT_SLAVE				2
#define BOT_MAIN				3

#define MK_OPS					1
#define MK_NONOPS				2
#define MK_ALL					3

#define BAN						0
#define EXEMPT					1
#define INVITE					2
#define REOP					3

#define IRC_BUFS				30

#ifndef TCP_NODELAY
	#define TCP_NODELAY				1
#endif

#define S_UL_UPLOAD_START		"00"
#define S_UL_UPLOAD_END			"01"
#define S_CHNICK				"02"
#define S_REGISTER				"03"
#define S_SECRET				"04"
#define S_INVITE				"05"
#define S_KEY					"06"
#define S_KEYRPL				"07"
#define S_REOP					"08"
#define S_CYCLE					"09"

#define S_ADDUSER				"11"
#define S_ADDHOST				"12"
#define S_ADDBOT				"13"
#define S_ADDCHAN				"14"

#define S_RMUSER				"15"
#define S_RMHOST				"16"
#define S_RMBOT					"17"
#define S_RMCHAN				"18"

#define S_CHATTR				"19"
#define S_ULSAVE				"20"
#define S_SN					"21"
#define S_SET					"22"
#define S_CHSET					"23"
#define S_GCHSET				"24"
#define S_GETOP					"25"
#define S_OP					"26"
#define S_BOP					"27"
#define S_FOO					"28"
#define S_NICK					"29"
#define S_PASSWD				"30"
#define S_MKO					"31"
#define S_MKN					"32"
#define S_MKA					"33"
#define S_LIST					"34"
//#define S_REDIR					"35"
#define S_OREDIR				"36"
#define S_BJOIN					"37"
#define S_BQUIT					"38"
#define S_ADDR					"39"
#define S_ULOK					"40"
#define S_UNLINK				"41"
#define S_JUMP					"42"
#define S_JUMP6					"43"
#define S_COMEON				"44"
#define S_BIDLIMIT				"45"
#define S_UNBANME				"46"
#define S_RESET					"47"
#define S_RDIE					"48"
#define S_NAMES					"49"
//#define S_PSOTGET				"50"
#define S_RESTART				"51"
#define S_RJOIN					"52"
#define S_RPART					"53"
#define S_JUMPS5				"54"
#define S_PROXYHOST				"55"
#define S_CHHANDLE				"56"
#define S_ADDINFO				"57"
#define S_ADDSHIT				"58"
#define S_RMSHIT				"59"
#define S_BOTCMD				"60"
#define S_IUSEMODULES			"61"
#define S_FORWARD				"62"
#define S_LISTRPL				"63"
#define S_STATUS				"64"
#define S_CHKHOST				"65"
#define S_ADDSTICK				"66"
#define S_PSOTUPDATE			"67"
#define S_STOPUPDATE			"68"
#define S_REQSHIT				"69"
#define S_DSET					"70"
#define S_ADDINVITE				"71"
#define S_RMINVITE				"72"
#define S_ADDEXEMPT				"73"
#define S_RMEXEMPT				"74"
#define S_ADDREOP				"75"
#define S_RMREOP				"76"
#define S_ADDOFFENCE				"77"
#define S_ADDIDIOT				"78"
#define S_CWHO					"79"
#define S_SHITOBSERVED			"80"
#define S_LASTUSED_PROTMODE		"81"

#define S_NOPERM				strerror(EACCES)
#define S_VERSION				"0.2.14rc1" 

#define S_BOTNAME				"psotnic"
#define S_COPYRIGHT				"Copyright (C) 2003-2007 Grzegorz Rusin <grusin@gmail.com>"

#define DIE_REASON				"Taking the blue pill"
#define DIE_REASON2				"told me to take the blue pill"

#define	bk						printf("### %s(): %s:%d\n", __FUNCTION__, __FILE__, __LINE__)
#define ircstrip(str)			(str[0] == ':') ? (str+1) : (str)
#define strcmp(s1, s2)			ircd_strcmp(s1, s2)
#define strncmp(s1, s2, n)		ircd_strncmp(s1, s2, n)
#define strcasecmp(s1, s2)		ircd_strcmp(s1, s2)
#define strncasecmp(s1, s2, n)	ircd_strncmp(s1, s2, n)

#define VER_NONE		0
#define VER_PSOTNIC		1
#define VER_IRSSI		2
#define VER_EPIC		3
#define VER_LICE		4
#define VER_BITCHX		5
#define VER_DZONY		6
#define VER_LUZIK		7
#define VER_MIRC		8

#ifdef HAVE_DEBUG
	#define DEBUG(x)	if(debug) x
#else
	#define DEBUG(x)
#endif

#define foreachSyncedChannel(ch)				\
	for(ch=ME.first; ch; ch=ch->next)			\
		if(ch->synced())

#define foreachNamedChanlist(chLst)						\
	for(int _i=0; _i<MAX_CHANNELS; ++_i)			\
		if((chLst = &userlist.chanlist[_i])->name)

#define foreachMatchingChanlist(chLst, mask)					\
	for(int _i=0; _i<MAX_CHANNELS; ++_i)							\
		if((chLst = &userlist.chanlist[_i])->name && ::match(mask, chLst->name))

typedef int* FUNCTION;
typedef unsigned int(*DLSYM_FUNCTION)();
typedef void* DLSYM_OBJECT;

#ifdef HAVE_MODULES
#define HOOK(__name, __fun)									\
{															\
	ptrlist<module>::iterator _i = modules.begin();			\
	while(_i)												\
	{														\
		if(_i->hooks->__name)								\
			_i->hooks->__fun;								\
		_i++;												\
	}														\
}
#else
#define HOOK(x, y)
#endif

#ifndef NS_IN6ADDRSZ
	#define NS_IN6ADDRSZ 16
#endif

//AIX compatibility
#ifndef hstrerror
	#define hstrerror(x) "unknown error"
#endif

#ifndef bzero
    #define bzero(a, b) memset(a, 0, b)
#endif
	
#ifndef MAX
	#define MAX(A, B) (A) > (B) ? (A) : (B)
#endif

#ifndef MIN
	#define MIN(a,b) (a) > (b) ? (b) : (a)
#endif
#endif
