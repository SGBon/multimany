package account;

//import java.lang.*;

/* activity 2:
* deadlock is possible because if two threads attempt to transfer
* to each other neither will be able to obtain a lock on
* each other as those objects own their own locks due to the method being
* a synchronized method
*/

/* activity 3:
 * enforce an ordering on which locks are acquired and released when two threads may attempt
 * to acquire the same two locks
 * the System.identityHashCode method provides a unique value for every object
 * so it's used to create the ordering: objects with smaller hash values represent
 * the first lock and the larger hash value will be the second lock.
 */

public class Account {
  double amount;
  String  name;

  //constructor
  public Account(String nm,double amnt ) {
    amount=amnt;
    name=nm;
  }
  //functions
  synchronized void depsite(double money){
    amount+=money;
  }

  synchronized void withdraw(double money){
    amount-=money;
  }

  void transfer(Account ac,double mn){
    Account first, second;
    if(System.identityHashCode(this) < System.identityHashCode(ac)){
      first = this;
      second = ac;
    }else{
      first = ac;
      second = this;
    }
    synchronized(first){
      amount-=mn;
      synchronized(second){
        ac.amount+=mn;
      }
    }
  }

  synchronized void print(){
    System.out.println(name + "--"+amount);
  }

}//end of class Acount
