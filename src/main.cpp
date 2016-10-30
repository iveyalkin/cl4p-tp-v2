#include <iostream>
#include "tgbot/tgbot.h"

using namespace std;
using namespace TgBot;

bool sigintGot = false;

void initBot(Bot&);

int main(int argCount, char** argVal) {
    if (argCount < 2) {
        printf("No token provided. Please specify the bot token.\n");
        return 0;
    }

    signal(SIGINT, [](int s) {
		printf("SIGINT got");
		sigintGot = true;
	});

    try {
        Bot bot(argVal[1]);
        initBot(bot);

        TgLongPoll longPoll(bot);
        printf("Polling...\n");
        while (!sigintGot) {
            longPoll.start();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }
    
    return 0;
}

void initBot(Bot& bot) {

    bot.getEvents().onCommand("greeting", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(
                message->chat->id,
                "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down."
        );
    });

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });
}