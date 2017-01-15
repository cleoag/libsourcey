///
//
// LibSourcey
// Copyright (c) 2005, Sourcey <http://sourcey.com>
//
// SPDX-License-Identifier:	LGPL-2.1+
//
/// @addtogroup base
/// @{


#ifndef SCY_Test_H
#define SCY_Test_H


#include "scy/base.h"
#include "scy/interface.h"
#include <mutex>
#include <iostream>
#include <list>
#include <map>


namespace scy {


/// Modern unit testing framework.
namespace test {


class SCY_EXTERN Test;
class SCY_EXTERN TestRunner;

typedef std::list<Test*> TestList;
typedef std::list<std::string> ErrorList;
typedef std::map<Test*, ErrorList> ErrorMap;

/// Initialize the test environment.
void initialize();

/// Finalize the test environment.
///
/// Destroy the TestRunner singleton instance and return the exit code.
int finalize();

/// Run all tests.
void runAll();

/// Describe a test environment implemented by the given lambda function.
void describe(const std::string& name, std::function<void()> target);

/// Describe a test environment implemented by the given test instance.
void describe(const std::string& name, Test* test);

/// Expect asserts that a condition is true (use expect() as defined below).
void expectImpl(bool passed, const char* assert, const char* file, long line);

// Shamelessly define macros to aesthetic name :)
#ifdef NDEBUG
#define expect(x) test::expectImpl(true, "", "", 0)
#else
#define expect(x) test::expectImpl(x, #x, __FILE__, __LINE__)
#endif


//
// Test
//


/// Test wrapper class.
///
/// This class is for implementing any kind of unit
/// test that can be executed by a `TestRunner`.
///
class SCY_EXTERN Test
{
public:
    Test(const std::string& name = "Unnamed Test");

    /// Should remain protected.
    virtual ~Test();

    /// Called by the TestRunner to run the test.
    virtual void run() = 0;

    /// Return true when the test passed without errors.
    bool passed();

    /// The name of the test.
    std::string name;

    /// A list of test errors.
    ErrorList errors;

    /// The test run duration for benchmarking.
    double duration;

protected:
    Test(const Test& test) = delete;
    Test& operator=(Test const&) = delete;

    /// Tests belong to a TestRunner instance.
    friend class TestRunner;
};


// Test class that runs a static or lambda function.
class SCY_EXTERN FunctionTest : public Test
{
public:
    std::function<void()> target;

    FunctionTest(std::function<void()> target,
                 const std::string& name = "Unnamed Test")
        : Test(name)
        , target(target)
    {
    }

protected:
    virtual ~FunctionTest() {}

    void run() { target(); }
};


//
// Test Runner
//


/// Test manager queue.
///
/// The `TestRunner` is a queue in charge of running one or many tests.
///
/// When `run()` the `TestRunner` loops through each test in the list calling
/// the test's `run()` method.
///
class SCY_EXTERN TestRunner
{
public:
    TestRunner();
    virtual ~TestRunner();

    /// Add a test to the runner.
    void add(Test* test);

    /// Return a pointer to the test matching the given name,
    /// or nullptr if no matching test exists.
    Test* get(const std::string& name) const;

    /// Called by the async context to run the next test.
    void run();

    /// Destroy and clears all managed tests.
    void clear();

    /// Return the currently active Test or nullptr.
    Test* current() const;

    /// Return the list of tests.
    TestList tests() const;

    /// Return a map of tests and errors.
    ErrorMap errors() const;

    /// Return true if all tests passed.
    bool passed() const;

    /// Return the default `TestRunner` singleton, although
    /// `TestRunner` instances may also be initialized individually.
    static TestRunner& getDefault();

protected:
    mutable std::mutex _mutex;
    TestList _tests;
    Test* _current;
};


} // namespace test
} // namespace scy


#endif // SCY_Test_H


/// @\}
