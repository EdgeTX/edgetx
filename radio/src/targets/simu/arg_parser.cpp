#include "arg_parser.h"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <climits>
#include <cstdarg>
#include <filesystem>

ArgumentParser::ArgumentParser(const std::string &prog_name)
    : program_name(prog_name) {}

bool ArgumentParser::parse(int argc, char *argv[]) {
  // Reserve stdout for protocol records as soon as automation is requested,
  // including parse failures that occur before the flag's position.
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--automation-stdio") {
      automation_stdio = true;
      break;
    }
  }

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
    } else if (arg == "--automation-stdio") {
      // Already recorded by the pre-scan above.
    } else if (arg == "--automation-output") {
      if (!getNextArg(argc, argv, i, automation_output_path,
                      "automation-output"))
        return false;
    } else if (arg == "-h" || arg == "--help") {
      help_requested = true;
      return true;
    } else {
      printMessage("Unknown option: %s\n", arg.c_str());
      printUsage();
      return false;
    }
  }
  return validateAutomationOptions();
}

void ArgumentParser::printUsage() const {
  printMessage("usage: %s [--width width] [--height height] [--storage path] "
               "[--settings path] [--automation-stdio "
               "--automation-output path] [-h | --help]\n",
               program_name.c_str());
}

void ArgumentParser::printHelp() const {
  printUsage();
  printMessage("\nOptions:\n");
  printMessage("  --width width             Set the width (integer)\n");
  printMessage("  --height height           Set the height (integer)\n");
  printMessage("  --storage path            Set the storage path\n");
  printMessage("  --settings path           Set the settings path\n");
  printMessage("  --automation-stdio        Enable the stdio automation protocol\n");
  printMessage("  --automation-output path  Set the automation artifact root\n");
  printMessage("  -h, --help                Show this help message\n");
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

const std::string &ArgumentParser::getAutomationOutputPath() const {
  return automation_output_path;
}

bool ArgumentParser::isAutomationStdio() const { return automation_stdio; }

bool ArgumentParser::hasWidth() const { return width != -1; }

bool ArgumentParser::hasHeight() const { return height != -1; }

bool ArgumentParser::hasStoragePath() const { return !storage_path.empty(); }

bool ArgumentParser::hasSettingsPath() const { return !settings_path.empty(); }

bool ArgumentParser::hasAutomationOutputPath() const {
  return !automation_output_path.empty();
}

bool ArgumentParser::validateAutomationOptions() {
  if (automation_stdio != hasAutomationOutputPath()) {
    printMessage("Options --automation-stdio and --automation-output must be "
                 "used together\n");
    return false;
  }

  if (!automation_stdio)
    return true;

  std::error_code error;
  std::filesystem::path output =
      std::filesystem::canonical(automation_output_path, error);
  if (error || !std::filesystem::is_directory(output, error) || error) {
    printMessage("Option --automation-output must name an existing directory\n");
    return false;
  }

  automation_output_path = output.string();
  return true;
}

void ArgumentParser::printMessage(const char *format, ...) const {
  va_list arguments;
  va_start(arguments, format);
  vfprintf(automation_stdio ? stderr : stdout, format, arguments);
  va_end(arguments);
}

bool ArgumentParser::getNextArg(int argc, char *argv[], int &i,
                                std::string &value,
                                const std::string &option_name) {
  if (i + 1 >= argc) {
    printMessage("Option --%s requires an argument\n", option_name.c_str());
    printUsage();
    return false;
  }
  value = argv[++i];
  return true;
}

bool ArgumentParser::parseIntOption(int argc, char *argv[], int &i, int &value,
                                    const std::string &option_name) {
  if (i + 1 >= argc) {
    printMessage("Option --%s requires an argument\n", option_name.c_str());
    printUsage();
    return false;
  }

  char *endptr;
  const char *str = argv[++i];

  // Check if string is empty or starts with non-digit (except for optional '+')
  if (!str || *str == '\0' || (!isdigit(*str) && *str != '+')) {
    printMessage("Option --%s requires a valid integer\n", option_name.c_str());
    return false;
  }

  long result = strtol(str, &endptr, 10);

  // Check for conversion errors
  if (*endptr != '\0' || result <= 0 || result > INT_MAX) {
    printMessage("Option --%s requires a valid positive integer\n",
                 option_name.c_str());
    return false;
  }

  value = static_cast<int>(result);
  return true;
}
