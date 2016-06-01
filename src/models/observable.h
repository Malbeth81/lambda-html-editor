#ifndef SUBJECT_H
#define	SUBJECT_H

class Observer {
  public:
    virtual void Update(int Message, int wParam, int lParam)
    {
    }
};

class Observable {
  private:
    Observer** Observers;
    unsigned short ObserverCount;
  public:
    Observable()
    {
      Observers = 0;
      ObserverCount = 0;
    }

    void AddObserver(Observer* Observer)
    {
      /* Resize array */
      Observer** Array = new Observer*[ObserverCount+1];
      for(int i = 0; i < ObserverCount; i++)
        Array[i] = Observers[i];
      if (Observers != 0)
        delete[] Observers;
      Observers = Array;
      Observers[ObserverCount] = Observer;
      ObserverCount++;
    }

    void NotifyObservers(int Message, int wParam, int lParam)
    {
      Observer* Observer;
      for(int i = 0; i < ObserverCount; i++)
      {
        Observer = (Observer*)Observers[i];
        Observer->Update(Message, wParam, lParam);
      }
    }
};

#endif
