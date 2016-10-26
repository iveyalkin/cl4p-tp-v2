#include <iostream>
#include "tgbot/tgbot.h"

using namespace std;
using namespace TgBot;

bool sigintGot = false;

Bot initBot(string);

int main(int argCount, char** argVal) {
    if (argCount < 2) {
        printf("No token provided. Please specify the bot token.");
        return 0;
    }

    try {
        TgLongPoll longPoll(initBot(string(argVal[1])));
        printf("Polling...\n");
        while (!sigintGot) {
            longPoll.start();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }
    
    return 0;
}

Bot initBot(string botToken) {
    Bot bot(botToken);

    bot.getEvents().onCommand("greeting", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(
                message->chat->id,
                "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down."
        );
    });

    return bot;
}