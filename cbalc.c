#include <assert.h>
#include <concord/discord.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/utsname.h>

struct command {
  char *command;
  char *usage;
  char *description;
  discord_ev_message callback;
};

char *avatar_url;

struct timeval start;

char system_info[_UTSNAME_SYSNAME_LENGTH + _UTSNAME_RELEASE_LENGTH];

char *fetch_avatar_url(char *url, const u64snowflake id, const char *avatar) {
  static const char avatar_base_url[] = "https://cdn.discordapp.com/avatars/";
  char id_buf[22];

  sprintf(id_buf, "%" PRIu64 "/", id);
  url = malloc(strlen(avatar_base_url) + strlen(id_buf) + strlen(avatar) + 1);
  url[0] = '\0';
  strcat(url, avatar_base_url);
  strcat(url, id_buf);
  strcat(url, avatar);
  return url;
}

void on_ready(struct discord *client) {
  const struct discord_user *bot = discord_get_self(client);
  log_info("Connected to Discord as %s#%s", bot->username, bot->discriminator);
}

void on_ping(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  struct discord_create_message params = {.content = "pong"};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

void on_echo(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  struct discord_create_message params = {.content = msg->content};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

void on_status(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  char start_time[13];
  strftime(start_time, sizeof(start_time), "%b %d %H:%M",
           localtime(&start.tv_sec));

  struct discord_embed_field fields[] = {
      {.name = "System", .value = system_info},
      {.name = "Start Time", .value = start_time},
  };

  struct discord_embed embeds[] = {{
      .title = "cbalc Status",
      .image = &(struct discord_embed_image){.url = avatar_url},
      .fields =
          &(struct discord_embed_fields){
              .size = sizeof(fields) / sizeof *fields,
              .array = fields,
          },
  }};

  struct discord_create_message params = {
      .embeds =
          &(struct discord_embeds){
              .size = sizeof(embeds) / sizeof *embeds,
              .array = embeds,
          },
      .allowed_mentions = NULL,
  };
  discord_create_message(client, msg->channel_id, &params, NULL);
}

static const struct command commands[] = {
    {.command = "ping", .description = "Send pong", .callback = &on_ping},
    {.command = "echo",
     .usage = "<text>",
     .description = "Display given text",
     .callback = &on_echo},
    {.command = "status",
     .description = "Get status of bot",
     .callback = &on_status},
};
static const size_t commands_size = sizeof(commands) / sizeof(struct command);

static char *help_content = NULL;

void on_help(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  if (!help_content) {
    // Precompute size of content
    size_t size = 0;
    for (size_t i = 0; i < commands_size; i++) {
      // Add 5 to account for extra characters
      size += strlen(commands[i].command) + strlen(commands[i].description) + 5;
      if (commands[i].usage) {
        // Add 1 to account for extra space
        size += strlen(commands[i].usage) + 1;
      }
    }

    help_content = malloc(size + 1);
    size_t pos = 0;
    for (size_t i = 0; i < commands_size; i++) {
      if (commands[i].usage) {
        pos += sprintf(&help_content[pos], "`%s %s`: %s\n", commands[i].command,
                       commands[i].usage, commands[i].description);
      } else {
        pos += sprintf(&help_content[pos], "`%s`: %s\n", commands[i].command,
                       commands[i].description);
      }
    }
    assert(size == pos);
    log_trace("Generated help content");
  }

  struct discord_create_message params = {.content = help_content};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

void set_commands(struct discord *client) {
  for (size_t i = 0; i < commands_size; i++) {
    discord_set_on_command(client, commands[i].command, commands[i].callback);
  }
  discord_set_on_command(client, "help", &on_help);
}

int main(int argc, char *argv[]) {
  ccord_global_init();

  struct discord *client =
      discord_config_init((argc > 1) ? argv[1] : "config.json");
  assert(client != NULL && "Could not initialize client");

  gettimeofday(&start, NULL);

  discord_set_on_ready(client, &on_ready);

  set_commands(client);

  const struct discord_user *bot = discord_get_self(client);
  avatar_url = fetch_avatar_url(avatar_url, bot->id, bot->avatar);

  struct utsname uname_data;
  uname(&uname_data);
  sprintf(system_info, "%s %s", uname_data.sysname, uname_data.release);

  discord_run(client);

  discord_cleanup(client);
  ccord_global_cleanup();
  free(avatar_url);
  free(help_content);
}
