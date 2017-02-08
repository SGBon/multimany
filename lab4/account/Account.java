package account;

//import java.lang.*;

/* activity 2:
 * deadlock is possible because if two threads attempt to transfer
 * to each other neither will be able to obtain a lock on
 * each other as those objects own their own locks due to the method being
 * a synchronized method
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

  synchronized void transfer(Account ac,double mn){
      amount-=mn;
      synchronized(ac){
        ac.amount+=mn;
      }
  }

 synchronized void print(){
  System.out.println(name + "--"+amount);
  }

      }//end of class Acount
