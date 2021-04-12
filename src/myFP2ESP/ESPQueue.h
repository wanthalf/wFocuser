// ======================================================================
// Queue.h: myFP2ESP Queue Routines
// (c) Copyright Steven de Salas. All Rights Reserved.
// Defines a templated (generic) class for a queue of things.
// ======================================================================
// Modified by Robert Brown for use with ESP32, August 2019

#ifndef ESPQueue_H
#define ESPQueue_H

class Queue {
  private:
    int _front, _back, _count;
    String *_data;
    int _maxitems;
  public:
    Queue(int maxitems = 256) 
    { 
      _front = 0;
      _back = 0;
      _count = 0;
      _maxitems = maxitems;
      _data = new String[maxitems + 1];   
    }
    ~Queue() 
    {
      delete[] _data;  
    }
    inline int count();
    inline int front();
    inline int back();
    void push(const String &item);
    String peek();
    String pop();
    void clear();
};

inline int Queue::count() 
{
  return _count;
}

inline int Queue::front() 
{
  return _front;
}

inline int Queue::back() 
{
  return _back;
}

void Queue::push(const String &item)
{
  if(_count < _maxitems) 
  { 
    // Drops out when full
    _data[_back++]=item;
    ++_count;
    // Check wrap around
    if (_back > _maxitems)
    {
      _back -= (_maxitems + 1);
    }
  }
}

String Queue::pop() {
  if(_count <= 0) 
  {
    return String(); // Returns empty
  }
  else 
  {
    String result = _data[_front];
    _front++;
    --_count;
    // Check wrap around
    if (_front > _maxitems) 
    {
      _front -= (_maxitems + 1);
    }
    return result; 
  }
}

String Queue::peek() {
  if(_count <= 0)
  {
    return String(); // Returns empty
  }
  else
  {
    return _data[_front];
  }
}

void Queue::clear() 
{
  _front = _back;
  _count = 0;
}

#endif
