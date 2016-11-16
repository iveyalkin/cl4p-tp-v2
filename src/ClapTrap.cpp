//
// Created by Basil Terkin on 10/31/16.
//

#include "ClapTrap.h"
#include "stdio.h"
#include <codecvt>

#define CHAT_ID message->chat->id

using namespace TgBot;
using namespace ClapTp;

std::string printUser(User::Ptr user) {
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

    getEvents().onCommand("wtf", [this](Message::Ptr message) {
        std::string reply;
        if (message->replyToMessage)
        {
            if (message->replyToMessage->text.empty()) {
                reply = std::string("Donno what you're talking about!");
            } else {

                reply = *flipQwerty(message->replyToMessage->text);
            }
        }
        else
        {
            reply = std::string("I need an original message in reply.");
        }
        replyToChat(CHAT_ID, reply, false, message->replyToMessage->messageId);
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
            {
                reply = std::string("I can only stash a text messages!");
            }
            else
            {
                _dbInstance.saveStash(message->from, message->replyToMessage->text);
                reply = std::string("Stashed!");
            }
        }
        else
        {
            reply = std::string("There is no quote in your message!");
        }
        replyToChat(CHAT_ID, reply);
    });

    getEvents().onCommand("unstash", [this](Message::Ptr message) {
        if (!message->from)
            return;

        auto stash = _dbInstance.loadStash(message->from);

        std::stringstream reply;

        if (stash.empty())
        {
            reply << "Your stash is empty!";
        }
        else
        {
            uint messagesToReply = std::min(3u, (uint)stash.size());
            _dbInstance.removeNumStashMessages(message->from, messagesToReply);

            reply << "Here's your stashed messages:\n";
            for (uint i = 0; i < messagesToReply; ++i)
            {
                reply << "\n" << i+1 << ") " << stash[i] << "\n";
            }

            if (stash.size() > messagesToReply)
            {
                reply << "\nAnd there's " << stash.size() - messagesToReply << " more left!";
            }
        }
        replyToChat(CHAT_ID, reply.str());
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
                            .append(printUser(message->from))
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

void ClapTrap::replyToChat(int64_t chatId, const std::string &message, bool disableWebPagePreview, int32_t replyToMessageId) const {
    auto prefix(_logPrefix);
    getApi().sendMessage(chatId,
                         prefix.append(message),
                         disableWebPagePreview,
                         replyToMessageId
    );
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

std::shared_ptr<std::string> ClapTrap::flipQwerty(std::string &originalText) {
    wchar_t rus[] {L'й', L'ц', L'у', L'к', L'е', L'н', L'г', L'ш', L'щ', L'з', L'х', L'ъ', L'ф', L'ы', L'в', L'а', L'п', L'р', L'о', L'л', L'д', L'ж', L'э', L'я', L'ч', L'с', L'м', L'и', L'т', L'ь', L'б', L'ю', L'Й', L'Ц', L'У', L'К', L'Е', L'Н', L'Г', L'Ш', L'Щ', L'З', L'Х', L'Ъ', L'Ф', L'Ы', L'В', L'А', L'П', L'Р', L'О', L'Л', L'Д', L'Ж', L'Э', L'Я', L'Ч', L'С', L'М', L'И', L'Т', L'Ь', L'Б', L'Ю', L',', L'ё', L'Ё'};
    wchar_t eng[] {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'"[0], 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '`', '~'};

    wchar_t output[originalText.length()];

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wUtfString = converter.from_bytes(originalText);
    wcscpy(output, wUtfString.c_str());

    for (int i = 0, size = (int) wUtfString.length(); i < size; i++) {
        wchar_t &at = wUtfString.at((unsigned long) i);
        for (int k = 0, rusSize = sizeof(rus) / sizeof(wchar_t); k < rusSize; k++){
            if (at == eng[k]) {
                output[i] = rus[k];
                break;
            } else if (at == rus[k]) {
                output[i] = eng[k];
                break;
            }
        }
    }

    const std::string &string = converter.to_bytes(output);
    return std::shared_ptr<std::string>(new std::string(string));
}
