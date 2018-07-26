//
// Created by ProDigital on 7/26/18.
//

#define BOOST_TEST_MODULE MyTests
#include <boost/test/unit_test.hpp>

#include "gp/utils/DispatchQueue.h"
#include "mytest.h"

using namespace gpproto;

BOOST_AUTO_TEST_CASE(test_test) {

    BOOST_TEST_MESSAGE("Initializing test_test");
    BOOST_CHECK_EQUAL(4, 4);
};


BOOST_AUTO_TEST_CASE(dispatch_test) {
    BOOST_TEST_MESSAGE("Running dispatch_test");

    auto semaphore = new Semaphore(-2);

    {
        auto queue = std::make_unique<DispatchQueue>("uz.gpproto.testQueue");

        BOOST_CHECK(!queue->isCurrentQueue());

        int x = 45;

        queue->async([&] {
            BOOST_TEST_MESSAGE("Inside queue");

            BOOST_CHECK(queue->isCurrentQueue());

            queue->sync([&] {
                x = 46;
                BOOST_TEST_MESSAGE("Inside queue sync");

                BOOST_CHECK(queue->isCurrentQueue());
            });

            BOOST_CHECK_EQUAL(x, 46);

            queue->asyncForce([&] {
                BOOST_TEST_MESSAGE("Inside force");
                //semaphore->notify();
            });

            BOOST_TEST_MESSAGE("After force");

            //semaphore->notify();
        });

        BOOST_CHECK(!queue->isCurrentQueue());

        queue->async([&] {
            BOOST_TEST_MESSAGE("Main thread");
        });

        queue->sync([&] {
            BOOST_TEST_MESSAGE("Inside another queue sync");
            x++;
        });

        queue->sync([&] {
            BOOST_TEST_MESSAGE("Inside last queue sync");
            BOOST_CHECK_EQUAL(x, 47);
        });

        //semaphore->wait();

        BOOST_TEST_MESSAGE("dispatch_test finished");
    }
};


