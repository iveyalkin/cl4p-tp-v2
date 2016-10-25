#include <iostream>
#include "tgbot/tgbot.h"

using namespace std;
using namespace TgBot;

bool sigintGot = false;

int main(int argCount, char** argVal) {
    if (argCount < 2) {
        printf("No token provided. Please specify the bot token.");
        return 0;
    }

    string botToken(argVal[1]);
    Bot bot(botToken);

    bot.getEvents().onCommand("greeting", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(
                message->chat->id,
                "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down."
        );
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());

        TgLongPoll longPoll(bot);

        while (!sigintGot) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }
    
    return 0;
}