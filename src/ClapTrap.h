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
        ClapTrap(
                const std::string &token,
                const std::string &logPrefix,
                SqlWrapper& sqlWrapper
        );

        bool shouldShutdown();

    protected:
        void replyToChat(int64_t chatId, const std::string &message);

    private:
        std::string _logPrefix;

        SqlWrapper _dbInstance;

        bool _shoudlShutdown = false;
    };
}


#endif //CL4PTP_CLAPTRAP_H
