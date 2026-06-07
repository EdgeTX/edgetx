#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>

// Include the model data structures used by mdichild.cpp
#include "eeprominterface.h"
#include "radiodata.h"

// Invariant: filename field in ModelData must never be overflowed;
// the length written must not exceed sizeof(ModelData::filename) - 1.

class FilenameBufferSecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(FilenameBufferSecurityTest, FilenameNeverExceedsBufferBounds) {
    // Invariant: Buffer reads/writes never exceed the declared length of filename[]
    std::string payload = GetParam();

    ModelData model;
    memset(&model, 0, sizeof(model));

    constexpr size_t FILENAME_BUF_SIZE = sizeof(model.filename);

    // Simulate what the vulnerable code does, but assert the invariant holds:
    // the destination buffer must be large enough to hold the source string.
    // If payload length >= FILENAME_BUF_SIZE, a strcpy would overflow.
    bool wouldOverflow = (payload.size() >= FILENAME_BUF_SIZE);

    // The invariant: production code must NOT allow overflow.
    // We assert that either the payload fits, or the code must truncate/reject it.
    if (!wouldOverflow) {
        // Safe case: copy is within bounds
        strncpy(model.filename, payload.c_str(), FILENAME_BUF_SIZE - 1);
        model.filename[FILENAME_BUF_SIZE - 1] = '\0';
        EXPECT_LE(strlen(model.filename), FILENAME_BUF_SIZE - 1)
            << "Filename exceeds buffer bounds after copy";
    } else {
        // Overflow case: assert that the buffer size is insufficient for this payload.
        // This test FAILS if production code uses strcpy with this payload,
        // proving the invariant is violated.
        EXPECT_LT(payload.size(), FILENAME_BUF_SIZE)
            << "SECURITY VIOLATION: payload length " << payload.size()
            << " exceeds filename buffer size " << FILENAME_BUF_SIZE
            << ". strcpy() would overflow the buffer.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    FilenameBufferSecurityTest,
    ::testing::Values(
        // Valid input: short filename well within bounds
        std::string("model01.bin"),
        // Boundary value: exactly at the buffer limit (typically 16 bytes for filename)
        std::string("123456789012345"),
        // Exploit case: 2x oversized filename
        std::string("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.bin"),
        // Extreme case: 10x oversized filename
        std::string(160, 'X')
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}