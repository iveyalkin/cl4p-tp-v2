/*
 * Copyright (c) 2015 Oleg Morozenkov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Extended by Ihar Veyalkin
 * - added start() method to adjust _lastUpdateId
 */

#ifndef CLAPTP_LONGPOLL_H
#define CLAPTP_LONGPOLL_H

#include "tgbot/Bot.h"
#include "tgbot/Api.h"
#include "tgbot/EventHandler.h"

using namespace TgBot;

namespace ClapTp {

/**
 * This class handles long polling and updates parsing.
 */
class LongPoll {

public:
	LongPoll(const Api* api, const EventHandler* eventHandler);
	LongPoll(const Bot& bot);

	/**
	 * Initial poll. Initializes update counter.
	 */
	void start();

	/**
	 * Step of a long poll. After new update will come, this method will parse it and send to EventHandler which invokes your listeners. Designed to be executed in a loop.
	 */
	void poll();

private:
	int32_t _lastUpdateId = 0;
	const Api* _api;
	const EventHandler* _eventHandler;
};

}

#endif //CLAPTP_LONGPOLL_H
