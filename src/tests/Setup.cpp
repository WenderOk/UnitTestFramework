#include "../Test.h"
#include <string_view>
#include <string>

std::string createTestTable()
{
    // If this was real code, it might open a
    // connection to a database, create a temp
    // table with a random name, and return the
    // table name.
    return "test_data_01";
}

void dropTestTable (std::string_view /*name*/)
{
    // Real code would use the name to drop
    // the table.
}

int createTestEntry()
{
    // If this was real code, it might open a
    // connection to a database, insert a row
    // of data, and return the row identifier.
    return 100;
}

void updateTestEntryName(int /*id*/, std::string_view name)
{
    if(name.empty())
        throw 1;
    // Real code would proceed to update the
    // data with the new name.
}

void deleteTestEntry(int /*id*/)
{
    // Real code would use the id to delete
    // the temporary row of data.
}

class TempEntry
{
    int mId;
public:
    void setup()
    { mId = createTestEntry(); }

    void teardown()
    { deleteTestEntry(mId); }

    int id()
    { return mId; }
};

class TempTable
{
    std::string mName;
public:
    void setup()
    { mName = createTestTable(); }

    void teardown()
    { dropTestTable(mName); }

    std::string tableName()
    { return mName; }
};

MereTDD::TestSuiteSetupAndTeardown<TempTable>
gTable1("Test suite setup/teardown 1", "Suite 1");

MereTDD::TestSuiteSetupAndTeardown<TempTable>
gTable2("Test suite setup/teardown 2", "Suite 1");

TEST_SUITE("Test part 1 of suite", "Suite 1")
{
    // If this was a project test, it could use
    // the table names from gTable1 and gTable2.
    CONFIRM("test_data_01", gTable1.tableName());
    CONFIRM("test_data_01", gTable2.tableName());
}

TEST_SUITE_EX("Test part 2 of suite", "Suite 1", int)
{
    // If this was a project test, it could use
    // the table names from gTable1 and gTable2.
    throw 1;
}

TEST_EX("Test will run setup and teardown code", int)
{
    MereTDD::SetupAndTeardown<TempEntry> entry;
    // If this was a project test, it might be called
    // "Updating empty name throws". And the type thrown
    // would not be an int.
    updateTestEntryName(entry.id(), "");

}