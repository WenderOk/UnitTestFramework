#include "../Test.h"

TEST("Test can be created")
{}

TEST("Test that throws unexpectedly can be created")
{
    setExpectedFailureReason("Unexpected exception thrown.");
    throw "Unexpected";
}

// This test should throw an unexpected exception
// but it doesn't. We need to somehow let the user
// know what happened. This will result in a missed failure.
TEST("Test that should throw unexpectedly can be created")
{
    setExpectedFailureReason("Unexpected exception thrown.");
}

TEST_EX("Test with throw can be vreated", int)
{
    throw 1;
}

TEST_EX("Test that never throws can be created", int)
{
    setExpectedFailureReason("Expected exception type int was not thrown.");
}

TEST_EX("Test that throws wrong type can be created", int)
{
    setExpectedFailureReason("Unexpected exception thrown.");
    throw "Wrong type";
}
