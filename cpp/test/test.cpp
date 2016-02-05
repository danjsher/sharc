#include <cstdlib>
#include <iostream>
#include "../CommandPacket.h"
#include "../ThreadQueue.h"

using namespace std;


int main() {

  float packet[8] = {50.23, 0.0, 0.0, 0.0, 0.0, 0.3, 4.0, 5.0} ;

  CommandPacket cp(packet);

  cout << "Thumb value: " << cp.getThumbADC() << endl;

  float *ptr = cp.getBicepYpr();

  for(int i =0; i < 3; i++)
    cout << ptr[i] <<endl;

  ThreadQueue<CommandPacket> aQueue;

  aQueue.push(cp);

  cout <<  "Thumb value: " << aQueue.pop().getThumbADC() << endl;
  aQueue.pop();
}
