#include <stdlib.h>
#include <iostream>
#include <errno.h>

using namespace std;

void main(){
  errno = 0;
  system("wget http://www.salon.com");
  if(errno != 0){
    perror("ERROR");
    errno = 0;
  }
  //system("cd ..");
}
