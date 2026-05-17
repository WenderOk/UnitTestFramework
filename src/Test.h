#ifndef MERETDD_TEST_H
#define MERETDD_TEST_H

#include <map>
#include <ostream>
#include <source_location>
#include <string_view>
#include <string>
#include <vector>

namespace MereTDD
{
    class ConfirmException
    {
    protected:
        std::string mReason;
        int mLine;
    public:
        ConfirmException(int line): mLine(line) {}
        virtual ~ConfirmException() = default;

        std::string_view reason() const
        { return mReason; }

        int line() const
        { return mLine; }
    };

    class BoolConfirmException: public ConfirmException
    {
    public:        
        BoolConfirmException(bool expected, int line): ConfirmException(line)
        {
            mReason += "    Expected: ";
            mReason += expected ? "true" : "false";
        }
    };

    class MissingException
    {
        std::string mExType;
    public:
        MissingException(std::string_view exType): mExType(exType)
        {}
        std::string_view exType() const
        { return mExType; }
    };

    class ActualConfirmException : public ConfirmException
    {
        void formatReason()
        {
            mReason += "    Expected: " + mExpected + "\n";
            mReason += "    Actual  : " + mActual;
        }
        std::string mExpected;
        std::string mActual;
    public:
        ActualConfirmException (std::string_view expected, std::string_view actual, int line)
        : ConfirmException(line),
         mExpected(expected),
         mActual(actual)
        { formatReason(); }
    };

    inline void confirm(bool expected, bool actual, const std::source_location location = 
        std::source_location::current())
    { if(actual != expected) throw BoolConfirmException(expected, location.line()); }

    inline void confirm(std::string_view expected, std::string_view actual, const std::source_location location = 
        std::source_location::current())
    { if(actual != expected) throw ActualConfirmException(expected, actual, location.line()); }

    inline void confirm(std::string const& expected, std::string const& actual, const std::source_location location = 
        std::source_location::current())
    { 
        confirm(
            std::string_view(expected), 
            std::string_view(actual), 
            location);
    }

    inline void confirm (float expected, float actual, const std::source_location location = 
        std::source_location::current())
    {
        if (actual < (expected - 0.0001f) ||
            actual > (expected + 0.0001f))
        {
            throw ActualConfirmException(
                std::to_string(expected),
                std::to_string(actual),
                location.line());
        }
    }

    inline void confirm (double expected, double actual, const std::source_location location = 
        std::source_location::current())
    {
        if (actual < (expected - 0.000001) ||
            actual > (expected + 0.000001))
        {
            throw ActualConfirmException(
                std::to_string(expected),
                std::to_string(actual),
                location.line());
        }
    }

    inline void confirm (long double expected, long double actual, const std::source_location location = 
        std::source_location::current())
    {
        if (actual < (expected - 0.000001) ||
            actual > (expected + 0.000001))
        {
            throw ActualConfirmException(
                std::to_string(expected),
                std::to_string(actual),
                location.line());
        }
    }

    template <typename T>
    void confirm (T const & expected, T const & actual, const std::source_location location = 
        std::source_location::current())
    {
        if (actual != expected)
        {
            throw ActualConfirmException(
                std::to_string(expected),
                std::to_string(actual),
                location.line());
        }
    }


    class Test;
    class TestSuite;

    inline std::map<std::string, std::vector<Test *>>& getTests()
    {
        static std::map<std::string, std::vector<Test *>> tests;
        return tests;
    }

    inline std::map<std::string, std::vector<TestSuite *>>&
    getTestSuites()
    {
        static std::map<std::string,
            std::vector<TestSuite *>> suites;
        return suites;
    }

    inline void addTest(std::string_view suiteName, Test* test)
    {
        std::string name{suiteName};
        if(!getTests().contains(name))
            getTests().try_emplace(name, std::vector<Test*>());
        getTests()[name].push_back(test);
    }

    inline void addTestSuite(std::string_view suiteName, TestSuite* suite)
    {
        std::string name{suiteName};
        if(!getTestSuites().contains(name))
            getTestSuites().try_emplace(name, std::vector<TestSuite*>());
        getTestSuites()[name].push_back(suite);
    }


    class TestBase
    {
        std::string mName;
        std::string mSuiteName;
        bool mPassed;
        std::string mReason;
        int mConfirmLocation;
    public:
        TestBase (std::string_view name, std::string_view suiteName)
        : mName{name}, mSuiteName{suiteName}, mPassed{true}, mConfirmLocation{-1}
        {}

        virtual ~TestBase () = default;

        std::string_view name() const
        { return mName; }

        std::string_view suiteName() const
        { return mSuiteName; }

        bool passed() const
        { return mPassed; }

        std::string_view reason() const
        { return mReason; }

        int confirmLocation() const
        { return mConfirmLocation; }

        void setFailed(std::string_view reason, int confirmLocation = -1)
        {
            mPassed = false;
            mReason = reason;
            mConfirmLocation = confirmLocation;
        }
    };

    class Test : public TestBase
    {
        std::string mExpectedReason;
    public:
        Test (std::string_view name, std::string_view suiteName)
        : TestBase(name, suiteName)
        { addTest(suiteName, this); }

        virtual void runEx ()
        { run(); }

        virtual void run () = 0;

        std::string_view expectedReason () const
        { return mExpectedReason; }

        void setExpectedFailureReason (std::string_view reason)
        { mExpectedReason = reason; }
    };

    template <typename ExceptionT>
    class TestEx : public Test
    {
        std::string mExceptionName;
    public:
        TestEx (std::string_view name,
            std::string_view suiteName,
            std::string_view exceptionName)
        : Test(name, suiteName), mExceptionName(exceptionName)
        { }
        void runEx() override
        {
            try
            { run(); }
            catch (ExceptionT const&)
            { return; }
            throw MissingException(mExceptionName);
        }
    };
    
    class TestSuite : public TestBase
    {
    public:
        TestSuite (
            std::string_view name,
            std::string_view suiteName)
        : TestBase(name, suiteName)
        { addTestSuite(suiteName, this); }

        virtual void suiteSetup () = 0;
        virtual void suiteTeardown () = 0;
    };

    template <typename T>
    class SetupAndTeardown: public T
    {
    public:
        SetupAndTeardown()
        { T::setup(); }

        ~SetupAndTeardown()
        { T::teardown(); }
    };

    template <typename T>
    class TestSuiteSetupAndTeardown :
        public T,
        public TestSuite
    {
    public:
        TestSuiteSetupAndTeardown (
            std::string_view name,
            std::string_view suite)
        : TestSuite(name, suite)
        { }
        void suiteSetup ()
        { T::setup(); }

        void suiteTeardown () override
        { T::teardown(); }
    };

    inline void runTest(std::ostream & output, Test * test,
    int & numPassed, int & numFailed, int & numMissedFailed)
    {
        output << "------- Test: "
               << test->name() << "\n";
        try
        { test->runEx(); }
        catch (ConfirmException const & ex)
        { test->setFailed(ex.reason(), ex.line()); }
        catch (MissingException const & ex)
        {
            std::string message = "Expected exception type ";
            message += ex.exType();
            message += " was not thrown.";
            test->setFailed(message);
        }
        catch (...)
        { test->setFailed("Unexpected exception thrown."); }

        if (test->passed())
        {
            if (!test->expectedReason().empty())
            {
                // This test passed but it was supposed
                // to have failed.
                ++numMissedFailed;
                output << "Missed expected failure\n"
                       << "Test passed but was expected to fail.\n";
            }
            else
            {
                ++numPassed;
                output << "Passed\n";
                       
            }
        }
        else if (!test->expectedReason().empty() &&
            test->expectedReason() == test->reason())
        {
            ++numPassed;
            output << "Expected failure\n"
                   << test->reason() << "\n";
        }
        else
        {
            ++numFailed;
            if (test->confirmLocation() != -1)
            {
                output << "Failed confirm on line "
                    << test->confirmLocation() << "\n";
            }
            else
                output << "Failed\n";

            output << test->reason() << "\n";
        }
    }

    inline bool runSuite (std::ostream & output,
        bool setup, std::string const & name,
        int & numPassed, int & numFailed)
    {
        for (auto & suite: getTestSuites()[name])
        {
            if (setup)
                output << "------- Setup: ";
            else
                output << "------- Teardown: ";

            output << suite->name() << "\n";

            try
            {
                if (setup)
                    suite->suiteSetup();
                else
                    suite->suiteTeardown();
            }
            catch (ConfirmException const & ex)
            { suite->setFailed(ex.reason(), ex.line()); }
            catch (...)
            { suite->setFailed("Unexpected exception thrown."); }

            if (suite->passed())
            {
                ++numPassed;
                output << "Passed\n";
            }
            else
            {
                ++numFailed;
                if (suite->confirmLocation() != -1)
                {
                    output << "Failed confirm on line "
                        << suite->confirmLocation() << "\n";
                }
                else
                    output << "Failed\n";

                output << suite->reason() << "\n";

                return false;
            }
        }
        return true;
    }

    inline int runTests(std::ostream & output)
    {
        output << "Running " << getTests().size() << " test suites\n";

        int numPassed{};
        int numMissedFailed{};
        int numFailed{};

        for (auto const & [key, value]: getTests())
        {
            std::string suiteDisplayName{"Suite: "};
            if(key.empty())
                suiteDisplayName += "Single Tests";
            else
                suiteDisplayName += key;

            output << "---------------\n"
                    << suiteDisplayName << "\n";

            if(!key.empty())
            {
                if(!getTestSuites().contains(key))
                {
                    output << "Test suite is not found."
                           << " Exiting test application.\n";
                    return ++numFailed;
                }

                if(!runSuite(output, true, key, numPassed, numFailed))
                {
                    output << "Test suite steup failed."
                           << " Skipping tests in suite.\n";
                    continue;
                }
            }

            for(auto * test: value)
                runTest(output, test, numPassed, numFailed, numMissedFailed);

            if(!key.empty())
            {
                if(!runSuite(output, false, key, numPassed, numFailed))
                    output << "Test suite teardown failed.\n";
            }
        }

        output << "---------------\n";
        output << "Tests passed: " << numPassed << "\nTests failed: " << numFailed << "\n";

        if(numMissedFailed != 0)
            output << "Tests failures missed: " << numMissedFailed << "\n";

        return numFailed;
    }
} // namespace MereTDD

#define MERETDD_CLASS_FINAL( line ) Test##line
#define MERETDD_CLASS_RELAY( line ) MERETDD_CLASS_FINAL( line )
#define MERETDD_CLASS MERETDD_CLASS_RELAY( __LINE__ )

#define MERETDD_INSTANCE_FINAL( line ) test##line
#define MERETDD_INSTANCE_RELAY( line ) MERETDD_INSTANCE_FINAL(line)
#define MERETDD_INSTANCE MERETDD_INSTANCE_RELAY( __LINE__ )


#define TEST_H

#define TEST(testName) \
namespace  \
{ \
class MERETDD_CLASS: public MereTDD::Test \
{ \
public: \
    MERETDD_CLASS (std::string_view name): Test(name, "") \
    {} \
    void run() override; \
}; \
} /*End of unnamed namespace*/ \
MERETDD_CLASS MERETDD_INSTANCE(testName); \
void MERETDD_CLASS::run()


#define TEST_EX(testName, exceptionType) \
namespace  \
{ \
class MERETDD_CLASS: public MereTDD::TestEx<exceptionType> \
{ \
public: \
    MERETDD_CLASS(std::string_view name, std::string_view exceptionName) \
    : TestEx(name, "", exceptionName) \
    {} \
    void run() override; \
}; \
} /*End of unnamed namespace*/ \
MERETDD_CLASS MERETDD_INSTANCE(testName, #exceptionType); \
void MERETDD_CLASS::run()


#define TEST_SUITE( testName, suiteName ) \
namespace \
{ \
class MERETDD_CLASS : public MereTDD::Test \
{ \
public: \
    MERETDD_CLASS (std::string_view name, \
        std::string_view suite) \
    : Test(name, suite) \
    { } \
    void run () override; \
}; \
} /* end of unnamed namespace */ \
MERETDD_CLASS MERETDD_INSTANCE(testName, suiteName); \
void MERETDD_CLASS::run ()


#define TEST_SUITE_EX( testName, suiteName, exceptionType ) \
namespace \
{ \
class MERETDD_CLASS : public MereTDD::TestEx<exceptionType> \
{ \
public: \
    MERETDD_CLASS (std::string_view name, \
        std::string_view suite, \
        std::string_view exceptionName) \
    : TestEx(name, suite, exceptionName) \
    { } \
    void run () override; \
}; \
} /* end of unnamed namespace */ \
MERETDD_CLASS MERETDD_INSTANCE(testName, suiteName, #exceptionType); \
void MERETDD_CLASS::run ()

#endif // TEST_H


#define CONFIRM_FALSE(actual) \
    MereTDD::confirm(false, actual)

#define CONFIRM_TRUE(actual) \
    MereTDD::confirm(true, actual)

#define CONFIRM(expected, actual) \
    MereTDD::confirm(expected, actual)
