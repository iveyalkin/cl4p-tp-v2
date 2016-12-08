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
        void replyToChat(int64_t chatId, const std::string &message, bool disableWebPagePreview = false, int32_t replyToMessageId = 0) const;

        std::shared_ptr<std::string> flipQwerty(std::string &originalText);

    private:
        SqlWrapper &_dbInstance;

        std::string _logPrefix;

        std::string _debugChatId;

        bool _shoudlShutdown = false;
    };
}


#endif //CL4PTP_CLAPTRAP_H
