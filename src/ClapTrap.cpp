//
// Created by Basil Terkin on 10/31/16.
//

#include "ClapTrap.h"
#include "stdio.h"

#define CHAT_ID message->chat->id

using namespace TgBot;
using namespace ClapTp;

std::string replaceMe(User::Ptr user) {
    if (user->firstName.empty()) {
        return user->username;
    } else if (user->lastName.empty()) {
        return user->firstName;
    } else {
        return std::string(user->firstName).append(" ").append(user->lastName);
    }
}

ClapTrap::ClapTrap(char *token, SqlWrapper &sqlWrapper, std::string &logPrefix, std::string &debugChatId)
        : Bot(token), _dbInstance(sqlWrapper), _logPrefix(logPrefix), _debugChatId(debugChatId) {

    getEvents().onCommand("greeting", [this](Message::Ptr message) {
        sendGreeting(CHAT_ID);
    });

    getEvents().onCommand("querydb", [this](Message::Ptr message) {
        char buffer[1024 * 10];
        _dbInstance.fetchUrls(buffer);
        getApi().sendMessage(CHAT_ID, buffer);
    });

    getEvents().onCommand("enough", [this](Message::Ptr message) {
        replyToChat(CHAT_ID,
                    "Oh my God, I'm leaking! I think I'm leaking! Ahhhh, I'm leaking! There's oil everywhere!");

        _shoudlShutdown = true;
    });

    getEvents().onCommand("echo", [this](Message::Ptr message) {
        std::string echoMsg = message->text.substr(strlen("/echo"));
        replyToChat(CHAT_ID,
                    std::string("You hear an echo: ").append(echoMsg));
    });

    getEvents().onCommand("stash", [this](Message::Ptr message) {
        std::string reply;
        if (message->replyToMessage)
        {
            if (message->replyToMessage->text.empty())
                reply = std::string("I can only stash a text messages!");
            else // TODO: store reply message text in DB for message->from->id key
                reply = std::string("Going to stash: \"")
                                    .append(message->replyToMessage->text)
                                    .append("\" for you in my personal Vault!");
        }
        else
        {
            reply = std::string("There is no quote in your message!");
        }
        replyToChat(CHAT_ID, reply);
    });

    getEvents().onCommand("unstash", [this](Message::Ptr message) {
        // TODO: load all message stored for message->from->id user
        // remove them from DB and send to chat
        replyToChat(CHAT_ID, "Did you really think I will store your junk!? Poor minion...");
    });

    getEvents().onCommand("me", [this](Message::Ptr message) {
        std::vector<MessageEntity::Ptr>::const_iterator
                itr = message->entities.begin(),
                end = message->entities.end();
        std::string messageText;
        while (itr != end) {
            if (!(*itr)->type.compare("bot_command")) {
                if (!message->text.substr((*itr)->offset, (*itr)->length).compare("/me")) {
                    std::string restText(message->text.substr((*itr)->length));
                    if ((*itr)->offset > 0) {
                        messageText = std::string(message->text.substr(0, (*itr)->offset));
                    }
                    messageText
                            .append(replaceMe(message->from))
                            .append(restText);

                    replyToChat(CHAT_ID, messageText);
                    return;
                }
            }
            itr++;
        }
    });

    getEvents().onAnyMessage([this](Message::Ptr message) {
        std::vector<MessageEntity::Ptr>::const_iterator
                itr = message->entities.begin(),
                end = message->entities.end();
        _dbInstance.saveUser(message->from);
        while (itr != end) {
            if (!(*itr)->type.compare("url")) {
                MessageEntity::Ptr entity = *itr;
                std::string url = message->text.substr(entity->offset, entity->length);
                std::string description;
                if (entity->offset > 0) {
                    description.append(message->text.substr(0, entity->offset));
                }
                description.append(message->text.substr(entity->offset + entity->length));
                std::printf("Found a link [%s]. Saving...\n", url.c_str());
                _dbInstance.saveUrl(message->from->id, url, description);
            }
            itr++;
        }
    });
}

void ClapTrap::replyToChat(int64_t chatId, const std::string &message) const {
    auto prefix(_logPrefix);
    getApi().sendMessage(chatId,
                        prefix.append(message));
}

bool ClapTrap::shouldShutdown() {
    return _shoudlShutdown;
}

void ClapTrap::sendGreeting(int64_t chatId) const {
    replyToChat(chatId, "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down.");
}

const std::string &ClapTrap::getDebugChatId() const {
    return _debugChatId;
}

bool ClapTrap::isDebug()const {
    return _debugChatId.length() > 0;
}
