#include <vector>

using namespace std;

class Klass{
  Klass():a(0),b(0){
  a += b;
  }
  public:
  int a;
  int b;
};

class Kloss{
  Klass():
  a  (  0  )  ,  b  (  0  )
  {
  a += b;
  }
  public:
  int a;
  int b;
};

enum class Color:int{
  red,
  green,
  blue
};
enum class Color:int{red, green, blue };

int fun(
  int a,
  int b,
  int c
) {
  return 0;
}

int fun2(
  int a, int b, int c ) {
  return 0;
}

int int3(
  ) {;}

auto main(int  /*argc*/, char const * /*argv*/[]) -> int
{
  auto aa=[](){return 0;}();
  auto aa2  =  []  ()  {  return 0;  }  (  );
  auto aa21  =  []{return 0;}  ();
  auto aa22  =  []  {  return 0;  }  (  );
  auto aa3=[=](){return 0;}();
  auto aa4  =  [  =  ]  ()  {  return 0;  }  (  );
  auto aa5=[=](int a){return a;}(0);
  auto aa6  =  [  =  ]  (  int  a  )  {  return a;  }  (  a  );
  auto bb=aa*4+5;
  auto bb2  =  -aa  *  4  +  5  ;
  bb+=aa;
  bb  *=  2;
  auto cc = true&&false;
  auto cc2 = true  &&  false;
  auto cc3 = 1>2;
  auto cc4 = 1  <  2;
  auto cc5 = 3*(  aa  +  bb  );
  auto cc5 = 3*(  (aa  +  bb)  );
  vector<vector<int>> dd{{1,2}};
  vector<vector<int>> dd2  {  {1,2}  };
  int*a;
  int  ee, ee2;

  if(aa==1){
    aa=2;
  }
  if  (  aa==1  )  {
    aa=2;
  }
  if (
      aa==1) {
    aa -= 1;
  }
  if (aa==1)
  {
    aa=2;
  }
  while(aa==1){
    aa=2;
  }
  while  (  aa==1  )  {
    aa=2;
  }
  while  (  aa==1  )
  {
    aa=2;
  }
  for(int i=0;i<10;i++){
    ;
  }
  for  (  int i=0  ;  i<10  ;  i++  )  {;
  }
  for  (  int i=0  ;  i<10  ;  i++  )
  {
  ;}
// shit
  for(;;) {}
  crap:
  do{
  }while(aa==1);
 stink:
  do  {
  ;}  while  (  aa==1  );
  do
    {;
  }
  while  (  aa==1  );
  {
    aa=2;
  }
  if (1 && 2 && 3 && 4 && 5 && 6 && 7 && 8 && 9 && 10 && 11 && 12 && 13 && 14 && 15 && 16 && 17 && 18 && 19 && 20) {;}
  switch(2) {
    case 1:
    ++aa;
    break;
    case (2+3):
    --aa;
    break;
    default:
    aa=0;
    break;
  }
  enum Enum {
    A=0,
    B  =  1,
    C
  };
  true?true:false;
  true  ?  true  :  false;
  true?
  false
  :
  true;
  return aa;
}

namespace {
void x(void (*shit)(int)) {return;}
void x2  (  void ( * shit)(int)  )
{
  return;
}
}

#define A(x) #x
#define A(x) #  x
#define B(x) "gaga"## #x
#define B(x) "gaga"  ##  #  x

template
