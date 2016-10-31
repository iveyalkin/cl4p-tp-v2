//
// Created by Basil Terkin on 10/30/16.
//

#ifndef CL4PTP_UTILS_H
#define CL4PTP_UTILS_H

#include "tgbot/tgbot.h"
#include "LongPoll.h"

const char* databseName = "cl4ptp.db";

void initBot(Bot& bot, boost::optional<std::string>& anOptional);

#endif //CL4PTP_UTILS_H
