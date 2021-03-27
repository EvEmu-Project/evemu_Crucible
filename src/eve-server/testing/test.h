
 /**
  * @name test.h
  *   Code for testing miscellaneous items in EVEmu
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */


#ifndef EVEMU_TESTING_TEST_H_
#define EVEMU_TESTING_TEST_H_

class Client;

class testing {
public:
    testing()                                           { /* do nothing here */ }
    ~testing()                                          { /* do nothing here */ }

    static void posTest(Client* pClient);

};







#endif  // EVEMU_TESTING_TEST_H_