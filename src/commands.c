#include <assert.h>
#include <concord/discord.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/utsname.h>

#include "commands.h"

struct command {
  char *command;
  char *usage;
  char *description;
  discord_ev_message callback;
};

// Forward declare command callbacks
void on_ping(struct discord *client, const struct discord_message *msg);
void on_echo(struct discord *client, const struct discord_message *msg);
void on_status(struct discord *client, const struct discord_message *msg);

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

static const size_t commands_size = sizeof(commands) / sizeof *commands;

// Data that should be precomputed
struct data {
  char *avatar_url;
  char *system_info;
  char *help_content;
  time_t start;
};

static struct data command_data;

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

  // Calculate uptime in seconds
  unsigned long uptime = (unsigned long)(time(NULL) - command_data.start);
  char *uptime_value;
  unsigned long hours = uptime / 3600;
  unsigned long minutes = (uptime / 60) % 60;
  unsigned long seconds = uptime % 60;

  char *hours_label, *minutes_label, *seconds_label;
  hours_label = minutes_label = "";
  if (hours > 0) {
    cog_asprintf(&hours_label, "%lu %s, ", hours,
                 hours == 1 ? "hour" : "hours");
  }
  if (minutes > 0) {
    cog_asprintf(&minutes_label, "%lu %s, ", minutes,
                 minutes == 1 ? "minute" : "minutes");
  }
  cog_asprintf(&seconds_label, "%lu %s", seconds,
               seconds == 1 ? "second" : "seconds");
  // Format uptime
  cog_asprintf(&uptime_value, "%s%s%s", hours_label, minutes_label,
               seconds_label);

  // Define embed fields
  struct discord_embed_field fields[] = {
      {.name = "System", .value = command_data.system_info},
      {.name = "Uptime", .value = uptime_value},
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

void on_help(struct discord *client, const struct discord_message *msg) {
  if (msg->author->bot) return;

  struct discord_create_message params = {.content = command_data.help_content};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

// Compose url to avatar image
char *fetch_avatar_url(char *url, const u64snowflake id, const char *avatar) {
  static const char avatar_base_url[] = "https://cdn.discordapp.com/avatars/";
  cog_asprintf(&url, "%s%" PRIu64 "/%s", avatar_base_url, id, avatar);
  return url;
}

void generate_help_content(char **help_content) {
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

  char content[size + 1];
  size_t pos = 0;
  for (size_t i = 0; i < commands_size; i++) {
    // Check if command usage has been defined
    if (commands[i].usage) {
      pos += snprintf(&content[pos], sizeof(content), "`%s %s`: %s\n",
                      commands[i].command, commands[i].usage,
                      commands[i].description);
    } else {
      pos += snprintf(&content[pos], sizeof(content), "`%s`: %s\n",
                      commands[i].command, commands[i].description);
    }
  }
  assert(size == pos);
  cog_asprintf(help_content, "%s", content);
  log_trace("Generated help content:\n%s", command_data.help_content);
}

// Precompute global command data
void commands_data_init(const struct discord_user *bot) {
  command_data.avatar_url =
      fetch_avatar_url(command_data.avatar_url, bot->id, bot->avatar);

  struct utsname uname_data;
  uname(&uname_data);
  cog_asprintf(&command_data.system_info, "%s %s", uname_data.sysname,
               uname_data.release);

  time(&command_data.start);

  generate_help_content(&command_data.help_content);
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
  free(command_data.system_info);
  free(command_data.help_content);
}
