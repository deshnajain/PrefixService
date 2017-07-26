#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#ifdef BAZEL_BUILD
#include "examples/protos/prefixservice.grpc.pb.h"
#else
#include "prefixservice.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using prefixservice::PrefixRequest;
using prefixservice::PrefixResponse;
using prefixservice::PrefixService;

class PrefixClient {
 public:
  PrefixClient(std::shared_ptr<Channel> channel)
      : stub_(PrefixService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::vector<std::string> ListTopScores(const std::string& search_string) {
    // Data we are sending to the server.
    PrefixRequest request;
    request.set_prefix(search_string);

    // Container for the data we expect from the server.
    PrefixResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->ListTopScores(&context, request, &reply);

    if (!status.ok()) {
      std::cout << "Received error: " << status.error_code()
                << " with message: " << status.error_message()
                << std::endl;
      throw std::exception();
    }

    std::vector<std::string> ans;
    for (const auto& element : reply.prefixes()) {
      ans.push_back(element);
    }

    return ans;
  }

 private:
  std::unique_ptr<PrefixService::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  PrefixClient topScores(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  std::string user("rev");
  std::vector<std::string> reply = topScores.ListTopScores(user);
  std::cout << "Names received: " << std::endl;
  for (std::string& name : reply)
    std::cout << name << std::endl;

  return 0;
}
