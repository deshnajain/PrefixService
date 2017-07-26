#include <iostream>
#include <cassert>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include <grpc++/grpc++.h>

#include <gtest/gtest.h>

#ifdef BAZEL_BUILD
#include "examples/protos/prefixservice.grpc.pb.h"
#else
#include "prefixservice.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using prefixservice::AddWordRequest;
using prefixservice::AddWordResponse;
using prefixservice::PrefixRequest;
using prefixservice::PrefixResponse;
using prefixservice::PrefixService;


// Logic and data behind the client's behavior.
class PrefixClient {
 public:
  PrefixClient(std::shared_ptr<Channel> channel)
      : stub_(PrefixService::NewStub(channel)) {}

  // Takes the `query_string` from client, sends it and presents the response
  // back from the server.
  std::vector<std::pair<std::string, int>> ListTopScores(
      const std::string& query_string)
  {
    // Data we are sending to the server.
    PrefixRequest request;
    request.set_query_string(query_string);

    // Container for the data i.e. `NameScorePairs` with top scores matching
    // the `query_string` we expect from the server.
    PrefixResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The RPC call to function 'ListTopScores` on server.
    Status status = stub_->ListTopScores(&context, request, &reply);

    if (!status.ok()) {
      std::cout << "Received error: " << status.error_code()
                << " with message: " << status.error_message()
                << std::endl;
      throw std::exception();
    }

    // Make a vector of `NameScorePair` returned in the response from the
    // server.
    std::vector<std::pair<std::string, int>> ans;
    for (const auto& element : reply.namescorepairs()) {
      ans.push_back(make_pair(element.name(), element.score()));
    }

    return ans;
  }


  // Takes a name-score list `pairs` from client and sends it to server to add
  // it to existing pairs.
  void AddWords(const std::vector<std::pair<std::string, int>>& pairs)
  {
    AddWordRequest request;

    // Set `NameScorePairs` in request.
    for (auto name_score : pairs){
      prefixservice::NameScorePair* element = request.add_namescorepairs();
      element->set_name(name_score.first);
      element->set_score(name_score.second);
    }

    AddWordResponse reply;
    ClientContext context;

    // The RPC call to function 'AddWords` on server.
    Status status = stub_->AddWords(&context, request, &reply);
    if (!status.ok()) {
      std::cout << "Received error: " << status.error_code()
                << " with message: " << status.error_message()
                << std::endl;
      throw std::exception();
    }
  }

  // Takes a name-score `pair` from client and sends it to server to add
  // it to existing pairs.
  void AddWords(const std::pair<std::string, int>& pair)
  {
    std::vector<std::pair<std::string, int>> pairs({pair});
    AddWords(pairs);
  }

  // Takes a `_filename` from client, reads it and prepares a name-score list
  // `pairs`, sends it to server to add it to existing pairs.
  void AddWordsFromFile(const std::string& _filename)
  {
    std::vector<std::pair<std::string, int>> pairs;
    std::string line, name;
    int score;
    std::ifstream myfile (_filename);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
          std::stringstream ss(line);
          ss >> name >> score;
          pairs.push_back(make_pair(name, score));
        }
        myfile.close();
    }
    AddWords(pairs);
  }

 private:
  std::unique_ptr<PrefixService::Stub> stub_;
};


class PrefixClientTest : public ::testing::Test
{
public:
  PrefixClientTest()
    : testClient(grpc::CreateChannel(
          "localhost:50051", grpc::InsecureChannelCredentials())) {}
protected:
  PrefixClient testClient;
};


TEST_F(PrefixClientTest, Test1)
{
  std::string query_string("re");
  std::vector<std::pair<std::string, int>> reply = testClient.ListTopScores(query_string);
  ASSERT_EQ(10, reply.size());
}


TEST_F(PrefixClientTest, Test2)
{
  std::string query_string("search");
  std::vector<std::pair<std::string, int>> reply = testClient.ListTopScores(query_string);
  ASSERT_EQ(0, reply.size());
  std::pair<std::string, int> add_pair({query_string, 3});
  testClient.AddWords(add_pair);
  reply = testClient.ListTopScores(query_string);
  ASSERT_EQ(1, reply.size());
}


TEST_F(PrefixClientTest, Test3)
{
  std::string query_string("search");
  std::vector<std::pair<std::string, int>> reply = testClient.ListTopScores(query_string);
  ASSERT_EQ(1, reply.size());
}


int main(int argc, char** argv)
{
  PrefixClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  client.AddWordsFromFile("example.txt");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
