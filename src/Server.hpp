#ifndef SERVER_HPP
#define SERVER_HPP 

#include <map>
#include <string>

using std::map;
using std::string;

/*! Stores server information.
 * \author patrick <patrick@psotnic.com>
 * \warning The values are only available when the bot is connected to an ircserver
 */
class Server
{
        public:
	// from 004
	char *name;		//!< name of the server
	char *version;		//!< ircd version
	char *usermodes;	//!< available user modes
	char *chanmodes;	//!< available channel modes (please prefer isupport.chanmodes)

	// from 005
	class Isupport
	{
		public:
		typedef map<string, string> isupportType;
		isupportType isupport_map;		//!< contains all 005 tokens
		Server *server;			//!< pointer to upper class

		/* the following variables are in the map too,
		 * but either they are used very often, so the bot should not search
		 * for them in map everytime or the value is not accessable without further parsing.
		 */

		char *chan_status_flags;		//!< specifies a list of channel status flags (usually: "ov")
		char *chanmodes;			//!< indicates the channel modes available and the arguments they take (format: "A,B,C,D")
		int maxchannels;			//!< maximum number of chans a client can join
		int maxlist;				//!< limits how many "variable" modes of type A a client may set in total on a channel
		int max_kick_targets;			//!< maximum number of users that can be kicked with one KICK command
		int max_who_targets;			//!< maximum number of targets that are allowed in a WHO command
		int max_mode_targets;			//!< maximum number of targets (channels and users) that are allowed in a MODE command e.g. MODE #chan1,#chan2,#chan3

		Isupport();
		~Isupport();
		void insert(const char *key, const char *value);
		const char *find(const char *key);
		void reset();
		void init();
	} isupport;

	Server();
	~Server();
	void reset();
};

#endif /* SERVER_HPP */
