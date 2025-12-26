/*================================================================================

File: test_spmc.cpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-12-26 15:28:24                                                 
last edited: 2025-12-26 17:07:36                                                

================================================================================*/

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <numeric>
#include <set>
#include "SPMCQueue.hpp"

using namespace itq::spmc;

TEST(SPMCQueueTest, PushAndPop)
{
  SharedData<int, 16> shared_data;
  Producer<int, 16> producer(shared_data);
  Consumer<int, 16> consumer(shared_data);

  producer.push(42);

  int value;
  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, 42);
}

TEST(SPMCQueueTest, PopFromEmptyQueue)
{
  SharedData<int, 16> shared_data;
  Consumer<int, 16> consumer(shared_data);

  int value;
  EXPECT_FALSE(consumer.pop(value));
}

TEST(SPMCQueueTest, MultiplePushAndPop)
{
  SharedData<int, 16> shared_data;
  Producer<int, 16> producer(shared_data);
  Consumer<int, 16> consumer(shared_data);

  for (int i = 0; i < 10; ++i)
    producer.push(i);

  for (int i = 0; i < 10; ++i)
  {
    int value;
    ASSERT_TRUE(consumer.pop(value));
    EXPECT_EQ(value, i);
  }

  int value;
  EXPECT_FALSE(consumer.pop(value));
}

TEST(SPMCQueueTest, EmplaceTest)
{
  SharedData<int, 16> shared_data;
  Producer<int, 16> producer(shared_data);
  Consumer<int, 16> consumer(shared_data);

  producer.emplace(123);

  int value;
  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, 123);
}

TEST(SPMCQueueTest, PushPopStrings)
{
  SharedData<std::string, 8> shared_data;
  Producer<std::string, 8> producer(shared_data);
  Consumer<std::string, 8> consumer(shared_data);

  producer.push("Hello");
  producer.push("World");
  producer.emplace("Test");

  std::string value;
  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, "Hello");

  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, "World");

  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, "Test");

  EXPECT_FALSE(consumer.pop(value));
}

struct TestStruct
{
  int x;
  double y;

  TestStruct() : x(0), y(0.0) {}
  TestStruct(int x_, double y_) : x(x_), y(y_) {}

  bool operator==(const TestStruct& other) const
  {
    return x == other.x && y == other.y;
  }
};

TEST(SPMCQueueTest, PushPopStructs)
{
  SharedData<TestStruct, 8> shared_data;
  Producer<TestStruct, 8> producer(shared_data);
  Consumer<TestStruct, 8> consumer(shared_data);

  TestStruct s1{42, 3.14};
  producer.push(s1);
  producer.emplace(100, 2.71);

  TestStruct value;
  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, s1);

  ASSERT_TRUE(consumer.pop(value));
  EXPECT_EQ(value, (TestStruct{100, 2.71}));
}

//TODO: test ProducerMultipleConsumersThreaded

TEST(SPMCQueueTest, MoveOnlyType)
{
  SharedData<std::unique_ptr<int>, 8> shared_data;
  Producer<std::unique_ptr<int>, 8> producer(shared_data);
  Consumer<std::unique_ptr<int>, 8> consumer(shared_data);

  producer.push(std::make_unique<int>(42));
  producer.emplace(std::make_unique<int>(100));

  std::unique_ptr<int> value;
  ASSERT_TRUE(consumer.pop(value));
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, 42);

  ASSERT_TRUE(consumer.pop(value));
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, 100);
}

TEST(SPSCQueueTest, FullBecomesEmpty)
{
  SharedData<int, 8> shared_data;
  Producer<int, 8> producer(shared_data);
  Consumer<int, 8> consumer(shared_data);

  for (int i = 0; i < 8; ++i)
    producer.push(i);

  int value;
  EXPECT_FALSE(consumer.pop(value));
}

TEST(SPMCQueueTest, SingleElementCapacity)
{
  SharedData<int, 1> shared_data;
  Producer<int, 1> producer(shared_data);
  Consumer<int, 1> consumer(shared_data);

  producer.push(1);

  int value;
  EXPECT_FALSE(consumer.pop(value));
}
