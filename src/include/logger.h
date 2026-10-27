#ifndef LOGGER_H_
#define LOGGER_H_

#define FOREACH_TAG(TAG) \
    TAG(ERROR) \
    TAG(INFO) \
    TAG(DEBUG) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum TAG_ENUM
{
  FOREACH_TAG(GENERATE_ENUM)
};

void
pioss_log(enum TAG_ENUM tag, const char *message);
void
pioss_logf(enum TAG_ENUM tag, const char *format, ...);

#endif
