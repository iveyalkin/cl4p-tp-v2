#ifndef CLAPTP_LONGPOLL_H
#define CLAPTP_LONGPOLL_H

#include <tgbot/net/TgLongPoll.h>
#include "tgbot/Bot.h"
#include "tgbot/Api.h"
#include "tgbot/EventHandler.h"
#include "ClapTrap.h"

using namespace TgBot;

namespace ClapTp {

/**
 * This class handles long polling and updates parsing.
 */
class LongPoll {

public:

	LongPoll(const ClapTrap& bot);

	/**
	 * Initial poll. Initializes update counter.
	 */
	void init();

	/**
	 * Step of a long poll. After new update will come, this method will parse it and send to EventHandler which invokes your listeners. Designed to be executed in a loop.
	 */
	void poll();

private:

	int32_t _lastUpdateId = 0;

	const Api* _api;

	const EventHandler* _eventHandler;

	const ClapTrap &_bot;
};

}

#endif //CLAPTP_LONGPOLL_H
