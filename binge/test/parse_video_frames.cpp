#include <binge/Binge.hpp>
#include <binge/Debug.hpp>
#include <iostream>

using namespace binge;
std::vector<Frame> contexts;
std::string queryString;

//Searching for an object
std::vector<Frame> enableSearchUI(){
  std::cout << "Please enter your search query: " << std::endl;
  std::string in;
  std::cin >> in;
  if(in.empty()){ // TODO add possible invalid conditions here
    std::cout << "Query invalid, ";
    return enableSearchUI();
  }
  queryString = in;
  std::vector<Frame> queriedFrames;
  for(auto context : contexts){
    for(auto object : context.objects){
      if(object == in){ // TODO use a grammer parser, use ML for detecting synonyms
        queriedFrames.push_back(context);
        continue;
      }
    }
  }
  return queriedFrames;
}

int main(int argc, char* argv[]) { //Main
  if (argc < 1) { //If input is null , crashes 
    std::cout << "Insufficient arguments" << std::endl;
    return -1;
  }

  Debug::enableLog();

  Binge binge;
  std::cout << "Processing..." << std::endl;
    binge.on(FrameReaderStates::READ, [&](Frame f){
    contexts.push_back(f);
  std::string str = std::to_string(f.timestamp);
  str+=": ";
  for(std::string object : f.objects)
  {
    str+=object+", ";
  }
    std::cout<<str<<std::endl;
 });
  binge.on(FrameReaderStates::COMPLETE, [&](Frame f){
    std::cout << "Video fingerprinting " << (contexts.empty()? "failed" : "completed") << ", ";
    if(contexts.empty())return;
    std::vector<Frame> queriedFrames = enableSearchUI();
    std::cout << queryString << " is found in the follwing timestamps" << std::endl;
    for(auto frame : queriedFrames){
      std::cout << frame.timestamp << std::endl;
    }
  });
  binge.read(argv[1]);
  return 0;
}