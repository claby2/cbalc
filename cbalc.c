#include <assert.h>
#include <concord/discord.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/utsname.h>

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

int main(int argc, char *argv[]) {
  ccord_global_init();

  struct discord *client =
      discord_config_init((argc > 1) ? argv[1] : "config.json");
  assert(client != NULL && "Could not initialize client");

  gettimeofday(&start, NULL);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_command(client, "ping", &on_ping);
  discord_set_on_command(client, "echo", &on_echo);
  discord_set_on_command(client, "status", &on_status);

  const struct discord_user *bot = discord_get_self(client);
  avatar_url = fetch_avatar_url(avatar_url, bot->id, bot->avatar);

  struct utsname uname_data;
  uname(&uname_data);
  sprintf(system_info, "%s %s", uname_data.sysname, uname_data.release);

  discord_run(client);

  discord_cleanup(client);
  ccord_global_cleanup();
}
