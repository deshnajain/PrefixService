#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#ifdef BAZEL_BUILD
#include "protos/prefixservice.grpc.pb.h"
#else
#include "prefixservice.grpc.pb.h"
#endif
#include "names.cpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using prefixservice::AddWordRequest;
using prefixservice::AddWordResponse;
using prefixservice::PrefixRequest;
using prefixservice::PrefixResponse;
using prefixservice::PrefixService;


// Logic and data behind the server's behavior.
class PrefixServiceImpl final : public PrefixService::Service {
  // Implements query logic; takes in the `query_string` from `request` and
  // returns the name-score pairs with top scores matching the `query_string`.
  Status ListTopScores(
      ServerContext* context,
      const PrefixRequest* request,
      PrefixResponse* reply) override
  {
    std::string query_string = request->query_string();
    std::vector<psi> ans = query(root, query_string, 0);

    for (psi& p : ans){
      // Set `NameScorePairs` in the response.
      prefixservice::NameScorePair* element= reply->add_namescorepairs();
      element->set_name(p.first);
      element->set_score(p.second);
    }

    return Status::OK;
  }

  // Implements end-point for adding name-score pair dynamically to the
  // existing search space; takes in a list of `NameScorePair` and inserts
  // them to existing data.
  Status AddWords(
      ServerContext* context,
      const AddWordRequest* request,
      AddWordResponse* reply) override
  {
    std::vector<std::pair<std::string, int>> pairs;

    for(auto element : request->namescorepairs()) {
      std::string word = element.name();
      int score = element.score();
      pairs.push_back(make_pair(word, score));
    }

    insert(root, pairs);
    return Status::OK;
  }

public:
  Node *root;
  PrefixServiceImpl() {
    root = new Node();
  }
};


// Starts the server.
void RunServer() {
  std::string server_address("0.0.0.0:50051");
  PrefixServiceImpl service;

  ServerBuilder builder;

  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);

  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}


int main(int argc, char** argv) {
  RunServer();

  return 0;
}
