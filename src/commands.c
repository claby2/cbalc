#include <assert.h>
#include <concord/discord.h>

#include "commands.h"

struct command {
  char *command;
  char *usage;
  char *description;
  discord_ev_message callback;
};

// Data that should be precomputed
struct data {
  char *avatar_url;
  char start_time[13];
  char system_info[_UTSNAME_SYSNAME_LENGTH + _UTSNAME_RELEASE_LENGTH];
};

struct data command_data;
static char *help_content = NULL;

// Compose url to avatar image
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

// Precompute global command data
void commands_data_init(const struct discord_user *bot) {
  command_data.avatar_url =
      fetch_avatar_url(command_data.avatar_url, bot->id, bot->avatar);

  struct utsname uname_data;
  uname(&uname_data);
  sprintf(command_data.system_info, "%s %s", uname_data.sysname,
          uname_data.release);

  struct timeval start;
  gettimeofday(&start, NULL);
  strftime(command_data.start_time, sizeof(command_data.start_time),
           "%b %d %H:%M", localtime(&start.tv_sec));
}

void on_ping(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  struct discord_create_message params = {.content = "pong"};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

void on_echo(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  struct discord_create_message params = {
      .content =
          // Set message content to given message content (not including prefix)
      msg->content};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

void on_status(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  // Define embed fields
  struct discord_embed_field fields[] = {
      {.name = "System", .value = command_data.system_info},
      {.name = "Start Time", .value = command_data.start_time},
  };

  struct discord_embed embeds[] = {{
      .title = "cbalc Status",
      .image = &(struct discord_embed_image){.url = command_data.avatar_url},
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

const struct command commands[] = {
    {.command = "ping", .description = "Send pong", .callback = &on_ping},
    {.command = "echo",
     .usage = "<text>",
     .description = "Display given text",
     .callback = &on_echo},
    {.command = "status",
     .description = "Get status of bot",
     .callback = &on_status},
};

const size_t commands_size = sizeof(commands) / sizeof *commands;

void on_help(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  // help_content should be lazily evaluated
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
      // Check if command usage has been defined
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

void commands_set(struct discord *client) {
  for (size_t i = 0; i < commands_size; i++) {
    discord_set_on_command(client, commands[i].command, commands[i].callback);
  }
  // Add additional help command
  discord_set_on_command(client, "help", &on_help);
}

void commands_free() {
  free(command_data.avatar_url);
  free(help_content);
}
