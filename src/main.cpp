#include "stdio.h"
#include "SqlWrapper.h"
#include "ClapTrap.h"
#include "LongPoll.h"

using namespace std;
using namespace TgBot;
using namespace ClapTp;

bool sigintGot = false;

int main(int argCount, char** argVal) {

    const char* DATABASE_NAME = "cl4ptp.db";

    if (argCount < 2) {
        printf("No token provided. Please specify the bot token.\n");
        return 0;
    } else {
        printf("Token is: %s\n", argVal[1]);
    }

    setbuf(stdout, NULL);

    signal(SIGINT, [](int s) {
		printf("SIGINT got\n");
		sigintGot = true;
	});

    boost::optional<std::string> logPrefix;

    if (argCount > 2) {
        printf("Log tag is: %s\n", argVal[2]);
        logPrefix = string(argVal[2]).append(": ");

        for(int i = 3; i < argCount; i++) {
            printf("Arg[%d] is: %s\n", i, argVal[i]);
        }
    }

    try {
        SqlWrapper sqlWrapper(DATABASE_NAME);
        ClapTrap bot(argVal[1], logPrefix, sqlWrapper);
        LongPoll longPoll(bot);

        longPoll.start();

        printf("Polling...\n");
        while (!sigintGot) {
            longPoll.poll();
            if (bot.shouldShutdown()) {
                break;
            }
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}