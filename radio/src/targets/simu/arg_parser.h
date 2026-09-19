#pragma once

#include <string>

class ArgumentParser {
private:
  int width = -1;
  int height = -1;
  std::string storage_path;
  std::string settings_path;
#if defined(WIDGET_STUDIO)
  std::string pipe_path;
#endif
  bool help_requested = false;
  std::string program_name;

public:
  ArgumentParser(const std::string &prog_name);

  bool parse(int argc, char *argv[]);
  void printUsage() const;
  void printHelp() const;

  // Getters
  bool isHelpRequested() const;
  int getWidth() const;
  int getHeight() const;
  const std::string &getStoragePath() const;
  const std::string &getSettingsPath() const;
#if defined(WIDGET_STUDIO)
  const std::string &getPipePath() const;
#endif

  // Check if option was provided
  bool hasWidth() const;
  bool hasHeight() const;
  bool hasStoragePath() const;
  bool hasSettingsPath() const;
#if defined(WIDGET_STUDIO)
  bool hasPipePath() const;
#endif

private:
  bool getNextArg(int argc, char *argv[], int &i, std::string &value,
                  const std::string &option_name);
  bool parseIntOption(int argc, char *argv[], int &i, int &value,
                      const std::string &option_name);
};
