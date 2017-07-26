# Resources
1. gRPC
2. protobuf
3. gtest
4. glog
5. Docker

# Design and Implementation

## What and How
The entire service has 3 main components:
### Data Representation: Components communicate with each other using messages.
We use google protobuf to define the schema of the Prefix Service. This gives us the ability to re-use the
serilization/deserialization logic of protobufs itself without the need for implementing one ourselves. Also,
protobufs allow us to extend our service in the future by adding/removing fields from the API in a backward
compatible manner.

### RPC Protocol used
We use _gRPC_ as the underlying RPC server implementation that handles the RPC communication between the server/client. gRPC is based on HTTP2 and has its own protocol for exchanging protobuf messages. The advantage of using gRPC here is to avoid us
writing a server implementation from scratch. Also, gRPC generates client implementations (handle retry, failures etc.) in various languages without the need for us to implement one.

### How logic of look-ups are implemented
We use Prefix tree data structure to save and query the name-score pairs. Client reads the name-score pairs (name and score whitespace separated) from a file and serializes them into a protobuf message. This message is passed to server which deserializes it and builds a [prefix tree](https://en.wikipedia.org/wiki/Trie) using names. For facilitating look ups of substrings starting with \'\_', name-score pairs are also associated with the relevant substrings.   
Each node of the tree consists of a hashmap where key is a char and value is the node pointer to the associated child, a list of name-score pairs associated if the node is a leaf for the name, and a list of 10 or less name-score pairs ranked highest by scores among the pairs at the node and those in its subtree.  
Time complexity of the solution is O(nm) where n is the number of pairs and m is the average length the names.  
One of the limitations of this solution is the look up for a string consiting of \'\_\' is not possible. However it can be attained with much more space complexity. 

## Alternatives

# Deployment Plan

## Deployment
The solution has a lot of dependencies e.g., protobuf, gRPC, C++ binaries. For ease of deployment, the plan is to
build docker image that can be hooked to an existing Jenkins or any other CI/CD solution. Once the image has been 
built; it can be deployed to production using a configuration management system like puppet, chef etc for deployment to actual hosts.

It is our choice whether we want to run the docker image on a bare metal or a VM eventually.

## Monitoring
gRPC exposed generic metrics for the service itself e.g., p95 latency of the requests, number of requests failing, total number of requests etc. that can be hooked up to Prometheus for ultimately getting nice dashboards for these metrics.

## Reporting & Alerting
Once the metrics are extracted using Prometheus, we can send them to a metrics solution e.g., Cloudwatch metrics, Datadog etc. and then set up alerting on them based on some thresholds. We can automate all this setup by writing scripts that automate monitor creating and setting the thresholds beforehand.

# Logging (TODO)
We have used _glog_ to log the service. This can be hooked to a logging solution like journald, syslog allowing us to debug the service when a monitoring alert is triggered. We can also ultimately send the logs to a logging service like AWS Cloudwatch logs, Sumologic, Splunk etc. for debugging logs that have been rotated from the machine.


# Test Plan
Currently, I used the google test suite to test the actual implementation of the server. The tests have been implemented in
a file called {{prefix_client.cpp}}. They are basically integration tests that test the entire end-to-end workflow. Allthough, for now the test coverage is very limited, but we can add more tests for testing more functionality.


# How to run the program
The entire prefix service has been packaged into a Docker image for easy
deployment. This would be useful for deploying it in production environments
since all the dependencies are already packaged inside the image.

## For running the image
### Building image locally
For running the server:
```
docker build -t prefix-server .
docker run -p 8080:50051 -t -i prefix-server ./prefix_server
```
This would start the server and port map 8080 on the localhost to the server running on port 50051 in the image. For running the tests thereafter:
```
docker ps // Find the image id of the running image
➜  ~ docker exec -i -t IMAGE_ID /bin/bash
root@79116076b75f:/prefix_service# ./prefix_client
[==========] Running 3 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 3 tests from PrefixClientTest
[ RUN      ] PrefixClientTest.Test1
[       OK ] PrefixClientTest.Test1 (1 ms)
[ RUN      ] PrefixClientTest.Test2
[       OK ] PrefixClientTest.Test2 (0 ms)
[ RUN      ] PrefixClientTest.Test3
[       OK ] PrefixClientTest.Test3 (1 ms)
[----------] 3 tests from PrefixClientTest (2 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 1 test case ran. (2 ms total)
[  PASSED  ] 3 tests.
```
### Running the image from DockerHub
For pulling the image from DockerHub and running the server:
```
docker pull deshnazen/prefix_service
docker run -p 8080:50051 -t -i prefix-server ./prefix_server

```
The instructions for running the tests thereafter are the same as above.
