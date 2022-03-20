#include <assert.h>
#include <concord/discord.h>

#include "../commands.h"

void on_ready(struct discord *client) {
  const struct discord_user *bot = discord_get_self(client);
  log_info("Connected to Discord as %s#%s", bot->username, bot->discriminator);
}

int main(int argc, char *argv[]) {
  ccord_global_init();

  struct discord *client =
      discord_config_init((argc > 1) ? argv[1] : "config.json");
  assert(client != NULL && "Could not initialize client");

  discord_set_on_ready(client, &on_ready);

  commands_data_init(discord_get_self(client));
  commands_set(client);

  discord_run(client);

  discord_cleanup(client);
  ccord_global_cleanup();
  commands_free();
}
