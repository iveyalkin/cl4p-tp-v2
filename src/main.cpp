#include <iostream>
#include "tgbot/tgbot.h"
#include "LongPoll.h"

using namespace std;
using namespace TgBot;
using namespace claptp;


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

        LongPoll longPoll(bot);
        longPoll.start();

        printf("Polling...\n");

        while (!sigintGot) {
            longPoll.poll();
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

    bot.getEvents().onCommand("enough", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(
                message->chat->id,
                "Oh my God, I'm leaking! I think I'm leaking! Ahhhh, I'm leaking! There's oil everywhere!"
        );
        sigintGot = true;
    });

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        if (!StringTools::startsWith(message->text, "/echo")) {
            return;
        }
        string echoMsg = message->text.substr(strlen("/echo"));
        printf("User wrote %s\n", echoMsg.c_str());
        bot.getApi().sendMessage(message->chat->id, "You hear echo: " + echoMsg);
    });
}