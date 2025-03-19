#include <assert.h>
#include <concord/discord.h>
#include <concord/log.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cockbot.h"
#include "ficl/ficl.h"
#include "forthFunctions.h"

void disRestart(ficlVm *forth_vm) { raise(SIGSEGV); }

// Nice example code that shows args and explains the actual forth discord
void disPin(ficlVm *forth_vm) {
  const struct discord_message *dis_msg;
  struct discord *bot_client;
  ficlDictionary *forth_dict = ficlVmGetDictionary(forth_vm);
  // FICL Words can't be normal strings since forth uses counted strings.
  ficlString tempString1;
  ficlString tempString2;
  // A nice little undocumented string converter for forth
  FICL_STRING_SET_FROM_CSTRING(tempString1, "dis_msg");
  FICL_STRING_SET_FROM_CSTRING(tempString2, "bot_client");
  // Execute constants cotaining pointers to our message and client structs
  ficlVmExecuteWord(forth_vm, ficlDictionaryLookup(forth_dict, tempString2));
  ficlVmExecuteWord(forth_vm, ficlDictionaryLookup(forth_dict, tempString1));
  // explicitly grab them as pointers and assign structs to them
  dis_msg = ficlStackPopPointer(forth_vm->dataStack);
  bot_client = ficlStackPopPointer(forth_vm->dataStack);
  // all discord centric code and the like can be done now grabbing from stack
  discord_pin_message(bot_client, dis_msg->channel_id,
                      ficlStackPopInteger(forth_vm->dataStack), NULL, NULL);
}

// This code was largely copied from extras.c
void disLoadScript(ficlVm *forth_vm) {
  char loaderBuffer[LINE_BUFFERSIZE];
  int loaderLine = 0;
  FILE *loaderFile;
  int loaderResult = 0;

  ficlCell loaderOldID;
  ficlString s;

  if ((loaderFile = fopen(ADMIN_SCRIPT, "r")) == NULL)
    ficlVmThrowError(forth_vm, "Script "ADMIN_SCRIPT" does not exist in the current path");

  loaderOldID = forth_vm->sourceId;
  forth_vm->sourceId.p = (void *)loaderFile;

  while (fgets(loaderBuffer, LINE_BUFFERSIZE, loaderFile)) {
    int lineLength = strlen(loaderBuffer) - 1;

    loaderLine++;

    if (lineLength <= 0)
      continue;

    if (loaderBuffer[lineLength] == '\n')
      loaderBuffer[lineLength--] == '\0';

    FICL_STRING_SET_POINTER(s, loaderBuffer);
    FICL_STRING_SET_LENGTH(s, lineLength + 1);

    loaderResult = ficlVmExecuteString(forth_vm, s);
    switch (loaderResult) {
    case FICL_VM_STATUS_OUT_OF_TEXT:
    case FICL_VM_STATUS_USER_EXIT:
      break;

    default:
      forth_vm->sourceId = loaderOldID;
      fclose(loaderFile);
      ficlVmThrowError(forth_vm, "Error loading script! Line: %d", loaderLine);
      break;
    }
  }
  forth_vm->sourceId.i = -1;
  FICL_STRING_SET_FROM_CSTRING(s, "");
  ficlVmExecuteString(forth_vm, s);

  forth_vm->sourceId = loaderOldID;
  fclose(loaderFile);

  if (loaderResult == FICL_VM_STATUS_USER_EXIT)
    ficlVmThrow(forth_vm, FICL_VM_STATUS_USER_EXIT);

  return;
}

static struct discord *get_client_from_forth(ficlVm *forth_vm) {
  const struct discord_message *dis_msg;
  struct discord *bot_client;
  ficlDictionary *forth_dict = ficlVmGetDictionary(forth_vm);
  ficlString tempString1;
  FICL_STRING_SET_FROM_CSTRING(tempString1, "bot_client");
  ficlVmExecuteWord(forth_vm, ficlDictionaryLookup(forth_dict, tempString1));
  return ficlStackPopPointer(forth_vm->dataStack);
}

static const struct discord_message *get_dis_msg_from_forth(ficlVm *forth_vm) {
  struct discord_message *dis_msg;
  ficlDictionary *forth_dict = ficlVmGetDictionary(forth_vm);
  ficlString tempString2;
  FICL_STRING_SET_FROM_CSTRING(tempString2, "bot_client");
  ficlVmExecuteWord(forth_vm, ficlDictionaryLookup(forth_dict, tempString2));
  return ficlStackPopPointer(forth_vm->dataStack);
}

void disUnOhio(ficlVm *forth_vm) {
  const struct discord_message *dis_msg;
  struct discord *bot_client = get_client_from_forth(forth_vm);
  uint64_t dis_userid = ficlStackPopInteger(forth_vm->dataStack);
  log_info("Muting: %lu", dis_userid);
  discord_remove_guild_member_role(bot_client, GUILD_ID, dis_userid, MUTE_ROLE,
                                NULL, NULL);
}


void disOhio(ficlVm *forth_vm) {
  const struct discord_message *dis_msg;
  struct discord *bot_client = get_client_from_forth(forth_vm);
  uint64_t dis_userid = ficlStackPopInteger(forth_vm->dataStack);
  log_info("Muting: %lu", dis_userid);
  discord_add_guild_member_role(bot_client, GUILD_ID, dis_userid, MUTE_ROLE,
                                NULL, NULL);
}

void disUptime(ficlVm *forth_vm) {
  const struct discord_message *dis_msg;
  struct discord *bot_client = get_client_from_forth(forth_vm);
  uint64_t dis_userid = ficlStackPopInteger(forth_vm->dataStack);
  log_info("Printing uptime", dis_userid);
}

void disKick(ficlVm *forth_vm) {
  const struct discord_message *dis_msg = get_client_from_forth(forth_vm);
  struct discord *bot_client = get_dis_msg_from_forth(forth_vm);
  uint64_t dis_userid = ficlStackPopInteger(forth_vm->dataStack);
  log_info("Kicking: %lu", dis_userid);
  discord_remove_guild_member(bot_client, GUILD_ID, dis_userid, NULL, NULL);
}

void disBan(ficlVm *forth_vm) {
  const struct discord_message *dis_msg = get_dis_msg_from_forth(forth_vm);
  struct discord *bot_client = get_client_from_forth(forth_vm);
  uint64_t dis_userid = ficlStackPopInteger(forth_vm->dataStack);
  log_info("Ban: %lu", dis_userid);
  discord_create_guild_ban(
      bot_client, GUILD_ID, dis_userid,
      &(struct discord_create_guild_ban){.delete_message_days = 0}, NULL);
}

void disDelBan(ficlVm *forth_vm) {
  const struct discord_message *dis_msg = get_dis_msg_from_forth(forth_vm);
  struct discord *bot_client = get_client_from_forth(forth_vm);
  uint64_t dis_userid = ficlStackPopInteger(forth_vm->dataStack);
  log_info("Ban: %lu", dis_userid);
  discord_create_guild_ban(
      bot_client, GUILD_ID, dis_userid,
      &(struct discord_create_guild_ban){
          .delete_message_days = ficlStackPopInteger(forth_vm->dataStack)},
      NULL);
}
