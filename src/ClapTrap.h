//
// Created by Basil Terkin on 10/31/16.
//

#ifndef CL4PTP_CLAPTRAP_H
#define CL4PTP_CLAPTRAP_H

#include <string>
#include "tgbot/Bot.h"
#include "tgbot/types/Message.h"
#include "SqlWrapper.h"

namespace ClapTp {

    class ClapTrap : public TgBot::Bot {

    public:
        ClapTrap(char *token, SqlWrapper &sqlWrapper, std::string &logPrefix, std::string &debugChatId);

        bool shouldShutdown();

        void sendGreeting(int64_t chatId) const;

        bool isDebug() const;

        const std::string &getDebugChatId() const;

    protected:
        void replyToChat(int64_t chatId, const std::string &message) const;

    private:
        std::string _logPrefix;

        std::string _debugChatId;

        SqlWrapper _dbInstance;

        bool _shoudlShutdown = false;
    };
}


#endif //CL4PTP_CLAPTRAP_H
