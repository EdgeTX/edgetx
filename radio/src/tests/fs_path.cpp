/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "gtests.h"
#include "lib_file.h"
#include "sdcard.h"

// Helper: normalize with the given CWD (set via etxChdir, which needs the dir
// to exist under the sim SD root = radio/src/tests). "/" and "/images" exist.
static std::string norm(const char* cwd, const char* in)
{
  if (etxChdir(cwd) != FR_OK) {
    ADD_FAILURE() << "chdir failed: " << cwd;
    return {};
  }
  char out[FF_MAX_LFN + 1];
  etxNormalizePath(in, out, sizeof(out));
  return std::string(out);
}

// Path normalization must collapse "."/".." to a clean absolute path so Lua
// file access behaves the same on exFAT as on FAT32 (FatFS cannot resolve ".."
// or report the CWD on exFAT).
TEST(FsPath, normalizeAbsolute)
{
  EXPECT_EQ(etxChdir("/"), FR_OK);
  char out[FF_MAX_LFN + 1];

  etxNormalizePath("/SCRIPTS/../IMAGE", out, sizeof(out));
  EXPECT_STREQ(out, "/IMAGE");

  etxNormalizePath("/SCRIPTS/../IMAGE/file.jpg", out, sizeof(out));
  EXPECT_STREQ(out, "/IMAGE/file.jpg");

  etxNormalizePath("/a/./b/./c", out, sizeof(out));
  EXPECT_STREQ(out, "/a/b/c");

  etxNormalizePath("//a///b//", out, sizeof(out));
  EXPECT_STREQ(out, "/a/b");

  // ".." can never escape the root
  etxNormalizePath("/..", out, sizeof(out));
  EXPECT_STREQ(out, "/");
  etxNormalizePath("/../../x", out, sizeof(out));
  EXPECT_STREQ(out, "/x");
}

TEST(FsPath, normalizeRelative)
{
  // relative paths resolve against the tracked CWD
  EXPECT_EQ(norm("/images", "logo.png"), "/images/logo.png");
  EXPECT_EQ(norm("/images", "../IMAGE"), "/IMAGE");
  EXPECT_EQ(norm("/images", "../IMAGE/file.jpg"), "/IMAGE/file.jpg");
  EXPECT_EQ(norm("/images", "./sub/x"), "/images/sub/x");
  EXPECT_EQ(norm("/", "a/b"), "/a/b");
  EXPECT_EQ(norm("/", ".."), "/");
}

TEST(FsPath, normalizeNullAndEmpty)
{
  EXPECT_EQ(etxChdir("/images"), FR_OK);
  char out[FF_MAX_LFN + 1];

  etxNormalizePath(nullptr, out, sizeof(out));
  EXPECT_STREQ(out, "/images");  // null/empty -> CWD

  etxNormalizePath("", out, sizeof(out));
  EXPECT_STREQ(out, "/images");
}

// etxChdir tracks an absolute CWD that etxGetcwd() returns (a replacement for
// the exFAT-broken f_getcwd()). A failed chdir must leave the CWD unchanged.
TEST(FsPath, chdirTracksCwd)
{
  EXPECT_EQ(etxChdir("/"), FR_OK);
  EXPECT_STREQ(etxGetcwd(), "/");

  EXPECT_EQ(etxChdir("/images"), FR_OK);
  EXPECT_STREQ(etxGetcwd(), "/images");

  // relative + dotted chdir is normalized to absolute
  EXPECT_EQ(etxChdir(".."), FR_OK);
  EXPECT_STREQ(etxGetcwd(), "/");

  // a chdir to a missing dir fails and leaves the CWD untouched
  EXPECT_EQ(etxChdir("/images"), FR_OK);
  EXPECT_NE(etxChdir("/no/such/dir"), FR_OK);
  EXPECT_STREQ(etxGetcwd(), "/images");
}

// Guard against buffer overruns: undersized output buffers must never write
// past outLen (ASan would abort), and must not fabricate a bogus "/" when the
// path did not actually collapse to root. (Regression for the buffer guard.)
TEST(FsPath, normalizeBufferBounds)
{
  EXPECT_EQ(etxChdir("/"), FR_OK);
  char out[8];

  // zero-length: no write at all
  out[0] = '#';
  etxNormalizePath("/x", out, 0);
  EXPECT_EQ(out[0], '#');

  // room for terminator only -> empty string
  etxNormalizePath("/x", out, 1);
  EXPECT_STREQ(out, "");

  // root just fits in 2 bytes
  etxNormalizePath("..", out, 2);
  EXPECT_STREQ(out, "/");

  // first segment does not fit -> empty, NOT a fabricated "/"
  etxNormalizePath("/VERYLONGNAME", out, 5);
  EXPECT_STREQ(out, "");

  // partial fit stays terminated and within bounds
  etxNormalizePath("/ab/cd", out, 4);
  EXPECT_STREQ(out, "/ab");
}

// A path too long for the work buffer must resolve to empty, not to a
// truncated (and therefore different) path.
TEST(FsPath, normalizeOverflow)
{
  // absolute: "/" + name + nul must fit FF_MAX_LFN + 1
  std::string fits = "/" + std::string(FF_MAX_LFN - 1, 'a');
  EXPECT_EQ(norm("/", fits.c_str()), fits);
  std::string tooLong = "/" + std::string(FF_MAX_LFN, 'a');
  EXPECT_EQ(norm("/", tooLong.c_str()), "");

  // relative: CWD "/" + "/" + name + nul must fit FF_MAX_LFN + 1
  fits = std::string(FF_MAX_LFN - 2, 'a');
  EXPECT_EQ(norm("/", fits.c_str()), "/" + fits);
  tooLong = std::string(FF_MAX_LFN - 1, 'a');
  EXPECT_EQ(norm("/", tooLong.c_str()), "");
}

// An over-long path must fail the chdir and leave the CWD alone: FatFs would
// otherwise take the empty normalized path as the current dir and succeed.
TEST(FsPath, chdirOverflow)
{
  EXPECT_EQ(etxChdir("/images"), FR_OK);
  std::string tooLong(FF_MAX_LFN + 45, 'a');
  EXPECT_NE(etxChdir(tooLong.c_str()), FR_OK);
  EXPECT_STREQ(etxGetcwd(), "/images");

  EXPECT_EQ(etxChdir("/"), FR_OK);  // later tests resolve paths from the root
}

// The basename must be found at any path length (an 8-bit index used to wrap
// past 127 chars and return the whole path).
TEST(FsPath, basename)
{
  EXPECT_STREQ(getBasename("/SCRIPTS/TOOLS/tool.lua"), "tool.lua");
  EXPECT_STREQ(getBasename("tool.lua"), "tool.lua");
  EXPECT_STREQ(getBasename("/dir/"), "");

  std::string longPath = "/" + std::string(200, 'd') + "/tool.lua";
  EXPECT_STREQ(getBasename(longPath.c_str()), "tool.lua");
}
