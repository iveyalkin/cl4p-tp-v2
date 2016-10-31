#include "main.h"
#include "sql_utils.h"

using namespace std;
using namespace TgBot;
using namespace claptp;

bool sigintGot = false;

int main(int argCount, char** argVal) {
    if (argCount < 2) {
        printf("No token provided. Please specify the bot token.\n");
        return 0;
    } else {
        printf("Token is: %s\n", argVal[1]);
    }

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
        initSqlite(databseName);

        execSql("");

        Bot bot(argVal[1]);
        initBot(bot, logPrefix);

        LongPoll longPoll(bot);
        longPoll.start();

        printf("Polling...\n");

        while (!sigintGot) {
            longPoll.poll();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }

    closeSqlite();

    return 0;
}

void initBot(Bot& bot, boost::optional<string>& logPrefix) {
    bot.getEvents().onCommand("greeting", [&bot, &logPrefix](Message::Ptr message) {
        bot.getApi().sendMessage(
                message->chat->id,
                (logPrefix ? *logPrefix : string())
                        .append(
                        "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down."
                        )
        );
    });

    bot.getEvents().onCommand("querydb", [&bot](Message::Ptr message) {
        char buffer[256];
        readSampleTable(buffer);
        bot.getApi().sendMessage(
                message->chat->id,
                buffer
        );
        /*execSql("SELECT * FROM SampleTable;",
                [*//*&bot, &message*//*](void *NotUsed, int argc, char **argv, char **azColName) -> int {
                    *//*string result;
                    for(int i = 0; i < argc; i++) {
                        result.append(azColName[i])
                                .append(" = ")
                                .append(argv[i] ? argv[i] : "NULL")
                                .append("\n");
                    }
                    bot.getApi().sendMessage(
                            message->chat->id,
                            result
                    );*//*
                    return SQLITE_OK;
                }
        );*/
    });

    bot.getEvents().onCommand("enough", [&bot, &logPrefix](Message::Ptr message) {
        bot.getApi().sendMessage(
                message->chat->id,
                (logPrefix ? *logPrefix : string())
                        .append(
                        "Oh my God, I'm leaking! I think I'm leaking! Ahhhh, I'm leaking! There's oil everywhere!"
                        )
        );
        sigintGot = true;
    });

     bot.getEvents().onAnyMessage([&bot, &logPrefix](Message::Ptr message) {
         if (!StringTools::startsWith(message->text, "/echo")) {
             return;
         }
         string echoMsg = message->text.substr(strlen("/echo"));
         printf("User wrote %s\n", echoMsg.c_str());
         bot.getApi().sendMessage(
                 message->chat->id,
                 (logPrefix ? *logPrefix : string())
                         .append("You hear an echo: ").append(echoMsg)
         );
     });
}