#include "arg_parser.h"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <climits>

ArgumentParser::ArgumentParser(const std::string &prog_name)
    : program_name(prog_name) {}

bool ArgumentParser::parse(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--width") {
      if (!parseIntOption(argc, argv, i, width, "width"))
        return false;
    } else if (arg == "--height") {
      if (!parseIntOption(argc, argv, i, height, "height"))
        return false;
    } else if (arg == "--storage") {
      if (!getNextArg(argc, argv, i, storage_path, "storage"))
        return false;
    } else if (arg == "--settings") {
      if (!getNextArg(argc, argv, i, settings_path, "settings"))
        return false;
    } else if (arg == "-h" || arg == "--help") {
      help_requested = true;
      return true;
    } else {
      printf("Unknown option: %s\n", arg.c_str());
      printUsage();
      return false;
    }
  }
  return true;
}

void ArgumentParser::printUsage() const {
  printf("usage: %s [--width width] [--height height] [--storage path] "
         "[--settings path] [-h | --help]\n",
         program_name.c_str());
}

void ArgumentParser::printHelp() const {
  printUsage();
  printf("\nOptions:\n");
  printf("  --width width      Set the width (integer)\n");
  printf("  --height height    Set the height (integer)\n");
  printf("  --storage path     Set the storage path\n");
  printf("  --settings path    Set the settings path\n");
  printf("  -h, --help         Show this help message\n");
}

bool ArgumentParser::isHelpRequested() const { return help_requested; }

int ArgumentParser::getWidth() const { return width; }

int ArgumentParser::getHeight() const { return height; }

const std::string &ArgumentParser::getStoragePath() const {
  return storage_path;
}

const std::string &ArgumentParser::getSettingsPath() const {
  return settings_path;
}

bool ArgumentParser::hasWidth() const { return width != -1; }

bool ArgumentParser::hasHeight() const { return height != -1; }

bool ArgumentParser::hasStoragePath() const { return !storage_path.empty(); }

bool ArgumentParser::hasSettingsPath() const { return !settings_path.empty(); }

bool ArgumentParser::getNextArg(int argc, char *argv[], int &i,
                                std::string &value,
                                const std::string &option_name) {
  if (i + 1 >= argc) {
    printf("Option --%s requires an argument\n", option_name.c_str());
    printUsage();
    return false;
  }
  value = argv[++i];
  return true;
}

bool ArgumentParser::parseIntOption(int argc, char *argv[], int &i, int &value,
                                    const std::string &option_name) {
  if (i + 1 >= argc) {
    printf("Option --%s requires an argument\n", option_name.c_str());
    printUsage();
    return false;
  }

  char *endptr;
  const char *str = argv[++i];

  // Check if string is empty or starts with non-digit (except for optional '+')
  if (!str || *str == '\0' || (!isdigit(*str) && *str != '+')) {
    printf("Option --%s requires a valid integer\n", option_name.c_str());
    return false;
  }

  long result = strtol(str, &endptr, 10);

  // Check for conversion errors
  if (*endptr != '\0' || result <= 0 || result > INT_MAX) {
    printf("Option --%s requires a valid positive integer\n",
           option_name.c_str());
    return false;
  }

  value = static_cast<int>(result);
  return true;
}
