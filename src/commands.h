#ifndef CBALC_COMMANDS_H
#define CBALC_COMMANDS_H

#include <concord/discord.h>
#include <sys/utsname.h>

void commands_data_init(const struct discord_user *bot);
void commands_set(struct discord *client);
void commands_free();

#endif
